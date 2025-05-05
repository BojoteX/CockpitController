// Auto-generated DCSBIOS Bridge Data (JSON‑only) - DO NOT EDIT
#pragma once

#include "../HIDDescriptors.h"
#include <stdint.h>

enum ControlType : uint8_t {
  CT_LED,
  CT_ANALOG,
  CT_SELECTOR,
  CT_DISPLAY,
  CT_METADATA
};

struct DcsOutputEntry { uint16_t addr, mask; uint8_t shift; uint16_t max_value; const char* label; ControlType controlType; };
static const DcsOutputEntry DcsOutputTable[] = {
    {0x740C,0x0008,3,1,"APU_FIRE_BTN",CT_SELECTOR},
    {0x740C,0x0004,2,1,"FIRE_APU_LT",CT_LED},
    {0x74A4,0x0100,8,1,"CLIP_APU_ACC_LT",CT_LED},
    {0x74A4,0x0200,9,1,"CLIP_BATT_SW_LT",CT_LED},
    {0x74A0,0x8000,15,1,"CLIP_CK_SEAT_LT",CT_LED},
    {0x74A4,0x4000,14,1,"CLIP_FCES_LT",CT_LED},
    {0x74A4,0x0400,10,1,"CLIP_FCS_HOT_LT",CT_LED},
    {0x74A4,0x2000,13,1,"CLIP_FUEL_LO_LT",CT_LED},
    {0x74A4,0x0800,11,1,"CLIP_GEN_TIE_LT",CT_LED},
    {0x74A8,0x0100,8,1,"CLIP_L_GEN_LT",CT_LED},
    {0x74A8,0x0200,9,1,"CLIP_R_GEN_LT",CT_LED},
    {0x74A4,0x1000,12,1,"CLIP_SPARE_CTN1_LT",CT_LED},
    {0x74A4,0x8000,15,1,"CLIP_SPARE_CTN2_LT",CT_LED},
    {0x74A8,0x0400,10,1,"CLIP_SPARE_CTN3_LT",CT_LED},
    {0x7514,0xFFFF,0,65535,"PRESSURE_ALT",CT_ANALOG},
    {0x7488,0x0800,11,1,"AUX_REL_SW",CT_SELECTOR},
    {0x7484,0x6000,13,2,"CMSD_DISPENSE_SW",CT_SELECTOR},
    {0x7484,0x8000,15,1,"CMSD_JET_SEL_BTN",CT_SELECTOR},
    {0x74D4,0x8000,15,1,"CMSD_JET_SEL_L",CT_LED},
    {0x7488,0x0700,8,4,"ECM_MODE_SW",CT_SELECTOR},
    {0x740E,0x0001,0,1,"FIRE_EXT_BTN",CT_SELECTOR},
    {0x754A,0xFFFF,0,65535,"CHART_DIMMER",CT_ANALOG},
    {0x74C8,0x0600,9,2,"COCKKPIT_LIGHT_MODE_SW",CT_SELECTOR},
    {0x7544,0xFFFF,0,65535,"CONSOLES_DIMMER",CT_ANALOG},
    {0x7548,0xFFFF,0,65535,"FLOOD_DIMMER",CT_ANALOG},
    {0x7546,0xFFFF,0,65535,"INST_PNL_DIMMER",CT_ANALOG},
    {0x74C8,0x0800,11,1,"LIGHTS_TEST_SW",CT_SELECTOR},
    {0x754C,0xFFFF,0,65535,"WARN_CAUTION_DIMMER",CT_ANALOG},
    {0x740A,0x0008,3,1,"LH_ADV_ASPJ_OH",CT_LED},
    {0x740A,0x0010,4,1,"LH_ADV_GO",CT_LED},
    {0x740A,0x0002,1,1,"LH_ADV_L_BAR_GREEN",CT_LED},
    {0x7408,0x8000,15,1,"LH_ADV_L_BAR_RED",CT_LED},
    {0x7408,0x0800,11,1,"LH_ADV_L_BLEED",CT_LED},
    {0x740A,0x0020,5,1,"LH_ADV_NO_GO",CT_LED},
    {0x740A,0x0001,0,1,"LH_ADV_REC",CT_LED},
    {0x7408,0x1000,12,1,"LH_ADV_R_BLEED",CT_LED},
    {0x7408,0x2000,13,1,"LH_ADV_SPD_BRK",CT_LED},
    {0x7408,0x4000,14,1,"LH_ADV_STBY",CT_LED},
    {0x740A,0x0004,2,1,"LH_ADV_XMIT",CT_LED},
    {0x7408,0x0040,6,1,"FIRE_LEFT_LT",CT_LED},
    {0x7408,0x0080,7,1,"LEFT_FIRE_BTN",CT_SELECTOR},
    {0x7408,0x0100,8,1,"LEFT_FIRE_BTN_COVER",CT_SELECTOR},
    {0x7408,0x0001,0,1,"LS_LOCK",CT_LED},
    {0x7408,0x0002,1,1,"LS_SHOOT",CT_LED},
    {0x7408,0x0004,2,1,"LS_SHOOT_STROBE",CT_LED},
    {0x7456,0xFFFF,0,65535,"HMD_OFF_BRT",CT_ANALOG},
    {0x742A,0xC000,14,2,"IR_COOL_SW",CT_SELECTOR},
    {0x742A,0x0800,11,1,"SPIN_LT",CT_LED},
    {0x742A,0x1000,12,1,"SPIN_RECOVERY_COVER",CT_SELECTOR},
    {0x742A,0x2000,13,1,"SPIN_RECOVERY_SW",CT_SELECTOR},
    {0x740C,0x2000,13,1,"MASTER_ARM_SW",CT_SELECTOR},
    {0x740C,0x0800,11,1,"MASTER_MODE_AA",CT_SELECTOR},
    {0x740C,0x0200,9,1,"MASTER_MODE_AA_LT",CT_LED},
    {0x740C,0x1000,12,1,"MASTER_MODE_AG",CT_SELECTOR},
    {0x740C,0x0400,10,1,"MASTER_MODE_AG_LT",CT_LED},
    {0x740C,0x4000,14,1,"MC_DISCH",CT_LED},
    {0x740C,0x8000,15,1,"MC_READY",CT_LED},
    {0x7408,0x0200,9,1,"MASTER_CAUTION_LT",CT_LED},
    {0x7408,0x0400,10,1,"MASTER_CAUTION_RESET_SW",CT_SELECTOR},
    {0x740A,0x0800,11,1,"RH_ADV_AAA",CT_LED},
    {0x740A,0x0400,10,1,"RH_ADV_AI",CT_LED},
    {0x740A,0x1000,12,1,"RH_ADV_CW",CT_LED},
    {0x740A,0x0100,8,1,"RH_ADV_DISP",CT_LED},
    {0x740A,0x0080,7,1,"RH_ADV_RCDR_ON",CT_LED},
    {0x740A,0x0200,9,1,"RH_ADV_SAM",CT_LED},
    {0x740A,0x2000,13,1,"RH_ADV_SPARE_RH1",CT_LED},
    {0x740A,0x4000,14,1,"RH_ADV_SPARE_RH2",CT_LED},
    {0x740A,0x8000,15,1,"RH_ADV_SPARE_RH3",CT_LED},
    {0x740C,0x0001,0,1,"RH_ADV_SPARE_RH4",CT_LED},
    {0x740C,0x0002,1,1,"RH_ADV_SPARE_RH5",CT_LED},
    {0x740C,0x0010,4,1,"FIRE_RIGHT_LT",CT_LED},
    {0x740C,0x0020,5,1,"RIGHT_FIRE_BTN",CT_SELECTOR},
    {0x740C,0x0040,6,1,"RIGHT_FIRE_BTN_COVER",CT_SELECTOR},
};
static const size_t DcsOutputTableSize = sizeof(DcsOutputTable)/sizeof(DcsOutputTable[0]);

// Static flat address-to-output entry lookup
struct AddressEntry {
  uint16_t addr;
  const DcsOutputEntry* entries[15]; // max entries per address
  uint8_t count;
};

static const AddressEntry dcsAddressTable[] = {
  { 0x740C, { &DcsOutputTable[0], &DcsOutputTable[1], &DcsOutputTable[50], &DcsOutputTable[51], &DcsOutputTable[52], &DcsOutputTable[53], &DcsOutputTable[54], &DcsOutputTable[55], &DcsOutputTable[56], &DcsOutputTable[68], &DcsOutputTable[69], &DcsOutputTable[70], &DcsOutputTable[71], &DcsOutputTable[72] }, 14 },
  { 0x74A4, { &DcsOutputTable[2], &DcsOutputTable[3], &DcsOutputTable[5], &DcsOutputTable[6], &DcsOutputTable[7], &DcsOutputTable[8], &DcsOutputTable[11], &DcsOutputTable[12] }, 8 },
  { 0x74A0, { &DcsOutputTable[4] }, 1 },
  { 0x74A8, { &DcsOutputTable[9], &DcsOutputTable[10], &DcsOutputTable[13] }, 3 },
  { 0x7514, { &DcsOutputTable[14] }, 1 },
  { 0x7488, { &DcsOutputTable[15], &DcsOutputTable[19] }, 2 },
  { 0x7484, { &DcsOutputTable[16], &DcsOutputTable[17] }, 2 },
  { 0x74D4, { &DcsOutputTable[18] }, 1 },
  { 0x740E, { &DcsOutputTable[20] }, 1 },
  { 0x754A, { &DcsOutputTable[21] }, 1 },
  { 0x74C8, { &DcsOutputTable[22], &DcsOutputTable[26] }, 2 },
  { 0x7544, { &DcsOutputTable[23] }, 1 },
  { 0x7548, { &DcsOutputTable[24] }, 1 },
  { 0x7546, { &DcsOutputTable[25] }, 1 },
  { 0x754C, { &DcsOutputTable[27] }, 1 },
  { 0x740A, { &DcsOutputTable[28], &DcsOutputTable[29], &DcsOutputTable[30], &DcsOutputTable[33], &DcsOutputTable[34], &DcsOutputTable[38], &DcsOutputTable[59], &DcsOutputTable[60], &DcsOutputTable[61], &DcsOutputTable[62], &DcsOutputTable[63], &DcsOutputTable[64], &DcsOutputTable[65], &DcsOutputTable[66], &DcsOutputTable[67] }, 15 },
  { 0x7408, { &DcsOutputTable[31], &DcsOutputTable[32], &DcsOutputTable[35], &DcsOutputTable[36], &DcsOutputTable[37], &DcsOutputTable[39], &DcsOutputTable[40], &DcsOutputTable[41], &DcsOutputTable[42], &DcsOutputTable[43], &DcsOutputTable[44], &DcsOutputTable[57], &DcsOutputTable[58] }, 13 },
  { 0x7456, { &DcsOutputTable[45] }, 1 },
  { 0x742A, { &DcsOutputTable[46], &DcsOutputTable[47], &DcsOutputTable[48], &DcsOutputTable[49] }, 4 },
};

// Address hash entry
struct DcsAddressHashEntry {
  uint16_t addr;
  const AddressEntry* entry;
};

static const DcsAddressHashEntry dcsAddressHashTable[53] = {
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  { 0x74C8, &dcsAddressTable[10] },
  { 0x742A, &dcsAddressTable[18] },
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  { 0x74D4, &dcsAddressTable[7] },
  { 0x74A0, &dcsAddressTable[2] },
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  { 0x74A4, &dcsAddressTable[1] },
  { 0x7544, &dcsAddressTable[11] },
  {0xFFFF, nullptr},
  { 0x7546, &dcsAddressTable[13] },
  { 0x74A8, &dcsAddressTable[3] },
  { 0x7548, &dcsAddressTable[12] },
  { 0x7514, &dcsAddressTable[4] },
  { 0x740C, &dcsAddressTable[0] },
  { 0x754A, &dcsAddressTable[9] },
  { 0x740E, &dcsAddressTable[8] },
  { 0x754C, &dcsAddressTable[14] },
  { 0x740A, &dcsAddressTable[15] },
  { 0x7408, &dcsAddressTable[16] },
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  { 0x7484, &dcsAddressTable[6] },
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  { 0x7488, &dcsAddressTable[5] },
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  { 0x7456, &dcsAddressTable[17] },
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
  {0xFFFF, nullptr},
};

// Simple address hash (modulo)
constexpr uint16_t addrHash(uint16_t addr) {
  return addr % 53;
}

inline const AddressEntry* findDcsOutputEntries(uint16_t addr) {
  uint16_t startH = addrHash(addr);
  for (uint16_t i = 0; i < 53; ++i) {
    uint16_t idx = (startH + i >= 53) ? (startH + i - 53) : (startH + i);
    const auto& entry = dcsAddressHashTable[idx];
    if (entry.addr == 0xFFFF) continue;
    if (entry.addr == addr) return entry.entry;
  }
  return nullptr;
}

struct SelectorEntry { const char* label; const char* dcsCommand; uint16_t value; const char* controlType; uint16_t group; };
static const SelectorEntry SelectorMap[] = {
    { "APU_FIRE_BTN","APU_FIRE_BTN",1,"momentary",0 },
    { "AUX_REL_SW_ENABLE","AUX_REL_SW",1,"selector",1 },
    { "AUX_REL_SW_NORM","AUX_REL_SW",0,"selector",1 },
    { "CMSD_DISPENSE_SW_BYPASS","CMSD_DISPENSE_SW",2,"selector",2 },
    { "CMSD_DISPENSE_SW_ON","CMSD_DISPENSE_SW",1,"selector",2 },
    { "CMSD_DISPENSE_SW_OFF","CMSD_DISPENSE_SW",0,"selector",2 },
    { "CMSD_JET_SEL_BTN","CMSD_JET_SEL_BTN",1,"momentary",0 },
    { "ECM_MODE_SW_XMIT","ECM_MODE_SW",4,"selector",3 },
    { "ECM_MODE_SW_REC","ECM_MODE_SW",3,"selector",3 },
    { "ECM_MODE_SW_BIT","ECM_MODE_SW",2,"selector",3 },
    { "ECM_MODE_SW_STBY","ECM_MODE_SW",1,"selector",3 },
    { "ECM_MODE_SW_OFF","ECM_MODE_SW",0,"selector",3 },
    { "FIRE_EXT_BTN","FIRE_EXT_BTN",1,"momentary",0 },
    { "CHART_DIMMER","CHART_DIMMER",65535,"analog",0 },
    { "COCKKPIT_LIGHT_MODE_SW_NVG","COCKKPIT_LIGHT_MODE_SW",2,"selector",4 },
    { "COCKKPIT_LIGHT_MODE_SW_NITE","COCKKPIT_LIGHT_MODE_SW",1,"selector",4 },
    { "COCKKPIT_LIGHT_MODE_SW_DAY","COCKKPIT_LIGHT_MODE_SW",0,"selector",4 },
    { "CONSOLES_DIMMER","CONSOLES_DIMMER",65535,"analog",0 },
    { "FLOOD_DIMMER","FLOOD_DIMMER",65535,"analog",0 },
    { "INST_PNL_DIMMER","INST_PNL_DIMMER",65535,"analog",0 },
    { "LIGHTS_TEST_SW_TEST","LIGHTS_TEST_SW",1,"selector",5 },
    { "LIGHTS_TEST_SW_OFF","LIGHTS_TEST_SW",0,"selector",5 },
    { "WARN_CAUTION_DIMMER","WARN_CAUTION_DIMMER",65535,"analog",0 },
    { "LEFT_FIRE_BTN","LEFT_FIRE_BTN",1,"momentary",0 },
    { "LEFT_FIRE_BTN_COVER","LEFT_FIRE_BTN_COVER",1,"momentary",0 },
    { "HMD_OFF_BRT","HMD_OFF_BRT",65535,"analog",0 },
    { "IR_COOL_SW_ORIDE","IR_COOL_SW",2,"selector",6 },
    { "IR_COOL_SW_NORM","IR_COOL_SW",1,"selector",6 },
    { "IR_COOL_SW_OFF","IR_COOL_SW",0,"selector",6 },
    { "SPIN_RECOVERY_COVER","SPIN_RECOVERY_COVER",1,"momentary",0 },
    { "SPIN_RECOVERY_SW_RCVY","SPIN_RECOVERY_SW",1,"selector",7 },
    { "SPIN_RECOVERY_SW_NORM","SPIN_RECOVERY_SW",0,"selector",7 },
    { "MASTER_ARM_SW_ARM","MASTER_ARM_SW",1,"selector",8 },
    { "MASTER_ARM_SW_SAFE","MASTER_ARM_SW",0,"selector",8 },
    { "MASTER_MODE_AA","MASTER_MODE_AA",1,"momentary",0 },
    { "MASTER_MODE_AG","MASTER_MODE_AG",1,"momentary",0 },
    { "MASTER_CAUTION_RESET_SW","MASTER_CAUTION_RESET_SW",1,"momentary",0 },
    { "RIGHT_FIRE_BTN","RIGHT_FIRE_BTN",1,"momentary",0 },
    { "RIGHT_FIRE_BTN_COVER","RIGHT_FIRE_BTN_COVER",1,"momentary",0 },
};
static const size_t SelectorMapSize = sizeof(SelectorMap)/sizeof(SelectorMap[0]);

struct TrackedStateEntry {
     const char* label;
     bool        value;
 };

// Tracked toggle & cover states
static TrackedStateEntry trackedStates[] = {
    { "APU_FIRE_BTN", false },
    { "CMSD_JET_SEL_BTN", false },
    { "FIRE_EXT_BTN", false },
    { "LEFT_FIRE_BTN", false },
    { "LEFT_FIRE_BTN_COVER", false },
    { "MASTER_CAUTION_RESET_SW", false },
    { "MASTER_MODE_AA", false },
    { "MASTER_MODE_AG", false },
    { "RIGHT_FIRE_BTN", false },
    { "RIGHT_FIRE_BTN_COVER", false },
    { "SPIN_RECOVERY_COVER", false },
};
static const size_t trackedStatesCount = sizeof(trackedStates)/sizeof(trackedStates[0]);

// Unified Command History Table (used for throttling, optional keep-alive, and HID dedupe)
struct CommandHistoryEntry {
    const char*     label;             // DCS command or HID control label
    uint16_t        lastValue;         // last DCS value sent
    unsigned long   lastSendTime;      // millis() of last DCS send
    bool            isSelector;        // part of a grouped selector
    uint16_t        group;             // selector group ID (>0)

    // buffering for grouped selectors:
    uint16_t        pendingValue;      // deferred DCS value
    unsigned long   lastChangeTime;    // millis() of last change
    bool            hasPending;        // pendingValue != lastValue

    // HID report dedupe/cache:
    uint8_t lastReport [ sizeof(report.raw) ];   // last raw HID bytes sent
    uint8_t pendingReport [ sizeof(report.raw) ];
    unsigned long   lastHidSendTime;   // millis() of last HID send
};

static CommandHistoryEntry commandHistory[] = {
    { "APU_FIRE_BTN", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "AUX_REL_SW", 0, 0, true, 1, 0,   0, false, {0}, {0}, 0 },
    { "CHART_DIMMER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "CMSD_DISPENSE_SW", 0, 0, true, 2, 0,   0, false, {0}, {0}, 0 },
    { "CMSD_JET_SEL_BTN", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "COCKKPIT_LIGHT_MODE_SW", 0, 0, true, 4, 0,   0, false, {0}, {0}, 0 },
    { "CONSOLES_DIMMER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "ECM_MODE_SW", 0, 0, true, 3, 0,   0, false, {0}, {0}, 0 },
    { "FIRE_EXT_BTN", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "FLOOD_DIMMER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "HMD_OFF_BRT", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "INST_PNL_DIMMER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "IR_COOL_SW", 0, 0, true, 6, 0,   0, false, {0}, {0}, 0 },
    { "LEFT_FIRE_BTN", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "LEFT_FIRE_BTN_COVER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "LIGHTS_TEST_SW", 0, 0, true, 5, 0,   0, false, {0}, {0}, 0 },
    { "MASTER_ARM_SW", 0, 0, true, 8, 0,   0, false, {0}, {0}, 0 },
    { "MASTER_CAUTION_RESET_SW", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "MASTER_MODE_AA", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "MASTER_MODE_AG", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "RIGHT_FIRE_BTN", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "RIGHT_FIRE_BTN_COVER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "SPIN_RECOVERY_COVER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
    { "SPIN_RECOVERY_SW", 0, 0, true, 7, 0,   0, false, {0}, {0}, 0 },
    { "WARN_CAUTION_DIMMER", 0, 0, false, 0, 0,   0, false, {0}, {0}, 0 },
};
static const size_t commandHistorySize = sizeof(commandHistory)/sizeof(CommandHistoryEntry);
