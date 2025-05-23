// THIS FILE IS AUTO-GENERATED; ONLY EDIT INDIVIDUAL RECORDS, DO NOT ADD OR DELETE THEM HERE
#pragma once

struct InputMapping {
    const char* label;        // Unique selector label
    const char* source;       // Hardware source identifier
    uint8_t     port;         // Port index
    uint8_t     bit;          // Bit position
    int8_t      hidId;        // HID usage ID
    const char* oride_label;  // Override command label (dcsCommand)
    uint16_t    oride_value;  // Override command value (value)
    const char* controlType;  // Control type, e.g., "selector"
    uint16_t    group;        // Group ID for exclusive selectors
};

//  label                       source     port bit hidId  DCSCommand           value   Type        group
static const InputMapping InputMappings[] = {
    { "APU_FIRE_BTN"               , "PCA_0x00" ,  0 ,  0 ,  12 , "APU_FIRE_BTN"           ,     1 , "momentary",  0 },
    { "AUX_REL_SW_ENABLE"          , "PCA_0x22" ,  0 ,  1 ,  11 , "AUX_REL_SW"             ,     1 , "selector" ,  1 },
    { "AUX_REL_SW_NORM"            , "PCA_0x22" ,  0 ,  1 ,  10 , "AUX_REL_SW"             ,     0 , "selector" ,  1 },
    { "CMSD_DISPENSE_SW_BYPASS"    , "PCA_0x22" ,  0 ,  2 ,   9 , "CMSD_DISPENSE_SW"       ,     2 , "selector" ,  2 },
    { "CMSD_DISPENSE_SW_ON"        , "PCA_0x22" ,  0 ,  2 ,   8 , "CMSD_DISPENSE_SW"       ,     1 , "selector" ,  2 },
    { "CMSD_DISPENSE_SW_OFF"       , "PCA_0x22" ,  0 ,  3 ,   7 , "CMSD_DISPENSE_SW"       ,     0 , "selector" ,  2 },
    { "CMSD_JET_SEL_BTN"           , "PCA_0x22" ,  0 ,  0 ,   1 , "CMSD_JET_SEL_BTN"       ,     1 , "momentary",  0 },
    { "ECM_MODE_SW_XMIT"           , "PCA_0x22" ,  0 ,  4 ,   6 , "ECM_MODE_SW"            ,     4 , "selector" ,  3 },
    { "ECM_MODE_SW_REC"            , "PCA_0x22" ,  0 ,  5 ,   5 , "ECM_MODE_SW"            ,     3 , "selector" ,  3 },
    { "ECM_MODE_SW_BIT"            , "PCA_0x22" ,  0 ,  6 ,   4 , "ECM_MODE_SW"            ,     2 , "selector" ,  3 },
    { "ECM_MODE_SW_STBY"           , "PCA_0x22" ,  0 ,  7 ,   3 , "ECM_MODE_SW"            ,     1 , "selector" ,  3 },
    { "ECM_MODE_SW_OFF"            , "PCA_0x22" ,  1 ,  0 ,   2 , "ECM_MODE_SW"            ,     0 , "selector" ,  3 },
    { "FIRE_EXT_BTN"               , "PCA_0x00" ,  0 ,  0 ,  16 , "FIRE_EXT_BTN"           ,     1 , "momentary",  0 },
    { "CHART_DIMMER"               , "PCA_0x00" ,  0 ,  0 ,  -1 , "CHART_DIMMER"           , 0xFFFF , "analog"   ,  0 },
    { "COCKKPIT_LIGHT_MODE_SW_NVG" , "PCA_0x00" ,  0 ,  0 ,  -1 , "COCKKPIT_LIGHT_MODE_SW" ,     2 , "selector" ,  4 },
    { "COCKKPIT_LIGHT_MODE_SW_NITE", "PCA_0x00" ,  0 ,  0 ,  -1 , "COCKKPIT_LIGHT_MODE_SW" ,     1 , "selector" ,  4 },
    { "COCKKPIT_LIGHT_MODE_SW_DAY" , "PCA_0x00" ,  0 ,  0 ,  -1 , "COCKKPIT_LIGHT_MODE_SW" ,     0 , "selector" ,  4 },
    { "CONSOLES_DIMMER"            , "PCA_0x00" ,  0 ,  0 ,  -1 , "CONSOLES_DIMMER"        , 0xFFFF , "analog"   ,  0 },
    { "FLOOD_DIMMER"               , "PCA_0x00" ,  0 ,  0 ,  -1 , "FLOOD_DIMMER"           , 0xFFFF , "analog"   ,  0 },
    { "INST_PNL_DIMMER"            , "PCA_0x00" ,  0 ,  0 ,  -1 , "INST_PNL_DIMMER"        , 0xFFFF , "analog"   ,  0 },
    { "LIGHTS_TEST_SW_TEST"        , "PCA_0x00" ,  0 ,  0 ,  -1 , "LIGHTS_TEST_SW"         ,     1 , "selector" ,  5 },
    { "LIGHTS_TEST_SW_OFF"         , "PCA_0x00" ,  0 ,  0 ,  -1 , "LIGHTS_TEST_SW"         ,     0 , "selector" ,  5 },
    { "WARN_CAUTION_DIMMER"        , "PCA_0x00" ,  0 ,  0 ,  -1 , "WARN_CAUTION_DIMMER"    , 0xFFFF , "analog"   ,  0 },
    { "LEFT_FIRE_BTN"              , "PCA_0x00" ,  0 ,  0 ,  15 , "LEFT_FIRE_BTN"          ,     1 , "momentary",  0 },
    { "LEFT_FIRE_BTN_COVER"        , "PCA_0x00" ,  0 ,  0 ,  -1 , "LEFT_FIRE_BTN_COVER"    ,     1 , "momentary",  0 },
    { "HMD_OFF_BRT"                , "PCA_0x00" ,  0 ,  0 ,  -1 , "HMD_OFF_BRT"            , 0xFFFF , "analog"   ,  0 },
    { "IR_COOL_SW_ORIDE"           , "PCA_0x26" ,  0 ,  0 ,  25 , "IR_COOL_SW"             ,     2 , "selector" ,  6 },
    { "IR_COOL_SW_NORM"            , "PCA_0x26" ,  0 ,  0 ,  24 , "IR_COOL_SW"             ,     1 , "selector" ,  6 },
    { "IR_COOL_SW_OFF"             , "PCA_0x26" ,  0 ,  0 ,  23 , "IR_COOL_SW"             ,     0 , "selector" ,  6 },
    { "SPIN_RECOVERY_COVER"        , "PCA_0x26" ,  0 ,  0 ,  -1 , "SPIN_RECOVERY_COVER"    ,     1 , "momentary",  0 },
    { "SPIN_RECOVERY_SW_RCVY"      , "PCA_0x00" ,  0 ,  0 ,  21 , "SPIN_RECOVERY_SW"       ,     1 , "selector" ,  7 },
    { "SPIN_RECOVERY_SW_NORM"      , "PCA_0x26" ,  0 ,  0 ,  22 , "SPIN_RECOVERY_SW"       ,     0 , "selector" ,  7 },
    { "MASTER_ARM_SW_ARM"          , "PCA_0x5B" ,  0 ,  0 ,  17 , "MASTER_ARM_SW"          ,     1 , "selector" ,  8 },
    { "MASTER_ARM_SW_SAFE"         , "PCA_0x5B" ,  0 ,  0 ,  18 , "MASTER_ARM_SW"          ,     0 , "selector" ,  8 },
    { "MASTER_MODE_AA"             , "PCA_0x5B" ,  0 ,  0 ,  19 , "MASTER_MODE_AA"         ,     1 , "momentary",  0 },
    { "MASTER_MODE_AG"             , "PCA_0x5B" ,  0 ,  0 ,  20 , "MASTER_MODE_AG"         ,     1 , "momentary",  0 },
    { "MASTER_CAUTION_RESET_SW"    , "PCA_0x00" ,  0 ,  0 ,  14 , "MASTER_CAUTION_RESET_SW",     1 , "momentary",  0 },
    { "RIGHT_FIRE_BTN"             , "PCA_0x00" ,  0 ,  0 ,  13 , "RIGHT_FIRE_BTN"         ,     1 , "momentary",  0 },
    { "RIGHT_FIRE_BTN_COVER"       , "PCA_0x00" ,  0 ,  0 ,  -1 , "RIGHT_FIRE_BTN_COVER"   ,     1 , "momentary",  0 },
};
static const size_t InputMappingSize = sizeof(InputMappings)/sizeof(InputMappings[0]);


// Static hash lookup table for InputMappings[]
struct InputHashEntry { const char* label; const InputMapping* mapping; };
static const InputHashEntry inputHashTable[79] = {
  {"HMD_OFF_BRT", &InputMappings[25]},
  {nullptr, nullptr},
  {"MASTER_ARM_SW_SAFE", &InputMappings[33]},
  {"LEFT_FIRE_BTN", &InputMappings[23]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"COCKKPIT_LIGHT_MODE_SW_NVG", &InputMappings[14]},
  {nullptr, nullptr},
  {"ECM_MODE_SW_BIT", &InputMappings[9]},
  {"FLOOD_DIMMER", &InputMappings[18]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"SPIN_RECOVERY_SW_RCVY", &InputMappings[30]},
  {"SPIN_RECOVERY_SW_NORM", &InputMappings[31]},
  {"FIRE_EXT_BTN", &InputMappings[12]},
  {"CMSD_DISPENSE_SW_ON", &InputMappings[4]},
  {"WARN_CAUTION_DIMMER", &InputMappings[22]},
  {"CHART_DIMMER", &InputMappings[13]},
  {"COCKKPIT_LIGHT_MODE_SW_NITE", &InputMappings[15]},
  {"AUX_REL_SW_NORM", &InputMappings[2]},
  {"IR_COOL_SW_ORIDE", &InputMappings[26]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"AUX_REL_SW_ENABLE", &InputMappings[1]},
  {"APU_FIRE_BTN", &InputMappings[0]},
  {"ECM_MODE_SW_XMIT", &InputMappings[7]},
  {"INST_PNL_DIMMER", &InputMappings[19]},
  {"LIGHTS_TEST_SW_TEST", &InputMappings[20]},
  {"LEFT_FIRE_BTN_COVER", &InputMappings[24]},
  {"RIGHT_FIRE_BTN", &InputMappings[37]},
  {"CONSOLES_DIMMER", &InputMappings[17]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"ECM_MODE_SW_STBY", &InputMappings[10]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"IR_COOL_SW_OFF", &InputMappings[28]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CMSD_DISPENSE_SW_BYPASS", &InputMappings[3]},
  {"SPIN_RECOVERY_COVER", &InputMappings[29]},
  {"MASTER_MODE_AA", &InputMappings[34]},
  {"MASTER_CAUTION_RESET_SW", &InputMappings[36]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"COCKKPIT_LIGHT_MODE_SW_DAY", &InputMappings[16]},
  {"MASTER_MODE_AG", &InputMappings[35]},
  {"RIGHT_FIRE_BTN_COVER", &InputMappings[38]},
  {nullptr, nullptr},
  {"IR_COOL_SW_NORM", &InputMappings[27]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"ECM_MODE_SW_REC", &InputMappings[8]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"LIGHTS_TEST_SW_OFF", &InputMappings[21]},
  {"CMSD_DISPENSE_SW_OFF", &InputMappings[5]},
  {"MASTER_ARM_SW_ARM", &InputMappings[32]},
  {"ECM_MODE_SW_OFF", &InputMappings[11]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CMSD_JET_SEL_BTN", &InputMappings[6]},
};

// Shared recursive hash implementation
constexpr uint16_t labelHash(const char* s) {
  return *s ? static_cast<uint16_t>(*s) + 31 * labelHash(s + 1) : 0;
}

// Preserve existing signature
constexpr uint16_t inputHash(const char* s) { return labelHash(s); }

inline const InputMapping* findInputByLabel(const char* label) {
  uint16_t startH = inputHash(label) % 79;
  for (uint16_t i = 0; i < 79; ++i) {
    uint16_t idx = (startH + i >= 79) ? (startH + i - 79) : (startH + i);
    const auto& entry = inputHashTable[idx];
    if (!entry.label) continue;
    if (strcmp(entry.label, label) == 0) return entry.mapping;
  }
  return nullptr;
}
