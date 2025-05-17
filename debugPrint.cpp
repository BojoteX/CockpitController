#include "src/Globals.h"

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#endif

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
}

void debugPrint(const char* msg) {
    if (debugToSerial) serialDebugPrint(msg);
    #if DEBUG_USE_WIFI
    if (debugToUDP)    wifiDebugPrint(msg);
    #endif
}

void debugPrintln(const char* msg) {
    if (debugToSerial) serialDebugPrintln(msg);
    #if DEBUG_USE_WIFI
    if (debugToUDP)    wifiDebugPrintln(msg);
    #endif
}

void debugPrintf(const char* format, ...) {
    char buf[256];  // Fixed-size stack buffer (safe)
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';  // Force null-termination
    debugPrint(buf);
}

void serialDebugPrintf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';
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