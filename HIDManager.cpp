// HIDManager.cpp - Corrected Refactor for ESP32 Arduino Core 3.2.0 (No TinyUSB)

// Our regular includes
#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"

// Flow-control flag: only send a new report once the previous one really left (not implemented for HID, only CDC)
static volatile bool hidTxReady = true;

#include <USBHID.h>
// Define HID + Load Descriptors
USBHID HID;
#include "src/HIDDescriptors.h"

#if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)
// Get the USBCDC class (we don't need unless we DISABLE CDC ON BOOT)
USBCDC USBSerial;
#endif

// Group bitmask store
#define MAX_GROUPS 32
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

// String Descriptor Helper
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
      case 4: return make_str_desc(USB_PRODUCT); // HID name
      case 5: return make_str_desc(USB_PRODUCT); // CDC name
      case 6: return make_str_desc(USB_PRODUCT); // Device name
      default: return nullptr;
    }
  }
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

void setupHIDEvents() {
    // Subscribe individually
    HID.onEvent(ARDUINO_USB_HID_SET_PROTOCOL_EVENT, hidSetProtocolHandler);
    HID.onEvent(ARDUINO_USB_HID_SET_IDLE_EVENT,     hidSetIdleHandler);
}

/// Returns true if the USB HID interface is up and its IN-endpoint is ready
/// to accept a new report.
static bool HID_can_send_report() {
    // 1) Are we fully enumerated (host-side mount complete)?
    if ( !tud_mounted() ) {
        // not yet enumerated → no traffic
        return false;
    }
    // 2) Give TinyUSB’s background task a chance to run
    yield();
    // 3) Check the HID IN endpoint is free
    if ( !HID.ready() ) {
        // still busy sending last report
        return false;
    }
    // 4) yield again if you want extra breathing room
    yield();
    return true;
}

// Initialize USB HID
void HIDManager_begin() {

  // Load USB Stack
  #if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)
  USB.VID(USB_VID); // Set Vendor ID
  USB.PID(USB_PID); // Set Product ID
  USB.begin();
  #endif

  Serial.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial.begin(250000);
  delay(3000);

  // Init DCSBIOS
  DCSBIOS_init();

// We only initialize this when in HID mode, so we'll be effectively CDC only
  if(!isModeSelectorDCS()) {
    HID.begin();
    while (!HID.ready()) { delay(10); }; //Do not continue until HID is ready
  }

  // Subscribe to HID events
  setupHIDEvents();

  // Load input bitmasks
  buildHIDGroupBitmasks();

}

void HIDManager_loop() {

  // Calls our DCSBIOSBridge Loop function
  DCSBIOS_loop(); 

  #if ENABLE_HID_KEEPALIVE
  // Dont send HID reports if using DCS-BIOS mode switch
  if (!isModeSelectorDCS()) HIDManager_keepAlive();
  #endif

}

static unsigned long lastHIDReportMillis[64] = {0};
void HIDManager_sendReport(const char* label, int32_t value) {
    if (!HID_can_send_report()) return;

    static uint8_t lastSent[sizeof(report.raw)] = {0};
    static unsigned long lastSendTime = 0;
    constexpr uint32_t HID_SEND_INTERVAL_MS = 1000 / HID_REPORT_RATE_HZ;

    unsigned long now = millis();

    // Skip identical report
    if (memcmp(lastSent, report.raw, sizeof(report.raw)) == 0) return;

    // Skip if sending too fast
    if (now - lastSendTime < HID_SEND_INTERVAL_MS) return;

    // Send report
    hidTxReady = false;
    gp.sendReport(report.raw, sizeof(report.raw));
    memcpy(lastSent, report.raw, sizeof(report.raw));
    lastSendTime = now;
    yield();

    // Debug output
    if (label) {
        if (value >= 0)
            debugPrintf("🛩️ [HID] Report: %s = %d\n", label, value);
        else
            debugPrintf("🛩️ [HID] Report from: %s\n", label);
    }
}

void HIDManager_moveAxis(const char* dcsIdentifier, uint8_t pin) {
    // TODO: Implement passing an axis, right now only rx implemented. 
    constexpr int DEADZONE_LOW = 60;
    constexpr int DEADZONE_HIGH = 4050;
    constexpr int THRESHOLD = 64;  
    constexpr int SMOOTHING_FACTOR = 8;
    constexpr int STABILIZATION_CYCLES = 10;

    static int lastFiltered[40] = {0};
    static int lastOutput[40]   = {-1};
    static unsigned int stabilizationCount[40] = {0};
    static bool stabilized[40] = {false};

    int raw = analogRead(pin);
    if (stabilizationCount[pin] == 0) {
        lastFiltered[pin] = raw;
    } else {
        lastFiltered[pin] = (lastFiltered[pin] * (SMOOTHING_FACTOR - 1) + raw) / SMOOTHING_FACTOR;
    }

    int filtered = lastFiltered[pin];
    if (filtered < DEADZONE_LOW)  filtered = 0;
    if (filtered > DEADZONE_HIGH) filtered = 4095;

    if (!stabilized[pin]) {
        stabilizationCount[pin]++;
        if (stabilizationCount[pin] >= STABILIZATION_CYCLES) {
            stabilized[pin] = true;
            lastOutput[pin] = filtered;
            if (isModeSelectorDCS()) {
                int dcsOutput = map(filtered, 0, 4095, 0, 65535);
                sendDCSBIOSCommand(dcsIdentifier, dcsOutput, false);
            } else {
                report.rx = filtered;
                HIDManager_sendReport(dcsIdentifier, filtered);
            }
        }
        return;
    }

    if (abs(filtered - lastOutput[pin]) > THRESHOLD) {
        lastOutput[pin] = filtered;
        if (isModeSelectorDCS()) {
            int dcsOutput = map(filtered, 0, 4095, 0, 65535);
            sendDCSBIOSCommand(dcsIdentifier, dcsOutput, false);
        } else {
            report.rx = filtered;
            HIDManager_sendReport(dcsIdentifier, filtered);
        }
    }
}

// Commit Deferred Report
void HIDManager_commitDeferredReport(const char* deviceName) {
  HIDManager_sendReport(deviceName);
}

// For polling rate on panels that need it
bool shouldPollMs(unsigned long &lastPoll) {
  const unsigned long pollingIntervalMs = 1000 / POLLING_RATE_HZ;
  unsigned long now = millis();
  if (now - lastPoll < pollingIntervalMs) return false;
  lastPoll = now;
  return true;
}

// For polling rate on panels that need it
bool dcsUpdateMs(unsigned long &lastPoll) {
  const unsigned long pollingIntervalMs = 1000 / DCS_UPDATE_RATE_HZ;
  unsigned long now = millis();
  if (now - lastPoll < pollingIntervalMs) return false;
  lastPoll = now;
  return true;
}

void HIDManager_keepAlive() {
    static unsigned long lastKeepAliveMillis = 0;
    static uint8_t lastKeepAliveReport[sizeof(report.raw)] = {0};
    constexpr uint32_t KEEP_ALIVE_INTERVAL_MS = HID_KEEP_ALIVE_INTERVAL_MS;

    if (!HID_can_send_report()) return;

    // Only send if report is unchanged AND it's been over 1 second
    if (memcmp(lastKeepAliveReport, report.raw, sizeof(report.raw)) == 0) {
        if (millis() - lastKeepAliveMillis >= KEEP_ALIVE_INTERVAL_MS) {
          hidTxReady = false;
          gp.sendReport(report.raw, sizeof(report.raw));
          lastKeepAliveMillis = millis();
          yield();
        }
    } else {
        // New report state → reset timer and store state
        memcpy(lastKeepAliveReport, report.raw, sizeof(report.raw));
        lastKeepAliveMillis = millis();
    }
}

void HIDManager_toggleIfPressed(bool isPressed, const char* label, bool deferSend) {
    static std::array<bool, 64> lastStates = {false};  // Up to 64 tracked buttons
    int index = trackedIndexFor(label);
    if (index == -1) return;

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

  bool newState = !getTrackedState(label);
  setTrackedState(label, newState);

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
    HIDManager_sendReport(name);
  }
}

void HIDManager_handleGuardedToggle(bool isPressed, const char* switchLabel, const char* coverLabel, const char* fallbackLabel, bool deferSend) {
  static std::array<bool, 64> lastState = {false};
  int index = trackedIndexFor(switchLabel);
  if (index == -1) return;

  bool wasPressed = lastState[index];
  lastState[index] = isPressed;

  if (isPressed && !wasPressed) {
    if (!isCoverOpen(coverLabel)) {
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
  static std::array<bool, 64> lastStates = {false};  // up to 64 tracked
  int index = trackedIndexFor(buttonLabel);
  if (index == -1) return;

  bool wasPressed = lastStates[index];
  lastStates[index] = isPressed;

  if (isPressed && !wasPressed) {
    if (!isCoverOpen(coverLabel)) {
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

  if (!deferSend) {
    HIDManager_sendReport(name);
  }
}