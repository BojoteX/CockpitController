#include <Arduino.h>
#include "Config.h"

extern "C" void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  // Ignore DTR checks completely — we always "pretend" connected
  (void)itf;
  (void)dtr;
  (void)rts;
}
#include "tusb.h" 

#if DEBUG_USE_WIFI
#include "Globals.h"
#include "DebugPrint.h"
#include "esp_pm.h"

WiFiUDP udp;
bool wifiConnected = false;

void sendDebug(const char* msg) {
  static unsigned long lastPrint = 0;
  static unsigned long startTime = millis();
  unsigned long now = millis();

  if (now - lastPrint >= 10000) {
    lastPrint = now;
    unsigned long uptimeSec = (now - startTime) / 1000;
    char buf[64];
    snprintf(buf, sizeof(buf), "[Uptime] %lus\n", uptimeSec);
    tud_cdc_n_write(0, buf, strlen(buf));
    tud_cdc_n_write_flush(0);
  }
}

/*
void sendDebug(const char* msg) {
  if (!wifiConnected || WiFi.status() != WL_CONNECTED || msg == nullptr) return;
  if (ESP.getFreeHeap() < 18000) return;
  udp.beginPacket(IP_ADDRESS, PORT);
  udp.write((const uint8_t*)msg, strlen(msg));  
  udp.endPacket();
}
*/

void wifi_setup() {
  esp_pm_config_esp32s2_t cfg = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 240,
    .light_sleep_enable = false
  };
  esp_pm_configure(&cfg);

  Serial.println("[WiFi DEBUG Enabled] Connecting...");
  delay(200);
  WiFi.disconnect(true);
  delay(200);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[!] WiFi connection failed. Restarting...");
    delay(100);
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

#else

void sendDebug(const char* msg) {

  Serial.write("TEST\n");
  Serial.flush();

  static unsigned long lastPrint = 0;
  static unsigned long startTime = millis();
  unsigned long now = millis();

  if (now - lastPrint >= 10000) {
    lastPrint = now;
    unsigned long uptimeSec = (now - startTime) / 1000;
    Serial.printf("%d %lus\n", msg, uptimeSec);
  }
}
#endif
