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

extern uint64_t _busyTimeAccum;
extern uint32_t _busyCount;

void performanceSetup();
void performanceLoop();

void beginProfiling(const char*);
void endProfiling  (const char*);

// macros so we can:
//   begin_profiling("DCSLoop");
//   … our code … (DcsBios::loop for example)
//   end_profiling("DCSTaskLoop");
#define begin_profiling(n) beginProfiling(n)
#define end_profiling(n)   endProfiling(n)

#endif // DEBUGPRINT_H
