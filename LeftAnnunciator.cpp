// LEFTAnnunciator.cpp
// Implementation for LEFT annunciator button panel integration

#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/Mappings.h"

// Track previous button state for debouncing
uint8_t prevLeftKeys = 0xFF;

// Initialization routine for LEFT annunciator buttons
void LeftAnnunciator_init() {

  HIDManager_begin();

  pinMode(LA_CLK_PIN, OUTPUT);
  pinMode(LA_DIO_PIN, OUTPUT);
  digitalWrite(LA_CLK_PIN, HIGH);
  digitalWrite(LA_DIO_PIN, HIGH);

  // Set initial button states as released
  HIDManager_setNamedButton("LEFT_ANNUN_ENG_FIRE", true, false);
  HIDManager_setNamedButton("LEFT_ANNUN_MASTER_CAUTION", true, false);
  HIDManager_commitDeferredReport();

  debugPrintln("✅ LEFT Annunciator initialized for buttons");
}

// Main loop for button state checking
void LeftAnnunciator_loop() {
  uint8_t leftKeys = tm1637_readKeys(LA_Device);
  if (leftKeys != prevLeftKeys) {
    // LEFT_ANNUN_ENG_FIRE (bit 2 → 0x04)
    if ((leftKeys & 0x08) != (prevLeftKeys & 0x08)) {
      bool pressed = !(leftKeys & 0x08);
      HIDManager_setNamedButton("LEFT_ANNUN_ENG_FIRE", false, pressed);
    }

    // LEFT_ANNUN_MASTER_CAUTION (bit 0 → 0x01)
    if ((leftKeys & 0x01) != (prevLeftKeys & 0x01)) {
      bool pressed = !(leftKeys & 0x01);
      HIDManager_setNamedButton("LEFT_ANNUN_MASTER_CAUTION", false, pressed);
    }

    prevLeftKeys = leftKeys;
  }  
}