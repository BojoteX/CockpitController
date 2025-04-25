#include <Arduino.h>
#include "src/Globals.h"   

#if DEBUG_ENABLED
bool DEBUG = true;
bool debugToSerial = true;
bool debugToUDP    = true;
#else
bool DEBUG = false;
bool debugToSerial = false;
bool debugToUDP    = false;
#endif

void debugSetOutput(bool toSerial, bool toUDP) {
    debugToSerial = toSerial;
    debugToUDP    = toUDP;
}

void debugPrint(const char* msg) {
    if (debugToSerial) serialDebugPrint(msg);
    if (debugToUDP)    wifiDebugPrint(msg);
}

void debugPrintln(const char* msg) {
    if (debugToSerial) serialDebugPrintln(msg);
    if (debugToUDP)    wifiDebugPrintln(msg);
}

void debugPrintf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    debugPrint(buf);
}

void sendDebug(const char* msg) {
    debugPrintln(msg);
}

void serialDebugPrint(const char* msg) {
    Serial.print(msg);
}

void serialDebugPrintln(const char* msg) {
    Serial.println(msg);
}