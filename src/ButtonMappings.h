#ifndef BUTTON_MAPPINGS_H
#define BUTTON_MAPPINGS_H

// Panel BitMappings
struct MappedBitCombo {
  uint8_t i2cAddress;
  uint8_t port;
  uint8_t bit;
  const char* name;
};

// Tabla de bits mapeados por panel
static const MappedBitCombo knownCombos[] = {
  // 🔹 ECM panel (0x22)
  {0x22, 0, 0, "JETT_SEL"},
  {0x22, 0, 1, "AUX_REL"},
  {0x22, 0, 2, "DISPENSER_BYPASS"},
  {0x22, 0, 3, "DISPENSER_OFF"},
  {0x22, 0, 4, "EMC_SELECTOR_OFF"},
  {0x22, 0, 5, "EMC_SELECTOR_STBY"},
  {0x22, 0, 6, "EMC_SELECTOR_BIT"},
  {0x22, 0, 7, "EMC_SELECTOR_REC"},
  {0x22, 1, 0, "EMC_SELECTOR_XMIT"},

  // 🔹 IR COOL panel (0x26)
  {0x26, 1, 0, "IR_COOL_ORIDE"},
  {0x26, 1, 1, "IR_COOL_OFF"},
  {0x26, 1, 2, "SPIN_RCVY"},

  // 🔹 Master Arm panel (0x5B)
  {0x5B, 0, 0, "MASTER_ARM_AG"},
  {0x5B, 0, 1, "MASTER_ARM_AA"},
  {0x5B, 0, 2, "MASTER_ARM_DISCH"},
  {0x5B, 0, 3, "MASTER_ARM_SWITCH"}
};

// Acceso desde fuera
static const MappedBitCombo* mappedCombos = knownCombos;
static const size_t mappedComboCount = sizeof(knownCombos) / sizeof(knownCombos[0]);

// 🔍 Nombre de panel por dirección I2C
static const std::map<uint8_t, const char*> panelNameLookup = {
  {0x22, "ECM"},
  {0x26, "IRCOOL"},
  {0x5B, "MASTER_ARM"}
};

// Función para obtener nombre de panel (seguro)
inline const char* getPanelName(uint8_t i2cAddr) {
  auto it = panelNameLookup.find(i2cAddr);
  return (it != panelNameLookup.end()) ? it->second : "UNKNOWN_PANEL";
}


// Panel Mappings
// -------------------------------------
// HID Button ID Assignments
// -------------------------------------

static const std::map<String, uint8_t> buttonMap = {
  // 🔹 ECM Panel
  { "JETT_SEL",              1 },
  { "EMC_SELECTOR_OFF",      2 },
  { "EMC_SELECTOR_STBY",     3 },
  { "EMC_SELECTOR_BIT",      4 },
  { "EMC_SELECTOR_REC",      5 },
  { "EMC_SELECTOR_XMIT",     6 },
  { "DISPENSER_ON",          7 },
  { "DISPENSER_BYPASS",      8 },
  { "DISPENSER_OFF",         9 },
  { "AUX_REL_ENABLE",       10 },
  { "AUX_REL_NORM",         11 },

  // 🔹 IR COOL Panel
  { "SPIN_RCVY",            12 },
  { "SPIN_NORM",            13 },
  { "IR_COOL_OFF",          14 },
  { "IR_COOL_NORM",         15 },
  { "IR_COOL_ORIDE",        16 },

  // 🔹 Master Arm Panel → TODO: Define mappings
  { "MASTER_ARM_AG",        17 },
  { "MASTER_ARM_AA",        18 },
  { "MASTER_ARM_DISCH",     19 },
  { "MASTER_ARM_OFF",       20 },
  { "MASTER_ARM_ON",        21 },

    // 🔹 Left/Right Annunciator buttons
  { "LEFT_ANNUN_ENG_FIRE",                22 },
  { "LEFT_ANNUN_MASTER_CAUTION",          23 },
  { "RIGHT_ANNUN_APU_FIRE",               24 },
  { "RIGHT_ANNUN_ENG_FIRE",               25 },
};

// -------------------------------------
// Exclusive Button Groups (bitmasks)
// -------------------------------------

static const std::map<uint8_t, uint32_t> exclusiveButtonGroups = {
  // 🔹 ECM Selector (IDs 2–6 → bits 1–5)
  {2, (1UL << 1) | (1UL << 2) | (1UL << 3) | (1UL << 4) | (1UL << 5)},
  {3, (1UL << 1) | (1UL << 2) | (1UL << 3) | (1UL << 4) | (1UL << 5)},
  {4, (1UL << 1) | (1UL << 2) | (1UL << 3) | (1UL << 4) | (1UL << 5)},
  {5, (1UL << 1) | (1UL << 2) | (1UL << 3) | (1UL << 4) | (1UL << 5)},
  {6, (1UL << 1) | (1UL << 2) | (1UL << 3) | (1UL << 4) | (1UL << 5)},

  // 🔹 DISPENSER (IDs 7–9 → bits 6–8)
  {7, (1UL << 6) | (1UL << 7) | (1UL << 8)},
  {8, (1UL << 6) | (1UL << 7) | (1UL << 8)},
  {9, (1UL << 6) | (1UL << 7) | (1UL << 8)},

  // 🔹 AUX_REL (IDs 10–11 → bits 9–10)
  {10, (1UL << 9) | (1UL << 10)},
  {11, (1UL << 9) | (1UL << 10)},

  // 🔹 SPIN (IDs 12–13 → bits 11–12)
  {12, (1UL << 11) | (1UL << 12)},
  {13, (1UL << 11) | (1UL << 12)},

  // 🔹 IR COOL (IDs 14–16 → bits 13–15)
  {14, (1UL << 13) | (1UL << 14) | (1UL << 15)},
  {15, (1UL << 13) | (1UL << 14) | (1UL << 15)},
  {16, (1UL << 13) | (1UL << 14) | (1UL << 15)},

  // 🔹 Master ARM Panel
  {20, (1UL << 19) | (1UL << 20)},
  {21, (1UL << 19) | (1UL << 20)},

};

#endif