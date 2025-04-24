#pragma once

// Misc Options Activate with 1 (ON) or 0 (OFF)
#define DEBUG_USE_WIFI 1
#define IS_REPLAY 0
#define DEBUG_PERFORMANCE 1
#define TEST_LEDS 0

#if DEBUG_USE_WIFI
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include "esp_wifi.h"
  #define WIFI_SSID "MRI"
  #define WIFI_PASS "4458e8c3c2"
  const IPAddress IP_ADDRESS(192, 168, 4, 22);
  const int PORT = 12345;
#endif

#ifdef DEFINE_MAPPINGS
bool DEBUG = true;
bool hasCA = false, hasLA = false, hasRA = false, hasIR = false,
     hasLockShoot = false, hasBrain = false, hasECM = false, hasMasterARM = false;
#else
extern bool DEBUG;
extern bool isModeSelectorDCS();
#endif