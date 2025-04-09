// HIDManager.cpp - Cleaned Version
// TEKCreations F/A-18C Firmware HID Report Management

#include <map>
#include "src/HIDManager.h"
#include "src/Globals.h"
#include <Adafruit_TinyUSB.h>
#include "src/DCSBIOSBridge.h"
#include "src/Mappings.h"

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

  TinyUSBDevice.setID(0xCafe, 0xF18F);
  TinyUSBDevice.setProductDescriptor("Cockpit Brain Controller");
  TinyUSBDevice.setManufacturerDescriptor("Bojote");
  TinyUSBDevice.setSerialDescriptor("FA18C-CBRAIN-1");

  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();
}

// Set axis value (Rx, 0–4095)
void HIDManager_moveAxis(int rxValue) {
  if (!usb_hid.ready()) return;
  rxValue = constrain(rxValue, 0, 4095);
  report.rx = rxValue;
  usb_hid.sendReport(0, &report, sizeof(report));
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

// Set exclusive button within a group, clearing others
void HIDManager_setExclusiveButton(uint8_t buttonID, bool deferSend) {
  if (buttonID < 1 || buttonID > 32) return;

  auto it = exclusiveButtonGroups.find(buttonID);
  if (it != exclusiveButtonGroups.end()) {
    uint32_t groupMask = it->second;
    report.buttons &= ~groupMask;
    report.buttons |= (1UL << (buttonID - 1));
  } else {
    report.buttons |= (1UL << (buttonID - 1));
  }

  if (!deferSend && usb_hid.ready()) {
    usb_hid.sendReport(0, &report, sizeof(report));
  }
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
  
  if (isModeSelectorDCS()) {
    DCSBIOS_sendCommandByLabel(name, pressed);
    return;
  }

  // HID Mode (Original Logic)
  auto it = buttonMap.find(name);
  if (it == buttonMap.end()) {
    debugPrint("⚠️ ");
    debugPrint(name);
    debugPrintln(" is UNKNOWN");
    return;
  }

  uint8_t buttonID = it->second;

  // Log action to Serial
  debugPrint("🔘 [HID MODE] ");
  debugPrint(name);
  debugPrintln(pressed ? " 1" : " 0");

  // Handle exclusive groups (if any)
  auto groupIt = exclusiveButtonGroups.find(buttonID);
  if (groupIt != exclusiveButtonGroups.end()) {
    if (pressed) {
      HIDManager_setExclusiveButton(buttonID, deferSend);
    }
    return;
  }

  // Handle regular button press
  if (deferSend) {
    if (pressed) {
      report.buttons |= (1UL << (buttonID - 1));
    } else {
      report.buttons &= ~(1UL << (buttonID - 1));
    }
  } else {
    HIDManager_setButton(buttonID, pressed);
  }
}