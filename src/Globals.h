// Globals.h

#pragma once

#include "PsramConfig.h"
#include "../Config.h"
#include "tusb.h"
#include <tusb_config.h>
#include <Arduino.h>
#include <USB.h>

#if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)
#include <USBCDC.h>
extern USBCDC USBSerial;
#define Serial USBSerial
#endif

// DO NOT CHANGE ANYTHING ABOVE THIS LINE....
#include "../lib/CUtils/src/CUtils.h" 

// So we can call our custom print functions from anywhere
#include "debugPrint.h"

#if DEBUG_PERFORMANCE
#include "PerfMonitor.h"
#endif

// Manual panel flags (don't change at runtime)
static constexpr bool hasCA         = LOAD_PANEL_CA;
static constexpr bool hasLA         = LOAD_PANEL_LA;
static constexpr bool hasRA         = LOAD_PANEL_RA;
static constexpr bool hasIR         = LOAD_PANEL_IR;
static constexpr bool hasLockShoot  = LOAD_PANEL_LOCKSHOOT;

///////////////////////////////////////////////////////////////////////////////////
// Extern declarations								  /
///////////////////////////////////////////////////////////////////////////////////

// So we can target difference devices for the same function from anywhere
extern TM1637Device RA_Device;
extern TM1637Device LA_Device;

// From our main .ino file
extern bool isModeSelectorDCS();	// To Check in which mode we are operating

// first feclared in debugPrint.cpp
extern bool DEBUG;
extern bool debugToSerial;   		// true = allow Serial prints
extern bool debugToUDP;      		// true = allow UDP prints

// Panel initialization functions (extern declarations)
extern void ECM_init();
extern void IRCool_init();
extern void MasterARM_init();

// PCA panels – determined at runtime
extern bool hasBrain;
extern bool hasECM;
extern bool hasMasterARM;

// Init panels from anywhere
extern void initializePanels();