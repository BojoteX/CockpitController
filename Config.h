#pragma once

// Misc Options (WiFi debug to check performance and action log. IS_REPLAY simulates a DCS Stream as if you were connected)
#define IS_REPLAY false
#define DEBUG_PERFORMANCE true
#define TEST_LEDS false
#define DEBUG_USE_WIFI true

#ifdef DEFINE_MAPPINGS
bool DEBUG = true; // You need set to true to output any data (console or WiFi)
bool hasCA = false, hasLA = false, hasRA = false, hasIR = false,
     hasLockShoot = false, hasBrain = false, hasECM = false, hasMasterARM = false;
#else
extern bool DEBUG;
extern bool isModeSelectorDCS();
#endif