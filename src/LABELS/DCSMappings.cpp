// Auto-generated DCS LED Callbacks
#include "DCSMappings.h"
#include "../LEDControl.h"

extern void handleLedChange(uint16_t address, uint16_t mask, uint8_t shift, unsigned int newValue);

void handleLedCallback0(unsigned int newValue) {
  handleLedChange(0x7408, 0x8000, 15, newValue);
}

void handleLedCallback1(unsigned int newValue) {
  handleLedChange(0x7408, 0x0800, 11, newValue);
}

void handleLedCallback2(unsigned int newValue) {
  handleLedChange(0x7408, 0x1000, 12, newValue);
}

void handleLedCallback3(unsigned int newValue) {
  handleLedChange(0x7408, 0x2000, 13, newValue);
}

void handleLedCallback4(unsigned int newValue) {
  handleLedChange(0x7408, 0x4000, 14, newValue);
}

void handleLedCallback5(unsigned int newValue) {
  handleLedChange(0x7408, 0x0001, 0, newValue);
}

void handleLedCallback6(unsigned int newValue) {
  handleLedChange(0x7408, 0x0002, 1, newValue);
}

void handleLedCallback7(unsigned int newValue) {
  handleLedChange(0x7408, 0x0004, 2, newValue);
}

void handleLedCallback8(unsigned int newValue) {
  handleLedChange(0x740A, 0x0008, 3, newValue);
}

void handleLedCallback9(unsigned int newValue) {
  handleLedChange(0x740A, 0x0010, 4, newValue);
}

void handleLedCallback10(unsigned int newValue) {
  handleLedChange(0x740A, 0x0002, 1, newValue);
}

void handleLedCallback11(unsigned int newValue) {
  handleLedChange(0x740A, 0x0020, 5, newValue);
}

void handleLedCallback12(unsigned int newValue) {
  handleLedChange(0x740A, 0x0001, 0, newValue);
}

void handleLedCallback13(unsigned int newValue) {
  handleLedChange(0x740A, 0x0004, 2, newValue);
}

void handleLedCallback14(unsigned int newValue) {
  handleLedChange(0x740A, 0x0800, 11, newValue);
}

void handleLedCallback15(unsigned int newValue) {
  handleLedChange(0x740A, 0x0400, 10, newValue);
}

void handleLedCallback16(unsigned int newValue) {
  handleLedChange(0x740A, 0x1000, 12, newValue);
}

void handleLedCallback17(unsigned int newValue) {
  handleLedChange(0x740A, 0x0100, 8, newValue);
}

void handleLedCallback18(unsigned int newValue) {
  handleLedChange(0x740A, 0x0080, 7, newValue);
}

void handleLedCallback19(unsigned int newValue) {
  handleLedChange(0x740A, 0x0200, 9, newValue);
}

void handleLedCallback20(unsigned int newValue) {
  handleLedChange(0x740A, 0x2000, 13, newValue);
}

void handleLedCallback21(unsigned int newValue) {
  handleLedChange(0x740A, 0x4000, 14, newValue);
}

void handleLedCallback22(unsigned int newValue) {
  handleLedChange(0x740A, 0x8000, 15, newValue);
}

void handleLedCallback23(unsigned int newValue) {
  handleLedChange(0x740C, 0x0200, 9, newValue);
}

void handleLedCallback24(unsigned int newValue) {
  handleLedChange(0x740C, 0x0400, 10, newValue);
}

void handleLedCallback25(unsigned int newValue) {
  handleLedChange(0x740C, 0x0001, 0, newValue);
}

void handleLedCallback26(unsigned int newValue) {
  handleLedChange(0x740C, 0x0002, 1, newValue);
}

void handleLedCallback27(unsigned int newValue) {
  handleLedChange(0x742A, 0x0800, 11, newValue);
}

void handleLedCallback28(unsigned int newValue) {
  handleLedChange(0x74A0, 0x8000, 15, newValue);
}

void handleLedCallback29(unsigned int newValue) {
  handleLedChange(0x74A4, 0x0100, 8, newValue);
}

void handleLedCallback30(unsigned int newValue) {
  handleLedChange(0x74A4, 0x0200, 9, newValue);
}

void handleLedCallback31(unsigned int newValue) {
  handleLedChange(0x74A4, 0x4000, 14, newValue);
}

void handleLedCallback32(unsigned int newValue) {
  handleLedChange(0x74A4, 0x0400, 10, newValue);
}

void handleLedCallback33(unsigned int newValue) {
  handleLedChange(0x74A4, 0x2000, 13, newValue);
}

void handleLedCallback34(unsigned int newValue) {
  handleLedChange(0x74A4, 0x0800, 11, newValue);
}

void handleLedCallback35(unsigned int newValue) {
  handleLedChange(0x74A4, 0x1000, 12, newValue);
}

void handleLedCallback36(unsigned int newValue) {
  handleLedChange(0x74A4, 0x8000, 15, newValue);
}

void handleLedCallback37(unsigned int newValue) {
  handleLedChange(0x74A8, 0x0100, 8, newValue);
}

void handleLedCallback38(unsigned int newValue) {
  handleLedChange(0x74A8, 0x0200, 9, newValue);
}

void handleLedCallback39(unsigned int newValue) {
  handleLedChange(0x74A8, 0x0400, 10, newValue);
}

const CallbackPtr ledCallbacks[NUM_DCS_LEDS] = {
  handleLedCallback0,
  handleLedCallback1,
  handleLedCallback2,
  handleLedCallback3,
  handleLedCallback4,
  handleLedCallback5,
  handleLedCallback6,
  handleLedCallback7,
  handleLedCallback8,
  handleLedCallback9,
  handleLedCallback10,
  handleLedCallback11,
  handleLedCallback12,
  handleLedCallback13,
  handleLedCallback14,
  handleLedCallback15,
  handleLedCallback16,
  handleLedCallback17,
  handleLedCallback18,
  handleLedCallback19,
  handleLedCallback20,
  handleLedCallback21,
  handleLedCallback22,
  handleLedCallback23,
  handleLedCallback24,
  handleLedCallback25,
  handleLedCallback26,
  handleLedCallback27,
  handleLedCallback28,
  handleLedCallback29,
  handleLedCallback30,
  handleLedCallback31,
  handleLedCallback32,
  handleLedCallback33,
  handleLedCallback34,
  handleLedCallback35,
  handleLedCallback36,
  handleLedCallback37,
  handleLedCallback38,
  handleLedCallback39,
};
