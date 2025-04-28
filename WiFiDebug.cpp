#include "src/Globals.h"

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
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
    scanNetworks();
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); 
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    debugPrint("Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        debugPrint(".");
    }
    debugPrintln( (" WiFi connected: " + WiFi.localIP().toString()).c_str() );
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

void wifiDebugPrintf(const char* format, ...) {
    if (WiFi.status() != WL_CONNECTED) return;
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    udp.beginPacket(DEBUG_REMOTE_IP, DEBUG_REMOTE_PORT);
    udp.write((const uint8_t*)buf, strlen(buf));
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