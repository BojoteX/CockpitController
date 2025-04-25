// RIGHTAnnunciator.cpp
// Implementation for RIGHT annunciator button panel integration

#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/Mappings.h"

// GPIO definitions are already globally defined in Mappings.h

// Track previous button state for debouncing
uint8_t prevRightKeys = 0xFF;

// Initialization routine for RIGHT annunciator buttons
void RightAnnunciator_init() {
  HIDManager_begin();

  pinMode(RA_CLK_PIN, OUTPUT);
  pinMode(RA_DIO_PIN, OUTPUT);
  digitalWrite(RA_CLK_PIN, HIGH);
  digitalWrite(RA_DIO_PIN, HIGH);

  // Set initial button states as released
  HIDManager_setNamedButton("RIGHT_ANNUN_APU_FIRE", true, false);
  HIDManager_setNamedButton("RIGHT_ANNUN_ENG_FIRE", true, false);
  HIDManager_commitDeferredReport();

  debugPrintln("✅ RIGHT Annunciator initialized for buttons");
}

// Main loop for button state checking
void RightAnnunciator_loop() {
  uint8_t rightKeys = tm1637_readKeys(RA_Device);

  if (rightKeys != prevRightKeys) {
    // RIGHT_ANNUN_APU_FIRE (bit 3 → 0x08)
    if ((rightKeys & 0x08) != (prevRightKeys & 0x08)) {
      bool pressed = !(rightKeys & 0x08);
      HIDManager_setNamedButton("RIGHT_ANNUN_APU_FIRE", false, pressed);
    }

    // RIGHT_ANNUN_ENG_FIRE (bit 0 → 0x01)
    if ((rightKeys & 0x01) != (prevRightKeys & 0x01)) {
      bool pressed = !(rightKeys & 0x01);
      HIDManager_setNamedButton("RIGHT_ANNUN_ENG_FIRE", false, pressed);
    }

    prevRightKeys = rightKeys;
  }
}