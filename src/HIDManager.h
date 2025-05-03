#pragma once

#include <cstdint>

// ───── Initialization / Main Loop ─────
void HIDManager_begin();
void HIDManager_loop();

// ───── Axis Input ─────
void HIDManager_moveAxis(const char* dcsIdentifier, uint8_t pin);

// ───── Named Button State Setters (zero heap) ─────
void HIDManager_setNamedButton(const char* name, bool deferSend = false, bool pressed = true);
void HIDManager_setToggleNamedButton(const char* name, bool deferSend = false);
void HIDManager_toggleIfPressed(bool isPressed, const char* label, bool deferSend = false);

// ───── Guarded Helpers ─────
void HIDManager_handleGuardedToggle(bool isPressed, const char* switchLabel, const char* coverLabel, const char* fallbackLabel = nullptr, bool deferSend = false);
void HIDManager_handleGuardedMomentary(bool isPressed, const char* buttonLabel, const char* coverLabel, bool deferSend = false);

// ───── Utility / Maintenance ─────
void HIDManager_commitDeferredReport(const char* deviceName);
bool shouldPollMs(unsigned long &lastPoll);
void HIDManager_keepAlive();
void HIDManager_sendReport(const char* label, int32_t value = -1);
