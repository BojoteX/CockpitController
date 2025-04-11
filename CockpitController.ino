// Cockpit Brain Controller Firmware by Jesus "Bojote" Altuve
// Dynamic I2C panel detection and configurable panel initialization

#define BAUD_RATE 250000
#define DCSBIOS_DEFAULT_SERIAL
#define DCSBIOS_DISABLE_SERVO
#include "src/DcsBios.h"

// Helps debug problems
bool DEBUG = false;

// -- GPIO Pin Configuration --
#define SDA_PIN 8                      // I2C Data Pin
#define SCL_PIN 9                      // I2C Clock Pin
#define MODE_SWITCH_PIN 33             // Mode Selection Pin (DCS-BIOS/HID)

// -- Serial Configuration --
#define SERIAL_STARTUP_DELAY 3000      // Delay (ms) allowing Serial Monitor to connect

#include <Wire.h>

// -- Project Headers --
#define DEFINE_MAPPINGS
#include "src/HIDManager.h"
#include "src/Globals.h"

// Auto-generated callbacks (copy-paste into main .ino)
void onLEDChange_LH_ADV_L_BAR_RED(unsigned int newValue) { setLED("LH_ADV_L_BAR_RED", newValue); }
void onLEDChange_LH_ADV_L_BLEED(unsigned int newValue) { setLED("LH_ADV_L_BLEED", newValue); }
void onLEDChange_LH_ADV_R_BLEED(unsigned int newValue) { setLED("LH_ADV_R_BLEED", newValue); }
void onLEDChange_LH_ADV_SPD_BRK(unsigned int newValue) { setLED("LH_ADV_SPD_BRK", newValue); }
void onLEDChange_LH_ADV_STBY(unsigned int newValue) { setLED("LH_ADV_STBY", newValue); }
void onLEDChange_LS_LOCK(unsigned int newValue) { setLED("LS_LOCK", newValue); }
void onLEDChange_LS_SHOOT(unsigned int newValue) { setLED("LS_SHOOT", newValue); }
void onLEDChange_LS_SHOOT_STROBE(unsigned int newValue) { setLED("LS_SHOOT_STROBE", newValue); }
void onLEDChange_LH_ADV_ASPJ_OH(unsigned int newValue) { setLED("LH_ADV_ASPJ_OH", newValue); }
void onLEDChange_LH_ADV_GO(unsigned int newValue) { setLED("LH_ADV_GO", newValue); }
void onLEDChange_LH_ADV_L_BAR_GREEN(unsigned int newValue) { setLED("LH_ADV_L_BAR_GREEN", newValue); }
void onLEDChange_LH_ADV_REC(unsigned int newValue) { setLED("LH_ADV_REC", newValue); }
void onLEDChange_LH_ADV_NO_GO(unsigned int newValue) { setLED("LH_NOGO_LT", newValue); }
void onLEDChange_LH_ADV_XMIT(unsigned int newValue) { setLED("LH_ADV_XMIT", newValue); }
void onLEDChange_RH_ADV_AAA(unsigned int newValue) { setLED("RH_ADV_AAA", newValue); }
void onLEDChange_RH_ADV_AI(unsigned int newValue) { setLED("RH_ADV_AI", newValue); }
void onLEDChange_RH_ADV_CW(unsigned int newValue) { setLED("RH_ADV_CW", newValue); }
void onLEDChange_RH_ADV_DISP(unsigned int newValue) { setLED("RH_ADV_DISP", newValue); }
void onLEDChange_RH_ADV_RCDR_ON(unsigned int newValue) { setLED("RH_ADV_RCDR_ON", newValue); }
void onLEDChange_RH_ADV_SAM(unsigned int newValue) { setLED("RH_ADV_SAM", newValue); }
void onLEDChange_RH_ADV_SPARE_RH1(unsigned int newValue) { setLED("RH_ADV_SPARE_RH1", newValue); }
void onLEDChange_RH_ADV_SPARE_RH2(unsigned int newValue) { setLED("RH_ADV_SPARE_RH2", newValue); }
void onLEDChange_RH_ADV_SPARE_RH3(unsigned int newValue) { setLED("RH_ADV_SPARE_RH3", newValue); }
void onLEDChange_MASTER_MODE_AA_LT(unsigned int newValue) { setLED("MASTER_MODE_AA_LT", newValue); }
void onLEDChange_MASTER_MODE_AG_LT(unsigned int newValue) { setLED("MASTER_MODE_AG_LT", newValue); }
void onLEDChange_RH_ADV_SPARE_RH4(unsigned int newValue) { setLED("RH_ADV_SPARE_RH4", newValue); }
void onLEDChange_RH_ADV_SPARE_RH5(unsigned int newValue) { setLED("RH_ADV_SPARE_RH5", newValue); }
void onLEDChange_SPIN_LT(unsigned int newValue) { setLED("SPIN_LT", newValue); }
void onLEDChange_CLIP_CK_SEAT_LT(unsigned int newValue) { setLED("CLIP_CK_SEAT_LT", newValue); }
void onLEDChange_CLIP_APU_ACC_LT(unsigned int newValue) { setLED("CLIP_APU_ACC_LT", newValue); }
void onLEDChange_CLIP_BATT_SW_LT(unsigned int newValue) { setLED("CLIP_BATT_SW_LT", newValue); }
void onLEDChange_CLIP_FCES_LT(unsigned int newValue) { setLED("CLIP_FCES_LT", newValue); }
void onLEDChange_CLIP_FCS_HOT_LT(unsigned int newValue) { setLED("CLIP_FCS_HOT_LT", newValue); }
void onLEDChange_CLIP_FUEL_LO_LT(unsigned int newValue) { setLED("CLIP_FUEL_LO_LT", newValue); }
void onLEDChange_CLIP_GEN_TIE_LT(unsigned int newValue) { setLED("CLIP_GEN_TIE_LT", newValue); }
void onLEDChange_CLIP_SPARE_CTN1_LT(unsigned int newValue) { setLED("CLIP_SPARE_CTN1_LT", newValue); }
void onLEDChange_CLIP_SPARE_CTN2_LT(unsigned int newValue) { setLED("CLIP_SPARE_CTN2_LT", newValue); }
void onLEDChange_CLIP_L_GEN_LT(unsigned int newValue) { setLED("CLIP_L_GEN_LT", newValue); }
void onLEDChange_CLIP_R_GEN_LT(unsigned int newValue) { setLED("CLIP_R_GEN_LT", newValue); }
void onLEDChange_CLIP_SPARE_CTN3_LT(unsigned int newValue) { setLED("CLIP_SPARE_CTN3_LT", newValue); }

// DCS-BIOS buffer declarations:
DcsBios::IntegerBuffer LH_ADV_L_BAR_RED_Buffer(0x7408, 0x8000, 15, onLEDChange_LH_ADV_L_BAR_RED);
DcsBios::IntegerBuffer LH_ADV_L_BLEED_Buffer(0x7408, 0x0800, 11, onLEDChange_LH_ADV_L_BLEED);
DcsBios::IntegerBuffer LH_ADV_R_BLEED_Buffer(0x7408, 0x1000, 12, onLEDChange_LH_ADV_R_BLEED);
DcsBios::IntegerBuffer LH_ADV_SPD_BRK_Buffer(0x7408, 0x2000, 13, onLEDChange_LH_ADV_SPD_BRK);
DcsBios::IntegerBuffer LH_ADV_STBY_Buffer(0x7408, 0x4000, 14, onLEDChange_LH_ADV_STBY);
DcsBios::IntegerBuffer LS_LOCK_Buffer(0x7408, 0x0001, 0, onLEDChange_LS_LOCK);
DcsBios::IntegerBuffer LS_SHOOT_Buffer(0x7408, 0x0002, 1, onLEDChange_LS_SHOOT);
DcsBios::IntegerBuffer LS_SHOOT_STROBE_Buffer(0x7408, 0x0004, 2, onLEDChange_LS_SHOOT_STROBE);
DcsBios::IntegerBuffer LH_ADV_ASPJ_OH_Buffer(0x740A, 0x0008, 3, onLEDChange_LH_ADV_ASPJ_OH);
DcsBios::IntegerBuffer LH_ADV_GO_Buffer(0x740A, 0x0010, 4, onLEDChange_LH_ADV_GO);
DcsBios::IntegerBuffer LH_ADV_L_BAR_GREEN_Buffer(0x740A, 0x0002, 1, onLEDChange_LH_ADV_L_BAR_GREEN);
DcsBios::IntegerBuffer LH_ADV_NO_GO_Buffer(0x740A, 0x0020, 5, onLEDChange_LH_ADV_NO_GO);
DcsBios::IntegerBuffer LH_ADV_REC_Buffer(0x740A, 0x0001, 0, onLEDChange_LH_ADV_REC);
DcsBios::IntegerBuffer LH_ADV_XMIT_Buffer(0x740A, 0x0004, 2, onLEDChange_LH_ADV_XMIT);
DcsBios::IntegerBuffer RH_ADV_AAA_Buffer(0x740A, 0x0800, 11, onLEDChange_RH_ADV_AAA);
DcsBios::IntegerBuffer RH_ADV_AI_Buffer(0x740A, 0x0400, 10, onLEDChange_RH_ADV_AI);
DcsBios::IntegerBuffer RH_ADV_CW_Buffer(0x740A, 0x1000, 12, onLEDChange_RH_ADV_CW);
DcsBios::IntegerBuffer RH_ADV_DISP_Buffer(0x740A, 0x0100, 8, onLEDChange_RH_ADV_DISP);
DcsBios::IntegerBuffer RH_ADV_RCDR_ON_Buffer(0x740A, 0x0080, 7, onLEDChange_RH_ADV_RCDR_ON);
DcsBios::IntegerBuffer RH_ADV_SAM_Buffer(0x740A, 0x0200, 9, onLEDChange_RH_ADV_SAM);
DcsBios::IntegerBuffer RH_ADV_SPARE_RH1_Buffer(0x740A, 0x2000, 13, onLEDChange_RH_ADV_SPARE_RH1);
DcsBios::IntegerBuffer RH_ADV_SPARE_RH2_Buffer(0x740A, 0x4000, 14, onLEDChange_RH_ADV_SPARE_RH2);
DcsBios::IntegerBuffer RH_ADV_SPARE_RH3_Buffer(0x740A, 0x8000, 15, onLEDChange_RH_ADV_SPARE_RH3);
DcsBios::IntegerBuffer MASTER_MODE_AA_LT_Buffer(0x740C, 0x0200, 9, onLEDChange_MASTER_MODE_AA_LT);
DcsBios::IntegerBuffer MASTER_MODE_AG_LT_Buffer(0x740C, 0x0400, 10, onLEDChange_MASTER_MODE_AG_LT);
DcsBios::IntegerBuffer RH_ADV_SPARE_RH4_Buffer(0x740C, 0x0001, 0, onLEDChange_RH_ADV_SPARE_RH4);
DcsBios::IntegerBuffer RH_ADV_SPARE_RH5_Buffer(0x740C, 0x0002, 1, onLEDChange_RH_ADV_SPARE_RH5);
DcsBios::IntegerBuffer SPIN_LT_Buffer(0x742A, 0x0800, 11, onLEDChange_SPIN_LT);
DcsBios::IntegerBuffer CLIP_CK_SEAT_LT_Buffer(0x74A0, 0x8000, 15, onLEDChange_CLIP_CK_SEAT_LT);
DcsBios::IntegerBuffer CLIP_APU_ACC_LT_Buffer(0x74A4, 0x0100, 8, onLEDChange_CLIP_APU_ACC_LT);
DcsBios::IntegerBuffer CLIP_BATT_SW_LT_Buffer(0x74A4, 0x0200, 9, onLEDChange_CLIP_BATT_SW_LT);
DcsBios::IntegerBuffer CLIP_FCES_LT_Buffer(0x74A4, 0x4000, 14, onLEDChange_CLIP_FCES_LT);
DcsBios::IntegerBuffer CLIP_FCS_HOT_LT_Buffer(0x74A4, 0x0400, 10, onLEDChange_CLIP_FCS_HOT_LT);
DcsBios::IntegerBuffer CLIP_FUEL_LO_LT_Buffer(0x74A4, 0x2000, 13, onLEDChange_CLIP_FUEL_LO_LT);
DcsBios::IntegerBuffer CLIP_GEN_TIE_LT_Buffer(0x74A4, 0x0800, 11, onLEDChange_CLIP_GEN_TIE_LT);
DcsBios::IntegerBuffer CLIP_SPARE_CTN1_LT_Buffer(0x74A4, 0x1000, 12, onLEDChange_CLIP_SPARE_CTN1_LT);
DcsBios::IntegerBuffer CLIP_SPARE_CTN2_LT_Buffer(0x74A4, 0x8000, 15, onLEDChange_CLIP_SPARE_CTN2_LT);
DcsBios::IntegerBuffer CLIP_L_GEN_LT_Buffer(0x74A8, 0x0100, 8, onLEDChange_CLIP_L_GEN_LT);
DcsBios::IntegerBuffer CLIP_R_GEN_LT_Buffer(0x74A8, 0x0200, 9, onLEDChange_CLIP_R_GEN_LT);
DcsBios::IntegerBuffer CLIP_SPARE_CTN3_LT_Buffer(0x74A8, 0x0400, 10, onLEDChange_CLIP_SPARE_CTN3_LT);

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

  // DCS-BIOS Bridge Initialization
  debugPrintln("Initializing DCS-BIOS Bridge...");
  DcsBios::setup();

  if (DEBUG) {
    enablePCA9555Logging(true);
    printLEDMenu();
    handleLEDSelection();
    debugPrintln("Exiting LED selection menu. Continuing execution...");
  }
  
  debugPrintln("\nInitialization Complete.\n");
}

// Arduino Loop Routine
void loop() {
  if (!isModeSelectorDCS()) HIDManager_keepAlive();

  if (hasLA) LeftAnnunciator_loop();
  if (hasRA) RightAnnunciator_loop();
  if (hasIR) IRCool_loop();

  if (hasECM) ECM_loop();
  if (hasMasterARM) MasterARM_loop();

  // PCA9555 Logging
  if (isPCA9555LoggingEnabled()) {
    byte p0, p1;
    readPCA9555(0x26, p0, p1);
    if (hasECM) readPCA9555(0x22, p0, p1);
    if (hasMasterARM) readPCA9555(0x5B, p0, p1);
  }

  // DCS-BIOS Bridge loop
  DcsBios::loop();
}