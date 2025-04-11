// CUtils.cpp
// Centralized management for controllers

#include <Wire.h>
#include <vector>
#include "CUtils/CUtils.h"
#include "src/Globals.h"
#include "src/debugPrint.h"

#include <map>
#include <array>

// All Controller Management is here
#include "CUtils/GPIO.cpp"
#include "CUtils/WS2812.cpp"
#include "CUtils/TM1637.cpp"
#include "CUtils/GN1640.cpp"
#include "CUtils/PCA9555.cpp" 

// ********************************************************
// Panel and Global meta-command handler
//
//
//
// ********************************************************
void setPanelAllLEDs(const char* panelPrefix, bool state) {
    for (int i = 0; i < panelLEDsCount; i++) {
        if (strncmp(panelLEDs[i].label, panelPrefix, strlen(panelPrefix)) == 0 &&
            panelLEDs[i].deviceType != DEVICE_NONE) {
            setLED(panelLEDs[i].label, state, state ? 100 : 0);
        }
    }
}

void setAllPanelsLEDs(bool state) {
    PCA9555_allLEDsByAddress(0x22, state);
    PCA9555_allLEDsByAddress(0x5B, state);
    GPIO_setAllLEDs(state);
    TM1637_setPanelAllLEDs(RA_Device, state);
    TM1637_setPanelAllLEDs(LA_Device, state);
    GN1640_setAllLEDs(state);
    WS2812_setAllLEDs(state);
}

// *****************************************************
// Panel Overrides
// *****************************************************

std::map<uint8_t, String> discoveredDevices;
// Scans the I2C bus and detects connected panels, storing results in map
void scanConnectedPanels() {
  discoveredDevices.clear();

  delay(500); // Allow time for PCA to wake up

  for (uint8_t addr = 0x03; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      String deviceDesc = "PCA_Device_at_0x";
      if (addr < 0x10) deviceDesc += "0";
      deviceDesc += String(addr, HEX);
      discoveredDevices[addr] = deviceDesc;
    }
  }
}

// Prints a formatted list of discovered devices
void printDiscoveredPanels() {
  if (discoveredDevices.empty()) {
    debugPrintln("No I2C devices found.");
    return;
  }

  debugPrintln("\n=== Discovered I2C Devices ===");
  debugPrintln("Address    | Device Description");
  debugPrintln("-----------|------------------");

  for (auto const& device : discoveredDevices) {
    debugPrint("0x");
    if (device.first < 0x10) debugPrint("0");
    debugPrint(device.first, HEX);
    int spaces = 11 - 4; // "0xNN" is 4 chars
    for (int i = 0; i < spaces; i++) debugPrint(" ");
    debugPrintln(device.second);
  }
  debugPrintln("============================\n");
}

// *****************************************************
// Panel LED Detection for DEBUGING 
// *****************************************************

// Index mapping for displayed LED menu
std::vector<int> displayedIndexes(panelLEDsCount);
int displayedCount = 0;

// LED selection menu
void printLEDMenu() {
  displayedCount = 0;
  debugPrintln("\n--- LED Selection Menu ---\n");

  int columns = 3;
  int colWidth = 25;

  for (int i = 0; i < panelLEDsCount; i++) {
    debugPrint(displayedCount);
    debugPrint(": ");
    debugPrint(panelLEDs[i].label);

    int len = strlen(panelLEDs[i].label);
    for (int s = 0; s < colWidth - len; s++) debugPrint(" ");

    displayedIndexes[displayedCount++] = i;

    if ((i + 1) % columns == 0 || i == panelLEDsCount - 1)
      debugPrintln("");
  }
}

void handleLEDSelection() {
  while (true) {
    debugPrintln("Enter LED number to activate (or press Enter to exit):");

    while (!Serial.available());
    String input = Serial.readStringUntil('\n');

    if (input.length() == 0) break;  // Exit if Enter is pressed without input

    int userSelection = input.toInt();
    if (userSelection >= 0 && userSelection < displayedCount) {
      int actualIndex = displayedIndexes[userSelection];
      debugPrint("Activating LED: ");
      debugPrintln(panelLEDs[actualIndex].label);

      setLED(panelLEDs[actualIndex].label, true, 100);
      delay(5000);
      setLED(panelLEDs[actualIndex].label, false, 0);

      debugPrint("Deactivated LED: ");
      debugPrintln(panelLEDs[actualIndex].label);

      // Clear screen
      debugWrite(27);
      debugPrint("[2J");
      debugWrite(27);
      debugPrint("[H");

      printLEDMenu();
    } else {
      debugPrintln("Invalid selection or unsupported LED.");
    }
  }
}