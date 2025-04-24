#pragma once
#ifdef DEBUG_USE_WIFI
#include <WiFiUdp.h>
extern WiFiUDP udp;
extern bool wifiConnected;
void wifi_setup();
void sendDebug(const char* msg);
#else
void sendDebug(const char* msg);
#endif