// LEDControl.cpp
// Centralized LED management implementation for TEKCreations F/A-18C Cockpit Firmware

// ===== Load Mappings =====
#include "src/Mappings.h"

// ===== HW Controller Utilities =====
#include "lib/CUtils/src/CUtils.h"

// ===== LED Control Implementation =====
#include "src/LEDControl.h"

#include "src/Globals.h"
#include <unordered_map>
#include <string>

// efficient LED lookup map
std::unordered_map<std::string, LEDMapping*> ledMap;

void initializeLEDs(const char* activePanels[], unsigned int panelCount) {

    // PCA9555 LEDs: Only init if panel detected
    if(hasMasterARM) PCA9555_autoInitFromLEDMap(0x5B); // Init MasterARM Leds
    if(hasECM) PCA9555_autoInitFromLEDMap(0x22); // Init ECM Leds

    if (hasLockShoot) {
      debugPrintln("✅ Lock/Shoot detected, initializing...");
      WS2812_init();
    } else debugPrintln("⚠️ Lock/Shoot NOT detected!");

    if (hasCA) {
      debugPrintln("✅ Caution Advisory detected, initializing...");
      GN1640_init(GLOBAL_CLK_PIN, CA_DIO_PIN);
    } else debugPrintln("⚠️ Caution Advisory NOT detected!");

    if (hasLA) {
      debugPrintln("✅ Left Annunciator detected, initializing...");
      tm1637_init(LA_Device, GLOBAL_CLK_PIN, LA_DIO_PIN);
    } else debugPrintln("⚠️ Left Annunciator NOT detected!");

    if (hasRA) {
      debugPrintln("✅ Right Annunciator detected, initializing...");
      tm1637_init(RA_Device, GLOBAL_CLK_PIN, RA_DIO_PIN);
    } else debugPrintln("⚠️ Right Annunciator NOT detected!");

    // Populate the efficient LED lookup map clearly for ACTIVE panels ONLY
    for (int i = 0; i < panelLEDsCount; i++) {
        std::string lbl(panelLEDs[i].label);
        ledMap[lbl] = &panelLEDs[i];
    }
    debugPrintln("✅ LED Lookup Map Initialized");

    // All ON (Clearly conditional initialization)
    if(hasCA) GN1640_allOn();
    if(hasLA) tm1637_allOn(LA_Device);
    if(hasRA) tm1637_allOn(RA_Device);
    if(hasLockShoot) WS2812_allOn(CRGB::Green);
    if(hasECM) PCA9555_allOn(0x22);
    if(hasMasterARM) PCA9555_allOn(0x5B);
    GPIO_setAllLEDs(true); // GPIO always included clearly

    delay(2000);

    // Turn off ALL (Clearly conditional initialization)
    if(hasCA) GN1640_allOff();
    if(hasLA) tm1637_allOff(LA_Device);
    if(hasRA) tm1637_allOff(RA_Device);
    if(hasLockShoot) WS2812_allOff();
    if(hasMasterARM) PCA9555_allOff(0x5B);
    if(hasECM) PCA9555_allOff(0x22);
    GPIO_setAllLEDs(false); // GPIO always included clearly
}

void setLED(const char* label, bool state, uint8_t intensity) {

    if (strcmp(label, "ALL_PANELS_ALL_LEDS") == 0) {
      // Handle ALL LEDs, including GPIO explicitly
      PCA9555_setAllLEDs(state);
      GPIO_setAllLEDs(state);
      WS2812_setAllLEDs(state);
      TM1637_setAllLEDs(state);
      GN1640_setAllLEDs(state);
      return;
    }

    // Handle meta-commands first
    if (strcmp(label, "ALL_PANELS_LEDS") == 0) {
        setAllPanelsLEDs(state);
        return;
    }

    if (strcmp(label, "LA_ALL_LEDS") == 0) {
        setPanelAllLEDs("LA_", state);
        return;
    }

    if (strcmp(label, "RA_ALL_LEDS") == 0) {
        setPanelAllLEDs("RA_", state);
        return;
    }

    if (strcmp(label, "ECM_ALL_LEDS") == 0) {
        setPanelAllLEDs("ECM_", state);
        return;
    }

    if (strcmp(label, "ARM_ALL_LEDS") == 0) {
        setPanelAllLEDs("ARM_", state);
        return;
    }

    if (strcmp(label, "CA_ALL_LEDS") == 0) {
        // setPanelAllLEDs("CA_", state);
        GN1640_setAllLEDs(state);
        return;
    }

    if (strcmp(label, "LOCKSHOOT_ALL_LEDS") == 0) {
        setPanelAllLEDs("LOCKSHOOT_", state);
        return;
    }

    auto it = ledMap.find(label);
    if (it != ledMap.end()) {
        LEDMapping* led = it->second;
        switch(led->deviceType) {
            case DEVICE_GPIO:
                pinMode(led->info.gpioInfo.gpio, OUTPUT);
                if (led->dimmable)
                    analogWrite(led->info.gpioInfo.gpio, map(intensity, 0, 100, 0, 255));
                else
                    digitalWrite(led->info.gpioInfo.gpio, state);
                break;

            case DEVICE_PCA9555: {
                bool writeState = led->activeLow ? !state : state;
                PCA9555_write(led->info.pcaInfo.address,
                              led->info.pcaInfo.port,
                              led->info.pcaInfo.bit,
                              writeState);
                break;
            }

            case DEVICE_TM1637:
                if (led->info.tm1637Info.dioPin == RA_DIO_PIN) {
                    tm1637_displaySingleLED(RA_Device, led->info.tm1637Info.segment, led->info.tm1637Info.bit, state);
                } else if (led->info.tm1637Info.dioPin == LA_DIO_PIN) {
                    tm1637_displaySingleLED(LA_Device, led->info.tm1637Info.segment, led->info.tm1637Info.bit, state);
                }
                break;

            case DEVICE_GN1640T:
                GN1640_setLED(led->info.gn1640Info.column,
                              led->info.gn1640Info.row, state);
                break;

            case DEVICE_WS2812:
                WS2812_setLEDColor(led->info.ws2812Info.index,
                                  state ? CRGB::White : CRGB::Black);
                break;
        }
    } else {
        debugPrintf("⚠️ LED label '%s' not found\n", label);
    }
}