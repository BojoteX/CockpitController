// Auto-generated DCS LED Definitions
#ifndef DCSMAPPINGS_H
#define DCSMAPPINGS_H

#include <Arduino.h>

struct DcsLed {
  const char* label;
  uint16_t address;
  uint16_t mask;
  uint8_t shift;
};

#define NUM_DCS_LEDS 40

const DcsLed dcsLeds[NUM_DCS_LEDS] = {
  {"LH_ADV_L_BAR_RED", 0x7408, 0x8000, 15},
  {"LH_ADV_L_BLEED", 0x7408, 0x0800, 11},
  {"LH_ADV_R_BLEED", 0x7408, 0x1000, 12},
  {"LH_ADV_SPD_BRK", 0x7408, 0x2000, 13},
  {"LH_ADV_STBY", 0x7408, 0x4000, 14},
  {"LS_LOCK", 0x7408, 0x0001, 0},
  {"LS_SHOOT", 0x7408, 0x0002, 1},
  {"LS_SHOOT_STROBE", 0x7408, 0x0004, 2},
  {"LH_ADV_ASPJ_OH", 0x740A, 0x0008, 3},
  {"LH_ADV_GO", 0x740A, 0x0010, 4},
  {"LH_ADV_L_BAR_GREEN", 0x740A, 0x0002, 1},
  {"LH_ADV_NO_GO", 0x740A, 0x0020, 5},
  {"LH_ADV_REC", 0x740A, 0x0001, 0},
  {"LH_ADV_XMIT", 0x740A, 0x0004, 2},
  {"RH_ADV_AAA", 0x740A, 0x0800, 11},
  {"RH_ADV_AI", 0x740A, 0x0400, 10},
  {"RH_ADV_CW", 0x740A, 0x1000, 12},
  {"RH_ADV_DISP", 0x740A, 0x0100, 8},
  {"RH_ADV_RCDR_ON", 0x740A, 0x0080, 7},
  {"RH_ADV_SAM", 0x740A, 0x0200, 9},
  {"RH_ADV_SPARE_RH1", 0x740A, 0x2000, 13},
  {"RH_ADV_SPARE_RH2", 0x740A, 0x4000, 14},
  {"RH_ADV_SPARE_RH3", 0x740A, 0x8000, 15},
  {"MASTER_MODE_AA_LT", 0x740C, 0x0200, 9},
  {"MASTER_MODE_AG_LT", 0x740C, 0x0400, 10},
  {"RH_ADV_SPARE_RH4", 0x740C, 0x0001, 0},
  {"RH_ADV_SPARE_RH5", 0x740C, 0x0002, 1},
  {"SPIN_LT", 0x742A, 0x0800, 11},
  {"CLIP_CK_SEAT_LT", 0x74A0, 0x8000, 15},
  {"CLIP_APU_ACC_LT", 0x74A4, 0x0100, 8},
  {"CLIP_BATT_SW_LT", 0x74A4, 0x0200, 9},
  {"CLIP_FCES_LT", 0x74A4, 0x4000, 14},
  {"CLIP_FCS_HOT_LT", 0x74A4, 0x0400, 10},
  {"CLIP_FUEL_LO_LT", 0x74A4, 0x2000, 13},
  {"CLIP_GEN_TIE_LT", 0x74A4, 0x0800, 11},
  {"CLIP_SPARE_CTN1_LT", 0x74A4, 0x1000, 12},
  {"CLIP_SPARE_CTN2_LT", 0x74A4, 0x8000, 15},
  {"CLIP_L_GEN_LT", 0x74A8, 0x0100, 8},
  {"CLIP_R_GEN_LT", 0x74A8, 0x0200, 9},
  {"CLIP_SPARE_CTN3_LT", 0x74A8, 0x0400, 10},
};

void handleLedCallback0(unsigned int newValue);
void handleLedCallback1(unsigned int newValue);
void handleLedCallback2(unsigned int newValue);
void handleLedCallback3(unsigned int newValue);
void handleLedCallback4(unsigned int newValue);
void handleLedCallback5(unsigned int newValue);
void handleLedCallback6(unsigned int newValue);
void handleLedCallback7(unsigned int newValue);
void handleLedCallback8(unsigned int newValue);
void handleLedCallback9(unsigned int newValue);
void handleLedCallback10(unsigned int newValue);
void handleLedCallback11(unsigned int newValue);
void handleLedCallback12(unsigned int newValue);
void handleLedCallback13(unsigned int newValue);
void handleLedCallback14(unsigned int newValue);
void handleLedCallback15(unsigned int newValue);
void handleLedCallback16(unsigned int newValue);
void handleLedCallback17(unsigned int newValue);
void handleLedCallback18(unsigned int newValue);
void handleLedCallback19(unsigned int newValue);
void handleLedCallback20(unsigned int newValue);
void handleLedCallback21(unsigned int newValue);
void handleLedCallback22(unsigned int newValue);
void handleLedCallback23(unsigned int newValue);
void handleLedCallback24(unsigned int newValue);
void handleLedCallback25(unsigned int newValue);
void handleLedCallback26(unsigned int newValue);
void handleLedCallback27(unsigned int newValue);
void handleLedCallback28(unsigned int newValue);
void handleLedCallback29(unsigned int newValue);
void handleLedCallback30(unsigned int newValue);
void handleLedCallback31(unsigned int newValue);
void handleLedCallback32(unsigned int newValue);
void handleLedCallback33(unsigned int newValue);
void handleLedCallback34(unsigned int newValue);
void handleLedCallback35(unsigned int newValue);
void handleLedCallback36(unsigned int newValue);
void handleLedCallback37(unsigned int newValue);
void handleLedCallback38(unsigned int newValue);
void handleLedCallback39(unsigned int newValue);

typedef void (*CallbackPtr)(unsigned int);
extern const CallbackPtr ledCallbacks[NUM_DCS_LEDS];

#endif
