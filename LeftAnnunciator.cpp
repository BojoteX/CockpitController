// LEFTAnnunciator.cpp
// Implementation for LEFT annunciator button panel integration

#include "src/Globals.h"
#include "src/HIDManager.h"

// Initialization routine for LEFT annunciator buttons
void LeftAnnunciator_init() {
  HIDManager_setNamedButton("LEFT_FIRE_BTN", true, false);
  HIDManager_setNamedButton("MASTER_CAUTION_RESET_SW", true, false);
  HIDManager_commitDeferredReport();

  debugPrintln("✅ LEFT Annunciator initialized for buttons");
}

void LeftAnnunciator_loop() {
    static unsigned long lastLAPoll = 0;
    if (!shouldPollMs(lastLAPoll)) return;

    static uint16_t laSampleCounter = 0;
    static uint8_t prevFinalKeysLA = 0xFF; // Track last stable state

    uint8_t finalKeys = 0;

    if (tm1637_handleSamplingWindow(LA_Device, laSampleCounter, finalKeys)) {
        if (finalKeys != prevFinalKeysLA) {
            // Detect real per-button changes
            uint8_t currFire = (finalKeys & 0x08);
            uint8_t prevFire = (prevFinalKeysLA & 0x08);
            if (currFire != prevFire) {
                HIDManager_setNamedButton("LEFT_FIRE_BTN", false, !(currFire));
            }

            uint8_t currCaution = (finalKeys & 0x01);
            uint8_t prevCaution = (prevFinalKeysLA & 0x01);
            if (currCaution != prevCaution) {
                HIDManager_setNamedButton("MASTER_CAUTION_RESET_SW", false, !(currCaution));
            }

            HIDManager_commitDeferredReport();

            // Update previous keys after handling
            prevFinalKeysLA = finalKeys;
        }
    }
}
