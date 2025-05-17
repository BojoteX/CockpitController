// Config.h

#pragma once

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


#define VALUE_THROTTLE_MS             50              // How long (ms) to skip sending the same value again
#define ANY_VALUE_THROTTLE_MS         33              // How long (ms) to skip sending different values (prevents spamming the CDC endpoint)
#define SELECTOR_DWELL_MS            100              // Wait time (in ms) for stable selector value. Used by our dwell-time fitering logic
#define DCS_GROUP_MIN_INTERVAL_US (1000000UL / DCS_UPDATE_RATE_HZ) // Derived spacing constant
#define HID_REPORT_MIN_INTERVAL_US  (1000000UL / HID_REPORT_RATE_HZ) // Derived spacing constant

// Simulate a loopback DCS stream to check your panel is working and debug via Serial
#define IS_REPLAY                                 0
#define DEBUG_ENABLED                             0
#define VERBOSE_MODE                              0
#define DEBUG_USE_WIFI                            1
#define VERBOSE_MODE_WIFI_ONLY                    1 
#define DEBUG_PERFORMANCE                         1
#define VERBOSE_PERFORMANCE_ONLY                  0
#define PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS     60

// Set to 1 to enable 400MHz PCA Bus FAST MODE   
#define PCA_FAST_MODE         1   

// Panel Load Flags (PCA panels auto-detected at runtime, no need for defines)
#define LOAD_PANEL_CA         1   // Caution Advisory
#define LOAD_PANEL_LA         1   // Left Annunciator
#define LOAD_PANEL_RA         1   // Right Annunciator
#define LOAD_PANEL_IR         1   // IR Cool Panel
#define LOAD_PANEL_LOCKSHOOT  1   // Lock-Shoot Panel

// #define CFG_TUSB_DEBUG       3
// #define CFG_TUD_LOG_LEVEL    2
// #define CONFIG_TINYUSB_CDC_RX_BUFSIZE             1024
// #define CONFIG_TINYUSB_CDC_TX_BUFSIZE             1024
// #define CFG_TUD_CDC_TX_BUFSIZE                    1024
// #define CFG_TUD_CDC_RX_BUFSIZE                    1024

// Your CDC/Serial receive buffer
#define SERIAL_RX_BUFFER_SIZE                     1024              // in bytes             
#define SERIAL_TX_TIMEOUT                         4                 // in ms 
// #define HID_SENDREPORT_TIMEOUT                    0                 // in ms
#define CDC_TIMEOUT_RX_TX                         100               // in ms

#define DCS_UPDATE_RATE_HZ                        30                // DCSBIOS Loop [Task] update rate
#define HID_REPORT_RATE_HZ                        20                // Use Max 60Hz HID report rate to avoid spamming the CDC Endpoint / USB
#define POLLING_RATE_HZ                          250                // Panel polling rate in Hz (125, 250, 500 Hz)
#define HID_RECOVERY_TIMEOUT_MS                  100                // After HID report sending failure, how long we wait in ms?
#define HID_REPORT_MIN_INTERVAL_US               (1000000UL / HID_REPORT_RATE_HZ) // To ensure spacing between reports

// temporary disable
#define DCS_KEEP_ALIVE_MS                       1000                // Re-send selector command every second if unchanged

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

#define MAX_TRACKED_RECORDS 512   // default safety cap
#define MAX_GROUPS 128            // default safety cap

// Define the Built-in LED if compiling with a board that does not define it
// #ifndef LED_BUILTIN
// #define LED_BUILTIN 2 // Default LED pin
// #endif

#if DEBUG_USE_WIFI
#include <IPAddress.h>
// Wi-Fi network credentials
static const char* WIFI_SSID = "Metro5600";
static const char* WIFI_PASS = "4458e8c3c2";
// Remote IP + Port - Run listener.py script and enter the IP address shown there (Port should not change) 
static const IPAddress DEBUG_REMOTE_IP(192, 168, 7, 255); // Broadcast address so no need to direct UDP to a specific IP
static const uint16_t DEBUG_REMOTE_PORT = 4210;
#endif

/*
// Fix for latest Adafruit TinyUSB with 3.2.0 Core. We are NOT using Adafruit's Library, this is just for testing
extern "C" bool __atomic_test_and_set(volatile void* ptr, int memorder) __attribute__((weak));
bool __atomic_test_and_set(volatile void* ptr, int memorder) {
  return false; // pretend the lock was not already set
}
*/