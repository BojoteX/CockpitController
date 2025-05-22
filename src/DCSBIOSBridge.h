// DCSBIOSBridge.h

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

// ───── Optional Helpers ─────
bool applyThrottle(CommandHistoryEntry &e, const char* label, uint16_t value, bool force=false);

// ───── Optional Replay Support ─────
void DcsbiosProtocolReplay();

// ───── Command Sender ─────
void sendDCSBIOSCommand(const char* label, uint16_t value, bool force);
void sendCommand(const char* label, const char* value);

// ───── Tracked State Accessors via CommandHistory ─────
#define isCoverOpen(label)     (findCmdEntry(label) ? (findCmdEntry(label)->lastValue > 0) : false)
#define isToggleOn(label)      isCoverOpen(label)
#define setCoverState(label,v) sendDCSBIOSCommand(label, v ? 1 : 0, true)
#define setToggleState(label,v) setCoverState(label,v)

// ───── Hooks ─────
void onAircraftName(char* str);
void onLedChange(const char* label, uint16_t value, uint16_t max_value);
void onSelectorChange(const char* label, unsigned int newValue);

void replayData();
void DCSBIOSBridge_setup();
void DCSBIOSBridge_loop();
void DCSBIOS_keepAlive();
void DcsBiosTask(void* param);
bool cdcEnsureRxReady(uint32_t timeoutMs = CDC_TIMEOUT_RX_TX); // make sure we are ACTIVELY receiving a stream
bool cdcEnsureTxReady(uint32_t timeoutMs = CDC_TIMEOUT_RX_TX); // make sure we are ACTIVELY receiving a stream