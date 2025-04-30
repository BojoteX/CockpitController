#pragma once

// ───── Tracked State Accessors (macros) ─────
#define isCoverOpen(label)     getTrackedState(label)
#define isToggleOn(label)      getTrackedState(label)
#define setCoverState(label,v) setTrackedState(label, v)
#define setToggleState(label,v) setTrackedState(label, v)

// ───── Core Init / Loop ─────
void DCSBIOS_init();
void DCSBIOS_loop();

// ───── Command Sender ─────
void sendDCSBIOSCommand(const char* label, uint16_t value);

// ───── Hooks ─────
void onAircraftName(char* str);
void onLedChange(const char* label, uint16_t value, uint16_t max_value);
void onSelectorChange(const char* label, unsigned int newValue);

// ───── Optional Replay Support ─────
void DcsbiosProtocolReplay();

// ───── Tracked Toggle / Cover States ─────
struct TrackedStateEntry {
    const char* label;
    bool value;
};

extern TrackedStateEntry trackedStates[];
extern const size_t trackedStatesCount;

// Tracked state lookup/setters
bool getTrackedState(const char* label);
void setTrackedState(const char* label, bool value);

// Used for efficient index lookup into trackedStates[]
int trackedIndexFor(const char* label);
