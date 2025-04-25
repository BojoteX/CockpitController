// Cockpit Brain Controller Firmware by Jesus "Bojote" Altuve
// Dynamic I2C panel detection and configurable panel initialization

// -- Serial Configuration --
#define BAUD_RATE 250000                // Not used, just legacy 
#define SERIAL_STARTUP_DELAY 3000       // Delay (ms) allowing Serial Monitor to connect

// -- Project Headers --
#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"
#include <Wire.h>

#if DEBUG_PERFORMANCE
#include "src/PerfMonitor.h"
#endif

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#endif

// -- GPIO Pin Configuration --
#define SDA_PIN 8                      // I2C Data Pin
#define SCL_PIN 9                      // I2C Clock Pin
#define MODE_SWITCH_PIN 33             // Mode Selection Pin (DCS-BIOS/HID)

// PCA Only panels will be autodetected
bool hasBrain = false;
bool hasECM = false;
bool hasMasterARM = false;

// is DCS connected?
bool dcsConnected = false; 

// -- Panel Modules --
#include "src/LeftAnnunciator.h"
#include "src/RightAnnunciator.h"
#include "src/ECMPanel.h"
#include "src/IRCoolPanel.h"
#include "src/MasterARMPanel.h"

// Force compilation of all CUtils internals (Arduino won't compile .cpps in lib automatically)
#include "lib/CUtils/src/CUtils.cpp"

// Checks mode selector state
bool isModeSelectorDCS() {
  return digitalRead(MODE_SWITCH_PIN) == HIGH;
}

// Arduino Setup Routine
void setup() {
  Serial.begin(BAUD_RATE); 
  unsigned long start = millis();
  while (!Serial && (millis() - start < SERIAL_STARTUP_DELAY)) delay(1);
  
  #if VERBOSE_MODE
  debugSetOutput(true, true); // First parameter is output to Serial, second one is output to UDP (if DEBUG_USE_WIFI enabled)
  #else
  debugSetOutput(debugToSerial, debugToUDP);
  #endif

  #if DEBUG_USE_WIFI
  wifi_setup();
  #endif

  // GPIO Setup
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // Always 1 during setup, DEBUG = true will keep it that way in main loop
  enablePCA9555Logging(1);

  // I2C Initialization
  Wire.begin(SDA_PIN, SCL_PIN);

  // Starts our HID device
  HIDManager_begin();

// Detect Panels (They are off by default)
  scanConnectedPanels();

  debugPrintln("\n=== Cockpit Brain Controller Initialization ===");
  debugPrint("Selected mode: ");
  debugPrintln(isModeSelectorDCS() ? "DCS-BIOS" : "HID");

  // Show PCA Panels we discovered
  printDiscoveredPanels();

  // Automatic PCA Panel detection (use the i2c scanner to get addresses)
  hasBrain     = discoveredDevices.count(0x26); // Brain Controller
  hasECM       = discoveredDevices.count(0x22); // ECM Panel
  hasMasterARM = discoveredDevices.count(0x5B); // Master ARM

  if(hasBrain) debugPrintln("Brain Controller detected");
  if(hasECM) debugPrintln("ECM Panel detected");
  if(hasMasterARM) debugPrintln("Master ARM Panel detected");

  debugPrintln("Initializing available PCA9555 Inputs...");
  for (const auto& [addr, label] : discoveredDevices) {
    initPCA9555AsInput(addr);
  }

  // Initialize PCA9555 Cached Port States explicitly to OFF (active-low LEDs)
  debugPrintln("Initializing PCA9555 Cached Port States...");
  for (auto const& device : discoveredDevices) {
    uint8_t addr = device.first;
    PCA9555_cachedPortStates[addr][0] = 0xFF;
    PCA9555_cachedPortStates[addr][1] = 0xFF;

    Wire.beginTransmission(addr);
    Wire.write(0x02);
    Wire.write(0xFF);
    Wire.write(0xFF);
    Wire.endTransmission();
  }

  debugPrintln("Initializing Panels....");
  if (hasLA) LeftAnnunciator_init();
  if (hasRA) RightAnnunciator_init();

  debugPrint("Initializing PCA Panels....");
  if (hasIR) IRCool_init();
  if (hasECM) ECM_init();
  if (hasMasterARM) MasterARM_init();

  // Active Panels for LED Initialization
  const char* activePanels[7];
  int panelCount = 0;

  #define ADD_PANEL_IF_ENABLED(flag, name) if (flag) activePanels[panelCount++] = name;

  ADD_PANEL_IF_ENABLED(hasIR, "IR");
  ADD_PANEL_IF_ENABLED(hasLA, "LA");
  ADD_PANEL_IF_ENABLED(hasRA, "RA");
  ADD_PANEL_IF_ENABLED(hasCA, "CA");
  ADD_PANEL_IF_ENABLED(hasLockShoot, "LOCKSHOOT");
  ADD_PANEL_IF_ENABLED(hasECM, "ECM");
  ADD_PANEL_IF_ENABLED(hasMasterARM, "ARM");

  debugPrintln("Initializing LEDs...");
  initializeLEDs(activePanels, panelCount);
  
  #if TEST_LEDS
    printLEDMenu();
    handleLEDSelection();
    debugPrintln("Exiting LED selection menu. Continuing execution...");
  #endif 

  DCSBIOS_init();

  // If we are not debugging we turn it off.
  if(!DEBUG) enablePCA9555Logging(0);

  // Ready to go!
  debugPrintln("Device is now ready!\n");
}

// Arduino Loop Routine
void loop() {
  #if DEBUG_PERFORMANCE
    beginProfiling("Main Loop");
  #endif

  HIDManager_keepAlive();

  // Shadow buffer implementation for Caution Advisory to guarantee row-coherent updates 
  // Solves problem with matrix scanning and partial row writes
  if (hasCA) GN1640_tick();

  if (hasLA) LeftAnnunciator_loop();
  if (hasRA) RightAnnunciator_loop();
  if (hasIR) IRCool_loop();

  if (hasECM) ECM_loop();
  if (hasMasterARM) MasterARM_loop();

  if (isPCA9555LoggingEnabled()) {
    if (hasBrain) {
        byte p0, p1;
        readPCA9555(0x26, p0, p1);
    }
    if (hasECM) {
        byte p0, p1;
        readPCA9555(0x22, p0, p1);
    }
    if (hasMasterARM) {
        byte p0, p1;
        readPCA9555(0x5B, p0, p1);
    }
  }

  // DCS-BIOS Bridge loop
  DCSBIOS_loop();

  #if DEBUG_PERFORMANCE
  endProfiling("Main Loop");
  #endif

  #if DEBUG_PERFORMANCE
  perfMonitorUpdate();
  #endif
}