#include "Config.h"
#include "WiFiDebug.h"

void setup() {
  Serial.begin(250000);
  delay(200);

  #if DEBUG_USE_WIFI
  wifi_setup();
  #endif
}

void loop() {
  sendDebug("[UPTIME]");  // ticks every 10s
  delay(100);
}
