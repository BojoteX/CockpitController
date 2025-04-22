// HIDManager.cpp - Corrected Refactor for ESP32 Arduino Core 3.2.0 (No TinyUSB)

#include <cstring>
#include <map>
#include <unordered_map>
#include <USB.h>
#include <USBHID.h>
#include "src/HIDManager.h"
#include "src/Globals.h"
#include "src/DCSBIOSBridge.h"
#include "src/Mappings.h"

// Group bitmask store
#define MAX_GROUPS 32
static uint32_t groupBitmask[MAX_GROUPS] = {0};

USBHID HID;

// HID Report Descriptor
const uint8_t hidReportDesc[] = {
  0x05, 0x01, 0x09, 0x05, 0xA1, 0x01,
  0x09, 0x36, 0x15, 0x00, 0x26, 0xFF, 0x0F,
  0x75, 0x10, 0x95, 0x01, 0x81, 0x02,
  0x05, 0x09, 0x19, 0x01, 0x29, 0x20,
  0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x20, 0x81, 0x02,
  0xC0
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

// Build HID group bitmasks
void buildHIDGroupBitmasks() {
  for (size_t i = 0; i < InputMappingSize; ++i) {
      const InputMapping& m = InputMappings[i];
      if (m.group > 0 && m.hidId > 0) {
          groupBitmask[m.group] |= (1UL << (m.hidId - 1));
      }
  }
}

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
  buildHIDGroupBitmasks();
  USB.begin();
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

// HID Keep Alive
void HIDManager_keepAlive() {
  static unsigned long lastRefresh = 0;
  if (!HID.ready()) return;
  if (millis() - lastRefresh > 100) {
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