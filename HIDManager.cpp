// HIDManager.cpp

#include "src/Globals.h"
#include "src/HIDDescriptors.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"

#if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)

// Descriptor Helper
static uint16_t _desc_str_buf[32];
static const uint16_t* make_str_desc(const char* s) {
  size_t len = strlen(s);
  if (len > 30) len = 30;
  // bDescriptorType=STRING (0x03), bLength = 2 + 2*len
  _desc_str_buf[0] = (TUSB_DESC_STRING << 8) | (uint16_t)(2 * len + 2);
  for (size_t i = 0; i < len; i++) {
    _desc_str_buf[1 + i] = (uint16_t)s[i];
  }
  return _desc_str_buf;
}

// Override the weak TinyUSB string callback to fix ESP32 Core not setting correct device names when using composite devices (e.g CDC+HID)
extern "C" {
  const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    switch (index) {
      case 0: {
        static const uint16_t lang_desc[] = { (TUSB_DESC_STRING << 8) | 4, USB_LANG_ID };
        return lang_desc;
      }
      case 1: return make_str_desc(USB_MANUFACTURER);
      case 2: return make_str_desc(USB_PRODUCT);
      case 3: return make_str_desc(USB_SERIAL);
      case 4: return make_str_desc(USB_PRODUCT_HID); // HID name
      case 5: return make_str_desc(USB_PRODUCT_CDC); // CDC name
      case 6: return make_str_desc(USB_PRODUCT); // Device name
      default: return nullptr;
    }
  }
}

#endif

// HID Report spacing & control
static uint32_t lastHidSendUs = 0;
static uint8_t lastSentReport[sizeof(report.raw)] = {0};
static bool reportPending = false;
static_assert(sizeof(report.raw) == sizeof(lastSentReport), "Report size mismatch!");

USBHID HID;
GamepadReport_t report = {0};

#if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)
GamepadDevice gamepad;
#else
GPDevice gamepad; // Load the class
#endif

// Wats the max number of groups?
constexpr size_t maxUsedGroup = []() {
    uint16_t max = 0;
    for (size_t i = 0; i < sizeof(InputMappings)/sizeof(InputMappings[0]); ++i) {
        if (InputMappings[i].group > max) max = InputMappings[i].group;
    }
    return max;
}();
static_assert(maxUsedGroup < MAX_GROUPS, "❌ Too many unique selector groups — increase MAX_GROUPS in Config.h");

// MAX groups in bitmasks
static uint32_t groupBitmask[MAX_GROUPS] = {0};

// Build HID group bitmasks
void buildHIDGroupBitmasks() {
  for (size_t i = 0; i < InputMappingSize; ++i) {
      const InputMapping& m = InputMappings[i];
      if (m.group > 0 && m.hidId > 0) {
          groupBitmask[m.group] |= (1UL << (m.hidId - 1));
      }
  }
}

// Find the HID mapping whose oride_label and oride_value match this DCS command+value (for selectors).
static const InputMapping* findHidMappingByDcs(const char* dcsLabel, uint16_t value) {
    for (size_t i = 0; i < InputMappingSize; ++i) {
        const auto& m = InputMappings[i];
        if (m.oride_label
            && strcmp(m.oride_label, dcsLabel) == 0
            && (uint16_t)m.oride_value == value) {
            return &m;
        }
    }
    return nullptr;
}

// Flush hasPending commands for selectors (dwell-time filtering logic)
void flushBufferedHidCommands() {
    auto history = dcsbios_getCommandHistory();
    size_t n     = dcsbios_getCommandHistorySize();
    unsigned long now = millis();

    CommandHistoryEntry* groupLatest[MAX_GROUPS] = { nullptr };

    // Step 1: Track most recent pending entry per group
    for (size_t i = 0; i < n; ++i) {
        auto& e = history[i];
        if (!e.hasPending || e.group == 0) continue;

        if (now - e.lastChangeTime >= SELECTOR_DWELL_MS) {
            uint16_t g = e.group;

            if (g >= MAX_GROUPS) {
              debugPrintf("❌ FATAL: group ID %u exceeds MAX_GROUPS (%u). Halting flush.\n", g, MAX_GROUPS);
              abort();  // Triggers clean panic for debugging
            }

            if (!groupLatest[g] || e.lastChangeTime > groupLatest[g]->lastChangeTime) {
                groupLatest[g] = &e;
            }
        }
    }

    // Step 2: For each group, clear all buttons and set the active one
    for (uint16_t g = 1; g < MAX_GROUPS; ++g) {
        CommandHistoryEntry* winner = groupLatest[g];
        if (!winner) continue;

        const InputMapping* m = findHidMappingByDcs(winner->label, winner->pendingValue);
        if (!m) {
            winner->hasPending = false;
            continue;
        }

        // Clear all bits in this group
        report.buttons &= ~groupBitmask[g];

        // Set winner bit
        if (winner->pendingValue > 0) {
            report.buttons |= (1UL << (m->hidId - 1));
        }

        // HIDManager_dispatchReport(false);

        // Finalize
        winner->lastValue = winner->pendingValue;
        winner->lastSendTime = now;
        winner->hasPending = false;
        debugPrintf("🛩️ [HID] GROUP %u FLUSHED: %s = %u\n", g, winner->label, winner->lastValue);
    }
    // if (reportPending) HIDManager_dispatchReport();
}

// Replace your current HIDManager_sendReport(...) with this:
void HIDManager_sendReport(const char* label, int32_t rawValue) {
    const InputMapping* m = findInputByLabel(label);
    if (!m) {
        debugPrintf("⚠️ [HID] %s UNKNOWN\n", label);
        return;
    }

    const char* dcsLabel = m->oride_label;
    uint16_t    dcsValue = rawValue < 0 ? 0 : (uint16_t)rawValue;

    // look up shared history
    CommandHistoryEntry* history = dcsbios_getCommandHistory();
    size_t n = dcsbios_getCommandHistorySize();
    CommandHistoryEntry* e = nullptr;
    for (size_t i = 0; i < n; ++i) {
        if (strcmp(history[i].label, dcsLabel) == 0) {
            e = &history[i];
            break;
        }
    }
    if (!e) {
        debugPrintf("⚠️ [HID] %s → no DCS entry (%s)\n", label, dcsLabel);
        return;
    }

    #if defined(SELECTOR_DWELL_MS) && (SELECTOR_DWELL_MS > 0)
    // buffer selectors
    if (e->group > 0) {
        e->pendingValue   = dcsValue;
        e->lastChangeTime = millis();
        e->hasPending     = true;
        // debugPrintf("🔁 [DCS] Buffer Selection for GroupID: %u - %s %u\n", e->group, label, e->pendingValue);
        return;
    }
    #endif

    // same throttle as DCS
    if (!applyThrottle(*e, dcsLabel, dcsValue, false))
        return;

    // flip just this bit
    uint32_t mask = (1UL << (m->hidId - 1));
    if (dcsValue)
        report.buttons |= mask;
    else
        report.buttons &= ~mask;

    // HIDManager_dispatchReport(false);

    // update history
    e->lastValue    = dcsValue;
    e->lastSendTime = millis();
    debugPrintf("🛩️ [HID] %s = %u\n", dcsLabel, dcsValue);
}

void HIDManager_moveAxis(const char* dcsIdentifier,
                        uint8_t      pin,
                        HIDAxis      axis /*= AXIS_RX*/) {
    constexpr int DEADZONE_LOW        = 60;
    constexpr int DEADZONE_HIGH       = 4050;
    constexpr int THRESHOLD           = 64;
    constexpr int SMOOTHING_FACTOR    = 8;
    constexpr int STABILIZATION_CYCLES = 10;

    static int lastFiltered[40]       = {0};
    static int lastOutput[40]         = {-1};
    static unsigned int stabCount[40] = {0};
    static bool stabilized[40]        = {false};

    // 1) Read & smooth
    int raw = analogRead(pin);
    if (stabCount[pin] == 0) {
        lastFiltered[pin] = raw;
    } else {
        lastFiltered[pin] = (lastFiltered[pin] * (SMOOTHING_FACTOR - 1) + raw)
                             / SMOOTHING_FACTOR;
    }
    int filtered = lastFiltered[pin];
    if (filtered < DEADZONE_LOW)  filtered = 0;
    if (filtered > DEADZONE_HIGH) filtered = 4095;

    // 2) Stabilization period
    if (!stabilized[pin]) {
        stabCount[pin]++;
        if (stabCount[pin] >= STABILIZATION_CYCLES) {
            stabilized[pin] = true;
            lastOutput[pin] = filtered;
            // fall through and send initial
        } else {
            return;
        }
    }

    // 3) On big enough change, send
    if (abs(filtered - lastOutput[pin]) <= THRESHOLD) {
        return;
    }
    lastOutput[pin] = filtered;

    // 4) Map filtered [0..4095] → DCS [0..65535]
    uint16_t dcsValue = map(filtered, 0, 4095, 0, 65535);

    // 5) Throttle & dispatch
    if (isModeSelectorDCS()) {
        // DCS path
        auto* e = findCmdEntry(dcsIdentifier);
        if (e && applyThrottle(*e, dcsIdentifier, dcsValue, /*force=*/false)) {
            sendDCSBIOSCommand(dcsIdentifier, dcsValue, false);
            e->lastValue    = dcsValue;
            e->lastSendTime = millis();
        }
    } else {
        // HID path: assign into the correct axis field
        switch (axis) {
          // case AXIS_X:       report.x  = filtered; break;
          // case AXIS_Y:       report.y  = filtered; break;
          // case AXIS_Z:       report.z  = filtered; break;
          case AXIS_RX:      report.rx = filtered; break;
          // case AXIS_RY:      report.ry = filtered; break;
          // case AXIS_RZ:      report.rz = filtered; break;
          // case AXIS_SLIDER1: report.slider[0] = filtered; break;
          // case AXIS_SLIDER2: report.slider[1] = filtered; break;
          default:           report.rx = filtered; break;
        }

        // Now throttle & send via HIDManager_sendReport
        // We reuse applyThrottle on the DCSIdentifier so that knob steps
        // get the same ANY_VALUE_THROTTLE_MS window
        auto* e = findCmdEntry(dcsIdentifier);
        if (e && applyThrottle(*e, dcsIdentifier, dcsValue, /*force=*/false)) {
            // send the raw HID report
            // HIDManager_dispatchReport(false);

            // update shared history
            e->lastValue    = dcsValue;
            e->lastSendTime = millis();
            debugPrintf("🛩️ [HID] Axis(%u) %s = %u\n",
                        axis, dcsIdentifier, dcsValue);
        }
    }
}

// Commit Deferred HID report for an entire panel
void HIDManager_commitDeferredReport(const char* deviceName) {

      // Skip is we are in DCS-MODE
    if (isModeSelectorDCS()) return;

    // 2) Send the raw 64-byte gamepad report
    // HIDManager_dispatchReport(false);

    // 3) (Optional) Debug
    debugPrintf("🛩️ [HID] Deferred report sent for: \"%s\"\n", deviceName);
}

// For polling rate on panels that need it
bool shouldPollMs(unsigned long &lastPoll) {
  const unsigned long pollingIntervalMs = 1000 / POLLING_RATE_HZ;
  unsigned long now = millis();
  if (now - lastPoll < pollingIntervalMs) return false;
  lastPoll = now;
  return true;
}

// —————————————————————————————————————————————————————————————
// Handler for HID Set_Protocol
// —————————————————————————————————————————————————————————————
static void hidSetProtocolHandler(void*            arg,
                                  esp_event_base_t base,
                                  int32_t          id,
                                  void*            event_data)
{
    auto* d = (arduino_usb_hid_event_data_t*)event_data;
    debugPrintf("[HID EVENT] SET_PROTOCOL  itf=%u → %s\n",
                (unsigned)d->instance,
                d->set_protocol.protocol ? "REPORT" : "BOOT");
}

// —————————————————————————————————————————————————————————————
// Handler for HID Set_Idle
// —————————————————————————————————————————————————————————————
static void hidSetIdleHandler(void*            arg,
                              esp_event_base_t base,
                              int32_t          id,
                              void*            event_data)
{
    auto* d = (arduino_usb_hid_event_data_t*)event_data;
    debugPrintf("[HID EVENT] SET_IDLE      itf=%u rate=%u×4ms\n",
                (unsigned)d->instance,
                (unsigned)d->set_idle.idle_rate);
}

// USB Events
void onUsbStarted(void* arg, esp_event_base_t base, int32_t event_id, void* data) {
  debugPrintln("🔌 USB Started");
}

void onUsbStopped(void* arg, esp_event_base_t base, int32_t event_id, void* data) {
  debugPrintln("❌ USB Stopped");
}

void onUsbSuspended(void* arg, esp_event_base_t base, int32_t event_id, void* data) {
  debugPrintln("💤 USB Suspended");
}

void onUsbResumed(void* arg, esp_event_base_t base, int32_t event_id, void* data) {
  debugPrintln("🔁 USB Resumed");
}

void setupHIDEvents() {
    // Subscribe individually
    // HID.onEvent(ARDUINO_USB_HID_SET_PROTOCOL_EVENT, hidSetProtocolHandler);
    // HID.onEvent(ARDUINO_USB_HID_SET_IDLE_EVENT,     hidSetIdleHandler);
}

void setupUSBEvents() {
    // USB.onEvent(ARDUINO_USB_STARTED_EVENT, onUsbStarted);
    // USB.onEvent(ARDUINO_USB_STOPPED_EVENT, onUsbStopped);
    // USB.onEvent(ARDUINO_USB_SUSPEND_EVENT, onUsbSuspended);
    // USB.onEvent(ARDUINO_USB_RESUME_EVENT, onUsbResumed);
}

inline static bool HID_can_send_report() {

    // DCS mode disables HID entirely
    if (isModeSelectorDCS()) return false;

    // HID endpoint must be idle
    // if (!tud_hid_ready()) return false;
    if (!HID.ready()) return false;

/*
    // CDC RX must have shown recent activity (proves USB is being polled)
    if (!cdcEnsureRxReady(CDC_TIMEOUT_RX_TX)) { // Wait for RX to be ready
        // debugPrintln("❌ HID block: Stream is not currently active");
        return false;
    }

    // CDC TX must have available space 
    if (!cdcEnsureTxReady(CDC_TIMEOUT_RX_TX)) { // Wait for TX to be ready
        // debugPrintln("❌ HID block: Tx not ready");
        return false;
    }
*/

    return true;  // ✅ All conditions met — safe to send HID
}

void HIDManager_dispatchReport(bool force) {

    uint32_t now = micros();
    if ((now - lastHidSendUs) < HID_REPORT_MIN_INTERVAL_US) return;

    // Skip if report is identical and not forced
    if (!force && memcmp(lastSentReport, report.raw, sizeof(report.raw)) == 0) return;

    // Attempt to send report
    if (!HID_can_send_report()) return;

    #if DEBUG_PERFORMANCE
    beginProfiling(PERF_HIDREPORTS);
    #endif

    bool success = HID.SendReport(0, report.raw, sizeof(report.raw), HID_SENDREPORT_TIMEOUT);
    // bool success = HID.SendReport(0, report.raw, sizeof(report.raw));
    // bool success = tud_hid_report(0, report.raw, sizeof(report.raw));
    // delay(HID_SENDREPORT_TIMEOUT);

    #if DEBUG_PERFORMANCE
    endProfiling(PERF_HIDREPORTS);
    #endif

/*
    if (!success) {
        debugPrintln("❌ Report failed");
        return;
    }
*/

    // Mark success
    memcpy(lastSentReport, report.raw, sizeof(report.raw));
    lastHidSendUs = now;
}

void HIDSenderTask(void* param) {
    constexpr TickType_t interval = pdMS_TO_TICKS(1000 / HID_REPORT_RATE_HZ);
    while (true) {
        // HIDManager_dispatchReport(true);  // Use internal gating logic
        // vTaskDelay(interval);
    }
}

void HIDManager_toggleIfPressed(bool isPressed, const char* label, bool deferSend) {
  
    CommandHistoryEntry* e = findCmdEntry(label);
    if (!e) return;

    static std::array<bool, MAX_TRACKED_RECORDS> lastStates = {false};
    int index = e - dcsbios_getCommandHistory();
    if (index < 0 || index >= MAX_TRACKED_RECORDS) return;

    bool prev = lastStates[index];
    lastStates[index] = isPressed;

    if (isPressed && !prev) {
        HIDManager_setToggleNamedButton(label, deferSend);
    }
}

void HIDManager_setToggleNamedButton(const char* name, bool deferSend) {
  const char* label = name;
  const InputMapping* m = findInputByLabel(label);
  if (!m) {
    debugPrintf("⚠️ [HIDManager] %s UNKNOWN (toggle)\n", label);
    return;
  }

  CommandHistoryEntry* e = findCmdEntry(label);
  if (!e) return;
  bool newState = !(e->lastValue > 0);
  e->lastValue = newState ? 1 : 0;

  if (isModeSelectorDCS()) {
    if (m->oride_label && m->oride_value >= 0)
      sendDCSBIOSCommand(m->oride_label, newState ? m->oride_value : 0, false);
    return;
  }

  if (m->hidId <= 0) return;

  uint32_t mask = (1UL << (m->hidId - 1));

  if (m->group > 0 && newState)
    report.buttons &= ~groupBitmask[m->group];

  if (newState)
    report.buttons |= mask;
  else
    report.buttons &= ~mask;

  if (!deferSend) {
    HIDManager_sendReport(name, newState ? 1 : 0);
  }
}

void HIDManager_handleGuardedToggle(bool isPressed, const char* switchLabel, const char* coverLabel, const char* fallbackLabel, bool deferSend) {

  CommandHistoryEntry* e = findCmdEntry(switchLabel);
  if (!e) return;

  static std::array<bool, MAX_TRACKED_RECORDS> lastStates = {false};
  int index = e - dcsbios_getCommandHistory();
  if (index < 0 || index >= MAX_TRACKED_RECORDS) return;
    
  bool wasPressed = lastStates[index];
  lastStates[index] = isPressed;

  if (isPressed && !wasPressed) {
    CommandHistoryEntry* cover = findCmdEntry(coverLabel);
    if (!cover || cover->lastValue == 0) {
      HIDManager_setToggleNamedButton(coverLabel, deferSend);
      debugPrintf("✅ Cover [%s] opened for [%s]\n", coverLabel, switchLabel);
    }
    HIDManager_setNamedButton(switchLabel, deferSend, true);
  }

  if (!isPressed && wasPressed) {
    HIDManager_setNamedButton(switchLabel, deferSend, false);
    if (isCoverOpen(coverLabel)) {
      HIDManager_setToggleNamedButton(coverLabel, deferSend);
      debugPrintf("✅ Cover [%s] closed after releasing [%s]\n", coverLabel, switchLabel);
    }
    if (fallbackLabel) {
      HIDManager_setNamedButton(fallbackLabel, deferSend, true);
    }
  }
}

void HIDManager_handleGuardedMomentary(bool isPressed, const char* buttonLabel, const char* coverLabel, bool deferSend) {

  CommandHistoryEntry* e = findCmdEntry(buttonLabel);
  if (!e) return;

  static std::array<bool, MAX_TRACKED_RECORDS> lastStates = {false};
  int index = e - dcsbios_getCommandHistory();
  if (index < 0 || index >= MAX_TRACKED_RECORDS) return;

  bool wasPressed = lastStates[index];
  lastStates[index] = isPressed;

  if (isPressed && !wasPressed) {
    CommandHistoryEntry* cover = findCmdEntry(coverLabel);
    if (!cover || cover->lastValue == 0) {
      HIDManager_setToggleNamedButton(coverLabel, deferSend);
      debugPrintf("✅ Cover [%s] auto-opened\n", coverLabel);
      return; // Do not press button yet — wait for user to release and press again
    }
    HIDManager_setNamedButton(buttonLabel, deferSend, true);
  }

  if (!isPressed && wasPressed) {
    HIDManager_setNamedButton(buttonLabel, deferSend, false);
  }
}

void HIDManager_setNamedButton(const char* name, bool deferSend, bool pressed) {
  const InputMapping* m = findInputByLabel(name);
  if (!m) {
    debugPrintf("⚠️ [HIDManager] %s UNKNOWN\n", name);
    return;
  }

  if (isModeSelectorDCS()) {
    if (m->oride_label && m->oride_value >= 0)
      sendDCSBIOSCommand(m->oride_label, pressed ? m->oride_value : 0, false);
    return;
  }

  if (m->hidId <= 0) return;

  uint32_t mask = (1UL << (m->hidId - 1));

  if (m->group > 0 && pressed)
    report.buttons &= ~groupBitmask[m->group];

  if (pressed)
    report.buttons |= mask;
  else
    report.buttons &= ~mask;

  // OJO Cambio recient
  if (!deferSend) {
    HIDManager_sendReport(name, pressed ? m->oride_value : 0);
  }
}

void HIDManager_setup() {

    // Load our Group Bitmasks
    buildHIDGroupBitmasks();    

    // Setup USB events
    setupUSBEvents();

    // Register Device (just before calling USB.begin) but only if we are in HID mode
    #if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)
    if (!isModeSelectorDCS()) {
        HID.addDevice(&gamepad, sizeof(hidReportDesc));
        setupHIDEvents();
    }
    #endif

    // Start HID device (if in HID mode)
    if (!isModeSelectorDCS()) HID.begin(); 

    // Start the interface
    USB.begin();  
    delay(3000);    
}

void HIDManager_loop() {

    #if defined(SELECTOR_DWELL_MS) && (SELECTOR_DWELL_MS > 0)
    // In HID mode, flush any buffered selector-group presses
    if (!isModeSelectorDCS()) flushBufferedHidCommands();
    #endif

}