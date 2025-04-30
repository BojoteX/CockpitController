// HIDManager.cpp - Corrected Refactor for ESP32 Arduino Core 3.2.0 (No TinyUSB)

// Load TinyUSB
#include <tusb.h>
#include <USB.h>
#include <USBHID.h>

#define Serial USBSerial

// Our regular includes
#include <Arduino.h>
#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"

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

// HID report descriptor: 1 axis (Rx), 32 buttons
uint8_t const hidReportDesc[] = {
  0x05, 0x01,       // Usage Page (Generic Desktop)
  0x09, 0x05,       // Usage (Gamepad)
  0xA1, 0x01,       // Collection (Application)

  0x09, 0x36,       // Usage (Rx)
  0x15, 0x00,       // Logical Minimum (0)
  0x26, 0xFF, 0x0F, // Logical Maximum (4095)
  0x75, 0x10,       // Report Size (16)
  0x95, 0x01,       // Report Count (1)
  0x81, 0x02,       // Input (Data, Var, Abs)

  0x05, 0x09,       // Usage Page (Buttons)
  0x19, 0x01,       // Usage Min (Button 1)
  0x29, 0x20,       // Usage Max (Button 32)
  0x15, 0x00,
  0x25, 0x01,
  0x75, 0x01,
  0x95, 0x20,
  0x81, 0x02,       // Input (Data, Var, Abs)

  0xC0              // End Collection
};

// HID report structure
typedef union {
  struct __attribute__((packed)) {
    uint16_t rx;
    uint32_t buttons;
  };
  uint8_t raw[6];
} GamepadReport_t;

static_assert(sizeof(GamepadReport_t) == 6, "GamepadReport_t must be 6 bytes");
static GamepadReport_t report;

USBHID HID;
class GPDevice : public USBHIDDevice {
public:
  GPDevice() { HID.addDevice(this, sizeof(hidReportDesc)); }
  uint16_t _onGetDescriptor(uint8_t* buf) override {
    memcpy(buf, hidReportDesc, sizeof(hidReportDesc));
    return sizeof(hidReportDesc);
  }
  bool sendReport(const void* data, int len) {
    return HID.SendReport(0, data, len);
  }
};
GPDevice gp;

// Initialize USB HID
void HIDManager_begin() {

  // Init DCSBIOS (Includes serial)
  DCSBIOS_init();
  delay(2000);
  
  // Load input bitmasks
  buildHIDGroupBitmasks();

  // Configure custom USB descriptors **before** starting HID
  USB.VID(0xCAFE);                                // Set Vendor ID&#8203;:contentReference[oaicite:6]{index=6}
  USB.PID(0x18FF);                                // Set Product ID
  USB.productName("FA-18C Cockpit Controller");   // Product string
  USB.manufacturerName("Bojote");                 // Manufacturer string
  USB.serialNumber("FA18C-AB-02");                // Serial number string

  // USB Stack init (seems to start on its own)
  USB.begin();

  // We only initialize this when in HID mode, so we'll be effectively CDC only
  if(!isModeSelectorDCS()) {
    HID.begin();
    while (!HID.ready()) { delay(10); }; //Do not continue until HID is ready
  }
}

void HIDManager_loop() {
  DCSBIOS_loop(); 
  
  // Dont send HID reports if using DCS-BIOS mode switch
  if (!isModeSelectorDCS()) HIDManager_keepAlive();

}

void HIDManager_moveAxis(const char* dcsIdentifier, uint8_t pin) {
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
                sendDCSBIOSCommand(dcsIdentifier, dcsOutput);
            } else {
                report.rx = filtered;
                gp.sendReport(report.raw, sizeof(report));
                debugPrintf("[HID MODE] %s %d\n", dcsIdentifier, filtered);
            }
        }
        return;
    }

    if (abs(filtered - lastOutput[pin]) > THRESHOLD) {
        lastOutput[pin] = filtered;
        if (isModeSelectorDCS()) {
            int dcsOutput = map(filtered, 0, 4095, 0, 65535);
            sendDCSBIOSCommand(dcsIdentifier, dcsOutput);
        } else {
            report.rx = filtered;
            gp.sendReport(report.raw, sizeof(report));
            debugPrintf("[HID MODE] %s %d\n", dcsIdentifier, filtered);
        }
    }
}

// Commit Deferred Report
void HIDManager_commitDeferredReport() {
  if (!HID.ready()) return;
  gp.sendReport(report.raw, sizeof(report));
}

// For polling rate on panels that need it
bool shouldPollMs(unsigned long &lastPoll) {
  const unsigned long pollingIntervalMs = 1000 / POLLING_RATE_HZ;
  unsigned long now = millis();
  if (now - lastPoll < pollingIntervalMs) return false;
  lastPoll = now;
  return true;
}

void HIDManager_keepAlive() {
  // Limit based on our poll rate
  static unsigned long lastHIDPoll = 0;
  if (!shouldPollMs(lastHIDPoll)) return;
  
  if (!HID.ready()) return;
  gp.sendReport(report.raw, sizeof(report));
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
      sendDCSBIOSCommand(m->oride_label, newState ? m->oride_value : 0);
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

  if (!deferSend && HID.ready())
    gp.sendReport(report.raw, sizeof(report));
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
      sendDCSBIOSCommand(m->oride_label, pressed ? m->oride_value : 0);
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

  if (!deferSend && HID.ready())
    gp.sendReport(report.raw, sizeof(report));
}