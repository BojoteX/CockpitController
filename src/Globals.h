// Globals.h
#pragma once

// Now pull in all of Arduino—this brings in HardwareSerial, Serial0, etc.
#include <Arduino.h>

// Continue loading the common #includes
#include "../Config.h"
#include "../lib/CUtils/src/CUtils.h" 
#include "DebugPrint.h"

// Load TinyUSB Support
#include "tusb.h"

#if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)
// Pull in the USB stack + HID (CDC ON BOOT ALREADY LOADS USB, SO DONT NEED IT)
#include <USB.h>
#include <USBCDC.h>
extern USBCDC USBSerial;
#define Serial USBSerial
#endif            

// Panel initialization functions (extern declarations)
extern void ECM_init();
extern void IRCool_init();
extern void MasterARM_init();

// Manual panel flags (don't change at runtime)
static constexpr bool hasCA         = LOAD_PANEL_CA;
static constexpr bool hasLA         = LOAD_PANEL_LA;
static constexpr bool hasRA         = LOAD_PANEL_RA;
static constexpr bool hasIR         = LOAD_PANEL_IR;
static constexpr bool hasLockShoot  = LOAD_PANEL_LOCKSHOOT;

// Init panels from anywhere
extern void initializePanels();

// PCA panels – determined at runtime
extern bool hasBrain;
extern bool hasECM;
extern bool hasMasterARM;

// To Check in which mode we are operating
extern bool isModeSelectorDCS();

// So we can target difference devices for the same function from anywhere
extern TM1637Device RA_Device;
extern TM1637Device LA_Device;

extern bool DEBUG;
extern bool debugToSerial;   // true = allow Serial prints
extern bool debugToUDP;      // true = allow UDP prints


#ifdef ESP_ARDUINO_VERSION_MAJOR
  #if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    #include <driver/ledc.h>
    static bool _esp32_pwm_init = false;
    static const ledc_channel_t _esp32_pwm_chan = LEDC_CHANNEL_0;
    inline void analogWrite(uint8_t pin, uint8_t duty) {
      if (!_esp32_pwm_init) {
        ledcSetup(_esp32_pwm_chan, 5000, 8);
        _esp32_pwm_init = true;
      }
      ledcAttachPin(pin, _esp32_pwm_chan);
      ledcWrite(_esp32_pwm_chan, duty);
    }
  #endif
#endif