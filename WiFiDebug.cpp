#include "src/Globals.h"

#if DEBUG_USE_WIFI
#include "src/PsramConfig.h"
#include "src/WiFiDebug.h"

static char dbgBuf[256] PS_ATTR;
static WiFiClient debugClient;

// Optional reconnect cooldown
static unsigned long lastConnectAttempt = 0;
static const unsigned long CONNECT_RETRY_INTERVAL_MS = 5000;

void scanNetworks() {
    WiFi.mode(WIFI_STA);
    Serial.println("\nScanning for Wi-Fi networks…");

    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("  ► No networks found");
    } else {
        for (int i = 0; i < n; i++) {
            // String ssid = WiFi.SSID(i);
            char ssidBuf[33];
            WiFi.SSID(i).toCharArray(ssidBuf, sizeof(ssidBuf));

            int    rssi = WiFi.RSSI(i);
            bool   sec  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            Serial.printf("  %2d: %-32s %4ddBm  %s\n",
                        i + 1,
                        ssidBuf,
                        rssi,
                        sec ? "🔒 Secured" : "🔓 Open");
            delay(5);
        }
    }
    WiFi.scanDelete();
    Serial.println("");
}

void wifiDebugInit(uint16_t) {
    if (WiFi.status() != WL_CONNECTED) return;
    debugClient.stop();  // Ensure fresh socket
    debugClient.connect(DEBUG_REMOTE_IP, DEBUG_REMOTE_PORT);
    lastConnectAttempt = millis();
}

void wifiDebugPrint(const char* msg) {
    if (WiFi.status() != WL_CONNECTED) return;

    if (!debugClient.connected()) {
        unsigned long now = millis();
        if (now - lastConnectAttempt >= CONNECT_RETRY_INTERVAL_MS) {
            debugClient.stop();
            debugClient.connect(DEBUG_REMOTE_IP, DEBUG_REMOTE_PORT);
            lastConnectAttempt = now;
        }
        return;
    }

    debugClient.write((const uint8_t*)msg, strlen(msg));
}

void wifiDebugPrintln(const char* msg) {
    if (WiFi.status() != WL_CONNECTED) return;
    if (!debugClient.connected()) return;

    static char lineBuf[300] PS_ATTR;  // 256 + CRLF + margin
    size_t len = strlen(msg);
    if (len > sizeof(lineBuf) - 3) len = sizeof(lineBuf) - 3;

    memcpy(lineBuf, msg, len);
    lineBuf[len++] = '\r';
    lineBuf[len++] = '\n';

    debugClient.write((const uint8_t*)lineBuf, len);
}

void wifiDebugPrintf(const char* format, ...) {
    if (WiFi.status() != WL_CONNECTED) return;
    if (!debugClient.connected()) return;

    static char dbgBuf[300] PS_ATTR;
    va_list args;
    va_start(args, format);
    int len = vsnprintf(dbgBuf, sizeof(dbgBuf) - 3, format, args);  // leave room for CRLF
    va_end(args);

    if (len < 0) return;
    if ((size_t)len > sizeof(dbgBuf) - 3) len = sizeof(dbgBuf) - 3;

    dbgBuf[len++] = '\r';
    dbgBuf[len++] = '\n';

    debugClient.write((const uint8_t*)dbgBuf, len);
}

void wifi_setup() {
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); 
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println((" WiFi connected: " + WiFi.localIP().toString()).c_str());
    wifiDebugPrintln((" WiFi connected: " + WiFi.localIP().toString()).c_str());
    wifiDebugInit();
}
#endif
