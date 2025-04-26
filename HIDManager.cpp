// HIDManager.cpp - Corrected Refactor for ESP32 Arduino Core 3.2.0 (No TinyUSB)

// Load TinyUSB
#include <USB.h>
#include <USBHID.h>

// Our regular includes
#include <Arduino.h>
#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"
#include "src/Mappings.h"

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
  // Load input bitmasks
  buildHIDGroupBitmasks();

  // Configure custom USB descriptors **before** starting HID
  USB.productName("FA-18C Cockpit Controller");   // Product string
  USB.manufacturerName("Bojote");                 // Manufacturer string
  USB.serialNumber("FA18C-AB-02");                // Serial number string
  USB.VID(0xCAFE);                                // Set Vendor ID&#8203;:contentReference[oaicite:6]{index=6}
  USB.PID(0x1102);                                // Set Product ID

// USB Stack init (seems to start on its own)
  USB.begin();

  // HID Device init (we dont need this in DCS Only mode)
  HID.begin(); if (!isModeSelectorDCS());
}

// Axis Movement
void HIDManager_moveAxis(const char* dcsIdentifier, uint8_t pin) {
  const int DEADZONE_LOW = 50, DEADZONE_HIGH = 4080;
  const int THRESHOLD = 512, OVERSAMPLE_COUNT = 32;

  static int lastRaw[40] = {-1};
  static int lastOutput[40] = {-1};

  long sum = 0;
  for (int i = 0; i < OVERSAMPLE_COUNT; i++) {
    sum += analogRead(pin);
    delayMicroseconds(250);
  }
  int raw = sum / OVERSAMPLE_COUNT;

  if (raw < DEADZONE_LOW) raw = 0;
  if (raw > DEADZONE_HIGH) raw = 4095;

  int output = map(raw, 0, 4095, 0, 65535);

  if (isModeSelectorDCS()) {
    if (abs(output - lastOutput[pin]) > THRESHOLD) {
      lastOutput[pin] = output;
      sendDCSBIOSCommand(dcsIdentifier, output);
    }
  } else {
    if ((raw == 0 && lastRaw[pin] != 0) ||
        (raw == 4095 && lastRaw[pin] != 4095) ||
        (raw != 0 && raw != 4095 && abs(raw - lastRaw[pin]) > THRESHOLD)) {
      lastRaw[pin] = raw;
      if (!HID.ready()) return;
      report.rx = raw;
      gp.sendReport(report.raw, sizeof(report));
      debugPrintf("[HID MODE] %s %d\n", dcsIdentifier, raw);
    }
  }
}

// Commit Deferred Report
void HIDManager_commitDeferredReport() {
  if (!HID.ready()) return;
  gp.sendReport(report.raw, sizeof(report));
}

// HID Poll Rate / Send Reports
// Sends a HID report at the configured polling rate (in Hz)
// Example: 250Hz → sends every 4ms
void HIDManager_keepAlive() {
  static const uint16_t pollingRateHz = 250;    // Set your desired polling rate here
  static const unsigned long pollingIntervalMs = 1000 / pollingRateHz; // Calculated interval
  static unsigned long lastRefresh = 0;
  if (!HID.ready()) return;
  if (millis() - lastRefresh >= pollingIntervalMs) {
    gp.sendReport(report.raw, sizeof(report));
    lastRefresh = millis();
  }
}

// Set Named Button State
void HIDManager_setNamedButton(const String& name, bool deferSend, bool pressed) {
  for (size_t i = 0; i < InputMappingSize; ++i) {
    const InputMapping& m = InputMappings[i];
    if (name == m.label) {
      if (isModeSelectorDCS()) {
        if (m.oride_label && m.oride_value >= 0)
          sendDCSBIOSCommand(m.oride_label, pressed ? m.oride_value : 0);
        return;
      }
      if (m.hidId <= 0) return;
      uint32_t mask = (1UL << (m.hidId - 1));
      if (m.group > 0 && pressed)
        report.buttons &= ~groupBitmask[m.group];
      if (pressed)
        report.buttons |= mask;
      else
        report.buttons &= ~mask;
      if (!deferSend && HID.ready())
        gp.sendReport(report.raw, sizeof(report));
      return;
    }
  }
  debugPrintf("⚠️ [HIDManager] %s UNKNOWN\n", name.c_str());
}