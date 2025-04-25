#include "src/WiFiDebug.h"

#if DEBUG_USE_WIFI
#include "src/Globals.h"

static WiFiUDP udp;

void scanNetworks() {
    // Make sure Wi-Fi is in station mode (needed for scan)
    WiFi.mode(WIFI_STA);
    // No blocking connect(), we just scan
    debugPrintln("\nScanning for Wi-Fi networks…");

    int n = WiFi.scanNetworks();
    if (n == 0) {
        debugPrintln("  ► No networks found");
    } else {
        for (int i = 0; i < n; i++) {
            String ssid = WiFi.SSID(i);
            int    rssi = WiFi.RSSI(i);
            bool   sec  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            debugPrintf("  %2d: %-32s %4ddBm  %s\n",
                        i + 1,
                        ssid.c_str(),
                        rssi,
                        sec ? "🔒 Secured" : "🔓 Open");
            delay(5);  // tiny yield so background tasks (and watchdog) stay happy
        }
    }
    // clean up scan results from memory
    WiFi.scanDelete();
    debugPrintln("");
}

void wifi_setup() {
    // scanNetworks();
    // WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); 
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    debugPrint("Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        debugPrint(".");
    }
    debugPrintln( ("WiFi connected: " + WiFi.localIP().toString()).c_str() );
    wifiDebugInit();
}

void wifiDebugInit(uint16_t localPort) {
    if (WiFi.status() != WL_CONNECTED) return;
    udp.begin(localPort);
}

void wifiDebugPrint(const char* msg) {
    if (WiFi.status() != WL_CONNECTED) return;
    udp.beginPacket(DEBUG_REMOTE_IP, DEBUG_REMOTE_PORT);
    udp.write((const uint8_t*)msg, strlen(msg));
    udp.endPacket();
}

void wifiDebugPrintln(const char* msg) {
    if (WiFi.status() != WL_CONNECTED) return;
    udp.beginPacket(DEBUG_REMOTE_IP, DEBUG_REMOTE_PORT);
    udp.write((const uint8_t*)msg, strlen(msg));
    udp.write((const uint8_t*)"\r\n", 2);
    udp.endPacket();
}
#endif

/*

#include "Config.h"

#if DEBUG_USE_WIFI
// #include "src/WiFiDebug.h"
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

  Serial.println("[WiFi DEBUG Enabled] Connecting...");  // ✅ Raw serial here!

  delay(200);
  if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect(true);
      delay(200); // 🔧 Reduced, safer
      WiFi.mode(WIFI_STA);
      esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
  }

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");  // ✅ Don't use debugPrint yet!
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[!] WiFi connection failed. Restarting...");
    delay(100);  // give time to flush
    ESP.restart();
  }

  udp.begin(PORT);
  wifiConnected = true;

  delay(250);

  char buf[128];
  snprintf(buf, sizeof(buf), "[✓] Connected | IP: %s\n", WiFi.localIP().toString().c_str());
  sendDebug(buf);
  debugPrint(buf);  // ✅ Now safe to use
}

#endif

*/