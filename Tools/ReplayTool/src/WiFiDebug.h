#pragma once
#include <WiFi.h>
#include <WiFiUdp.h>
#include "secure/password.h"

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