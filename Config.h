#pragma once

// Panel Load Flags (PCA panels auto-detected at runtime, no need for defines)
#define LOAD_PANEL_CA         0   // Caution Advisory
#define LOAD_PANEL_LA         1   // Left Annunciator
#define LOAD_PANEL_RA         0   // Right Annunciator
#define LOAD_PANEL_IR         0   // IR Cool Panel
#define LOAD_PANEL_LOCKSHOOT  0   // Lock-Shoot Panel

// Simulae a DCS stream to check your panel is working
#define IS_REPLAY 0

// Add verbosity and lots of information on what your device is doing, when DEBUG_ENABLED = 0 you wont see any output
#define DEBUG_ENABLED 0

// enables always-on output regardless of DEBUG without the performance penalty of DEBUG_ENABLED
#define VERBOSE_MODE 1

// If DEBUG_ENABLED and TEST_LEDS is active you see a Menu when you start the device to test LEDS via console  
#define TEST_LEDS 0

// Enable WiFi debug (UDP output) when set to 1; disable with 0
#define DEBUG_USE_WIFI 1 // DEBUG_ENABLED should be set to 1 for you to see any output

// Enable Profiling for specific blocks with beginProfiling / endProfiling (not for use in production)
#define DEBUG_PERFORMANCE 1 // DEBUG_ENABLED should be set to 1 for you to see any output

// How frequent you want to see the performance snapshot + profiling blocks? requires DEBUG_PERFORMANCE enabled
#define PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS 30

#if DEBUG_USE_WIFI
// Wi-Fi network credentials
static const char* WIFI_SSID = "Metro5600";
static const char* WIFI_PASS = "4458e8c3c2";
static const IPAddress DEBUG_REMOTE_IP(192, 168, 7, 163);
static const uint16_t DEBUG_REMOTE_PORT = 4210;
#endif