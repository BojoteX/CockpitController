#include "src/Globals.h"
#include "src/DCSBIOSBridge.h"
#include "src/LABELS/DCSBIOSBridgeData.h"
#include "src/LEDControl.h"
#include "src/HIDManager.h"
#include "src/PsramConfig.h" // We init PSRAM when available only
#if DEBUG_PERFORMANCE
#include "src/PerfMonitor.h"
#endif

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#endif

#define DCSBIOS_DISABLE_SERVO
#define DCSBIOS_ESP32_CDC_SERIAL
#include "lib/DCS-BIOS/src/DcsBios.h"
#include "lib/DCS-BIOS/src/internal/Protocol.cpp"

// Command History tracking
static_assert(commandHistorySize <= MAX_TRACKED_RECORDS, "Not enough space for tracked entries. Increase MAX_TRACKED_RECORDS in Config.h");

struct PendingUpdate {
    const char* label;
    uint16_t value;
    uint16_t max_value;
};

// ——— Configuration ———
#define MAX_PENDING_UPDATES 220

// Used for reliably detecting an active stream and when it goes up or down.
static constexpr unsigned long STREAM_TIMEOUT_MS = 500;  // ms without writes → consider dead

// DcsBiosSniffer Listener (Hornet FA-18C)
class DcsBiosSniffer : public DcsBios::ExportStreamListener {
public:
    DcsBiosSniffer()
      : DcsBios::ExportStreamListener(0x7400, 0x77FF),
        pendingUpdateCount(0),
        pendingUpdateOverflow(0),
        _lastWriteMs(0),
        _streamUp(false)
    {}

    void onDcsBiosWrite(unsigned int addr, unsigned int value) override {
        unsigned long now = millis();

        // 1) Stream-health logic
        _lastWriteMs = now;
        if (!_streamUp) {
            _streamUp = true;
            onStreamUp();    // one-time “stream came up” hook
        }

        static uint16_t prevValues[DcsOutputTableSize] = {0};

        const AddressEntry* ae = findDcsOutputEntries(addr);
        if (!ae) return;

        // 2) Dispatch per control type
        for (uint8_t i = 0; i < ae->count; ++i) {
            const DcsOutputEntry* entry = ae->entries[i];
            uint16_t val = (value & entry->mask) >> entry->shift;

            size_t index = entry - DcsOutputTable;
            if (index >= DcsOutputTableSize || prevValues[index] == val) continue;
            prevValues[index] = val;

            switch (entry->controlType) {
                case CT_LED:
                case CT_ANALOG:
                    if (pendingUpdateCount < MAX_PENDING_UPDATES) {
                        pendingUpdates[pendingUpdateCount++] = {entry->label, val, entry->max_value};
                    } else {
                        pendingUpdateOverflow++;
                    }
                    break;

                case CT_SELECTOR:
                    onSelectorChange(entry->label, val);
                    break;

                case CT_DISPLAY:
                    // Placeholder — will later call onDisplayChange(entry->label, val);
                    break;

                case CT_METADATA:
                    // Placeholder — will later call onMetadataChange(entry->label, val);
                    break;
            }
        }
    }

    void onConsistentData() override {
        if (_streamUp && (millis() - _lastWriteMs) >= STREAM_TIMEOUT_MS) {
            _streamUp = false;
            onStreamDown();
        }

        // Only flushing LED/Analog pending updates for now
        for (uint16_t i = 0; i < pendingUpdateCount; ++i) {
            const PendingUpdate& u = pendingUpdates[i];
            onLedChange(u.label, u.value, u.max_value);
        }
        pendingUpdateCount = 0;

        if (pendingUpdateOverflow > 0) {
            debugPrintf("[WARNING] %u LED updates dropped\n", pendingUpdateOverflow);
            pendingUpdateOverflow = 0;
        }
    }

    bool isStreamAlive() const {
        return (millis() - _lastWriteMs) < STREAM_TIMEOUT_MS;
    }

protected:
    virtual void onStreamUp()   { debugPrintln("[STREAM] UP"); }
    virtual void onStreamDown() { debugPrintln("[STREAM] DOWN"); }

private:
    PendingUpdate    pendingUpdates[MAX_PENDING_UPDATES];
    uint16_t         pendingUpdateCount;
    uint32_t         pendingUpdateOverflow;
    unsigned long    _lastWriteMs;
    bool             _streamUp;
};
DcsBiosSniffer mySniffer;

// ——— Corrected Mission Handler ———
void onAircraftName(char* str) {
    if (str != nullptr && str[0] != '\0') {
        char buf[128];
        snprintf(buf, sizeof(buf), "[MISSION START] %s.\n", str);
        debugPrintf("%s", buf);

        // This ensures your cockpit and sim always sync on mission start
        initializePanels();

        if(isModeSelectorDCS()) {
            flushBufferedDcsCommands();
        }
        else {
            flushBufferedHidCommands();
        }

    } else {
        debugPrintln("[MISSION STOP]");
    }
}

// Get MetaData to init cockpit state
DcsBios::StringBuffer<24> aicraftName(0x0000, onAircraftName); // Its safe to do StringBuffer and Integer buffer for addresses OUTSIDE our listener, we do so for MetaData

// Frame counter for reference measure frame skips
void onUpdateCounterChange(unsigned int value) {
    uint8_t frameCounter = value & 0xFF;          // low byte
    uint8_t frameSkipCounter = (value >> 8) & 0xFF; // high byte
    // Used for debugging only
    debugPrintf("Frame: %u | Skips: %u\n", frameCounter, frameSkipCounter);
}
// Activate only when debugging frame skips and checking frame counter
// DcsBios::IntegerBuffer updateCounterBuffer(0xFFFE, 0xFFFF, 0, onUpdateCounterChange);

void onLedChange(const char* label, uint16_t value, uint16_t max_value) {
    if (max_value <= 1) {
        setLED(label, value > 0);
        char buf[128];
        snprintf(buf, sizeof(buf), "[LED] %s is set to %u", label, value);
        debugPrintln(buf);    
    } else {
        uint8_t intensity = (value * 100UL) / max_value;
        if (intensity < 3) {
            setLED(label, false, 0);  // treat as OFF
            char buf[128];
            snprintf(buf, sizeof(buf), "[LED] %s Intensity was set to 0", label);
            if(DEBUG) debugPrintln(buf);
        } else {
            if (intensity > 97) intensity = 100;
            setLED(label, true, intensity);
            char buf[128];
            snprintf(buf, sizeof(buf), "[LED] %s Intensity %u\%.", label, value);
            if(DEBUG) debugPrintln(buf);
        }
    }
}

void onSelectorChange(const char* label, unsigned int value) {
    CommandHistoryEntry* e = findCmdEntry(label);
    if (e) e->lastValue = value;

    const char* stateStr = nullptr;

    // 1. Covers
    if (strstr(label, "_COVER")) {
        stateStr = (value > 0) ? "OPEN" : "CLOSED";

    // 2. Strict _BTN suffix match (exclude _COVER_BTN)
    } else if (strlen(label) >= 4 && strcmp(label + strlen(label) - 4, "_BTN") == 0) {
        stateStr = (value > 0) ? "ON" : "OFF";

    // 3. Lookup in SelectorMap by exact (label + value) or (dcsCommand + value)
    } else {
        const SelectorEntry* match = nullptr;

        for (size_t i = 0; i < SelectorMapSize; ++i) {
            const SelectorEntry& entry = SelectorMap[i];

            bool matchLabelAndValue = (strcmp(entry.label, label) == 0 && entry.value == value);
            bool matchDcsAndValue   = (strcmp(entry.dcsCommand, label) == 0 && entry.value == value);

            if (matchLabelAndValue || matchDcsAndValue) {
                match = &entry;
                break;
            }
        }

        if (match && match->posLabel && match->posLabel[0] != '\0') {
            stateStr = match->posLabel;
        } else {
            static char fallback[16];
            snprintf(fallback, sizeof(fallback), "POS %u", value);
            stateStr = fallback;
        }
    }

    debugPrintf("[STATE UPDATE] 🔁 %s = %s\n", label, stateStr);
}

// DcsbiosReplayData.h is generated from dcsbios_data.json using Python (see ReplayData directory). This was used in early development
// to test locally via Serial Console. The preferred method for live debugging is WiFi UDP. See Config.h for configuration.
#if IS_REPLAY
// ReplayDataBuffer
uint8_t* replayBuffer = nullptr;
#include "ReplayData/DcsbiosReplayData.h"

void replayData() {
  // Uses a header object (created from dcsbios_data.json) to simulate DCS traffic internally WITHOUT using your serial port (great for debugging) 
  if (initPSRAM()) {
      replayBuffer = (uint8_t*)PS_MALLOC(dcsbiosReplayLength);
      if (replayBuffer) {
          memcpy(replayBuffer, dcsbiosReplayData, dcsbiosReplayLength);
          debugPrintln("[PSRAM] ✅ Data loaded into PSRAM.");
      } else {
          debugPrintln("[PSRAM] ❌ Failed to allocate.");
      }
  } else {
      debugPrintln("[PSRAM] ❌ Not available.");
  }
  // Begin simulated loop. 
  runReplayWithPrompt();
}

void DcsbiosProtocolReplay() {
    debugPrintln("\n[REPLAY PROTOCOL] 🔁 Playing stream from binary blob...");

    if (!replayBuffer) return;

    const uint8_t* ptr = replayBuffer;
    const uint8_t* end = replayBuffer + dcsbiosReplayLength;

    while (ptr < end) {

        #if DEBUG_PERFORMANCE
            beginProfiling("Replay Loop");
        #endif

        float frameDelay;
        memcpy_P(&frameDelay, ptr, sizeof(float));
        ptr += sizeof(float);

        uint16_t len = pgm_read_byte(ptr) | (pgm_read_byte(ptr + 1) << 8);
        ptr += 2;

        for (uint16_t i = 0; i < len; i++) {
            uint8_t b = pgm_read_byte(ptr + i);
            DcsBios::parser.processChar(b);
            DcsBios::loop();            // ✅ Loop after each byte
            delayMicroseconds(1);       // simulate serial pace
        }
        ptr += len;

        DcsBios::loop();                // catch final updates

        #if DEBUG_PERFORMANCE
            endProfiling("Replay Loop");
        #endif

        #if DEBUG_PERFORMANCE
            perfMonitorUpdate();
        #endif

        delay((unsigned long)(frameDelay * 1000));

    }
    debugPrintln("[REPLAY PROTOCOL] ✅ Complete.\n");
}
#endif

uint16_t getLastKnownState(const char* label) {
    CommandHistoryEntry* e = findCmdEntry(label);
    return e ? e->lastValue : 0;
}

void DCSBIOS_keepAlive() {
    constexpr unsigned long DCS_KEEP_ALIVE_MS = DCS_KEEP_ALIVE_INTERVAL_MS;

    static unsigned long lastPoll = 0;

    unsigned long now = millis();
    if (now - lastPoll < DCS_KEEPALIVE_POLL_INTERVAL) return;
    lastPoll = now;

    for (size_t i = 0; i < commandHistorySize; ++i) {
        CommandHistoryEntry& entry = commandHistory[i];

        if (!entry.isSelector) continue;

        if (now - entry.lastSendTime >= DCS_KEEP_ALIVE_MS) {
            sendDCSBIOSCommand(entry.label, entry.lastValue, true);
            // debugPrintf("🔁 [DCS COMMAND KEEP-ALIVE] %s = %u\n", entry.label, entry.lastValue);
        }
    }
}

// Callback to lets us know FIFO drained
static volatile bool cdcTxReady = true;
static void cdcTxHandler(void* arg,
                         esp_event_base_t base,
                         int32_t id,
                         void* event_data)
{
    cdcTxReady = true;
}

// Let us know when there is an overflow event
static void cdcRxOvfHandler(void* arg,
                            esp_event_base_t base,
                            int32_t id,
                            void* event_data)
{
    auto* ev = (arduino_usb_cdc_event_data_t*)event_data;
    debugPrintf("[CDC RX_OVERFLOW] ❌ dropped=%u\n",
                (unsigned)ev->rx_overflow.dropped_bytes);
}

void setupCDCEvents() {
  Serial.onEvent(ARDUINO_USB_CDC_TX_EVENT,           cdcTxHandler);
  Serial.onEvent(ARDUINO_USB_CDC_RX_OVERFLOW_EVENT,  cdcRxOvfHandler);
}

// Accessors into our TU static
CommandHistoryEntry* dcsbios_getCommandHistory() {
    return commandHistory;
}
size_t dcsbios_getCommandHistorySize() {
    return commandHistorySize;
}

// ----------------------------------------------------------------------------
// Lookup helper (O(N) for now, can swap to hash later)
// ----------------------------------------------------------------------------
CommandHistoryEntry* findCmdEntry(const char* label) {
    for (size_t i = 0; i < commandHistorySize; ++i) {
        if (strcmp(commandHistory[i].label, label) == 0) {
            return &commandHistory[i];
        }
    }
    return nullptr;
}

static void flushBufferedDcsCommands() {
    unsigned long now = millis();
    CommandHistoryEntry* groupLatest[MAX_GROUPS] = { nullptr };

    for (size_t i = 0; i < commandHistorySize; ++i) {
        CommandHistoryEntry& e = commandHistory[i];
        if (!e.hasPending || e.group == 0) continue;

        if (now - e.lastChangeTime >= SELECTOR_DWELL_MS) {
            uint16_t g = e.group;

            if (g >= MAX_GROUPS) {
              debugPrintf("❌ FATAL: group ID %u exceeds MAX_GROUPS (%u). Halting flush.\n", g, MAX_GROUPS);
              abort();  // Triggers clean panic for debugging
            }

            if (!groupLatest[g] || e.lastChangeTime > groupLatest[g]->lastChangeTime) {
                groupLatest[g] = &e;
            }
        }
    }

    // Step 2: For each group, flush winner and clear all others
    for (uint16_t g = 1; g < MAX_GROUPS; ++g) {

        CommandHistoryEntry* winner = groupLatest[g];

        if (!winner) continue;    

        if (!cdcTxReady) {
            // debugPrintf("[DCS] ❌ CDC not ready, skipping group %u\n", g);
            continue;
        }  

        // Clear all other selectors in the same group
        for (size_t i = 0; i < commandHistorySize; ++i) {
            CommandHistoryEntry& e = commandHistory[i];
            if (e.group != g || &e == winner) continue;

            if (e.lastValue != 0) {
                char buf[10];
                snprintf(buf, sizeof(buf), "0");
                cdcTxReady = false;
                DcsBios::sendDcsBiosMessage(e.label, buf);
                e.lastValue = 0;
                e.lastSendTime = now;
                debugPrintf("🛩️ [DCS] CLEAR: %s = 0\n", e.label);
            }

            e.hasPending = false;
        }

        // Send the selected winner’s value
        if (winner->pendingValue != winner->lastValue) {
            char buf[10];
            snprintf(buf, sizeof(buf), "%u", winner->pendingValue);
            cdcTxReady = false;
            DcsBios::sendDcsBiosMessage(winner->label, buf);
            winner->lastValue    = winner->pendingValue;
            winner->lastSendTime = now;
            debugPrintf("🛩️ [DCS] SELECTED: %s = %u\n", winner->label, winner->pendingValue);
        }

        winner->hasPending = false;
    }
}

// ----------------------------------------------------------------------------
// Send command to DCS-BIOS sendDcsBiosCommand 
// ----------------------------------------------------------------------------
bool applyThrottle(CommandHistoryEntry &e,
                          const char*    label,
                          uint16_t       value,
                          bool           force) {
    // Always allow forced and release events
    if (force || value == 0) {
        return true;
    }
    unsigned long now = millis();
    unsigned long dt  = now - e.lastSendTime;

    if (value == 1) {
        // BUTTON logic: very-fast duplicate first -> identical, slower -> rate
        if (dt < VALUE_THROTTLE_MS) {
            debugPrintf("[DCS] ⚠️ SKIP: %s debounced (%lums < %lums)\n",
                        label, dt, VALUE_THROTTLE_MS);
            return false;
        }
    } else {
        // KNOB/AXIS logic: simple rate limiting
        if (dt < ANY_VALUE_THROTTLE_MS) {
            debugPrintf("[DCS] ⚠️ SKIP: %s rate limited (%lums < %lums)\n",
                        label, dt, ANY_VALUE_THROTTLE_MS);
            return false;
        }
    }
    return true;
}

/**
 * sendDCSBIOSCommand: shared DCS command sender, with selector buffering & throttle.
 */
void sendDCSBIOSCommand(const char* label, uint16_t value, bool force /*=false*/) {

    static char buf[10];
    snprintf(buf, sizeof(buf), "%u", value);
    debugPrintf("🛩️ [DCS] ATTEMPING SEND: %s = %u%s\n", label, value, force ? " (forced)" : "");

    // Always flush anything in the buffer before even attempting a send...
    tud_cdc_write_flush();
    yield();
    
    // Lookup history entry
    auto* e = findCmdEntry(label);
    if (!e) {
        debugPrintf("⚠️ [DCS] REJECTED untracked: %s = %u\n", label, value);
        return;
    }
    unsigned long now = millis();

    // 2) Host-ready check
    if (!cdcTxReady) {
        debugPrintf("[DCS] ❌ Host NOT ready\n");
        return;
    }

    #if defined(SELECTOR_DWELL_MS) && (SELECTOR_DWELL_MS > 0)
    // 3) Selector-group buffering (unchanged)
    if (!force && e->group > 0) {
        e->pendingValue   = value;
        e->lastChangeTime = now;
        e->hasPending     = true;
        // debugPrintf("🔁 [DCS] Buffer Selection for GroupID: %u - %s %u\n", e->group, label, e->pendingValue);
        return;
    }
    #endif

    // 4) Apply unified throttle for non-zero
    if (!applyThrottle(*e, label, value, force)) {
        return;
    }

    // 5) Build & send message
    cdcTxReady = false;
    if(DcsBios::sendDcsBiosMessage(label, buf)) {
        debugPrintf("🛩️ [DCS] CONFIRMED SEND: %s = %u%s\n", label, value, force ? " (forced)" : "");
    }
    else {
        debugPrintf("❌ [DCS] COULD NOT SEND: %s = %u%s\n", label, value, force ? " (forced)" : "");
    }

    // 6) Update history
    e->lastValue    = value;
    e->lastSendTime = now;
}

void DCSBIOS_init() {

    #if DEBUG_PERFORMANCE
    initPerfMonitor(); // this is used for profiling, see debugPrint for details
    #endif

    DcsBios::setup();

    // Subscribe to each CDC event by ID
    setupCDCEvents();

    // Check if PSRAM is available
    if (initPSRAM()) {
        debugPrintln("PSRAM is available on this device");
    }
    else {
        debugPrintln("PSRAM is NOT available on this device");
    }
}

void DCSBIOS_loop() {
    #if DEBUG_PERFORMANCE
    beginProfiling("DCS-BIOS Loop");
    #endif

    DcsBios::loop();
   
    #if ENABLE_DCS_COMMAND_KEEPALIVE
    if (isModeSelectorDCS()) DCSBIOS_keepAlive();
    #endif

    #if DEBUG_PERFORMANCE
    endProfiling("DCS-BIOS Loop");
    #endif

    #if defined(SELECTOR_DWELL_MS) && (SELECTOR_DWELL_MS > 0)
    if (isModeSelectorDCS()) flushBufferedDcsCommands();
    #endif

    #if DEBUG_PERFORMANCE
    perfMonitorUpdate();
    #endif
}