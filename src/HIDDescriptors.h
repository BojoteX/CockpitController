// HIDDescriptors.h
#pragma once

#include <USBHID.h>

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
    uint16_t rx; // 2 bytes
    uint32_t buttons; // 4 bytes
  };
  uint8_t raw[6]; // 6 bytes total (see above) this has to be updated if you change your descriptor
} GamepadReport_t;

static_assert(sizeof(GamepadReport_t) == 6, "GamepadReport_t must be 6 bytes");

extern GamepadReport_t report;
extern struct GamepadDevice gamepad;