// HEADER  ESP32S2
// PIN 1   GND/GND
// PIN 2   3V3
// PIN 3   GPIO 6  (Panel Backlight)
// PIN 4   GPIO 18 (HMD Knob)
// PIN 5   N/A
// PIN 6   N/A
// PIN 7   N/A
// PIN 8   GPIO 34 (SPIN RCVY LED)
//
// Shared across all panels GPIO 8 (SDA) and GPIO 9 (SCL)

// IRCoolPanel.cpp
// TEKCreations F/A-18C IR COOL Panel Firmware Logic
// Author: Bojote

#include "src/IRCoolPanel.h"
#include "lib/CUtils/src/CUtils.h"
#include "src/HIDManager.h"
#include "src/debugPrint.h"

// Previous state cache for change detection
static byte prevIRCPort0 = 0xFF;
static byte prevIRCPort1 = 0xFF;

// Analog pin connected to HMD Knob (Rx axis)
constexpr int HMD_KNOB_PIN = 18;

// Port bit mappings for panel @0x26
enum Port0Bits {
  SPIN_RCVY = 0        // LOW = RCVY, HIGH = NORM
};

enum Port1Bits {
  IR_COOL_ORIDE = 0,   // LOW = ORIDE
  IR_COOL_OFF   = 1    // LOW = OFF
  // NORM = both HIGH
};

// Initializes panel by reading initial state and deferring HID report
void IRCool_init() {
  delay(50);  // Asegura que el PCA esté inicializado antes de leer
  
  byte port0, port1;
  if (readPCA9555(IRCOOL_PCA_ADDR, port0, port1)) {
    prevIRCPort0 = port0;
    prevIRCPort1 = port1;

    // SPIN switch (2-position)
    if (!bitRead(port1, 2)) {
      HIDManager_setNamedButton("SPIN_RCVY",true);
    }
    else {
      HIDManager_setNamedButton("SPIN_NORM",true);
    }

    // IR COOL 3-position logic (PORT1 bits 0 & 1)
    if (!bitRead(port1, IR_COOL_OFF))
      HIDManager_setNamedButton("IR_COOL_OFF", true);
    else if (!bitRead(port1, IR_COOL_ORIDE))
      HIDManager_setNamedButton("IR_COOL_ORIDE", true);
    else
      HIDManager_setNamedButton("IR_COOL_NORM", true);

    HIDManager_moveAxis("HMD_OFF_BRT", HMD_KNOB_PIN);

    // Commit all deferred button states
    HIDManager_commitDeferredReport();

  } else {
    debugPrintln("❌ Could not read initial state of IRCool panel.");
  }
}

// Runtime loop for polling panel changes
void IRCool_loop() {
  byte port0, port1;
  if (!readPCA9555(IRCOOL_PCA_ADDR, port0, port1)) return;

  // SPIN switch
  if (bitRead(prevIRCPort1, 2) != bitRead(port1, 2)) {
    if (!bitRead(port1, 2)) {
      HIDManager_setNamedButton("SPIN_RCVY");
    }
    else {
      HIDManager_setNamedButton("SPIN_NORM");
    }
  }

  // IR COOL 3-position logic
  if ((bitRead(prevIRCPort1, IR_COOL_OFF) != bitRead(port1, IR_COOL_OFF)) ||
      (bitRead(prevIRCPort1, IR_COOL_ORIDE) != bitRead(port1, IR_COOL_ORIDE))) {

    if (!bitRead(port1, IR_COOL_OFF))
      HIDManager_setNamedButton("IR_COOL_OFF");
    else if (!bitRead(port1, IR_COOL_ORIDE))
      HIDManager_setNamedButton("IR_COOL_ORIDE");
    else
      HIDManager_setNamedButton("IR_COOL_NORM");
  }

  // update the HMD knob
  HIDManager_moveAxis("HMD_OFF_BRT", HMD_KNOB_PIN);

  // Update previous states
  prevIRCPort0 = port0;
  prevIRCPort1 = port1;
}