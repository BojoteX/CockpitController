#include <Arduino.h>
#include "src/Globals.h"   

#if DEBUG_ENABLED
bool DEBUG           = true;
#else
bool DEBUG           = false;
#endif

#if VERBOSE_MODE
bool debugToSerial   = true;
bool debugToUDP      = true;
#elif VERBOSE_MODE_WIFI_ONLY
bool debugToSerial   = false;
bool debugToUDP      = true;
#else
bool debugToSerial   = false;
bool debugToUDP      = false;
#endif

void debugSetOutput(bool toSerial, bool toUDP) {
    debugToSerial = toSerial;
    debugToUDP    = toUDP;
    debugPrintf("Output to Serial is set to %s\nOutput to UDP is set to %s\n", debugToSerial ? "ON" : "OFF", debugToUDP ? "ON" : "OFF");
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

void serialDebugPrintf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    serialDebugPrint(buf);
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