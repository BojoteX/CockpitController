#ifndef LED_MAPPINGS_H
#define LED_MAPPINGS_H

inline const LEDMapping panelLEDs[] = {
  // GPIO LEDs & Analog Gauges (Servo controlled)
  {"INST_PNL_DIMMER", DEVICE_GPIO, {.gpioInfo = {6}}, true},
  {"SPIN_LT", DEVICE_GPIO, {.gpioInfo = {34}}, false},
  {"PRESSURE_ALT", DEVICE_GPIO, {.gpioInfo = {6}}, true}, // Pressure Altimeter gauge

  // PCA9555 LEDs
  {"CMSD_JET_SEL_L", DEVICE_PCA9555, {.pcaInfo = {0x22, 1, 1}}, false, true},
  {"MASTER_MODE_AA_LT", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 3}}, false, true},
  {"MASTER_MODE_AG_LT", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 4}}, false, true},
  {"MC_READY", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 5}}, false, true},
  {"MC_DISCH", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 6}}, false, true},

  // Right Annunciator (TM1637 - CLK 37, DIO 40)
  {"FIRE_APU_LT", DEVICE_TM1637, {.tm1637Info = {37, 40, 5, 0}}, false},
  {"FIRE_RIGHT_LT", DEVICE_TM1637, {.tm1637Info = {37, 40, 0, 2}}, false},
  {"RH_ADV_RCDR_ON", DEVICE_TM1637, {.tm1637Info = {37, 40, 0, 1}}, false},
  {"RH_ADV_AI", DEVICE_TM1637, {.tm1637Info = {37, 40, 4, 1}}, false},
  {"RH_ADV_CW", DEVICE_TM1637, {.tm1637Info = {37, 40, 5, 1}}, false},
  {"RH_ADV_DISP", DEVICE_TM1637, {.tm1637Info = {37, 40, 0, 0}}, false},
  {"RH_ADV_SAM", DEVICE_TM1637, {.tm1637Info = {37, 40, 3, 0}}, false},
  {"RH_ADV_AAA", DEVICE_TM1637, {.tm1637Info = {37, 40, 4, 0}}, false},
  {"RH_ADV_SPARE_RH1", DEVICE_TM1637, {.tm1637Info = {37, 40, 1, 1}}, false},
  {"RH_ADV_SPARE_RH2", DEVICE_TM1637, {.tm1637Info = {37, 40, 2, 1}}, false},
  {"RH_ADV_SPARE_RH3", DEVICE_TM1637, {.tm1637Info = {37, 40, 3, 1}}, false},
  {"RH_ADV_SPARE_RH4", DEVICE_TM1637, {.tm1637Info = {37, 40, 1, 0}}, false},
  {"RH_ADV_SPARE_RH5", DEVICE_TM1637, {.tm1637Info = {37, 40, 2, 0}}, false},

  // Left Annunciator (TM1637 - CLK 37, DIO 39)
  {"MASTER_CAUTION_LT", DEVICE_TM1637, {.tm1637Info = {37, 39, 5, 0}}, false},
  {"FIRE_LEFT_LT", DEVICE_TM1637, {.tm1637Info = {37, 39, 0, 2}}, false},
  {"LH_ADV_GO", DEVICE_TM1637, {.tm1637Info = {37, 39, 0, 0}}, false},
  {"LH_ADV_NO_GO", DEVICE_TM1637, {.tm1637Info = {37, 39, 0, 1}}, false},
  {"LH_ADV_L_BLEED", DEVICE_TM1637, {.tm1637Info = {37, 39, 1, 0}}, false},
  {"LH_ADV_R_BLEED", DEVICE_TM1637, {.tm1637Info = {37, 39, 1, 1}}, false},
  {"LH_ADV_SPD_BRK", DEVICE_TM1637, {.tm1637Info = {37, 39, 2, 0}}, false},
  {"LH_ADV_STBY", DEVICE_TM1637, {.tm1637Info = {37, 39, 2, 1}}, false},
  {"LH_ADV_L_BAR_RED", DEVICE_TM1637, {.tm1637Info = {37, 39, 3, 0}}, false},
  {"LH_ADV_REC", DEVICE_TM1637, {.tm1637Info = {37, 39, 3, 1}}, false},
  {"LH_ADV_L_BAR_GREEN", DEVICE_TM1637, {.tm1637Info = {37, 39, 4, 0}}, false},
  {"LH_ADV_XMIT", DEVICE_TM1637, {.tm1637Info = {37, 39, 4, 1}}, false},
  {"LH_ADV_ASPJ_OH", DEVICE_TM1637, {.tm1637Info = {37, 39, 5, 1}}, false},

  // Caution Advisory (GN1640T)
  {"CLIP_CK_SEAT_LT", DEVICE_GN1640T, {.gn1640Info = {0, 0, 0}}, false},
  {"CLIP_APU_ACC_LT", DEVICE_GN1640T, {.gn1640Info = {0, 1, 0}}, false},
  {"CLIP_BATT_SW_LT", DEVICE_GN1640T, {.gn1640Info = {0, 2, 0}}, false},
  {"CLIP_FCS_HOT_LT", DEVICE_GN1640T, {.gn1640Info = {0, 0, 1}}, false},
  {"CLIP_GEN_TIE_LT", DEVICE_GN1640T, {.gn1640Info = {0, 1, 1}}, false},
  {"CLIP_SPARE_CTN1_LT", DEVICE_GN1640T, {.gn1640Info = {0, 2, 1}}, false},
  {"CLIP_FUEL_LO_LT", DEVICE_GN1640T, {.gn1640Info = {0, 0, 2}}, false},
  {"CLIP_FCES_LT", DEVICE_GN1640T, {.gn1640Info = {0, 1, 2}}, false},
  {"CLIP_SPARE_CTN2_LT", DEVICE_GN1640T, {.gn1640Info = {0, 2, 2}}, false},
  {"CLIP_L_GEN_LT", DEVICE_GN1640T, {.gn1640Info = {0, 0, 3}}, false},
  {"CLIP_R_GEN_LT", DEVICE_GN1640T, {.gn1640Info = {0, 1, 3}}, false},
  {"CLIP_SPARE_CTN3_LT", DEVICE_GN1640T, {.gn1640Info = {0, 2, 3}}, false},

  // LOCKSHOOT LEDs (WS2812)
  {"LS_LOCK", DEVICE_WS2812, {.ws2812Info = {0}}, false},
  {"LS_SHOOT", DEVICE_WS2812, {.ws2812Info = {1}}, false},
  {"LS_SHOOT_STROBE", DEVICE_WS2812, {.ws2812Info = {2}}, false},
};

inline constexpr uint16_t panelLEDsCount = sizeof(panelLEDs) / sizeof(panelLEDs[0]);

#endif