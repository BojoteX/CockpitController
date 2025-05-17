// WiFiDebug.h

#pragma once

#if DEBUG_USE_WIFI
#include <WiFi.h>
#include <WiFiUdp.h>
void scanNetworks();
void wifiDebugInit(uint16_t localPort = DEBUG_REMOTE_PORT);
void wifiDebugPrint(const char* msg);
void wifiDebugPrintf(const char* format, ...);
void wifiDebugPrintln(const char* msg);
void wifi_setup();
#endif