#pragma once
#include "../Config.h"
#include <Arduino.h>
#include <stdint.h>

#if DEBUG_USE_WIFI
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFiDebug.h - UDP debug output (enabled when USE_WIFI_DEBUG == 1)
void wifiDebugInit(uint16_t localPort = 4210);
void wifiDebugSetRemote(IPAddress ip, uint16_t port);
void wifiDebugPrint(const char* msg);
void wifiDebugPrintln(const char* msg);
void wifi_setup();
#else
// Stubs when WiFi debug is disabled
inline void wifiDebugInit(uint16_t = 0) {}
inline void wifiDebugSetRemote(IPAddress, uint16_t) {}
inline void wifiDebugPrint(const char*) {}
inline void wifiDebugPrintln(const char*) {}
#endif

/*
#if DEBUG_USE_WIFI
extern WiFiUDP udp;
extern bool wifiConnected;
void sendDebug(const char* msg);
void wifi_setup();
#endif
*/