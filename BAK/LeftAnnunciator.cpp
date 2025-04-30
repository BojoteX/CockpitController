// LEFTAnnunciator.cpp
// Implementation for LEFT annunciator button panel integration

#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"

// Initialization routine for LEFT annunciator buttons
void LeftAnnunciator_init() {
  // HIDManager_setNamedButton("LEFT_FIRE_BTN", true, false);
  // HIDManager_setNamedButton("MASTER_CAUTION_RESET_SW", true, false);
  // HIDManager_commitDeferredReport();
  debugPrintln("✅ LEFT Annunciator initialized for buttons");
}

void LeftAnnunciator_loop() {
    static unsigned long lastLAPoll = 0;
    if (!shouldPollMs(lastLAPoll)) return;

    static uint16_t laSampleCounter = 0;
    static uint8_t prevFinalKeysLA = 0xFF;

    // 🔒 Press intent latch
    static const char* latchedFireLabel = nullptr;

    uint8_t finalKeys = 0;

    if (tm1637_handleSamplingWindow(LA_Device, laSampleCounter, finalKeys)) {
        if (finalKeys != prevFinalKeysLA) {
            // FIRE button bit (bit 3)
            bool currFire = !(finalKeys & 0x08);
            bool prevFire = !(prevFinalKeysLA & 0x08);

            if (currFire && !prevFire) {
                // Latch target label at press time
                latchedFireLabel = isCoverOpen("LEFT_FIRE_BTN_COVER")
                    ? "LEFT_FIRE_BTN"
                    : "LEFT_FIRE_BTN_COVER";
                HIDManager_setNamedButton(latchedFireLabel, false, true);
            }

            if (!currFire && prevFire && latchedFireLabel) {
                HIDManager_setNamedButton(latchedFireLabel, false, false);
                latchedFireLabel = nullptr;
            }

            // MASTER_CAUTION_RESET_SW (bit 0) — no gating
            bool currCaution = !(finalKeys & 0x01);
            bool prevCaution = !(prevFinalKeysLA & 0x01);
            if (currCaution != prevCaution) {
                HIDManager_setNamedButton("MASTER_CAUTION_RESET_SW", false, currCaution);
            }

            HIDManager_commitDeferredReport();
            prevFinalKeysLA = finalKeys;
        }
    }
}

/*
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
                const char* target = isCoverOpen("LEFT_FIRE_BTN_COVER") 
                    ? "LEFT_FIRE_BTN" 
                    : "LEFT_FIRE_BTN_COVER";
                HIDManager_setNamedButton(target, false, !currFire);
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
*/