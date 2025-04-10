// ------------------ OUTPUT MAPPING STRUCT ------------------
typedef struct {
  uint16_t address;
  uint16_t mask;
  uint8_t shift;
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
  {29804, 256,   8,  "LA_MASTER_CAUTION"},
  {29708, 16,    4,  "LA_ENG_FIRE"},
  {29708, 4,     2,  "RA_APU_FIRE"},
  {29708, 32,    5,  "RA_ENG_FIRE"},

  {29908, 32768, 15, "ECM_JETT_SEL"},

  {29804, 512,   9,  "ARM_READY"},
  {29804, 1024, 10,  "ARM_DISCH"},
  {29804, 2048, 11,  "ARM_AA"},
  {29804, 4096, 12,  "ARM_AG"},

  {29704, 64,    6,  "IR_SPN_RCVY"},

  {29710, 256,   8,  "LOCKSHOOT_LOCK"},
  {29710, 512,   9,  "LOCKSHOOT_SHOOT"},
  {29710, 1024, 10,  "LOCKSHOOT_BLANK"}
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