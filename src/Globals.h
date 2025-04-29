// Globals.h
#pragma once

#include "Config.h"
#include "DebugPrint.h"
#include "../lib/CUtils/src/CUtils.h"             

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