#pragma once

#include <Arduino.h>

void DCSBIOS_init();
void DCSBIOS_loop();
void sendDCSBIOSCommand(const char* label, uint16_t value);

void onLedChange(const char* label, uint16_t value, uint16_t max_value);

void onSelectorChange(const char* label, unsigned int newValue);

void DcsbiosProtocolReplay();
