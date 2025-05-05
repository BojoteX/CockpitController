#pragma once

#include "HIDDescriptors.h"
// Shared by HID and DCSBIOS for Dwell-time filtering logic 
struct CommandHistoryEntry {
    const char*     label;
    uint16_t        lastValue;
    unsigned long   lastSendTime;
    bool            isSelector;
    uint16_t        group;
    uint16_t        pendingValue;
    unsigned long   lastChangeTime;
    bool            hasPending;
    uint8_t         lastReport[sizeof(report.raw)];
    uint8_t         pendingReport[sizeof(report.raw)];
    unsigned long   lastHidSendTime;
};

// declare references to the one true table
CommandHistoryEntry*  dcsbios_getCommandHistory();
size_t                dcsbios_getCommandHistorySize();

// Tracked state lookup/setters
bool getTrackedState(const char* label);
void setTrackedState(const char* label, bool value);

// Used for efficient index lookup into trackedStates[]
int trackedIndexFor(const char* label);

// Throttling logic
bool throttleIdenticalValue(const char* label, CommandHistoryEntry &e, uint16_t value, bool force);
CommandHistoryEntry* findCmdEntry(const char* label);

// ───── Tracked State Accessors (macros) ─────
#define isCoverOpen(label)     getTrackedState(label)
#define isToggleOn(label)      getTrackedState(label)
#define setCoverState(label,v) setTrackedState(label, v)
#define setToggleState(label,v) setTrackedState(label, v)

// ───── Core Init / Loop ─────
void DCSBIOS_init();
void DCSBIOS_loop();

// ───── Command Sender ─────
void sendDCSBIOSCommand(const char* label, uint16_t value, bool force);

// ───── Hooks ─────
void onAircraftName(char* str);
void onLedChange(const char* label, uint16_t value, uint16_t max_value);
void onSelectorChange(const char* label, unsigned int newValue);

// ───── Optional Replay Support ─────
void DcsbiosProtocolReplay();

// ───── Optional Helpers ─────
bool applyThrottle(CommandHistoryEntry &e, const char* label, uint16_t value, bool force=false);
