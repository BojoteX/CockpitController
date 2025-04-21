// HEADER ESP32S2
// PIN 1	3V3
// PIN 2	GND
// PIN 3	GPIOs 14, 17, 18 (ALIVE BUT NOT USED) 
// PIN 4	GPIOs 14, 17, 18 (ALIVE BUT NOT USED)
// PIN 5	GPIO 6
//
// Shared across all panels GPIO 8 (SDA) and GPIO 9 (SCL)

// ECMPanel.cpp
// TEKCreations F/A-18C ECM Panel Firmware Logic
// Author: Bojote

#include "src/ECMPanel.h"
#include "lib/CUtils/src/CUtils.h"
#include "src/HIDManager.h"
#include "src/debugPrint.h"

// Track previous PCA9555 state for ECM panel only
static byte prevECMPort0 = 0xFF;
static byte prevECMPort1 = 0xFF;

// Port 0 Bit Assignments (from PCA9555 @ 0x22)
enum Port0Bits {
  JETT_SEL = 0,             // Momentary button
  AUX_REL = 1,              // 2-position switch
  DISPENSER_BYPASS = 2,     // 3-position group
  DISPENSER_OFF = 3,
  ECM_OFF = 4,              // EMC rotary selector (4-pos group)
  ECM_STBY = 5,
  ECM_BIT = 6,
  ECM_REC = 7
};

// Initializes panel by reading its current state and deferring USB report
void ECM_init() {
  delay(50);  // Asegura que el PCA esté inicializado antes de leer
  
  byte port0, port1;
  if (readPCA9555(ECM_PCA_ADDR, port0, port1)) {
    prevECMPort0 = port0;
    prevECMPort1 = port1;

    // Momentary button (pressed = LOW)
    HIDManager_setNamedButton("JETT_SEL", true, !bitRead(port0, JETT_SEL));
    
    // DISPENSER 3-pos group: OFF / BYPASS / ON (default)
    if (!bitRead(port0, DISPENSER_OFF))
      HIDManager_setNamedButton("DISPENSER_OFF", true);
    else if (!bitRead(port0, DISPENSER_BYPASS))
      HIDManager_setNamedButton("DISPENSER_BYPASS", true);
    else
      HIDManager_setNamedButton("DISPENSER_ON", true);

    // AUX_REL 2-pos switch: ENABLE / NORM
    HIDManager_setNamedButton(
      bitRead(port0, AUX_REL) ? "AUX_REL_ENABLE" : "AUX_REL_NORM", true
    );

    // EMC Selector (4-pos rotary via 4 bits)
    if (!bitRead(port0, ECM_OFF))
      HIDManager_setNamedButton("ECM_MODE_SW_OFF", true);
    else if (!bitRead(port0, ECM_STBY))
      HIDManager_setNamedButton("ECM_MODE_SW_STBY", true);
    else if (!bitRead(port0, ECM_BIT))
      HIDManager_setNamedButton("ECM_MODE_SW_BIT", true);
    else if (!bitRead(port0, ECM_REC))
      HIDManager_setNamedButton("ECM_MODE_SW_REC", true);
    else if (!bitRead(port1, 0))
      HIDManager_setNamedButton("ECM_MODE_SW_XMIT", true);

    // Commit all deferred button reports at once
    HIDManager_commitDeferredReport();

  } else {
    debugPrintln("❌ Could not read initial state of ECM panel.");
  }
}

// Runtime loop for polling and reacting to state changes
void ECM_loop() {
  byte port0, port1;
  if (!readPCA9555(ECM_PCA_ADDR, port0, port1)) return;

  // JETT_SEL - Momentary press detection
  if (bitRead(prevECMPort0, JETT_SEL) != bitRead(port0, JETT_SEL)) {
    HIDManager_setNamedButton("JETT_SEL", false, !bitRead(port0, JETT_SEL));
  }

  // DISPENSER 3-pos switch
  if ((bitRead(prevECMPort0, DISPENSER_BYPASS) != bitRead(port0, DISPENSER_BYPASS)) ||
      (bitRead(prevECMPort0, DISPENSER_OFF) != bitRead(port0, DISPENSER_OFF))) {
    if (!bitRead(port0, DISPENSER_OFF))
      HIDManager_setNamedButton("DISPENSER_OFF");
    else if (!bitRead(port0, DISPENSER_BYPASS))
      HIDManager_setNamedButton("DISPENSER_BYPASS");
    else
      HIDManager_setNamedButton("DISPENSER_ON");
  }

  // AUX_REL 2-pos switch
  if (bitRead(prevECMPort0, AUX_REL) != bitRead(port0, AUX_REL)) {
    HIDManager_setNamedButton(
      bitRead(port0, AUX_REL) ? "AUX_REL_ENABLE" : "AUX_REL_NORM"
    );
  }

  // EMC rotary selector (OFF / STBY / BIT / REC / XMIT)
  for (int bit = ECM_OFF; bit <= ECM_REC; bit++) {
    if (bitRead(prevECMPort0, bit) != bitRead(port0, bit)) {
      switch (bit) {
        case ECM_OFF:
          if (!bitRead(port0, ECM_OFF)) HIDManager_setNamedButton("ECM_MODE_SW_OFF");
          break;
        case ECM_STBY:
          if (!bitRead(port0, ECM_STBY)) HIDManager_setNamedButton("ECM_MODE_SW_STBY");
          break;
        case ECM_BIT:
          if (!bitRead(port0, ECM_BIT)) HIDManager_setNamedButton("ECM_MODE_SW_BIT");
          break;
        case ECM_REC:
          if (!bitRead(port0, ECM_REC)) HIDManager_setNamedButton("ECM_MODE_SW_REC");
          break;
      }
    }
  }

  // EMC XMIT (bit 0 of PORT1, not used by REC in this revision)
  if (bitRead(prevECMPort1, 0) != bitRead(port1, 0)) {
    if (!bitRead(port1, 0)) {
      HIDManager_setNamedButton("ECM_MODE_SW_XMIT");
    }
  }

  // Update previous states
  prevECMPort0 = port0;
  prevECMPort1 = port1;
}