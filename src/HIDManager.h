#pragma once
#include <cstdint>
#include <WString.h>

// Initialize HID device and descriptors
void HIDManager_begin();

// Set RX axis (0–4095)
void HIDManager_moveAxis(const char* dcsIdentifier, uint8_t pin);

// Named button setter (handles exclusivity and optional defer)
void HIDManager_setNamedButton(const String& name, bool deferSend = false, bool pressed = true);

// Apply all deferred state changes
void HIDManager_commitDeferredReport();

// Polling logic
bool shouldPollMs(unsigned long &lastPoll);

// Makes sure our status is always seen by windows
void HIDManager_keepAlive();