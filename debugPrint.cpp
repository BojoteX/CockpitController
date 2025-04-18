#include "src/DebugPrint.h"
#include "src/Globals.h"  // For isModeSelectorDCS()

extern bool DEBUG;
extern bool isModeSelectorDCS(); // ensure you have this defined somewhere globally

// Basic prints
void debugPrint(const char* msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(msg);
}
void debugPrint(const String& msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(msg);
}
void debugPrintln(const char* msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(msg);
}
void debugPrintln(const String& msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(msg);
}

// Numeric overloads
void debugPrint(uint8_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}
void debugPrint(uint16_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}
void debugPrint(uint32_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}
void debugPrint(int val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}

void debugPrintln(uint8_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}
void debugPrintln(uint16_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}
void debugPrintln(uint32_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}
void debugPrintln(int val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}

// Byte write (special chars)
void debugWrite(uint8_t val) {
  if (DEBUG || !isModeSelectorDCS()) Serial.write(val);
}

// Formatted printf-style (renamed explicitly)
void debugPrintf(const char *format, ...) {
  if (DEBUG || !isModeSelectorDCS()) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
  }
}

void debugPrintfln(const char *format, ...) {
  if (DEBUG || !isModeSelectorDCS()) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.println(buffer);
  }
}
