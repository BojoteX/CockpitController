// LEDControl.cpp
// Optimized Centralized LED management for TEKCreations F/A-18C Cockpit Firmware

#include "src/Globals.h"
#include "src/LEDControl.h"
#include "src/LABELS/LEDMapping.h"
#if DEBUG_PERFORMANCE
#include "src/PerfMonitor.h"
#endif

// Pre-initialize all GPIO pins used by LEDs
static void preconfigureGPIO() {
    for (uint16_t i = 0; i < panelLEDsCount; ++i) {
        const auto& led = panelLEDs[i];
        if (led.deviceType == DEVICE_GPIO) {
            pinMode(led.info.gpioInfo.gpio, OUTPUT);
            digitalWrite(led.info.gpioInfo.gpio, LOW); // Default OFF
        }
    }
}

// Initialize all LEDs and their associated devices
void initializeLEDs(const char* activePanels[], unsigned int panelCount) {
    preconfigureGPIO();

    if (hasMasterARM) PCA9555_autoInitFromLEDMap(0x5B);
    if (hasECM) PCA9555_autoInitFromLEDMap(0x22);
    if (hasBrain) PCA9555_autoInitFromLEDMap(0x26);

    if (hasLockShoot) {
        debugPrintln("✅ Lock/Shoot detected, initializing...");
        WS2812_init();
    } else {
        debugPrintln("⚠️ Lock/Shoot NOT detected!");
    }

    if (hasCA) {
        debugPrintln("✅ Caution Advisory detected, initializing...");
        GN1640_init(GLOBAL_CLK_PIN, CA_DIO_PIN);
    } else {
        debugPrintln("⚠️ Caution Advisory NOT detected!");
    }

    if (hasLA && hasRA) {
        debugPrintln("✅ Left & Right Annunciators detected, initializing...");
        tm1637_init(LA_Device, GLOBAL_CLK_PIN, LA_DIO_PIN);
        tm1637_init(RA_Device, GLOBAL_CLK_PIN, RA_DIO_PIN);
    }
    else if (hasLA) {
        debugPrintln("✅ Left Annunciator detected, initializing...");
        tm1637_init(LA_Device, GLOBAL_CLK_PIN, LA_DIO_PIN);
    }
    else if (hasRA) {
        debugPrintln("✅ Right Annunciator detected, initializing...");
        tm1637_init(RA_Device, GLOBAL_CLK_PIN, RA_DIO_PIN);
    } else {
        debugPrintln("⚠️ No Annunciators detected!");
    }

    // Annunciators
    if (hasLA && hasRA) { tm1637_allOn(RA_Device); tm1637_allOn(LA_Device); delay(1000); tm1637_allOff(RA_Device); tm1637_allOff(LA_Device); }
    else if (hasLA) { tm1637_allOn(LA_Device); delay(1000); tm1637_allOff(LA_Device); }
    else if (hasRA) { tm1637_allOn(RA_Device); delay(1000); tm1637_allOff(RA_Device); }

    // Flash sequence
    if (hasMasterARM) { PCA9555_allOn(0x5B); delay(1000); PCA9555_allOff(0x5B); }
    if (hasECM) { PCA9555_allOn(0x22); delay(1000); PCA9555_allOff(0x22); }
    if (hasCA) { GN1640_allOn(); delay(1000); GN1640_allOff(); }
    if (hasLockShoot) { WS2812_allOn(CRGB::Green); delay(1000); WS2812_allOff(); }
    GPIO_setAllLEDs(true); delay(1000); GPIO_setAllLEDs(false);
}

// Blazing fast setLED()
void setLED(const char* label, bool state, uint8_t intensity) {

    const LEDMapping* led = findLED(label);
    if (!led) {
        if(DEBUG) debugPrintf("⚠️ LED label '%s' not found\n", label);
        return;
    }

    switch (led->deviceType) {
        case DEVICE_GPIO:
            #if DEBUG_PERFORMANCE
            beginProfiling("setLED: GPIO");
            #endif
            if (led->dimmable) {
                analogWrite(led->info.gpioInfo.gpio, state ? map(intensity, 0, 100, 0, 255) : 0);
            } else {
                digitalWrite(led->info.gpioInfo.gpio, state);
            }
            #if DEBUG_PERFORMANCE
            endProfiling("setLED: GPIO");
            #endif            
            break;

        case DEVICE_PCA9555:
            #if DEBUG_PERFORMANCE
            beginProfiling("setLED: PCA9555");
            #endif        
            PCA9555_write(
                led->info.pcaInfo.address,
                led->info.pcaInfo.port,
                led->info.pcaInfo.bit,
                led->activeLow ? !state : state
            );
            #if DEBUG_PERFORMANCE
            endProfiling("setLED: PCA9555");
            #endif            
            break;

        case DEVICE_TM1637: {
            #if DEBUG_PERFORMANCE
            beginProfiling("setLED: TM1637");
            #endif            
            auto& device = (led->info.tm1637Info.dioPin == RA_DIO_PIN) ? RA_Device : LA_Device;
            tm1637_displaySingleLED(device,
                led->info.tm1637Info.segment,
                led->info.tm1637Info.bit,
                state
            );
            #if DEBUG_PERFORMANCE
            endProfiling("setLED: TM1637");
            #endif            
            break;
        }

        case DEVICE_GN1640T:
            #if DEBUG_PERFORMANCE
            beginProfiling("setLED: GN1640");
            #endif        
            GN1640_setLED(
                led->info.gn1640Info.column,
                led->info.gn1640Info.row,
                state
            );
            #if DEBUG_PERFORMANCE
            endProfiling("setLED: GN1640");
            #endif            
            break;

        case DEVICE_WS2812:
            #if DEBUG_PERFORMANCE
            beginProfiling("setLED: WS2812");
            #endif        
            WS2812_setLEDColor(
                led->info.ws2812Info.index,
                state ? CRGB::Green : CRGB::Black
            );
            #if DEBUG_PERFORMANCE
            endProfiling("setLED: WS2812");
            #endif            
            break;

        case DEVICE_NONE:
        default:
            #if DEBUG_PERFORMANCE
            beginProfiling("setLED: Unknown Device");
            #endif        
            if(DEBUG) debugPrintf("⚠️ '%s' is NOT a LED or has not being configured yet\n", label);
            #if DEBUG_PERFORMANCE
            endProfiling("setLED: Unknown Device");
            #endif            
            break;
    }
}