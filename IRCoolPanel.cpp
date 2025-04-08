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
#include "CUtils/CUtils.h"
#include "src/HIDManager.h"

// Previous state cache for change detection
static byte prevIRCPort0 = 0xFF;
static byte prevIRCPort1 = 0xFF;

// Analog pin connected to HMD Knob (Rx axis)
constexpr int HMD_KNOB_PIN = 18;
static int prevAnalog = -1;

void Axis_updateRx(uint8_t pin, int& prevValue, bool debug = false) {
  const int DEADZONE = 100;
  const int NOISE_THRESHOLD = 200;
  const int LOW_CLAMP = 100;
  const int HIGH_CLAMP = 4015;

  static int lastStable = -1;
  static int jitterCounter = 0;

  int raw = analogRead(pin);

  // Clamp extremes
  if (raw < LOW_CLAMP) raw = 0;
  else if (raw > HIGH_CLAMP) raw = 4095;

  // Noise filtering (smoothing)
  if (lastStable >= 0 && abs(raw - lastStable) < NOISE_THRESHOLD) {
    jitterCounter++;
    if (jitterCounter < 3) return;  // Ignore unless it's persistent
  } else {
    jitterCounter = 0;
  }

  lastStable = raw;

  if (abs(raw - prevValue) > DEADZONE) {
    HIDManager_moveAxis(raw);
    if (debug) {
      Serial.print("🧭 HMD Knob → ");
      Serial.println(raw);
    }
    prevValue = raw;
  }
}

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

    // Set initial value for HMD Knob
    int initial = analogRead(HMD_KNOB_PIN);
    initial = constrain(initial, 0, 4095);
    prevAnalog = initial;
    HIDManager_moveAxis(initial);

    // Commit all deferred button states
    HIDManager_commitDeferredReport();

  } else {
    Serial.println("❌ Could not read initial state of IRCool panel.");
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
  Axis_updateRx(HMD_KNOB_PIN, prevAnalog, true);

  // Update previous states
  prevIRCPort0 = port0;
  prevIRCPort1 = port1;
}