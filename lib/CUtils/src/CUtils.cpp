// CUtils.cpp
#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>
#include <cstring>
#include <cstdio>
#include "CUtils.h"

// Choose a compile-time constant that’s safely ≥ panelLEDsCount.
// Here we use 128 as a reasonable upper bound:
static int displayedIndexes[128];
static int displayedCount = 0;

// —— globals —— 
I2CDeviceInfo   discoveredDevices[MAX_DEVICES];
uint8_t         discoveredDeviceCount = 0;
const char*     panelNameByAddr[I2C_ADDR_SPACE] = { nullptr };

// TM1637 device instances (must match externs)
TM1637Device RA_Device;
TM1637Device LA_Device;

// PCA9555 write/read cache
// indexed by (address - 0x20), port 0 or 1
uint8_t PCA9555_cachedPortStates[8][2] = {{0}};

// — include and compile each internal module ——
#include "internal/GPIO.cpp"    
#include "internal/WS2812.cpp"  
#include "internal/TM1637.cpp"  
#include "internal/GN1640.cpp"  
#include "internal/PCA9555.cpp" 

// —— your single‐source panel table —— 
struct PanelDef { uint8_t addr; PanelID id; const char* label; };
static constexpr PanelDef kPanels[] = {
  { 0x22, PanelID::ECM,   "ECM Panel"            },
  { 0x26, PanelID::BRAIN, "Brain / IRCool Panel" },
  { 0x5B, PanelID::ARM,   "Master Arm Panel"     },
};

// —— scan only known panels —— 
void scanConnectedPanels() {
  discoveredDeviceCount = 0;
  memset(panelNameByAddr, 0, sizeof(panelNameByAddr));

  for (auto &p : kPanels) {
    bool present = false;
    for (uint8_t attempt = 0; attempt < 3; ++attempt) {
      Wire.beginTransmission(p.addr);
      Wire.write((uint8_t)0x00);
      if (Wire.endTransmission() == 0 &&
          Wire.requestFrom(p.addr, (uint8_t)1) == 1) {
        Wire.read();
        present = true;
        break;
      }
      delay(5);
    }
    if (!present || discoveredDeviceCount >= MAX_DEVICES) continue;
    discoveredDevices[discoveredDeviceCount++] = { p.addr, p.label };
    panelNameByAddr[p.addr] = p.label;
  }
}

PanelID getPanelID(uint8_t address) {
  for (auto &p : kPanels)
    if (p.addr == address) return p.id;
  return PanelID::UNKNOWN;
}

const char* panelIDToString(PanelID id) {
  for (auto &p : kPanels)
    if (p.id == id) return p.label;
  return "Unknown Panel";
}

bool panelExists(uint8_t targetAddr) {
  for (uint8_t i = 0; i < discoveredDeviceCount; ++i)
    if (discoveredDevices[i].address == targetAddr) return true;
  return false;
}

void printDiscoveredPanels() {
  if (discoveredDeviceCount == 0) {
    debugPrintln("No I2C devices found.");
    return;
  }
  debugPrintln("\n🔎 === Discovered I2C Devices ===");
  debugPrintln("📋 Address    | Device Description");
  debugPrintln("──────────────|─────────────────────────────");
  for (uint8_t i = 0; i < discoveredDeviceCount; ++i) {
    char buf[64];
    snprintf(buf, sizeof(buf),
             "📡 0x%02X       | %s",
             discoveredDevices[i].address,
             discoveredDevices[i].label);
    debugPrintln(buf);
  }
  debugPrintln("────────────────────────────────────────────\n");
}

// —— LED Debug Menu  —— 
void printLEDMenu() {
  displayedCount = 0;
  constexpr int columns = 3;
  constexpr int colWidth = 25;
  constexpr int bufSize = 4096;
  char buffer[bufSize];
  int cursor = 0;

  cursor += snprintf(buffer + cursor, bufSize - cursor,
                     "\n--- LED Selection Menu ---\n\n");

  for (int i = 0; i < panelLEDsCount; ++i) {
    cursor += snprintf(buffer + cursor, bufSize - cursor,
                       "%d: %s", displayedCount, panelLEDs[i].label);
    int len = strlen(panelLEDs[i].label);
    for (int s = 0; s < colWidth - len && cursor < bufSize - 1; ++s) {
      buffer[cursor++] = ' ';
    }
    displayedIndexes[displayedCount++] = i;
    if ((i + 1) % columns == 0 || i == panelLEDsCount - 1) {
      if (cursor < bufSize - 1) buffer[cursor++] = '\n';
    }
  }

  buffer[cursor] = '\0';
  Serial.print(buffer);
  #if DEBUG_USE_WIFI
  wifiDebugPrintln("See serial console for LED test");
  #endif
}

void handleLEDSelection() {
  while (true) {
    Serial.println("Enter LED number to activate (or press Enter to exit):");
    while (!Serial.available()) yield();

    char inputBuf[128];
    size_t len = Serial.readBytesUntil('\n', inputBuf, sizeof(inputBuf) - 1);
    inputBuf[len] = '\0';

    if (len == 0) break;

    int sel = atoi(inputBuf);
    if (sel >= 0 && sel < displayedCount) {
      int idx = displayedIndexes[sel];
      Serial.printf("Activating LED: %s\n", panelLEDs[idx].label);
      setLED(panelLEDs[idx].label, true, 100);
      delay(5000);
      setLED(panelLEDs[idx].label, false, 0);
      Serial.printf("Deactivated LED: %s\n", panelLEDs[idx].label);
      Serial.printf("\033[2J\033[H");
      printLEDMenu();
    } else {
      Serial.println("Invalid selection or unsupported LED.");
    }
  }
}

// —— Replay SOCAT Stream for debugging without using serial —— 
void runReplayWithPrompt() {
  bool infinite = false;
  while (true) {
    DcsbiosProtocolReplay();
    if (infinite) continue;

    Serial.println("\n=== REPLAY FINISHED ===");
    Serial.println("1) One more iteration");
    Serial.println("2) Run infinitely");
    Serial.println("3) Quit to main program");
    Serial.print("Choose [1-3]: ");
    while (!Serial.available()) yield();
    char c = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(c);

    switch (c) {
      case '1': break;
      case '2': infinite = true; Serial.println(">>> infinite replay mode <<<"); break;
      case '3': Serial.println(">>> exiting replay <<<"); return;
      default:  Serial.println("Invalid choice; please enter 1, 2, or 3."); break;
    }
  }
}