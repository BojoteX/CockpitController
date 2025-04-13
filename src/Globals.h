// Globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>   
#include "../lib/CUtils/src/CUtils.h"             
#include "DebugPrint.h"

// Panel initialization functions (extern declarations)
extern void ECM_init();
extern void IRCool_init();
extern void MasterARM_init();

// Just to check what panels we have included
extern bool hasIR;
extern bool hasLA;
extern bool hasRA;
extern bool hasCA;
extern bool hasLockShoot;
extern bool hasMasterARM;
extern bool hasECM;
extern bool hasBrain;

// To Check in which mode we are operating
extern bool isModeSelectorDCS();

// So we can target difference devices for the same function from anywhere
extern TM1637Device RA_Device;
extern TM1637Device LA_Device;

// TinyUSB
extern Adafruit_USBD_HID usb_hid;

#endif // GLOBALS_H