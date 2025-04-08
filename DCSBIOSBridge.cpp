#include "src/DCSBIOSBridge.h"

// Simple buffer for incoming serial lines
String dcsLine = "";

void processDCSLine(const String& line);

// Map signal → pin
struct DCSMapping {
  const char* signal;
  uint8_t pin;
};

// Expand freely
DCSMapping dcsMap[] = {
  {"LOCK_LIGHT", 10},
  {"SHOOT_LIGHT", 11},
  {"MASTER_ARM", 12}
};

const size_t numMappings = sizeof(dcsMap) / sizeof(dcsMap[0]);

void DCSBIOSBridge_init() {
  Serial.println("DCS-BIOS Bridge Initialized");
  for (size_t i = 0; i < numMappings; ++i) {
    pinMode(dcsMap[i].pin, OUTPUT);
    digitalWrite(dcsMap[i].pin, LOW);  // initial state is OFF
  }
}

void DCSBIOSBridge_loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (dcsLine.length() > 0) {
        processDCSLine(dcsLine);
        dcsLine = "";
      }
    } else {
      dcsLine += c;
    }
  }
}

void processDCSLine(const String& line) {
  int sep = line.indexOf('=');
  if (sep == -1) return;

  String key = line.substring(0, sep);
  key.trim();
  String valueStr = line.substring(sep + 1);
  valueStr.trim();
  int value = valueStr.toInt();

  for (size_t i = 0; i < numMappings; ++i) {
    if (key.equalsIgnoreCase(dcsMap[i].signal)) {
      digitalWrite(dcsMap[i].pin, value ? HIGH : LOW);
      Serial.print("💡 "); Serial.print(dcsMap[i].signal);
      Serial.print(" → Pin "); Serial.print(dcsMap[i].pin);
      Serial.print(" = "); Serial.println(value);
      break;
    }
  }
}