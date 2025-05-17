#include "src/Globals.h"

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
static WiFiUDP udp;

void scanNetworks() {
    // Make sure Wi-Fi is in station mode (needed for scan)
    WiFi.mode(WIFI_STA);
    Serial.println("\nScanning for Wi-Fi networks…");

    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("  ► No networks found");
    } else {
        for (int i = 0; i < n; i++) {
            char ssidBuf[33];  // Max SSID length is 32 + null
            WiFi.SSID(i).toCharArray(ssidBuf, sizeof(ssidBuf));

            int  rssi = WiFi.RSSI(i);
            bool sec  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);

            Serial.printf("  %2d: %-32s %4ddBm  %s\n",
                          i + 1,
                          ssidBuf,
                          rssi,
                          sec ? "🔒 Secured" : "🔓 Open");
            delay(5);  // let background tasks breathe
        }
    }

    WiFi.scanDelete();  // free internal scan memory
    Serial.println("");
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

void wifi_setup() {
    // scanNetworks();
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); 
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    IPAddress ip = WiFi.localIP();
    char ipbuf[24];
    snprintf(ipbuf, sizeof(ipbuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    Serial.print(" WiFi connected: ");
    Serial.println(ipbuf);
    wifiDebugInit();
    wifiDebugPrintf("WiFi connected: %s\n", ipbuf);
}
#endif