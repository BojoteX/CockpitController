// LEDControl.cpp
// Centralized LED management implementation for TEKCreations F/A-18C Cockpit Firmware

#include <Arduino.h>  // for pinMode, digitalWrite, analogWrite, delay, etc.
#include <string>
#include <unordered_map>
#include "src/Globals.h"
#include "src/LEDControl.h"
#include "src/Mappings.h"

// Actual definitions here (ONLY ONCE)
TM1637Device RA_Device;
TM1637Device LA_Device;

// efficient LED lookup map
std::unordered_map<std::string, const LEDMapping*> ledMap;

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

    GPIO_setAllLEDs(true); // GPIO always included clearly
    delay(1000);
    GPIO_setAllLEDs(false); // GPIO always included clearly

    // All ON (Clearly conditional initialization)
    if(hasCA) {
        GN1640_allOn();
        delay(1000);
        GN1640_allOff();
    }

    if(hasLockShoot) {
        WS2812_allOn(CRGB::Green);
        delay(1000);
        WS2812_allOff();
    }

    if(hasLA) {
        tm1637_allOn(LA_Device);
        delay(1000);
    }
    if(hasRA) {
        tm1637_allOn(RA_Device);
        delay(1000);
    }
    
    if(hasLA) {
        tm1637_allOff(LA_Device);
    }

    if(hasRA) { 
        tm1637_allOff(RA_Device);
    }
    
    if(hasMasterARM) { 
        PCA9555_allOn(0x5B);
        delay(1000);
        PCA9555_allOff(0x5B);
    }

    if(hasECM) {
        PCA9555_allOn(0x22);
        delay(1000);
        PCA9555_allOff(0x22);
    }
}
 
void setLED(const char* label, bool state, uint8_t intensity) {

    auto it = ledMap.find(label);
    if (it != ledMap.end()) {
        const LEDMapping* led = it->second;
        // LEDMapping* led = it->second;
        switch(led->deviceType) {

            case DEVICE_GPIO:
                pinMode(led->info.gpioInfo.gpio, OUTPUT);
                if (led->dimmable) {
                    if (!state)
                        analogWrite(led->info.gpioInfo.gpio, 0);  // TRUE OFF
                    else
                        analogWrite(led->info.gpioInfo.gpio, map(intensity, 0, 100, 0, 255));
                } else {
                    digitalWrite(led->info.gpioInfo.gpio, state);
                }
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
                                  state ? CRGB::Green : CRGB::Black);
                break;
        }
    } else {
        debugPrintf("⚠️ LED label '%s' not found\n", label);
    }
}