#pragma once

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

extern Adafruit_USBD_HID usb_hid;

// Initialize HID device and descriptors
void HIDManager_begin();

// Set RX axis (0–4095)
void HIDManager_moveAxis(int xValue);

// Momentary button press (simulates press + release)
void HIDManager_pressButton(uint8_t buttonID);

// Set or clear button state directly
void HIDManager_setButton(uint8_t buttonID, bool pressed);

// Named button setter (handles exclusivity and optional defer)
void HIDManager_setNamedButton(const String& name, bool deferSend = false, bool pressed = true);

// Apply all deferred state changes
void HIDManager_commitDeferredReport();

// Set exclusive button within a group
void HIDManager_setExclusiveButton(uint8_t buttonID, bool deferSend);

// Makes sure our status is always seen by windows
void HIDManager_keepAlive();