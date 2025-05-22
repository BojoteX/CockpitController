// Config.h

#pragma once

// Panel Load Flags (PCA panels auto-detected at runtime, no need for defines)
#define LOAD_PANEL_CA         1   // Caution Advisory
#define LOAD_PANEL_LA         1   // Left Annunciator
#define LOAD_PANEL_RA         1   // Right Annunciator
#define LOAD_PANEL_IR         1   // IR Cool Panel
#define LOAD_PANEL_LOCKSHOOT  1   // Lock-Shoot Panel

// Debugging ONLY. For production, ALL THESE should be set to 0, except the Interval which will be ignored anyways if all are set to 0
#define IS_REPLAY                                 0   // Simulate a loopback DCS stream to check your panel is working and debug via Serial
#define DEBUG_ENABLED                             0   // Use it when identifying Port/bit/mask with PCA devices
#define VERBOSE_MODE                              0   // Logs everything without actual debugging to both Serial and WiFi (if enabled)
#define DEBUG_USE_WIFI                            1   // Enables WiFi for outputing VERBOSE and DEBUG messages
#define VERBOSE_MODE_WIFI_ONLY                    1   // Verbose will only output to WiFi so Serial port is clean
#define DEBUG_PERFORMANCE                         1   // Shows a performance snapshot every x seconds (interval can be configured below)
#define PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS     60  // Interval between snapshots (in seconds)
#define VERBOSE_PERFORMANCE_ONLY                  0   // the only output will be performance snapshots 

// Misc Values
#define PCA_FAST_MODE                              1                // Set to 1 to enable 400MHz PCA Bus FAST MODE   
#define DCS_UPDATE_RATE_HZ                       250                // DCSBIOS Loop [Task] update rate
#define HID_REPORT_RATE_HZ                       125                // HID report sending rate in Hz. 
#define POLLING_RATE_HZ                          250                // Panel/buttons update rate in Hz (125, 250, 500 Hz)
#define DCS_KEEP_ALIVE_MS                       1000                // Re-send selector command every second if unchanged

// HID, Button and Selector logic and tracking 
#define MAX_TRACKED_RECORDS          512  // default safety cap
#define MAX_GROUPS                   128  // default safety cap
#define VALUE_THROTTLE_MS             50  // How long (ms) to skip sending the same value again
#define ANY_VALUE_THROTTLE_MS         33  // How long (ms) to skip sending different values (prevents spamming the CDC endpoint)
#define SELECTOR_DWELL_MS            100  // Wait time (in ms) for stable selector value. Used by our dwell-time fitering logic
#define DCS_GROUP_MIN_INTERVAL_US   (1000000UL / DCS_UPDATE_RATE_HZ) // min spacing/separation between selector positions
#define HID_REPORT_MIN_INTERVAL_US  (1000000UL / HID_REPORT_RATE_HZ) // min spacing/separation between reports

// Your CDC/Serial receive buffer
#define SERIAL_RX_BUFFER_SIZE                     4096              // in bytes             
#define SERIAL_TX_TIMEOUT                         8                 // in ms 
#define CDC_TIMEOUT_RX_TX                         8                 // in ms
#define HID_SENDREPORT_TIMEOUT                    0                 // in ms (Only used with ESP32 Arduino Core HID.SendReport implementation)
#define DCSBIOS_SERIAL_CHUNK_SIZE                 64                // How many max bytes on each loop pass

// Does the device have a HID/DCS Mode selector? if so set HAS_HID_MODE_SELECTOR to 1 and PIN GPIO, otherwise HAS_HID_MODE_SELECTOR 0  
#define HAS_HID_MODE_SELECTOR                     1
#define MODE_SWITCH_PIN                           33                // Mode Selection Pin (DCS-BIOS/HID)

// Global GPIO centralized PIN assignments
#define SDA_PIN                                   8 // I2C PCA9555 Data Pin
#define SCL_PIN                                   9 // I2C PCA9555 Clock Pin
#define GLOBAL_CLK_PIN                           37 // Clock Pin
#define CA_DIO_PIN                               36 // Caution Advisory DIO Pin
#define LA_DIO_PIN                               39 // Left Annunciator DIO Pin
#define LA_CLK_PIN                               GLOBAL_CLK_PIN // Left Annunciator Clock Pin
#define RA_DIO_PIN                               40 // Right Annunciator DIO Pin
#define RA_CLK_PIN                               GLOBAL_CLK_PIN // right Annunciator Clock Pin
#define LOCKSHOOT_DIO_PIN                        35 // Lock-Shoot Indicator DIO Pin

// Required for Descriptor handling
#define USB_VID		                                0xCAFE
#define USB_PID		                                0xC43E
#define USB_MANUFACTURER                          "Bojote Inc"
#define USB_PRODUCT_HID                           "FA-18 Brain Controller HID"  
#define USB_PRODUCT_CDC                           "FA-18 Brain Serial Interface"
#define USB_PRODUCT                               "FA-18 Brain Controller"
#define USB_SERIAL                                "SN-XFA18C-0003"
#define USB_LANG_ID                               0x0409  // English (US)
#define USB_IF_CDC_NAME                           USB_PRODUCT_CDC
#define USB_IF_HID_NAME                           USB_PRODUCT_HID
#define USB_CFG_NAME                              USB_PRODUCT

// Define the Built-in LED if compiling with a board that does not define it
// #ifndef LED_BUILTIN
// #define LED_BUILTIN 2 // Default LED pin
// #endif

#if DEBUG_USE_WIFI
#include <IPAddress.h>
// Wi-Fi network credentials
static const char* WIFI_SSID = "Metro5600";
// static const char* WIFI_SSID = "LENOVOLAPTOP";
static const char* WIFI_PASS = "4458e8c3c2";
// Remote IP + Port - Run listener.py script and enter the IP address shown there (Port should not change) 
static const IPAddress DEBUG_REMOTE_IP(192, 168, 7, 255); // Broadcast address so no need to direct UDP to a specific IP
// static const IPAddress DEBUG_REMOTE_IP(192, 168, 137, 255); // Broadcast address so no need to direct UDP to a specific IP
// static const IPAddress DEBUG_REMOTE_IP(239, 255, 50, 10); // Broadcast address so no need to direct UDP to a specific IP
static const uint16_t DEBUG_REMOTE_PORT = 4210;
#endif

/*
// Fix for latest Adafruit TinyUSB with 3.2.0 Core. We are NOT using Adafruit's Library, this is just for testing
extern "C" bool __atomic_test_and_set(volatile void* ptr, int memorder) __attribute__((weak));
bool __atomic_test_and_set(volatile void* ptr, int memorder) {
  return false; // pretend the lock was not already set
}
*/