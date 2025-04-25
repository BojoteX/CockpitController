#pragma once

// Simulae a DCS stream to check your panel is working
#define IS_REPLAY 0

// Just add verbosity and lots of information on what your device is doing
#define DEBUG_ENABLED 0

// If DEBUG_ENABLED and TEST_LEDS is active you see a Menu when you start the device to test LEDS via console  
#define TEST_LEDS 0

// Enable WiFi debug (UDP output) when set to 1; disable with 0
#define DEBUG_USE_WIFI 0 // DEBUG_ENABLED should be set to 1 for you to see any output

// Enable Profiling for specific blocks with beginProfiling / endProfiling (not for use in production)
#define DEBUG_PERFORMANCE 0 // DEBUG_ENABLED should be set to 1 for you to see any output

#if DEBUG_USE_WIFI
// Wi-Fi network credentials
static const char* WIFI_SSID = "Metro5600";
static const char* WIFI_PASS = "4458e8c3c2";
// Debug UDP destination
#include <Arduino.h> // for IPAddress
static const IPAddress DEBUG_REMOTE_IP(192, 168, 7, 163);
static const uint16_t DEBUG_REMOTE_PORT = 4210;
#endif