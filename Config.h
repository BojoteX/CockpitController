#pragma once

// Fix for latest Adafruit TinyUSB with 3.2.0 Core
extern "C" bool __atomic_test_and_set(volatile void* ptr, int memorder) __attribute__((weak));
bool __atomic_test_and_set(volatile void* ptr, int memorder) {
  return false; // pretend the lock was not already set
}

// Misc Options
#define DEBUG_USE_WIFI false
#define IS_REPLAY true