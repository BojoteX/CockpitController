#pragma once

#include "HIDDescriptors.h"

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

// Throttling logic
bool throttleIdenticalValue(const char* label, CommandHistoryEntry &e, uint16_t value, bool force);
CommandHistoryEntry* findCmdEntry(const char* label);

// ───── Tracked State Accessors via CommandHistory ─────
#define isCoverOpen(label)     (findCmdEntry(label) ? (findCmdEntry(label)->lastValue > 0) : false)
#define isToggleOn(label)      isCoverOpen(label)
#define setCoverState(label,v) sendDCSBIOSCommand(label, v ? 1 : 0, true)
#define setToggleState(label,v) setCoverState(label,v)

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

// ───── Forward declarations ─────
static void flushBufferedDcsCommands();
uint16_t getLastKnownState(const char* label);

void replayData();