#ifndef DEBUGPRINT_H
#define DEBUGPRINT_H

#include <Arduino.h>

// Basic prints
void debugPrint(const char* msg);
void debugPrint(const String& msg);
void debugPrintln(const char* msg);
void debugPrintln(const String& msg);

// Numeric overloads
void debugPrint(uint8_t val, int format = DEC);
void debugPrint(uint16_t val, int format = DEC);
void debugPrint(uint32_t val, int format = DEC);
void debugPrint(int val, int format = DEC);

void debugPrintln(uint8_t val, int format = DEC);
void debugPrintln(uint16_t val, int format = DEC);
void debugPrintln(uint32_t val, int format = DEC);
void debugPrintln(int val, int format = DEC);

// Direct byte write (for special chars)
void debugWrite(uint8_t val);

// Formatted printf-style (renamed explicitly)
void debugPrintf(const char *format, ...);
void debugPrintfln(const char *format, ...);

#endif // DEBUGPRINT_H
