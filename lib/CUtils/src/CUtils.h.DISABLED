#pragma once

// CUtils.h
// Centralized management for controllers

#ifndef CUTILS_H
#define CUTILS_H

#include <Arduino.h>
#include "../../../src/Mappings.h"

#ifdef DEFINE_MAPPINGS

// Actual definitions here (ONLY ONCE)
TM1637Device RA_Device;
TM1637Device LA_Device;

#else

// Extern declarations of global annunciator devices
// extern TM1637Device RA_Device;
// extern TM1637Device LA_Device;

#endif

// Pines físicos compartidos entre dispositivos
#define GLOBAL_CLK_PIN 37
#define CA_DIO_PIN 36
#define LA_DIO_PIN 39
#define LA_CLK_PIN GLOBAL_CLK_PIN
#define RA_DIO_PIN 40
#define RA_CLK_PIN GLOBAL_CLK_PIN
#define LOCKSHOOT_DIO_PIN 35

// GPIO helpers
void GPIO_setAllLEDs(bool state);

// WS2812
#include <FastLED.h>
#define NUM_LEDS 3
extern CRGB leds[NUM_LEDS];
void WS2812_init();
void WS2812_setLEDColor(uint8_t ledIndex, CRGB color);
void WS2812_clearAll();
void WS2812_allOn(CRGB color);
void WS2812_allOff();
void WS2812_sweep(const CRGB* colors, uint8_t count);
void WS2812_sweep();
void WS2812_testPattern();
void WS2812_setAllLEDs(bool state);

// TM1637
void TM1637_setAllLEDs(bool state);
void TM1637_setPanelAllLEDs(TM1637Device &dev, bool state);
void tm1637_init(TM1637Device &device, uint8_t clkPin, uint8_t dioPin);
void tm1637_displaySingleLED(TM1637Device &device, uint8_t grid, uint8_t segment, bool state);
void tm1637_clearDisplay(TM1637Device &device);
void tm1637_start(TM1637Device &dev);
void tm1637_stop(TM1637Device &dev);
bool tm1637_writeByte(TM1637Device &dev, uint8_t b);
uint8_t tm1637_readKeys(TM1637Device &dev);
void tm1637_allOn();
void tm1637_allOff();
void tm1637_sweepPanel();
void tm1637_testPattern();
void tm1637_allOn(TM1637Device& dev);
void tm1637_allOff(TM1637Device& dev);
void tm1637_sweep(TM1637Device& dev, const char* deviceName);
void tm1637_testPattern(TM1637Device& dev, const char* deviceName);

// GN1640
void GN1640_init(uint8_t clkPin, uint8_t dioPin);
void GN1640_setLED(uint8_t column, uint8_t row, bool state);
void GN1640_clearAll();
void GN1640_allOff();
void GN1640_allOn();
void GN1640_sweepPanel();
void GN1640_sweep();
void GN1640_testPattern();
void GN1640_command(uint8_t cmd);
void GN1640_write(uint8_t column, uint8_t value);
void GN1640_setAllLEDs(bool state);

// PCA Utils
void PCA9555_setAllLEDs(bool state);
void PCA9555_allLEDsByAddress(uint8_t addr, bool state);
void PCA9555_allOn(uint8_t addr);
void PCA9555_allOff(uint8_t addr);
void PCA9555_sweep(uint8_t addr);
void PCA9555_patternTesting(uint8_t addr);
static uint8_t PCA9555_cachedPortStates[256][2] = {0}; // Cache PCA9555 output port states, initialized to 0x00 (LEDs off initially)
void PCA9555_autoInitFromLEDMap(uint8_t address);
void PCA9555_initAsOutput(uint8_t address, uint8_t port0_output_mask, uint8_t port1_output_mask);
bool isPCA9555LoggingEnabled(); // Used for PCA Logging
void enablePCA9555Logging(bool enable);
bool isPCA9555LoggingEnabled();
bool readPCA9555(uint8_t address, byte &port0, byte &port1);
void initPCA9555AsInput(uint8_t address);
void logPCA9555State(uint8_t address, byte port0, byte port1);
bool shouldLogChange(uint8_t address, byte port0, byte port1);
void PCA9555_write(uint8_t addr, uint8_t port, uint8_t bit, bool state); // PCAWrite (LEDs)

// Meta commands per panel
void setPanelAllLEDs(const char* panelPrefix, bool state);

// Global meta-command
void setAllPanelsLEDs(bool state);

// Panel Detection
void detectAllPanels(bool &hasLockShoot, bool &hasCA, bool &hasLA, bool &hasRA, bool &hasIR, bool &hasECM, bool &hasMasterARM);
void scanConnectedPanels();
void printDiscoveredPanels();
void handleLEDSelection();
void printLEDMenu();

#endif // CUTILS_H