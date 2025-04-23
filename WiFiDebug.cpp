#include "Config.h"

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
  Serial.print("[WiFi DEBUG Enabled] Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("\n[!] WiFi connection failed. Restarting...\n");
    ESP.restart();
  }

  udp.begin(udpPort);
  wifiConnected = true;

  delay(250);

  char buf[128];
  snprintf(buf, sizeof(buf), "[✓] Connected | IP: %s\n", WiFi.localIP().toString().c_str());
  sendDebug(buf);
}

#endif DEBUG_USE_WIFI