#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// When you make changes here make sure you CLEAN your prev compile or close your IDE and open again
// If you don't and re-compile you wont see your options take effect.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

// Self explanatory, don't change if you don't know what you are doing
#define POLLING_RATE_HZ       250 // Panel and HID polling rate (125, 250, 500 and 1000 are all valid)

#define PCA_FAST_MODE         0   // Set to 1 to enable 400MHz PCA Bus FAST MODE   

// Panel Load Flags (PCA panels auto-detected at runtime, no need for defines)
#define LOAD_PANEL_CA         1   // Caution Advisory
#define LOAD_PANEL_LA         1   // Left Annunciator
#define LOAD_PANEL_RA         1   // Right Annunciator
#define LOAD_PANEL_IR         1   // IR Cool Panel
#define LOAD_PANEL_LOCKSHOOT  1   // Lock-Shoot Panel

// Simulae a DCS stream to check your panel is working
#define IS_REPLAY 0

// Add verbosity and lots of information on what your device is doing, when DEBUG_ENABLED = 0 you wont see any output
#define DEBUG_ENABLED 0

// Enable WiFi debug (UDP output) when set to 1; disable with 0
#define DEBUG_USE_WIFI 1 // DEBUG_ENABLED should be set to 1 for you to see any output

// Enable Profiling for specific blocks with beginProfiling / endProfiling (not for use in production)
#define DEBUG_PERFORMANCE 1 

// enables always-on output regardless of DEBUG without the performance penalty of DEBUG_ENABLED
#define VERBOSE_MODE 1

// Same as above but outputs to WiFi (very useful)
#define VERBOSE_MODE_WIFI_ONLY 0

// This will output via UDP ONLY so you need to enable DEBUG_USE_WIFI.
#define VERBOSE_PERFORMANCE_ONLY 0

// You'll see a Menu when you start the device to test LEDS via console.  
#define TEST_LEDS 0

// How frequent you want to see the performance snapshot + profiling blocks? requires DEBUG_PERFORMANCE enabled
#define PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS 30

#if DEBUG_USE_WIFI
#include <cstdint>
#include <IPAddress.h>
// Wi-Fi network credentials
static const char* WIFI_SSID = "Metro5600";
static const char* WIFI_PASS = "4458e8c3c2";
static const IPAddress DEBUG_REMOTE_IP(192, 168, 7, 163);
static const uint16_t DEBUG_REMOTE_PORT = 4210;
#endif