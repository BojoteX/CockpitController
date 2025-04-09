// ------------------ OUTPUT MAPPING STRUCT ------------------
typedef struct {
  uint16_t address;
  uint16_t mask;
  const char* ledLabel;
} DcsOutputMapping;

// ------------------ INPUT MAPPING STRUCT WITH VALUE SUPPORT ------------------
typedef struct {
  const char* command;     // DCS-BIOS command
  const char* inputLabel;  // Your firmware label
  const char* onValue;     // Value to send when pressed/on
  const char* offValue;    // Value to send when released/off
} DcsCommand;

// ------------------ OUTPUT MAPPINGS (DCS => LED) ------------------
static const DcsOutputMapping DCSOutputs[] = {
  {1, 0xFFFF, "LOCK"},
  {2, 0xFFFF, "SHOOT"},
  {3, 0xFFFF, "SHOOT_STROBE"},
  {10, 0xFFFF, "LA_ENG_FIRE"},
  {13, 0xFFFF, "LA_MASTER_CAUTION"},
  {15, 0xFFFF, "LA_GO"},
  {16, 0xFFFF, "LA_NO_GO"},
  {17, 0xFFFF, "LA_L_BLEED"},
  {18, 0xFFFF, "LA_R_BLEED"},
  {19, 0xFFFF, "LA_SPD_BRK"},
  {20, 0xFFFF, "LA_STBY"},
  {21, 0xFFFF, "LA_L_BAR_RED"},
  {22, 0xFFFF, "LA_REC"},
  {23, 0xFFFF, "LA_L_BAR_GREEN"},
  {24, 0xFFFF, "LA_XMT"},
  {25, 0xFFFF, "CA_DASH_1"},
  {31, 0xFFFF, "RA_RCDR_ON"},
  {32, 0xFFFF, "RA_DISP"},
  {38, 0xFFFF, "RA_SAM"},
  {39, 0xFFFF, "RA_AI"},
  {40, 0xFFFF, "RA_AAA"},
  {41, 0xFFFF, "RA_CW"},
  {44, 0xFFFF, "ARM_READY"},
  {45, 0xFFFF, "ARM_DISCH"},
  {47, 0xFFFF, "ARM_AA"},
  {48, 0xFFFF, "ARM_AG"},
  {376, 0xFFFF, "RA_APU_FIRE"},
  {26, 0xFFFF, "RA_ENG_FIRE"},
  {304, 0xFFFF, "CA_FUEL_LO"},
  {305, 0xFFFF, "CA_FCES"},
  {307, 0xFFFF, "CA_L_GEN"},
  {308, 0xFFFF, "CA_R_GEN"},
  {300, 0xFFFF, "CA_BATT_SW"},
  {301, 0xFFFF, "CA_FCS_HOT"},
  {302, 0xFFFF, "CA_GEN_TIE"},
  {299, 0xFFFF, "CA_APU_ACC"},
  {298, 0xFFFF, "CA_CK_SEAT"},
  {303, 0xFFFF, "CA_DASH_2"},
  {309, 0xFFFF, "CA_DASH_3"},
};
static const size_t numDCSOutputs = sizeof(DCSOutputs) / sizeof(DcsOutputMapping);

// ------------------ INPUT MAPPINGS (COCKPIT INPUT => DCS COMMAND) ------------------
static const DcsCommand DCSCommands[] = {
  { "CMSD_DISPENSE_SW", "DISPENSER_ON",     "1", "0" },
  { "CMSD_DISPENSE_SW", "DISPENSER_BYPASS", "0", "1" },
  { "CMSD_DISPENSE_SW", "DISPENSER_OFF",    "2", "1" },

  { "CMSD_JET_SEL_BTN", "JETT_SEL",         "1", "0" },
  { "MODE_SELECTOR_SW", "EMC_SELECTOR_XMIT","0", "1" },
  { "MODE_SELECTOR_SW", "EMC_SELECTOR_REC", "1", "0" },
  { "MODE_SELECTOR_SW", "EMC_SELECTOR_STBY","2", "1" },
  { "MODE_SELECTOR_SW", "EMC_SELECTOR_OFF", "3", "1" },
  { "MODE_SELECTOR_SW", "EMC_SELECTOR_BIT", "4", "1" },

  { "AUX_REL_SW",       "AUX_REL_ENABLE",   "1", "0" },
  { "AUX_REL_SW",       "AUX_REL_NORM",     "0", "1" },

  { "IR_COOL_SW",       "IR_COOL_ORIDE",    "0", "1" },
  { "IR_COOL_SW",       "IR_COOL_NORM",     "1", "0" },
  { "IR_COOL_SW",       "IR_COOL_OFF",      "2", "1" },

  { "SPIN_RECOVERY_SW", "SPIN_RCVY",        "1", "0" },
  { "SPIN_RECOVERY_SW", "SPIN_NORM",        "0", "1" },

  { "MASTER_MODE_AG",   "MASTER_ARM_AG",    "1", "0" },
  { "MASTER_MODE_AA",   "MASTER_ARM_AA",    "1", "0" },
  { "FIRE_EXT_BTN",     "MASTER_ARM_DISCH", "1", "0" },
  { "MASTER_ARM_SW",    "MASTER_ARM_ON",    "1", "0" },
  { "MASTER_ARM_SW",    "MASTER_ARM_OFF",   "0", "1" },

  { "LEFT_FIRE_BTN",               "LEFT_ANNUN_ENG_FIRE",     "1", "0" },
  { "MASTER_CAUTION_RESET_SW",    "LEFT_ANNUN_MASTER_CAUTION", "1", "0" },
  { "APU_FIRE_BTN",               "RIGHT_ANNUN_APU_FIRE",    "1", "0" },
  { "RIGHT_FIRE_BTN",             "RIGHT_ANNUN_ENG_FIRE",    "1", "0" },
};
static const size_t numDCSInputs = sizeof(DCSCommands) / sizeof(DcsCommand);