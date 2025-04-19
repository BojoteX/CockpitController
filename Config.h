#pragma once

// Skips MAX3421 (we don't need it) as it causes an Adafruit error with latest ESP32 Core 3.2.0
// #define CFG_TUH_MAX3421 0

// Fix for latest Adafruit TinyUSB with 3.2.0 Core
extern "C" bool __atomic_test_and_set(volatile void* ptr, int memorder) __attribute__((weak));
bool __atomic_test_and_set(volatile void* ptr, int memorder) {
  return false; // pretend the lock was not already set
}

#define DEBUG_USE_WIFI false
// #define DEBUG_MODE