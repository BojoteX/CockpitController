// LEDControl.h
// Centralized LED management header for TEKCreations F/A-18C Cockpit Firmware

#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <Arduino.h>

// ADD THE STRUCT HERE:
struct TM1637Device {
    uint8_t clkPin;
    uint8_t dioPin;
    uint8_t ledData[6];
};

// ===== Initialize LED routine =====
void initializeLEDs(const char* activePanels[], unsigned int panelCount);

// ===== Enumerations =====
enum LEDDeviceType {
  DEVICE_GPIO,
  DEVICE_PCA9555,
  DEVICE_TM1637,
  DEVICE_GN1640T,
  DEVICE_WS2812,
  DEVICE_NONE
};

// ===== Structures =====
struct LEDMapping {
  const char* label;
  LEDDeviceType deviceType;
  union {
    struct { uint8_t gpio; } gpioInfo;
    struct { uint8_t address; uint8_t port; uint8_t bit; } pcaInfo;
    struct { uint8_t clkPin; uint8_t dioPin; uint8_t segment; uint8_t bit; } tm1637Info; // TM1637: Uses segments
    struct { uint8_t address; uint8_t column; uint8_t row; } gn1640Info;                 // GN1640T: Uses column-row
    struct { uint8_t index; } ws2812Info;
  } info;
  bool dimmable;
  bool activeLow;
};

// ===== Public LED Control API =====
void setLED(const char* label, bool state, uint8_t intensity = 100);

// ===== External LED Map Declaration =====
extern LEDMapping panelLEDs[];
extern const uint16_t panelLEDsCount;

#endif // LEDCONTROL_H