#pragma once

void DCSBIOS_init();
void DCSBIOS_loop();

void sendDCSBIOSCommand(const char* label, uint16_t value);

void onAircraftName(char* str);
void onLedChange(const char* label, uint16_t value, uint16_t max_value);
void onSelectorChange(const char* label, unsigned int newValue);

void DcsbiosProtocolReplay();

namespace cover {
    extern volatile bool gain_switch;
    extern volatile bool gen_tie;
    extern volatile bool left_eng_fire;
    extern volatile bool right_eng_fire;
    extern volatile bool spin_recovery;
}