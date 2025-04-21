#pragma once
#include <WiFi.h>
#include <WiFiUdp.h>
#include "../Tools/ReplayTool/src/secure/password.h"

extern uint64_t _busyTimeAccum;
extern uint32_t _busyCount;

// === WiFi Config === (Uncomment Below and set credentials)
// #define WIFI_SSID "XXXXXXXXXX"
// #define WIFI_PASS "YYYYYYYYYY"

// Set your own IP
const IPAddress udpTarget(192, 168, 7, 116);
const int udpPort = 12345;

extern WiFiUDP udp;
extern bool wifiConnected;

void wifi_setup();
void sendDebug(const char* msg);
void performanceLoop();

void beginProfiling(const char*);
void endProfiling  (const char*);

// macros so we can:
//   begin_profiling("DCSLoop");
//   … our code … (DcsBios::loop for example)
//   end_profiling("DCSTaskLoop");
#define begin_profiling(n) beginProfiling(n)
#define end_profiling(n)   endProfiling(n)