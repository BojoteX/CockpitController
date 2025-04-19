#include "Config.h" // use it to set IS_REPLAY or WIFI

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"

WiFiUDP udp;
bool wifiConnected = false;

void sendDebug(const char* msg) {
  if (!wifiConnected) return;
  udp.beginPacket(udpTarget, udpPort);
  udp.write((const uint8_t*)msg, strlen(msg));  
  udp.endPacket();
}

void wifi_setup() {
  sendDebug("[WiFi] Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    sendDebug(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    sendDebug("\n[!] WiFi connection failed. Restarting...\n");
    ESP.restart();
  }

  sendDebug("\n[✓] Connected | IP: ");
  sendDebug(WiFi.localIP().toString().c_str());
  sendDebug("\n");

  udp.begin(udpPort);
  wifiConnected = true;
}

#endif