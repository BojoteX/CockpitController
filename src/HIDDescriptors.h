#pragma once

#include <USBHID.h>

// HID report descriptor: 1 axis (Rx), 32 buttons, padded to 8 bytes
static const uint8_t hidReportDesc[] = {
  0x05, 0x01,       // Usage Page (Generic Desktop)
  0x09, 0x05,       // Usage (Gamepad)
  0xA1, 0x01,       // Collection (Application)

  0x09, 0x36,       // Usage (Rx)
  0x15, 0x00,       // Logical Minimum (0)
  0x26, 0xFF, 0x0F, // Logical Maximum (4095)
  0x75, 0x10,       // Report Size (16 bits)
  0x95, 0x01,       // Report Count (1)
  0x81, 0x02,       // Input (Data, Var, Abs)

  0x05, 0x09,       // Usage Page (Buttons)
  0x19, 0x01,       // Usage Min (Button 1)
  0x29, 0x20,       // Usage Max (Button 32)
  0x15, 0x00,
  0x25, 0x01,
  0x75, 0x01,       // Report Size (1 bit)
  0x95, 0x20,       // Report Count (32 bits)
  0x81, 0x02,       // Input (Data, Var, Abs)

  0x75, 0x08,       // Report Size (8 bits)
  0x95, 0x02,       // Report Count (2 bytes)
  0x81, 0x03,       // Input (Constant, Variable, Absolute)

  0xC0              // End Collection
};

// HID report structure
typedef union {
  struct __attribute__((packed)) {
    uint16_t rx;         // 2 bytes
    uint32_t buttons;    // 4 bytes
    uint16_t padding;    // 2 bytes padding
  };
  uint8_t raw[8];
} GamepadReport_t;

static_assert(sizeof(GamepadReport_t) == 8, "GamepadReport_t must be 8 bytes");

extern const uint8_t hidReportDesc[];
extern GamepadReport_t report;

// Declare HID so the class can reference it
extern USBHID HID;

#if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)

struct GamepadDevice : USBHIDDevice {
  uint16_t _onGetDescriptor(uint8_t* buf) override {
    memcpy(buf, hidReportDesc, sizeof(hidReportDesc));
    return sizeof(hidReportDesc);
  }
};
extern GamepadDevice gamepad;

#else

class GPDevice : public USBHIDDevice {
public:
  GPDevice() {
    HID.addDevice(this, sizeof(hidReportDesc));
  }

  uint16_t _onGetDescriptor(uint8_t* buf) override {
    memcpy(buf, hidReportDesc, sizeof(hidReportDesc));
    return sizeof(hidReportDesc);
  }

  bool sendReport(const void* data, int len) {
    return HID.SendReport(0, data, len, HID_SENDREPORT_TIMEOUT);
  }
};
extern GPDevice gamepad;

#endif

