// Auto-generated DCSBIOS Data Mapping Header with Multi-label Fast Lookup
#pragma once

#include <stdint.h>
#include <unordered_map>
#include <vector>

inline uint32_t dcsHash(uint16_t addr, uint16_t mask, uint8_t shift) {
    return ((uint32_t)addr << 16) ^ (uint32_t)mask ^ ((uint32_t)shift << 1);
}

static const std::unordered_map<uint32_t, std::vector<const char*>> DcsOutputHashTable = {
    { 0x740C0000, { "FIRE_APU_LT", "RH_ADV_SPARE_RH5" } },
    { 0x74A40110, { "CLIP_APU_ACC_LT" } },
    { 0x74A40212, { "CLIP_BATT_SW_LT" } },
    { 0x74A0801E, { "CLIP_CK_SEAT_LT" } },
    { 0x74A4401C, { "CLIP_FCES_LT" } },
    { 0x74A40414, { "CLIP_FCS_HOT_LT" } },
    { 0x74A4201A, { "CLIP_FUEL_LO_LT" } },
    { 0x74A40816, { "CLIP_GEN_TIE_LT" } },
    { 0x74A80110, { "CLIP_L_GEN_LT" } },
    { 0x74A80212, { "CLIP_R_GEN_LT" } },
    { 0x74A41018, { "CLIP_SPARE_CTN1_LT" } },
    { 0x74A4801E, { "CLIP_SPARE_CTN2_LT" } },
    { 0x74A80414, { "CLIP_SPARE_CTN3_LT" } },
    { 0x74D4801E, { "CMSD_JET_SEL_L" } },
    { 0x740A000E, { "LH_ADV_ASPJ_OH" } },
    { 0x740A0018, { "LH_ADV_GO" } },
    { 0x740A0000, { "LH_ADV_L_BAR_GREEN", "LH_ADV_XMIT" } },
    { 0x7408801E, { "LH_ADV_L_BAR_RED" } },
    { 0x74080816, { "LH_ADV_L_BLEED" } },
    { 0x740A002A, { "LH_ADV_NO_GO" } },
    { 0x740A0001, { "LH_ADV_REC" } },
    { 0x74081018, { "LH_ADV_R_BLEED" } },
    { 0x7408201A, { "LH_ADV_SPD_BRK" } },
    { 0x7408401C, { "LH_ADV_STBY" } },
    { 0x7408004C, { "FIRE_LEFT_LT" } },
    { 0x74080001, { "LS_LOCK" } },
    { 0x74080000, { "LS_SHOOT", "LS_SHOOT_STROBE" } },
    { 0x742A0816, { "SPIN_LT" } },
    { 0x740C0212, { "MASTER_MODE_AA_LT" } },
    { 0x740C0414, { "MASTER_MODE_AG_LT" } },
    { 0x740C401C, { "MC_DISCH" } },
    { 0x740C801E, { "MC_READY" } },
    { 0x74080212, { "MASTER_CAUTION_LT" } },
    { 0x740A0816, { "RH_ADV_AAA" } },
    { 0x740A0414, { "RH_ADV_AI" } },
    { 0x740A1018, { "RH_ADV_CW" } },
    { 0x740A0110, { "RH_ADV_DISP" } },
    { 0x740A008E, { "RH_ADV_RCDR_ON" } },
    { 0x740A0212, { "RH_ADV_SAM" } },
    { 0x740A201A, { "RH_ADV_SPARE_RH1" } },
    { 0x740A401C, { "RH_ADV_SPARE_RH2" } },
    { 0x740A801E, { "RH_ADV_SPARE_RH3" } },
    { 0x740C0001, { "RH_ADV_SPARE_RH4" } },
    { 0x740C0018, { "FIRE_RIGHT_LT" } },
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
