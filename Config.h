#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// When you make changes here make sure you CLEAN your prev compile or close your IDE and open again
// If you don't and re-compile you wont see your options take effect.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

// Required for Descriptor handling
#define USB_VID		        0xCAFE
#define USB_PID		        0xFEE2
#define USB_MANUFACTURER  "Bojote"
#define USB_PRODUCT       "F/A-18C Cockpit Controller"
#define USB_SERIAL        "SN-ZZ18-99"
#define USB_LANG_ID       0x0409  // English (US)
#define USB_IF_CDC_NAME   USB_PRODUCT
#define USB_IF_HID_NAME   USB_PRODUCT
#define USB_CFG_NAME      USB_PRODUCT

// There is really no need for this, but if for you, ensuring cockpit sync is a top priority, use it. The design and architecture of this 
// program is already fault-tolerant and throttles ALL sends to match DCS_UPDATE_RATE_HZ and avoid skipping commands. Like a PRO LEVEL
// cockpit sim. 
#define ENABLE_DCS_COMMAND_KEEPALIVE  1             // Explained above
#define ENABLE_HID_KEEPALIVE          1             // Same, but for HID

// Self explanatory, don't change if you don't know what you are doing
#define DCS_UPDATE_RATE_HZ            30            // Change only if DCSBIOS ever changes its update freq (highly unlinkely)
#define HID_REPORT_RATE_HZ            60            // Max 60Hz HID report rate to avoid spamming the CDC Endpoint / USB
#define POLLING_RATE_HZ              250            // Panel and HID polling rate in Hz (125, 250, 500 Hz)
#define HID_KEEP_ALIVE_INTERVAL_MS  1000            // Resend unchanged HID report after 1s
#define DCS_KEEP_ALIVE_INTERVAL_MS  1000            // Re-send selector command every 1s if unchanged
#define DCS_KEEPALIVE_POLL_INTERVAL POLLING_RATE_HZ // How often to check for keep-alive conditions

// Set to 1 to enable 400MHz PCA Bus FAST MODE   
#define PCA_FAST_MODE         1   

// Panel Load Flags (PCA panels auto-detected at runtime, no need for defines)
#define LOAD_PANEL_CA         1   // Caution Advisory
#define LOAD_PANEL_LA         1   // Left Annunciator
#define LOAD_PANEL_RA         1   // Right Annunciator
#define LOAD_PANEL_IR         1   // IR Cool Panel
#define LOAD_PANEL_LOCKSHOOT  1   // Lock-Shoot Panel

// Simulate a loopback DCS stream to check your panel is working and debug via Serial
#define IS_REPLAY 0

// Add verbosity and lots of information on what your device is doing, when DEBUG_ENABLED = 0 you wont see any output
#define DEBUG_ENABLED 0

// Enables always-on output regardless of DEBUG without the performance penalty of DEBUG_ENABLED
#define VERBOSE_MODE 0

// Same as above but outputs to WiFi (very useful)
#define VERBOSE_MODE_WIFI_ONLY 1

// Enable WiFi debug (UDP output) when set to 1; disable with 0
#define DEBUG_USE_WIFI 1 

// Enable Profiling for specific blocks with beginProfiling / endProfiling (not for use in production)
#define DEBUG_PERFORMANCE 1 

// This will output via UDP ONLY so you need to enable DEBUG_USE_WIFI.
#define VERBOSE_PERFORMANCE_ONLY 0

// You'll see a Menu when you start the device to test LEDS via console.  
#define TEST_LEDS 0

// Does the device have a HID/DCS Mode selector? if so, what PIN? 
#define HAS_HID_MODE_SELECTOR 1
#define MODE_SWITCH_PIN 33 // Mode Selection Pin (DCS-BIOS/HID)

// Define the Built-in LED if not defined
#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

// How frequent you want to see the performance snapshot + profiling blocks? requires DEBUG_PERFORMANCE enabled
#define PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS 30

#if DEBUG_USE_WIFI
#include <cstdint>
#include <IPAddress.h>

// Wi-Fi network credentials
static const char* WIFI_SSID = "Metro5600";
static const char* WIFI_PASS = "4458e8c3c2";

// Run listener.py script and enter the IP address shown there (Port should not change) 
static const IPAddress DEBUG_REMOTE_IP(192, 168, 7, 163);
static const uint16_t DEBUG_REMOTE_PORT = 4210;
#endif

// Fix for latest Adafruit TinyUSB with 3.2.0 Core
extern "C" bool __atomic_test_and_set(volatile void* ptr, int memorder) __attribute__((weak));
bool __atomic_test_and_set(volatile void* ptr, int memorder) {
  return false; // pretend the lock was not already set
}