#ifndef DCS_BIOS_BRIDGE_H
#define DCS_BIOS_BRIDGE_H

#include <Arduino.h>

// Initialize DCS-BIOS Serial Bridge
void DCSBIOSBridge_init();

// Main loop to process incoming/outgoing DCS-BIOS data
void DCSBIOSBridge_loop();

// Sends a DCS-BIOS command for a named input control
void DCSBIOS_sendCommandByLabel(const String& name, bool pressed);

#endif // DCS_BIOS_BRIDGE_H