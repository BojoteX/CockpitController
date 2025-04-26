// THIS FILE IS AUTO-GENERATED; do not edit by hand
#pragma once

#include "src/LEDControl.h"

// Auto-generated panelLEDs array
static const LEDMapping panelLEDs[] = {
  {"FIRE_APU_LT", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"CLIP_APU_ACC_LT", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"RH_ADV_SPARE_RH2", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"RH_ADV_SPARE_RH3", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"RH_ADV_SPARE_RH4", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"RH_ADV_SPARE_RH5", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"FIRE_RIGHT_LT", DEVICE_NONE, {.gpioInfo = {0}}, false}
};

static constexpr uint16_t panelLEDsCount = sizeof(panelLEDs)/sizeof(panelLEDs[0]);

// Auto-generated hash table
struct LEDHashEntry { const char* label; const LEDMapping* led; };
static const LEDHashEntry ledHashTable[53] = {
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"FIRE_APU_LT", &panelLEDs[0]},
  {"RH_ADV_AI", &panelLEDs[38]},
  {"RH_ADV_DISP", &panelLEDs[40]},
  {"CLIP_FCES_LT", &panelLEDs[4]},
  {"CLIP_SPARE_CTN1_LT", &panelLEDs[11]},
  {"LH_ADV_ASPJ_OH", &panelLEDs[15]},
  {"LH_ADV_L_BLEED", &panelLEDs[19]},
  {"CLIP_SPARE_CTN2_LT", &panelLEDs[12]},
  {"LH_ADV_REC", &panelLEDs[21]},
  {"LH_ADV_R_BLEED", &panelLEDs[22]},
  {"CLIP_SPARE_CTN3_LT", &panelLEDs[13]},
  {"LH_ADV_SPD_BRK", &panelLEDs[23]},
  {"CAGADA", &panelLEDs[10]},
  {"LS_LOCK", &panelLEDs[27]},
  {"LS_SHOOT_STROBE", &panelLEDs[29]},
  {"MASTER_MODE_AA_LT", &panelLEDs[32]},
  {"MC_DISCH", &panelLEDs[34]},
  {"LH_ADV_GO", &panelLEDs[16]},
  {"LH_ADV_XMIT", &panelLEDs[25]},
  {"CLIP_APU_ACC_LT", &panelLEDs[1]},
  {"CLIP_FUEL_LO_LT", &panelLEDs[6]},
  {"MC_READY", &panelLEDs[35]},
  {"LH_ADV_L_BAR_GREEN", &panelLEDs[17]},
  {"RH_ADV_AAA", &panelLEDs[37]},
  {"CLIP_R_GEN_LT", &panelLEDs[9]},
  {"LH_ADV_L_BAR_RED", &panelLEDs[18]},
  {"MASTER_MODE_AG_LT", &panelLEDs[33]},
  {"RH_ADV_RCDR_ON", &panelLEDs[41]},
  {"CLIP_GEN_TIE_LT", &panelLEDs[7]},
  {"CLIP_CK_SEAT_LT", &panelLEDs[3]},
  {"CLIP_FCS_HOT_LT", &panelLEDs[5]},
  {"FIRE_LEFT_LT", &panelLEDs[26]},
  {"LH_ADV_STBY", &panelLEDs[24]},
  {"RH_ADV_CW", &panelLEDs[39]},
  {"RH_ADV_SAM", &panelLEDs[42]},
  {"HMD_OFF_BRT", &panelLEDs[30]},
  {"MASTER_CAUTION_LT", &panelLEDs[36]},
  {"RH_ADV_SPARE_RH1", &panelLEDs[43]},
  {"RH_ADV_SPARE_RH2", &panelLEDs[44]},
  {"RH_ADV_SPARE_RH3", &panelLEDs[45]},
  {"RH_ADV_SPARE_RH4", &panelLEDs[46]},
  {"RH_ADV_SPARE_RH5", &panelLEDs[47]},
  {"CLIP_BATT_SW_LT", &panelLEDs[2]},
  {"FIRE_RIGHT_LT", &panelLEDs[48]},
  {"CLIP_L_GEN_LT", &panelLEDs[8]},
  {"CMSD_JET_SEL_L", &panelLEDs[14]},
  {"LH_ADV_NO_GO", &panelLEDs[20]},
  {"LS_SHOOT", &panelLEDs[28]},
  {"SPIN_LT", &panelLEDs[31]}
};

// djb2-based constexpr hash
constexpr uint16_t ledHash(const char* str) {
    uint16_t h = 5381;
    while (*str) { h = ((h << 5) + h) + *str++; }
    return h % 53;
}

// findLED via open addressing
inline const LEDMapping* findLED(const char* label) {
    uint16_t h = ledHash(label);
    for (int i = 0; i < 53; ++i) {
        const auto& e = ledHashTable[(h + i) % 53];
        if (!e.label) return nullptr;
        if (strcmp(e.label, label) == 0) return e.led;
    }
    return nullptr;
}
