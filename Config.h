#pragma once

// Misc Options (WiFi debug to check performance and action log. IS_REPLAY simulates a DCS Stream as if you were connected)
#define DEBUG_USE_WIFI true
#define IS_REPLAY false
#define DEBUG_PERFORMANCE true

#ifdef DEFINE_MAPPINGS
bool DEBUG = false; // You need set to true to output the above data to console. Use WiFi + Listener for better debugging
bool hasCA = false, hasLA = false, hasRA = false, hasIR = false,
     hasLockShoot = false, hasBrain = false, hasECM = false, hasMasterARM = false;
#else
extern bool DEBUG;
extern bool isModeSelectorDCS();
#endif