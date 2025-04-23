#pragma once
#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_wifi.h"

void wifi_setup();
void sendDebug(const char* msg);

// === WiFi Config === (set credentials for your WiFI)
#define WIFI_SSID "Metro5600"
#define WIFI_PASS "4458e8c3c2"

// Set your own IP
const IPAddress IP_ADDRESS(192, 168, 7, 37);
const int PORT = 12345;

extern WiFiUDP udp;
extern bool wifiConnected;