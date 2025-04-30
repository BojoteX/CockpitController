// THIS FILE IS AUTO-GENERATED; ONLY EDIT INDIVIDUAL RECORDS, DO NOT ADD OR DELETE THEM HERE
#pragma once

struct InputMapping {
    const char* label;        // Unique selector label
    const char* source;       // Hardware source identifier
    uint8_t     port;         // Port index
    uint8_t     bit;          // Bit position
    int8_t      hidId;        // HID usage ID
    const char* oride_label;  // Override command label (dcsCommand)
    int16_t     oride_value;  // Override command value (value)
    const char* controlType;  // Control type, e.g., "selector"
    uint16_t    group;        // Group ID for exclusive selectors
};

//  label                       source     port bit hidId  DCSCommand           value   Type        group
static const InputMapping InputMappings[] = {
    { "APU_FIRE_BTN"               , "PCA_0x00" ,  0 ,  0 ,  -1 , "APU_FIRE_BTN"           ,   1 , "momentary",  0 },
    { "AUX_REL_SW_ENABLE"          , "PCA_0x00" ,  0 ,  0 ,  -1 , "AUX_REL_SW"             ,   1 , "selector" ,  1 },
    { "AUX_REL_SW_NORM"            , "PCA_0x00" ,  0 ,  0 ,  -1 , "AUX_REL_SW"             ,   0 , "selector" ,  1 },
    { "CMSD_DISPENSE_SW_BYPASS"    , "PCA_0x00" ,  0 ,  0 ,  -1 , "CMSD_DISPENSE_SW"       ,   2 , "selector" ,  2 },
    { "CMSD_DISPENSE_SW_ON"        , "PCA_0x00" ,  0 ,  0 ,  -1 , "CMSD_DISPENSE_SW"       ,   1 , "selector" ,  2 },
    { "CMSD_DISPENSE_SW_OFF"       , "PCA_0x00" ,  0 ,  0 ,  -1 , "CMSD_DISPENSE_SW"       ,   0 , "selector" ,  2 },
    { "CMSD_JET_SEL_BTN"           , "PCA_0x00" ,  0 ,  0 ,  -1 , "CMSD_JET_SEL_BTN"       ,   1 , "momentary",  0 },
    { "ECM_MODE_SW_XMIT"           , "PCA_0x00" ,  0 ,  0 ,  -1 , "ECM_MODE_SW"            ,   4 , "selector" ,  3 },
    { "ECM_MODE_SW_REC"            , "PCA_0x00" ,  0 ,  0 ,  -1 , "ECM_MODE_SW"            ,   3 , "selector" ,  3 },
    { "ECM_MODE_SW_BIT"            , "PCA_0x00" ,  0 ,  0 ,  -1 , "ECM_MODE_SW"            ,   2 , "selector" ,  3 },
    { "ECM_MODE_SW_STBY"           , "PCA_0x00" ,  0 ,  0 ,  -1 , "ECM_MODE_SW"            ,   1 , "selector" ,  3 },
    { "ECM_MODE_SW_OFF"            , "PCA_0x00" ,  0 ,  0 ,  -1 , "ECM_MODE_SW"            ,   0 , "selector" ,  3 },
    { "FIRE_EXT_BTN"               , "PCA_0x00" ,  0 ,  0 ,  -1 , "FIRE_EXT_BTN"           ,   1 , "momentary",  0 },
    { "COCKKPIT_LIGHT_MODE_SW_NVG" , "PCA_0x00" ,  0 ,  0 ,  -1 , "COCKKPIT_LIGHT_MODE_SW" ,   2 , "selector" ,  4 },
    { "COCKKPIT_LIGHT_MODE_SW_NITE", "PCA_0x00" ,  0 ,  0 ,  -1 , "COCKKPIT_LIGHT_MODE_SW" ,   1 , "selector" ,  4 },
    { "COCKKPIT_LIGHT_MODE_SW_DAY" , "PCA_0x00" ,  0 ,  0 ,  -1 , "COCKKPIT_LIGHT_MODE_SW" ,   0 , "selector" ,  4 },
    { "LIGHTS_TEST_SW_TEST"        , "PCA_0x00" ,  0 ,  0 ,  -1 , "LIGHTS_TEST_SW"         ,   1 , "selector" ,  5 },
    { "LIGHTS_TEST_SW_OFF"         , "PCA_0x00" ,  0 ,  0 ,  -1 , "LIGHTS_TEST_SW"         ,   0 , "selector" ,  5 },
    { "LEFT_FIRE_BTN"              , "PCA_0x00" ,  0 ,  0 ,  -1 , "LEFT_FIRE_BTN"          ,   1 , "momentary",  0 },
    { "LEFT_FIRE_BTN_COVER"        , "PCA_0x00" ,  0 ,  0 ,  -1 , "LEFT_FIRE_BTN_COVER"    ,   1 , "momentary",  0 },
    { "IR_COOL_SW_ORIDE"           , "PCA_0x00" ,  0 ,  0 ,  -1 , "IR_COOL_SW"             ,   2 , "selector" ,  6 },
    { "IR_COOL_SW_NORM"            , "PCA_0x00" ,  0 ,  0 ,  -1 , "IR_COOL_SW"             ,   1 , "selector" ,  6 },
    { "IR_COOL_SW_OFF"             , "PCA_0x00" ,  0 ,  0 ,  -1 , "IR_COOL_SW"             ,   0 , "selector" ,  6 },
    { "SPIN_RECOVERY_COVER"        , "PCA_0x00" ,  0 ,  0 ,  -1 , "SPIN_RECOVERY_COVER"    ,   1 , "momentary",  0 },
    { "SPIN_RECOVERY_SW_RCVY"      , "PCA_0x00" ,  0 ,  0 ,  -1 , "SPIN_RECOVERY_SW"       ,   1 , "selector" ,  7 },
    { "SPIN_RECOVERY_SW_NORM"      , "PCA_0x00" ,  0 ,  0 ,  -1 , "SPIN_RECOVERY_SW"       ,   0 , "selector" ,  7 },
    { "MASTER_ARM_SW_ARM"          , "PCA_0x00" ,  0 ,  0 ,  -1 , "MASTER_ARM_SW"          ,   1 , "selector" ,  8 },
    { "MASTER_ARM_SW_SAFE"         , "PCA_0x00" ,  0 ,  0 ,  -1 , "MASTER_ARM_SW"          ,   0 , "selector" ,  8 },
    { "MASTER_MODE_AA"             , "PCA_0x00" ,  0 ,  0 ,  -1 , "MASTER_MODE_AA"         ,   1 , "momentary",  0 },
    { "MASTER_MODE_AG"             , "PCA_0x00" ,  0 ,  0 ,  -1 , "MASTER_MODE_AG"         ,   1 , "momentary",  0 },
    { "MASTER_CAUTION_RESET_SW"    , "PCA_0x00" ,  0 ,  0 ,  -1 , "MASTER_CAUTION_RESET_SW",   1 , "momentary",  0 },
    { "RIGHT_FIRE_BTN"             , "PCA_0x00" ,  0 ,  0 ,  -1 , "RIGHT_FIRE_BTN"         ,   1 , "momentary",  0 },
    { "RIGHT_FIRE_BTN_COVER"       , "PCA_0x00" ,  0 ,  0 ,  -1 , "RIGHT_FIRE_BTN_COVER"   ,   1 , "momentary",  0 },
};
static const size_t InputMappingSize = sizeof(InputMappings)/sizeof(InputMappings[0]);


// Static hash lookup table for InputMappings[]
struct InputHashEntry { const char* label; const InputMapping* mapping; };
static const InputHashEntry inputHashTable[67] = {
  {nullptr, nullptr},
  {"COCKKPIT_LIGHT_MODE_SW_NVG", &InputMappings[13]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"IR_COOL_SW_OFF", &InputMappings[22]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"ECM_MODE_SW_STBY", &InputMappings[10]},
  {"AUX_REL_SW_NORM", &InputMappings[2]},
  {"CMSD_JET_SEL_BTN", &InputMappings[6]},
  {"LIGHTS_TEST_SW_TEST", &InputMappings[16]},
  {"LIGHTS_TEST_SW_OFF", &InputMappings[17]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"APU_FIRE_BTN", &InputMappings[0]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"CMSD_DISPENSE_SW_OFF", &InputMappings[5]},
  {"FIRE_EXT_BTN", &InputMappings[12]},
  {"MASTER_ARM_SW_ARM", &InputMappings[26]},
  {"COCKKPIT_LIGHT_MODE_SW_DAY", &InputMappings[15]},
  {"SPIN_RECOVERY_COVER", &InputMappings[23]},
  {"SPIN_RECOVERY_SW_RCVY", &InputMappings[24]},
  {"MASTER_CAUTION_RESET_SW", &InputMappings[30]},
  {"AUX_REL_SW_ENABLE", &InputMappings[1]},
  {nullptr, nullptr},
  {"CMSD_DISPENSE_SW_ON", &InputMappings[4]},
  {nullptr, nullptr},
  {"LEFT_FIRE_BTN_COVER", &InputMappings[19]},
  {"ECM_MODE_SW_OFF", &InputMappings[11]},
  {"MASTER_MODE_AA", &InputMappings[28]},
  {nullptr, nullptr},
  {"RIGHT_FIRE_BTN_COVER", &InputMappings[32]},
  {"ECM_MODE_SW_XMIT", &InputMappings[7]},
  {"MASTER_MODE_AG", &InputMappings[29]},
  {"COCKKPIT_LIGHT_MODE_SW_NITE", &InputMappings[14]},
  {"RIGHT_FIRE_BTN", &InputMappings[31]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"ECM_MODE_SW_REC", &InputMappings[8]},
  {"IR_COOL_SW_ORIDE", &InputMappings[20]},
  {"CMSD_DISPENSE_SW_BYPASS", &InputMappings[3]},
  {"LEFT_FIRE_BTN", &InputMappings[18]},
  {"SPIN_RECOVERY_SW_NORM", &InputMappings[25]},
  {"MASTER_ARM_SW_SAFE", &InputMappings[27]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {"ECM_MODE_SW_BIT", &InputMappings[9]},
  {"IR_COOL_SW_NORM", &InputMappings[21]},
  {nullptr, nullptr},
  {nullptr, nullptr},
  {nullptr, nullptr},
};

// DJB2 hash function for input labels
constexpr uint16_t inputHash(const char* str) {
  uint16_t hash = 5381;
  while (*str) { hash = ((hash << 5) + hash) + *str++; }
  return hash;
}

inline const InputMapping* findInputByLabel(const char* label) {
  uint16_t startH = inputHash(label) % 67;
  for (uint16_t i = 0; i < 67; ++i) {
    uint16_t idx = (startH + i >= 67) ? (startH + i - 67) : (startH + i);
    const auto& entry = inputHashTable[idx];
    if (!entry.label) continue;
    if (strcmp(entry.label, label) == 0) return entry.mapping;
  }
  return nullptr;
}
