#ifndef DEBUGPRINT_H
#define DEBUGPRINT_H

#pragma once
// DebugPrint.h - Unified debug interface

#include "WiFiDebug.h"

// Set output flags: toSerial, toUDP (only if USE_WIFI_DEBUG)
void debugSetOutput(bool toSerial, bool toUDP);

// Legacy debug functions
void debugPrint(const char* msg);
void debugPrintln(const char* msg);
void debugPrintf(const char* format, ...);
void serialDebugPrintf(const char* format, ...);
void sendDebug(const char* msg);

// API for Serial debug
void serialDebugPrint(const char* msg);
void serialDebugPrintln(const char* msg);

#endif // DEBUGPRINT_H