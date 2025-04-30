// THIS FILE IS AUTO-GENERATED; ONLY EDIT INDIVIDUAL LED/GAUGE RECORDS, DO NOT ADD OR DELETE THEM HERE
#pragma once

// Embedded LEDMapping structure and enums
enum LEDDeviceType {
  DEVICE_NONE,
  DEVICE_WS2812,
  DEVICE_GPIO,
  DEVICE_PCA9555,
  DEVICE_TM1637,
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
  { "FIRE_APU_LT"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_APU_ACC_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_BATT_SW_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_CK_SEAT_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_FCES_LT"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_FCS_HOT_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_FUEL_LO_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_GEN_TIE_LT"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_L_GEN_LT"      , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_R_GEN_LT"      , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_SPARE_CTN1_LT" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_SPARE_CTN2_LT" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CLIP_SPARE_CTN3_LT" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "PRESSURE_ALT"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CMSD_JET_SEL_L"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CHART_DIMMER"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "CONSOLES_DIMMER"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "FLOOD_DIMMER"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "INST_PNL_DIMMER"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "WARN_CAUTION_DIMMER", DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_ASPJ_OH"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_GO"          , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_L_BAR_GREEN" , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_L_BAR_RED"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_L_BLEED"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_NO_GO"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_REC"         , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_R_BLEED"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_SPD_BRK"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_STBY"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LH_ADV_XMIT"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "FIRE_LEFT_LT"       , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LS_LOCK"            , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LS_SHOOT"           , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "LS_SHOOT_STROBE"    , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "HMD_OFF_BRT"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "SPIN_LT"            , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "MASTER_MODE_AA_LT"  , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "MASTER_MODE_AG_LT"  , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "MC_DISCH"           , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "MC_READY"           , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "MASTER_CAUTION_LT"  , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_AAA"         , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_AI"          , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_CW"          , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_DISP"        , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_RCDR_ON"     , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_SAM"         , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_SPARE_RH1"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_SPARE_RH2"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_SPARE_RH3"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_SPARE_RH4"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "RH_ADV_SPARE_RH5"   , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info,
  { "FIRE_RIGHT_LT"      , DEVICE_NONE    , {.gpioInfo = {0}}, false, false }, // No Info
};

static constexpr uint16_t panelLEDsCount = sizeof(panelLEDs)/sizeof(panelLEDs[0]);

// Auto-generated hash table
struct LEDHashEntry { const char* label; const LEDMapping* led; };
static const LEDHashEntry ledHashTable[109] = {
  {"RH_ADV_SAM", &panelLEDs[47]},
  {nullptr, nullptr},
  {"PRESSURE_ALT", &panelLEDs[13]},
  {"RH_ADV_DISP", &panelLEDs[45]},
  {nullptr, nullptr},
  {"CMSD_JET_SEL_L", &panelLEDs[14]},
  {"LH_ADV_R_BLEED", &panelLEDs[27]},
  {"RH_ADV_AAA", &panelLEDs[42]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_L_BLEED", &panelLEDs[24]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_ASPJ_OH", &panelLEDs[20]},
  {nullptr, nullptr},
  {"MASTER_MODE_AA_LT", &panelLEDs[37]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_SPARE_CTN3_LT", &panelLEDs[12]},
  {nullptr, nullptr},
  {"LH_ADV_L_BAR_RED", &panelLEDs[23]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"INST_PNL_DIMMER", &panelLEDs[18]},
  {"MASTER_CAUTION_LT", &panelLEDs[41]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_CK_SEAT_LT", &panelLEDs[3]},
  {"LS_SHOOT", &panelLEDs[33]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_REC", &panelLEDs[26]},
  {"SPIN_LT", &panelLEDs[36]},
  {"CLIP_BATT_SW_LT", &panelLEDs[2]},
  {"MASTER_MODE_AG_LT", &panelLEDs[38]},
  {nullptr, nullptr},
  {"LH_ADV_L_BAR_GREEN", &panelLEDs[22]},
  {"LH_ADV_STBY", &panelLEDs[29]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CHART_DIMMER", &panelLEDs[15]},
  {"LS_LOCK", &panelLEDs[32]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"RH_ADV_RCDR_ON", &panelLEDs[46]},
  {nullptr, nullptr},
  {"CLIP_SPARE_CTN2_LT", &panelLEDs[11]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_FCES_LT", &panelLEDs[4]},
  {"FIRE_LEFT_LT", &panelLEDs[31]},
  {nullptr, nullptr},
  {"FLOOD_DIMMER", &panelLEDs[17]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CONSOLES_DIMMER", &panelLEDs[16]},
  {"CLIP_APU_ACC_LT", &panelLEDs[1]},
  {"LH_ADV_NO_GO", &panelLEDs[25]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_GEN_TIE_LT", &panelLEDs[7]},
  {"WARN_CAUTION_DIMMER", &panelLEDs[19]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_FCS_HOT_LT", &panelLEDs[5]},
  {"CLIP_L_GEN_LT", &panelLEDs[8]},
  {"RH_ADV_CW", &panelLEDs[44]},
  {"CLIP_FUEL_LO_LT", &panelLEDs[6]},
  {"LS_SHOOT_STROBE", &panelLEDs[34]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"HMD_OFF_BRT", &panelLEDs[35]},
  {"LH_ADV_GO", &panelLEDs[21]},
  {"RH_ADV_SPARE_RH1", &panelLEDs[48]},
  {"RH_ADV_SPARE_RH2", &panelLEDs[49]},
  {"RH_ADV_SPARE_RH3", &panelLEDs[50]},
  {"CLIP_SPARE_CTN1_LT", &panelLEDs[10]},
  {"LH_ADV_SPD_BRK", &panelLEDs[28]},
  {"RH_ADV_SPARE_RH4", &panelLEDs[51]},
  {"RH_ADV_SPARE_RH5", &panelLEDs[52]},
  {"FIRE_RIGHT_LT", &panelLEDs[53]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CLIP_R_GEN_LT", &panelLEDs[9]},
  {"MC_READY", &panelLEDs[40]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"MC_DISCH", &panelLEDs[39]},
  {nullptr, nullptr},
  {"FIRE_APU_LT", &panelLEDs[0]},
  {"RH_ADV_AI", &panelLEDs[43]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LH_ADV_XMIT", &panelLEDs[30]},
};

// djb2 hash function
constexpr uint16_t ledHash(const char* str) {
  uint16_t hash = 5381;
  while (*str) { hash = ((hash << 5) + hash) + *str++; }
  return hash;
}

// findLED lookup
inline const LEDMapping* findLED(const char* label) {
  uint16_t startH = ledHash(label) % 109;
  for (uint16_t i = 0; i < 109; ++i) {
    uint16_t idx = (startH + i >= 109) ? (startH + i - 109) : (startH + i);
    const auto& entry = ledHashTable[idx];
    if (!entry.label) continue;
    if (strcmp(entry.label, label) == 0) return entry.led;
  }
  return nullptr;
}
