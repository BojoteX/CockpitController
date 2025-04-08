#ifndef LED_MAPPINGS_H
#define LED_MAPPINGS_H

#ifdef DEFINE_MAPPINGS

LEDMapping panelLEDs[] = {

  // Panel-specific meta-commands
  {"LA_ALL_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"RA_ALL_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"ECM_ALL_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"ARM_ALL_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"CA_ALL_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},
  {"LOCKSHOOT_ALL_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},

  // Global meta-command (ALL PANELS)
  {"ALL_PANELS_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},

  // Global meta-command (ALL PANELS)
  {"ALL_PANELS_ALL_LEDS", DEVICE_NONE, {.gpioInfo = {0}}, false},

  // GPIO LEDs
  {"BKLT_PANELS", DEVICE_GPIO, {.gpioInfo = {6}}, true},
  {"IR_SPN_RCVY", DEVICE_GPIO, {.gpioInfo = {34}}, false},

  // PCA9555 LEDs
  {"ECM_JETT_SEL", DEVICE_PCA9555, {.pcaInfo = {0x22, 1, 1}}, false, true},
  {"ARM_AA", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 3}}, false, true},
  {"ARM_AG", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 4}}, false, true},
  {"ARM_READY", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 5}}, false, true},
  {"ARM_DISCH", DEVICE_PCA9555, {.pcaInfo = {0x5B, 1, 6}}, false, true},

  // Right Annunciator (TM1637 - CLK 37, DIO 40)
  {"RA_APU_FIRE", DEVICE_TM1637, {.tm1637Info = {37, 40, 5, 0}}, false},
  {"RA_ENG_FIRE", DEVICE_TM1637, {.tm1637Info = {37, 40, 0, 2}}, false},
  {"RA_RCDR_ON", DEVICE_TM1637, {.tm1637Info = {37, 40, 0, 1}}, false},
  {"RA_AI", DEVICE_TM1637, {.tm1637Info = {37, 40, 4, 1}}, false},
  {"RA_CW", DEVICE_TM1637, {.tm1637Info = {37, 40, 5, 1}}, false},
  {"RA_DISP", DEVICE_TM1637, {.tm1637Info = {37, 40, 0, 0}}, false},
  {"RA_SAM", DEVICE_TM1637, {.tm1637Info = {37, 40, 3, 0}}, false},
  {"RA_AAA", DEVICE_TM1637, {.tm1637Info = {37, 40, 4, 0}}, false},
  {"RA_DASH_L1", DEVICE_TM1637, {.tm1637Info = {37, 40, 1, 1}}, false},
  {"RA_DASH_L2", DEVICE_TM1637, {.tm1637Info = {37, 40, 2, 1}}, false},
  {"RA_DASH_L3", DEVICE_TM1637, {.tm1637Info = {37, 40, 3, 1}}, false},
  {"RA_DASH_R1", DEVICE_TM1637, {.tm1637Info = {37, 40, 1, 0}}, false},
  {"RA_DASH_R2", DEVICE_TM1637, {.tm1637Info = {37, 40, 2, 0}}, false},

  // Left Annunciator (TM1637 - CLK 37, DIO 39)
  {"LA_GO", DEVICE_TM1637, {.tm1637Info = {37, 39, 0, 0}}, false},
  {"LA_NO_GO", DEVICE_TM1637, {.tm1637Info = {37, 39, 0, 1}}, false},
  {"LA_ENG_FIRE", DEVICE_TM1637, {.tm1637Info = {37, 39, 0, 2}}, false},
  {"LA_L_BLEED", DEVICE_TM1637, {.tm1637Info = {37, 39, 1, 0}}, false},
  {"LA_R_BLEED", DEVICE_TM1637, {.tm1637Info = {37, 39, 1, 1}}, false},
  {"LA_SPD_BRK", DEVICE_TM1637, {.tm1637Info = {37, 39, 2, 0}}, false},
  {"LA_STBY", DEVICE_TM1637, {.tm1637Info = {37, 39, 2, 1}}, false},
  {"LA_L_BAR_RED", DEVICE_TM1637, {.tm1637Info = {37, 39, 3, 0}}, false},
  {"LA_REC", DEVICE_TM1637, {.tm1637Info = {37, 39, 3, 1}}, false},
  {"LA_L_BAR_GREEN", DEVICE_TM1637, {.tm1637Info = {37, 39, 4, 0}}, false},
  {"LA_XMT", DEVICE_TM1637, {.tm1637Info = {37, 39, 4, 1}}, false},
  {"LA_MASTER_CAUTION", DEVICE_TM1637, {.tm1637Info = {37, 39, 5, 0}}, false},
  {"LA_ASPJ_ON", DEVICE_TM1637, {.tm1637Info = {37, 39, 5, 1}}, false},


  // Caution Advisory (GN1640T)
  {"CA_CK_SEAT", DEVICE_GN1640T, {.gn1640Info = {0, 0, 0}}, false},
  {"CA_APU_ACC", DEVICE_GN1640T, {.gn1640Info = {0, 1, 0}}, false},
  {"CA_BATT_SW", DEVICE_GN1640T, {.gn1640Info = {0, 2, 0}}, false},
  {"CA_FCS_HOT", DEVICE_GN1640T, {.gn1640Info = {0, 0, 1}}, false},
  {"CA_GEN_TIE", DEVICE_GN1640T, {.gn1640Info = {0, 1, 1}}, false},
  {"CA_DASH_1", DEVICE_GN1640T, {.gn1640Info = {0, 2, 1}}, false},
  {"CA_FUEL_LO", DEVICE_GN1640T, {.gn1640Info = {0, 0, 2}}, false},
  {"CA_FCES", DEVICE_GN1640T, {.gn1640Info = {0, 1, 2}}, false},
  {"CA_DASH_2", DEVICE_GN1640T, {.gn1640Info = {0, 2, 2}}, false},
  {"CA_L_GEN", DEVICE_GN1640T, {.gn1640Info = {0, 0, 3}}, false},
  {"CA_R_GEN", DEVICE_GN1640T, {.gn1640Info = {0, 1, 3}}, false},
  {"CA_DASH_3", DEVICE_GN1640T, {.gn1640Info = {0, 2, 3}}, false},

  // LOCKSHOOT LEDs (WS2812)
  {"LOCKSHOOT_LOCK", DEVICE_WS2812, {.ws2812Info = {0}}, false},
  {"LOCKSHOOT_SHOOT", DEVICE_WS2812, {.ws2812Info = {1}}, false},
  {"LOCKSHOOT_BLANK", DEVICE_WS2812, {.ws2812Info = {2}}, false},
};

const uint16_t panelLEDsCount = sizeof(panelLEDs) / sizeof(panelLEDs[0]);

#else

// Extern declarations (for all other files including Mappings.h)
extern LEDMapping panelLEDs[];
extern const uint16_t panelLEDsCount;

#endif

#endif