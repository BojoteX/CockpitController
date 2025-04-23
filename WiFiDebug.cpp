#include "Config.h"

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#include "src/Globals.h"

WiFiUDP udp;
bool wifiConnected = false;

void sendDebug(const char* msg) {
  if (!wifiConnected) return;
  udp.beginPacket(IP_ADDRESS, PORT);
  udp.write((const uint8_t*)msg, strlen(msg));  
  udp.endPacket();
}

void wifi_setup() {
  reduceCpuFreq();

  debugPrint("[WiFi DEBUG Enabled] Connecting...");

  delay(100);
  if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect(true);
      delay(20000);
      WiFi.mode(WIFI_STA);
      esp_wifi_set_ps(WIFI_PS_MAX_MODEM);  // ✅ Aggressive power save (may affect latency)
      WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    debugPrint(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    debugPrint("\n[!] WiFi connection failed. Restarting...\n");
    ESP.restart();
  }

  udp.begin(PORT);
  wifiConnected = true;

  delay(250);

  char buf[128];
  snprintf(buf, sizeof(buf), "[✓] Connected | IP: %s\n", WiFi.localIP().toString().c_str());
  sendDebug(buf);
  debugPrint(buf);
}

#endif DEBUG_USE_WIFI