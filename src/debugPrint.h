// debugPrint.h

#pragma once

void debugSetOutput(bool toSerial, bool toUDP);
void debugPrint(const char* msg);
void debugPrintln(const char* msg);
void debugPrintf(const char* format, ...);
void serialDebugPrintf(const char* format, ...);
void sendDebug(const char* msg);
void serialDebugPrint(const char* msg);
void serialDebugPrintln(const char* msg);