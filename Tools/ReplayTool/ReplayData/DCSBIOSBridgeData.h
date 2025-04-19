// Auto-generated DCSBIOS Data Mapping Header using structured DcsOutputEntry
#pragma once

#include <stdint.h>

struct DcsOutputEntry {
    uint16_t addr;
    uint16_t mask;
    uint8_t shift;
    uint16_t max_value;
    const char* label;
};

static const DcsOutputEntry DcsOutputTable[] = {
    { 0x740C, 0x0004, 2, 1, "FIRE_APU_LT" },
    { 0x74A4, 0x0100, 8, 1, "CLIP_APU_ACC_LT" },
    { 0x74A4, 0x0200, 9, 1, "CLIP_BATT_SW_LT" },
    { 0x74A0, 0x8000, 15, 1, "CLIP_CK_SEAT_LT" },
    { 0x74A4, 0x4000, 14, 1, "CLIP_FCES_LT" },
    { 0x74A4, 0x0400, 10, 1, "CLIP_FCS_HOT_LT" },
    { 0x74A4, 0x2000, 13, 1, "CLIP_FUEL_LO_LT" },
    { 0x74A4, 0x0800, 11, 1, "CLIP_GEN_TIE_LT" },
    { 0x74A8, 0x0100, 8, 1, "CLIP_L_GEN_LT" },
    { 0x74A8, 0x0200, 9, 1, "CLIP_R_GEN_LT" },
    { 0x74A4, 0x1000, 12, 1, "CLIP_SPARE_CTN1_LT" },
    { 0x74A4, 0x8000, 15, 1, "CLIP_SPARE_CTN2_LT" },
    { 0x74A8, 0x0400, 10, 1, "CLIP_SPARE_CTN3_LT" },
    { 0x74D4, 0x8000, 15, 1, "CMSD_JET_SEL_L" },
    { 0x754A, 0xFFFF, 0, 65535, "CHART_DIMMER" },
    { 0x7544, 0xFFFF, 0, 65535, "CONSOLES_DIMMER" },
    { 0x7548, 0xFFFF, 0, 65535, "FLOOD_DIMMER" },
    { 0x7546, 0xFFFF, 0, 65535, "INST_PNL_DIMMER" },
    { 0x754C, 0xFFFF, 0, 65535, "WARN_CAUTION_DIMMER" },
    { 0x740A, 0x0008, 3, 1, "LH_ADV_ASPJ_OH" },
    { 0x740A, 0x0010, 4, 1, "LH_ADV_GO" },
    { 0x740A, 0x0002, 1, 1, "LH_ADV_L_BAR_GREEN" },
    { 0x7408, 0x8000, 15, 1, "LH_ADV_L_BAR_RED" },
    { 0x7408, 0x0800, 11, 1, "LH_ADV_L_BLEED" },
    { 0x740A, 0x0020, 5, 1, "LH_ADV_NO_GO" },
    { 0x740A, 0x0001, 0, 1, "LH_ADV_REC" },
    { 0x7408, 0x1000, 12, 1, "LH_ADV_R_BLEED" },
    { 0x7408, 0x2000, 13, 1, "LH_ADV_SPD_BRK" },
    { 0x7408, 0x4000, 14, 1, "LH_ADV_STBY" },
    { 0x740A, 0x0004, 2, 1, "LH_ADV_XMIT" },
    { 0x7408, 0x0040, 6, 1, "FIRE_LEFT_LT" },
    { 0x7408, 0x0001, 0, 1, "LS_LOCK" },
    { 0x7408, 0x0002, 1, 1, "LS_SHOOT" },
    { 0x7408, 0x0004, 2, 1, "LS_SHOOT_STROBE" },
    { 0x7456, 0xFFFF, 0, 65535, "HMD_OFF_BRT" },
    { 0x742A, 0x0800, 11, 1, "SPIN_LT" },
    { 0x740C, 0x0200, 9, 1, "MASTER_MODE_AA_LT" },
    { 0x740C, 0x0400, 10, 1, "MASTER_MODE_AG_LT" },
    { 0x740C, 0x4000, 14, 1, "MC_DISCH" },
    { 0x740C, 0x8000, 15, 1, "MC_READY" },
    { 0x7408, 0x0200, 9, 1, "MASTER_CAUTION_LT" },
    { 0x740A, 0x0800, 11, 1, "RH_ADV_AAA" },
    { 0x740A, 0x0400, 10, 1, "RH_ADV_AI" },
    { 0x740A, 0x1000, 12, 1, "RH_ADV_CW" },
    { 0x740A, 0x0100, 8, 1, "RH_ADV_DISP" },
    { 0x740A, 0x0080, 7, 1, "RH_ADV_RCDR_ON" },
    { 0x740A, 0x0200, 9, 1, "RH_ADV_SAM" },
    { 0x740A, 0x2000, 13, 1, "RH_ADV_SPARE_RH1" },
    { 0x740A, 0x4000, 14, 1, "RH_ADV_SPARE_RH2" },
    { 0x740A, 0x8000, 15, 1, "RH_ADV_SPARE_RH3" },
    { 0x740C, 0x0001, 0, 1, "RH_ADV_SPARE_RH4" },
    { 0x740C, 0x0002, 1, 1, "RH_ADV_SPARE_RH5" },
    { 0x740C, 0x0010, 4, 1, "FIRE_RIGHT_LT" },
};

static const size_t DcsOutputTableSize = sizeof(DcsOutputTable) / sizeof(DcsOutputTable[0]);

static const std::unordered_map<uint16_t, std::vector<const DcsOutputEntry*>> addressToEntries = {
    { 0x740C, { &DcsOutputTable[0], &DcsOutputTable[36], &DcsOutputTable[37], &DcsOutputTable[38], &DcsOutputTable[39], &DcsOutputTable[50], &DcsOutputTable[51], &DcsOutputTable[52] } },
    { 0x74A4, { &DcsOutputTable[1], &DcsOutputTable[2], &DcsOutputTable[4], &DcsOutputTable[5], &DcsOutputTable[6], &DcsOutputTable[7], &DcsOutputTable[10], &DcsOutputTable[11] } },
    { 0x74A0, { &DcsOutputTable[3] } },
    { 0x74A8, { &DcsOutputTable[8], &DcsOutputTable[9], &DcsOutputTable[12] } },
    { 0x74D4, { &DcsOutputTable[13] } },
    { 0x754A, { &DcsOutputTable[14] } },
    { 0x7544, { &DcsOutputTable[15] } },
    { 0x7548, { &DcsOutputTable[16] } },
    { 0x7546, { &DcsOutputTable[17] } },
    { 0x754C, { &DcsOutputTable[18] } },
    { 0x740A, { &DcsOutputTable[19], &DcsOutputTable[20], &DcsOutputTable[21], &DcsOutputTable[24], &DcsOutputTable[25], &DcsOutputTable[29], &DcsOutputTable[41], &DcsOutputTable[42], &DcsOutputTable[43], &DcsOutputTable[44], &DcsOutputTable[45], &DcsOutputTable[46], &DcsOutputTable[47], &DcsOutputTable[48], &DcsOutputTable[49] } },
    { 0x7408, { &DcsOutputTable[22], &DcsOutputTable[23], &DcsOutputTable[26], &DcsOutputTable[27], &DcsOutputTable[28], &DcsOutputTable[30], &DcsOutputTable[31], &DcsOutputTable[32], &DcsOutputTable[33], &DcsOutputTable[40] } },
    { 0x7456, { &DcsOutputTable[34] } },
    { 0x742A, { &DcsOutputTable[35] } },
};

struct DcsInputEntry {
    const char* label;
    uint16_t max_value;
    const char* description;
};

static const DcsInputEntry DcsInputTable[] = {
    { "APU_FIRE_BTN", 1, "APU Fire Warning/Extinguisher Light" },
    { "AUX_REL_SW", 1, "Auxiliary Release Switch" },
    { "CMSD_DISPENSE_SW", 2, "DISPENSER Switch" },
    { "CMSD_JET_SEL_BTN", 1, "ECM JETT JETT SEL Button - Push to jettison" },
    { "ECM_MODE_SW", 4, "ECM Mode Switch" },
    { "FIRE_EXT_BTN", 1, "Fire Extinguisher Pushbutton" },
    { "CHART_DIMMER", 65535, "CHART Light Dimmer" },
    { "COCKKPIT_LIGHT_MODE_SW", 2, "MODE Switch" },
    { "CONSOLES_DIMMER", 65535, "CONSOLES Lights Dimmer" },
    { "FLOOD_DIMMER", 65535, "FLOOD Light Dimmer" },
    { "INST_PNL_DIMMER", 65535, "INST PNL Dimmer" },
    { "LIGHTS_TEST_SW", 1, "Lights Test Switch" },
    { "WARN_CAUTION_DIMMER", 65535, "WARN/CAUTION Light Dimmer" },
    { "LEFT_FIRE_BTN", 1, "Left Engine/AMAD Fire Warning/Extinguisher Light" },
    { "LEFT_FIRE_BTN_COVER", 1, "Left Engine/AMAD Fire Warning Cover" },
    { "HMD_OFF_BRT", 65535, "HMD OFF/BRT Knob" },
    { "IR_COOL_SW", 2, "IR Cooling Switch" },
    { "SPIN_RECOVERY_COVER", 1, "Spin Recovery Switch Cover" },
    { "SPIN_RECOVERY_SW", 1, "Spin Recovery Switch" },
    { "MASTER_ARM_SW", 1, "Master Arm Switch" },
    { "MASTER_MODE_AA", 1, "Master Mode Button" },
    { "MASTER_MODE_AG", 1, "Master Mode Button" },
    { "MASTER_CAUTION_RESET_SW", 1, "MASTER CAUTION Reset Button - Press to reset" },
    { "RIGHT_FIRE_BTN", 1, "Right Engine/AMAD Fire Warning/Extinguisher Light" },
    { "RIGHT_FIRE_BTN_COVER", 1, "Right Engine/AMAD Fire Warning Cover" },
};

// ---- Enhanced Selector Info (from LUA) ----
// APU_FIRE_BTN (APU Fire Warning/Extinguisher Light): 0=APU Fire Warning, 1=Extinguisher Light
// CMSD_DISPENSE_SW (DISPENSER Switch): 0=BYPASS, 1=ON, 2=OFF
// ECM_MODE_SW (ECM Mode Switch): 0=XMIT, 1=REC, 2=BIT, 3=STBY, 4=OFF
// WARN_CAUTION_DIMMER (WARN/CAUTION Light Dimmer): 0=WARN, 1=CAUTION Light Dimmer
// LEFT_FIRE_BTN (Left Engine/AMAD Fire Warning/Extinguisher Light): 0=Left Engine, 1=AMAD Fire Warning, 2=Extinguisher Light
// LEFT_FIRE_BTN_COVER (Left Engine/AMAD Fire Warning Cover): 0=Left Engine, 1=AMAD Fire Warning Cover
// HMD_OFF_BRT (HMD OFF/BRT Knob): 0=HMD OFF, 1=BRT Knob
// IR_COOL_SW (IR Cooling Switch): 0=ORIDE, 1=NORM, 2=OFF
// RIGHT_FIRE_BTN (Right Engine/AMAD Fire Warning/Extinguisher Light): 0=Right Engine, 1=AMAD Fire Warning, 2=Extinguisher Light
// RIGHT_FIRE_BTN_COVER (Right Engine/AMAD Fire Warning Cover): 0=Right Engine, 1=AMAD Fire Warning Cover
