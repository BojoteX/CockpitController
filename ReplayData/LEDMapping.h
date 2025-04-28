// THIS FILE IS AUTO-GENERATED; DO NOT EDIT MANUALLY
#pragma once

// Embedded LEDMapping structure and enums
enum LEDDeviceType {
  DEVICE_PCA9555,
  DEVICE_TM1637,
  DEVICE_NONE,
  DEVICE_GPIO,
  DEVICE_WS2812,
  DEVICE_GN1640T,
};

struct LEDMapping {
  const char* label;
  LEDDeviceType deviceType;
  union {
    struct { uint8_t gpio; } gpioInfo;
    struct { uint8_t address; uint8_t port; uint8_t bit; } pcaInfo;
    struct { uint8_t clkPin; uint8_t dioPin; uint8_t segment; uint8_t bit; } tm1637Info;
    struct { uint8_t address; uint8_t column; uint8_t row; } gn1640Info;
    struct { uint8_t index; } ws2812Info;
  } info;
  bool dimmable;
  bool activeLow;
};

// Auto-generated panelLEDs array
static const LEDMapping panelLEDs[] = {
  { "FIRE_APU_LT"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_APU_ACC_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_BATT_SW_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_CK_SEAT_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_FCES_LT"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_FCS_HOT_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_FUEL_LO_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_GEN_TIE_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_L_GEN_LT"      , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_R_GEN_LT"      , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_SPARE_CTN1_LT" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_SPARE_CTN2_LT" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CLIP_SPARE_CTN3_LT" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CMSD_JET_SEL_L"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CHART_DIMMER"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "CONSOLES_DIMMER"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "FLOOD_DIMMER"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "INST_PNL_DIMMER"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "WARN_CAUTION_DIMMER", DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_ASPJ_OH"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_GO"          , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_L_BAR_GREEN" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_L_BAR_RED"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_L_BLEED"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_NO_GO"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_REC"         , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_R_BLEED"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_SPD_BRK"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_STBY"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LH_ADV_XMIT"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "FIRE_LEFT_LT"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LS_LOCK"            , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LS_SHOOT"           , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "LS_SHOOT_STROBE"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "HMD_OFF_BRT"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "SPIN_LT"            , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "MASTER_MODE_AA_LT"  , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "MASTER_MODE_AG_LT"  , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "MC_DISCH"           , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "MC_READY"           , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "MASTER_CAUTION_LT"  , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_AAA"         , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_AI"          , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_CW"          , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_DISP"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_RCDR_ON"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_SAM"         , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_SPARE_RH1"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_SPARE_RH2"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_SPARE_RH3"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_SPARE_RH4"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "RH_ADV_SPARE_RH5"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false },
  { "FIRE_RIGHT_LT"      , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }
};

static constexpr uint16_t panelLEDsCount = sizeof(panelLEDs)/sizeof(panelLEDs[0]);

// Auto-generated hash table
struct LEDHashEntry { const char* label; const LEDMapping* led; };
static const LEDHashEntry ledHashTable[107] = {
  {"CLIP_SPARE_CTN2_LT", &panelLEDs[11]},
  {"WARN_CAUTION_DIMMER", &panelLEDs[18]},
  {"LS_SHOOT", &panelLEDs[32]},
  {"RH_ADV_CW", &panelLEDs[43]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_R_GEN_LT", &panelLEDs[9]},
  {"LH_ADV_REC", &panelLEDs[25]},
  {"CONSOLES_DIMMER", &panelLEDs[15]},
  {"SPIN_LT", &panelLEDs[35]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_SPARE_CTN1_LT", &panelLEDs[10]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LS_LOCK", &panelLEDs[31]},
  {"LH_ADV_L_BAR_GREEN", &panelLEDs[21]},
  {nullptr, nullptr},
  {"LH_ADV_SPD_BRK", &panelLEDs[27]},
  {"RH_ADV_DISP", &panelLEDs[44]},
  {"LH_ADV_STBY", &panelLEDs[28]},
  {nullptr, nullptr},
  {"MASTER_MODE_AA_LT", &panelLEDs[36]},
  {"LS_SHOOT_STROBE", &panelLEDs[33]},
  {"RH_ADV_AI", &panelLEDs[42]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"RH_ADV_SPARE_RH1", &panelLEDs[47]},
  {"RH_ADV_SPARE_RH2", &panelLEDs[48]},
  {"RH_ADV_SPARE_RH3", &panelLEDs[49]},
  {"RH_ADV_SPARE_RH4", &panelLEDs[50]},
  {"RH_ADV_SPARE_RH5", &panelLEDs[51]},
  {"FIRE_RIGHT_LT", &panelLEDs[52]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_XMIT", &panelLEDs[29]},
  {"MASTER_MODE_AG_LT", &panelLEDs[37]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_GO", &panelLEDs[20]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"FIRE_LEFT_LT", &panelLEDs[30]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_FCS_HOT_LT", &panelLEDs[5]},
  {"INST_PNL_DIMMER", &panelLEDs[17]},
  {"CLIP_FCES_LT", &panelLEDs[4]},
  {"CLIP_FUEL_LO_LT", &panelLEDs[6]},
  {"LH_ADV_R_BLEED", &panelLEDs[26]},
  {"HMD_OFF_BRT", &panelLEDs[34]},
  {"MASTER_CAUTION_LT", &panelLEDs[40]},
  {"RH_ADV_AAA", &panelLEDs[41]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"FLOOD_DIMMER", &panelLEDs[16]},
  {"RH_ADV_RCDR_ON", &panelLEDs[45]},
  {nullptr, nullptr},
  {"CLIP_L_GEN_LT", &panelLEDs[8]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_L_BAR_RED", &panelLEDs[22]},
  {"CLIP_GEN_TIE_LT", &panelLEDs[7]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CMSD_JET_SEL_L", &panelLEDs[13]},
  {"CLIP_BATT_SW_LT", &panelLEDs[2]},
  {nullptr, nullptr},
  {"CLIP_SPARE_CTN3_LT", &panelLEDs[12]},
  {"CHART_DIMMER", &panelLEDs[14]},
  {"RH_ADV_SAM", &panelLEDs[46]},
  {nullptr, nullptr},
  {"LH_ADV_L_BLEED", &panelLEDs[23]},
  {"MC_DISCH", &panelLEDs[38]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_APU_ACC_LT", &panelLEDs[1]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_NO_GO", &panelLEDs[24]},
  {"MC_READY", &panelLEDs[39]},
  {"LH_ADV_ASPJ_OH", &panelLEDs[19]},
  {"FIRE_APU_LT", &panelLEDs[0]},
  {"CLIP_CK_SEAT_LT", &panelLEDs[3]},
};

// djb2 hash function
constexpr uint16_t ledHash(const char* str) {
  uint16_t hash = 5381;
  while (*str) { hash = ((hash << 5) + hash) + *str++; }
  return hash % 107;
}

// findLED lookup
inline const LEDMapping* findLED(const char* label) {
  uint16_t h = ledHash(label);
  for (int i = 0; i < 107; ++i) {
    const auto& entry = ledHashTable[(h + i) % 107];
    if (!entry.label) return nullptr;
    if (strcmp(entry.label, label) == 0) return entry.led;
  }
  return nullptr;
}
