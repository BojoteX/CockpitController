// Cockpit Brain Controller Firmware by Jesus "Bojote" Altuve
// Dynamic I2C panel detection and configurable panel initialization

// Helps debug problems, set both...
// #define DEBUG_MODE
bool DEBUG = false; // Needed for alternate debugPrint

// -- GPIO Pin Configuration --
#define SDA_PIN 8                      // I2C Data Pin
#define SCL_PIN 9                      // I2C Clock Pin
#define MODE_SWITCH_PIN 33             // Mode Selection Pin (DCS-BIOS/HID)

// -- Serial Configuration --
#define BAUD_RATE 250000
#define SERIAL_STARTUP_DELAY 3000      // Delay (ms) allowing Serial Monitor to connect

// -- Project Headers --
#define DEFINE_MAPPINGS
#include "src/HIDManager.h"
#include "src/Globals.h"
#include "src/DCSBIOSBridge.h"
#include <Wire.h>

// Definitions for Panel Detection logic
bool hasIR, hasLA, hasRA, hasCA, hasLockShoot, hasMasterARM, hasECM, hasBrain;

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
  return digitalRead(MODE_SWITCH_PIN) == LOW;
}

// Arduino Setup Routine
void setup() {
  // GPIO Setup
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // I2C Initialization
  Wire.begin(SDA_PIN, SCL_PIN);

  // USB HID Initialization
  TinyUSBDevice.begin(0);

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

  // Force Panels
  hasLockShoot = true;
  hasCA = true;
  hasLA = true;
  hasRA = true;
  hasIR = true;
  hasECM = true;
  hasMasterARM = true;

  debugPrintln("Initializing Panels....");
  if (hasLA) LeftAnnunciator_init();
  if (hasRA) RightAnnunciator_init();
  if (hasIR) IRCool_init();

  // PCA9555 Expander Initialization
  debugPrintln("Initializing PCA9555 Inputs...");
  if (hasBrain) initPCA9555AsInput(0x26); //Assumes Brain controller connected
  if (hasECM) initPCA9555AsInput(0x22);
  if (hasMasterARM) initPCA9555AsInput(0x5B);

  // Initialize PCA9555 Cached Port States explicitly to OFF (active-low LEDs)
  debugPrintln("Initializing PCA9555 Cached Port States...");
  uint8_t pcaAddresses[] = {0x26, 0x22, 0x5B};
  for (int i = 0; i < sizeof(pcaAddresses); i++) {
    uint8_t addr = pcaAddresses[i];
    PCA9555_cachedPortStates[addr][0] = 0xFF;  // Port 0 all OFF (active-low LEDs)
    PCA9555_cachedPortStates[addr][1] = 0xFF;  // Port 1 all OFF (active-low LEDs)

    // Write these initial states explicitly to hardware
    Wire.beginTransmission(addr);
    Wire.write(0x02); // Output Port 0 register
    Wire.write(0xFF); // All OFF (active-low LEDs)
    Wire.write(0xFF); // Port 1 All OFF (active-low LEDs)
    Wire.endTransmission();
  }

  // Initialize selected panels
  debugPrintln("Initializing PCA Panels...");
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

  DCSBIOS_init();
  debugPrintln("\nDCSBIOS Library Initialization Complete.\n");

  #ifdef DEBUG_MODE
    enablePCA9555Logging(true);
    printLEDMenu();
    handleLEDSelection();
    debugPrintln("Exiting LED selection menu. Continuing execution...");
  #endif

  debugPrintln("\nREADY\n");
}

// Arduino Loop Routine
void loop() {
  if (!isModeSelectorDCS()) HIDManager_keepAlive();

  if (hasLA) LeftAnnunciator_loop();
  if (hasRA) RightAnnunciator_loop();
  if (hasIR) IRCool_loop();

  if (hasECM) ECM_loop();
  if (hasMasterARM) MasterARM_loop();

  // PCA9555 Logging (only used when building our own masks/detecting bit position/port for PCA)
  if (isPCA9555LoggingEnabled()) {
    byte p0, p1;
    if (hasBrain) readPCA9555(0x26, p0, p1);
    if (hasECM) readPCA9555(0x22, p0, p1);
    if (hasMasterARM) readPCA9555(0x5B, p0, p1);
  }

  // DCS-BIOS Bridge loop
  DCSBIOS_loop();
}