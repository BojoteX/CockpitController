// HIDManager.cpp - Cleaned Version
// TEKCreations F/A-18C Firmware HID Report Management

#include <cstring>
#include <map>
#include <unordered_map>
#include "src/HIDManager.h"
#include "src/Globals.h"
#include "src/DCSBIOSBridge.h"
#include "src/Mappings.h"

// Group bitmask store — zero heap churn, pointer-keyed
#define MAX_GROUPS 32
static uint32_t groupBitmask[MAX_GROUPS] = {0};
void buildHIDGroupBitmasks() {
  for (size_t i = 0; i < InputMappingSize; ++i) {
      const InputMapping& m = InputMappings[i];
      if (m.group > 0 && m.hidId > 0) {
          groupBitmask[m.group] |= (1UL << (m.hidId - 1));
      }
  }
}

#include <Adafruit_TinyUSB.h>
Adafruit_USBD_HID usb_hid;

// HID report descriptor: 1 axis (Rx), 32 buttons
uint8_t const desc_hid_report[] = {
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

// Report structure: 1 axis + 32-bit button mask
typedef union {
  struct __attribute__((packed)) {
    uint16_t rx;
    uint32_t buttons;
  };
  uint8_t raw[6];
} GamepadReport_t;

static_assert(sizeof(GamepadReport_t) == 6, "GamepadReport_t must be 6 bytes");
static GamepadReport_t report;

// Initialize USB HID interface
void HIDManager_begin() {

  // This generates out fast uint32_t masks based purely on InputMapping[]
  buildHIDGroupBitmasks();

  TinyUSBDevice.setID(0xCafe, 0x18C0);
  TinyUSBDevice.setProductDescriptor("F/A-18C Hornet Cockpit Controller");
  TinyUSBDevice.setManufacturerDescriptor("Bojote");
  TinyUSBDevice.setSerialDescriptor("FA18C-BC-1");

  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();

  // Wait for USB ready
  while (!TinyUSBDevice.mounted()) delay(1);
}

void HIDManager_moveAxis(const char* dcsIdentifier, uint8_t pin) {
  // -------- Config per pin (extendable switch) --------
  int DEADZONE_LOW = 50;
  int DEADZONE_HIGH = 4080;
  int THRESHOLD = 512;
  int OVERSAMPLE_COUNT = 32;

  static int lastRaw[40];
  static int lastOutput[40];
  static bool initialized[40];

  if (!initialized[pin]) {
    lastRaw[pin] = -1;
    lastOutput[pin] = -1;
    initialized[pin] = true;
  }

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
      if ((raw == 0 && lastRaw[pin] != 0)  || 
        (raw == 4095 && lastRaw[pin] != 4095) || 
        (raw != 0 && raw != 4095 && abs(raw - lastRaw[pin]) > THRESHOLD)) {

      lastRaw[pin] = raw;
      if (!usb_hid.ready()) return;
      report.rx = raw;
      usb_hid.sendReport(0, &report, sizeof(report));
      debugPrintf("[HID MODE] %s %d\n", dcsIdentifier, raw);
    }
  }
}

// Momentary button press simulation
void HIDManager_pressButton(uint8_t buttonID) {
  if (!usb_hid.ready()) return;
  if (buttonID < 1 || buttonID > 32) return;

  report.buttons |= (1UL << (buttonID - 1));
  usb_hid.sendReport(0, &report, sizeof(report));
  delay(250);
  report.buttons &= ~(1UL << (buttonID - 1));
  usb_hid.sendReport(0, &report, sizeof(report));
}

// Directly set button state
void HIDManager_setButton(uint8_t buttonID, bool pressed) {
  if (!usb_hid.ready()) return;
  if (buttonID < 1 || buttonID > 32) return;

  if (pressed) {
    report.buttons |= (1UL << (buttonID - 1));
  } else {
    report.buttons &= ~(1UL << (buttonID - 1));
  }
  usb_hid.sendReport(0, &report, sizeof(report));
}

// Commit all deferred button changes
void HIDManager_commitDeferredReport() {
  if (!usb_hid.ready()) return;
  usb_hid.sendReport(0, &report, sizeof(report));
}

void HIDManager_keepAlive() {
  static unsigned long lastRefresh = 0;
  const unsigned long refreshInterval = 100;

  if (!usb_hid.ready()) return;

  if (millis() - lastRefresh > refreshInterval) {
    usb_hid.sendReport(0, &report, sizeof(report));
    lastRefresh = millis();
  }
}

void HIDManager_setNamedButton(const String& name, bool deferSend, bool pressed) {
  const InputMapping* mapping = nullptr;

  for (size_t i = 0; i < InputMappingSize; ++i) {
    if (strcmp(name.c_str(), InputMappings[i].label) == 0) {
      mapping = &InputMappings[i];
      break;
    }
  }

  if (!mapping) {
    debugPrint("⚠️ [HIDManager] ");
    debugPrint(name);
    debugPrintln(" is UNKNOWN");
    return;
  }

  // Debug
  debugPrint("🔘 [");
  debugPrint(isModeSelectorDCS() ? "DCS" : "HID");
  debugPrint("] ");
  debugPrint(mapping->label);
  debugPrint(" = ");
  debugPrintln(pressed ? "1" : "0");

  // DCS MODE
  if (isModeSelectorDCS()) {
    if (mapping->oride_label && mapping->oride_value >= 0) {
      uint16_t valueToSend;

      if (strcmp(mapping->controlType, "momentary") == 0) {
        valueToSend = pressed ? 1 : 0;
      } else {
        if (!pressed) return; // only send on press for selector
        valueToSend = mapping->oride_value;
      }

      sendDCSBIOSCommand(mapping->oride_label, valueToSend);
    }
    return;
  }

  // HID MODE
  if (mapping->hidId <= 0) return;

  uint32_t mask = (1UL << (mapping->hidId - 1));

  if (mapping->group > 0 && pressed) {
    report.buttons &= ~groupBitmask[mapping->group];  // clear all in group
  }
  report.buttons |= mask;  // set current

  if (!deferSend && usb_hid.ready()) {
    usb_hid.sendReport(0, &report, sizeof(report));
  }
}