// CUtils.h
#pragma once
#ifndef CUTILS_H
#define CUTILS_H

#include <Arduino.h>
#include <FastLED.h>
#include <Wire.h>
#include "../../../Config.h"
#include "../../../src/LABELS/InputMapping.h"
#include "../../../src/LEDControl.h"

// —— panel discovery types ——
#define MAX_DEVICES 10
struct I2CDeviceInfo { uint8_t address; const char* label; };
extern I2CDeviceInfo  discoveredDevices[MAX_DEVICES];
extern uint8_t        discoveredDeviceCount;

// direct-index lookup table size
static constexpr uint8_t I2C_ADDR_SPACE = 0x80;
extern const char* panelNameByAddr[I2C_ADDR_SPACE];

// panel enum + unified table
enum class PanelID : uint8_t { ECM = 0x22, BRAIN = 0x26, ARM = 0x5B, UNKNOWN = 0x00 };
PanelID     getPanelID(uint8_t address);
const char* panelIDToString(PanelID id);

// scan & query
void        scanConnectedPanels();
void        printDiscoveredPanels();
bool        panelExists(uint8_t targetAddr);
inline const char* getPanelName(uint8_t addr) {
  const char* n = (addr < I2C_ADDR_SPACE) ? panelNameByAddr[addr] : nullptr;
  return n ? n : panelIDToString(getPanelID(addr));
}

// —— GPIO (discrete LEDs) —— 
void GPIO_setAllLEDs(bool state);  

// —— WS2812 (FastLED) —— 
#define NUM_LEDS 3
extern CRGB    leds[NUM_LEDS];
extern uint8_t brightness;
void WS2812_init();
void WS2812_setLEDColor(uint8_t idx, CRGB color);
void WS2812_clearAll();
void WS2812_allOn(CRGB color);
void WS2812_allOff();
void WS2812_sweep(const CRGB* colors, uint8_t count);
void WS2812_testPattern();
void WS2812_setAllLEDs(bool state);  

// —— TM1637 (4-digit displays + buttons) —— 
struct TM1637Device { uint8_t clkPin, dioPin, ledData[6]; };
extern TM1637Device RA_Device;
extern TM1637Device LA_Device;
bool   tm1637_handleSamplingWindow(TM1637Device&, uint16_t& sampleCounter, uint8_t& finalKeys);
void   tm1637_addSample(TM1637Device&, uint8_t raw);
uint8_t tm1637_evaluateResult(TM1637Device&);
void   tm1637_resetSampling(TM1637Device&);
uint8_t tm1637_readKeys(TM1637Device&);
void   tm1637_startBothDevices();
void   tm1637_start(TM1637Device&);
void   tm1637_stop(TM1637Device&);
bool   tm1637_writeByte(TM1637Device&, uint8_t b);
void   tm1637_updateDisplay(TM1637Device&);
void   tm1637_init(TM1637Device&, uint8_t clk, uint8_t dio);
void   tm1637_displaySingleLED(TM1637Device&, uint8_t grid, uint8_t seg, bool on);
void   tm1637_clearDisplay(TM1637Device&);
void   tm1637_allOn(TM1637Device&);
void   tm1637_allOff(TM1637Device&);
void   tm1637_sweep(TM1637Device&, const char* name);
void   tm1637_sweepPanel();
void   tm1637_testPattern();
void   TM1637_setPanelAllLEDs(TM1637Device& dev, bool state);
void   TM1637_setAllLEDs(bool state);  

// —— GN1640 (matrix LED driver) —— 
void GN1640_startCondition();
void GN1640_stopCondition();
void GN1640_sendByte(uint8_t data);
void GN1640_command(uint8_t cmd);
void GN1640_init(uint8_t clkPin, uint8_t dioPin);
void GN1640_setLED(uint8_t row, uint8_t col, bool state);
void GN1640_write(uint8_t column, uint8_t value);
void GN1640_clearAll();
void GN1640_allOff();
void GN1640_allOn();
void GN1640_sweepPanel();
void GN1640_testPattern();
void GN1640_setAllLEDs(bool state);
bool GN1640_detect(uint8_t clkPin, uint8_t dioPin);  

// —— PCA9555 (I²C expander) —— 
extern uint8_t PCA9555_cachedPortStates[8][2];  // address 0x20–0x27 → [port0,port1]
void PCA9555_setAllLEDs(bool state);
void PCA9555_allLEDsByAddress(uint8_t addr, bool state);
void PCA9555_allOn(uint8_t addr);
void PCA9555_allOff(uint8_t addr);
void PCA9555_sweep(uint8_t addr);
void PCA9555_patternTesting(uint8_t addr);
void PCA9555_autoInitFromLEDMap(uint8_t addr);
void PCA9555_initAsOutput(uint8_t addr, uint8_t mask0, uint8_t mask1);
void initPCA9555AsInput(uint8_t addr);
bool readPCA9555(uint8_t addr, byte &p0, byte &p1);
void PCA9555_write(uint8_t addr, uint8_t port, uint8_t bit, bool state);
bool isPCA9555LoggingEnabled();
void enablePCA9555Logging(bool);
void logExpanderState(uint8_t p0, uint8_t p1);
void logPCA9555State(uint8_t addr, byte p0, byte p1);
// (All other helpers live in PCA9555.cpp) 

// —— Meta & Debug helpers —— 
void setPanelAllLEDs(const char* panelPrefix, bool state);
void setAllPanelsLEDs(bool state);
void detectAllPanels(bool &hasLockShoot, bool &hasCA, bool &hasLA,
                     bool &hasRA, bool &hasIR, bool &hasECM, bool &hasMasterARM);
void printLEDMenu();
void handleLEDSelection();
void runReplayWithPrompt();

#endif // CUTILS_H
