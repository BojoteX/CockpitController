// Cockpit Brain Controller Firmware by Jesus "Bojote" Altuve
// Dynamic I2C panel detection and configurable panel initialization

// See that file first to enable WiFI debug or replay mode (simulating a stream from DCS)
#include "Config.h"

// Helps debug problems (To DEBUG over WiFi via UDP see Config.h)
bool DEBUG = true; // Needed for alternate debugPrint

// Init panel flags
bool hasCA, hasLA, hasRA, hasIR, hasLockShoot, hasBrain, hasECM, hasMasterARM;

// -- Serial Configuration --
#define BAUD_RATE 250000
#define SERIAL_STARTUP_DELAY 3000      // Delay (ms) allowing Serial Monitor to connect

// -- GPIO Pin Configuration --
#define SDA_PIN 8                      // I2C Data Pin
#define SCL_PIN 9                      // I2C Clock Pin
#define MODE_SWITCH_PIN 33             // Mode Selection Pin (DCS-BIOS/HID)

// -- Project Headers --
#define DEFINE_MAPPINGS
#include "src/HIDManager.h"
#include "src/Globals.h"
#include "src/DCSBIOSBridge.h"
#include <Wire.h>

// Definitions for Panel Detection logic
// bool hasIR, hasLA, hasRA, hasCA, hasLockShoot, hasMasterARM, hasECM, hasBrain;

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
  // GPIO Setup
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // I2C Initialization
  Wire.begin(SDA_PIN, SCL_PIN);

  // Starts our HID device
  HIDManager_begin();

  // Initialize serial
  Serial.begin(BAUD_RATE); 
  unsigned long start = millis();
  while (!Serial && (millis() - start < SERIAL_STARTUP_DELAY)) delay(1);

// Detect Panels (They are off by default)
  scanConnectedPanels();

  debugPrintln("\n=== Cockpit Brain Controller Initialization ===");
  debugPrint("Selected mode: ");
  debugPrintln(isModeSelectorDCS() ? "DCS-BIOS" : "HID");

  // Show PCA Panels we discovered
  printDiscoveredPanels();

  // Choose panels that are present only (PCA Panels are detected automatically)
  hasCA = false;        // Caution Advisory Panel
  hasLA = true;         // Left Annunciator
  hasRA = false;        // Right Annunciator
  hasIR = false;        // IR Cool Panel
  hasLockShoot = false; // LockShoot Panel

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

  debugPrintln("Initializing PCA Panels....");
  if (hasIR) IRCool_init();
  if (hasECM) ECM_init();
  if (hasMasterARM) MasterARM_init();

  // Active Panels for LED Initialization
  const char* activePanels[7];
  int panelCount = 0;

  if (hasIR) activePanels[panelCount++] = "IR";
  if (hasLA) activePanels[panelCount++] = "LA";
  if (hasRA) activePanels[panelCount++] = "RA";
  if (hasCA) activePanels[panelCount++] = "CA";
  if (hasLockShoot) activePanels[panelCount++] = "LOCKSHOOT";
  if (hasECM) activePanels[panelCount++] = "ECM";
  if (hasMasterARM) activePanels[panelCount++] = "ARM";

  debugPrintln("Initializing LEDs...");
  initializeLEDs(activePanels, panelCount);

  if(DEBUG) {
    enablePCA9555Logging(1);
    printLEDMenu();
    handleLEDSelection();
    debugPrintln("Exiting LED selection menu. Continuing execution...");
    debugPrintln("DEBUG mode is ENABLED");
  }
  else {
    enablePCA9555Logging(0);
  }

  DCSBIOS_init();

  // Ready to go!
  debugPrintln("Device is now ready!\n");
}

// Arduino Loop Routine
void loop() {
  if (!isModeSelectorDCS()) HIDManager_keepAlive();

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
}