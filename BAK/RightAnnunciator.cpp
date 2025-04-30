// RIGHTAnnunciator.cpp
// Implementation for RIGHT annunciator button panel integration

#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"

// Initialization routine for RIGHT annunciator buttons
void RightAnnunciator_init() {
  // HIDManager_setNamedButton("APU_FIRE_BTN", true, false);
  // HIDManager_setNamedButton("RIGHT_FIRE_BTN", true, false);
  // HIDManager_commitDeferredReport();
  debugPrintln("✅ RIGHT Annunciator initialized for buttons");
}

void RightAnnunciator_loop() {
    static unsigned long lastRAPoll = 0;
    if (!shouldPollMs(lastRAPoll)) return;

    static uint16_t raSampleCounter = 0;
    static uint8_t prevFinalKeysRA = 0xFF;

    static const char* latchedRightFireLabel = nullptr;

    uint8_t finalKeys = 0;

    if (tm1637_handleSamplingWindow(RA_Device, raSampleCounter, finalKeys)) {
        if (finalKeys != prevFinalKeysRA) {
            // APU_FIRE_BTN (bit 3)
            bool currApuFire = !(finalKeys & 0x08);
            bool prevApuFire = !(prevFinalKeysRA & 0x08);
            if (currApuFire != prevApuFire) {
                HIDManager_setNamedButton("APU_FIRE_BTN", false, currApuFire);
            }

            // RIGHT_FIRE_BTN (bit 0) — gated by cover
            bool currEngFire = !(finalKeys & 0x01);
            bool prevEngFire = !(prevFinalKeysRA & 0x01);

            if (currEngFire && !prevEngFire) {
                latchedRightFireLabel = isCoverOpen("RIGHT_FIRE_BTN_COVER")
                    ? "RIGHT_FIRE_BTN"
                    : "RIGHT_FIRE_BTN_COVER";
                HIDManager_setNamedButton(latchedRightFireLabel, false, true);
            }

            if (!currEngFire && prevEngFire && latchedRightFireLabel) {
                HIDManager_setNamedButton(latchedRightFireLabel, false, false);
                latchedRightFireLabel = nullptr;
            }

            HIDManager_commitDeferredReport();
            prevFinalKeysRA = finalKeys;
        }
    }
}

/*
void RightAnnunciator_loop() {
    static unsigned long lastRAPoll = 0;
    if (!shouldPollMs(lastRAPoll)) return;

    static uint16_t raSampleCounter = 0;
    static uint8_t prevFinalKeysRA = 0xFF; // Last stable evaluated keys

    uint8_t finalKeys = 0;

    if (tm1637_handleSamplingWindow(RA_Device, raSampleCounter, finalKeys)) {
        if (finalKeys != prevFinalKeysRA) {
            // Only trigger if something actually changed!

            uint8_t currApuFire = (finalKeys & 0x08);
            uint8_t prevApuFire = (prevFinalKeysRA & 0x08);
            if (currApuFire != prevApuFire) {
                HIDManager_setNamedButton("APU_FIRE_BTN", false, !(currApuFire));
            }

            uint8_t currEngFire = (finalKeys & 0x01);
            uint8_t prevEngFire = (prevFinalKeysRA & 0x01);
            if (currEngFire != prevEngFire) {
                HIDManager_setNamedButton("RIGHT_FIRE_BTN", false, !(currEngFire));
            }

            HIDManager_commitDeferredReport();

            // Update previous stable
            prevFinalKeysRA = finalKeys;
        }
    }
}
*/