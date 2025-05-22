// DCSBIOSBridge.cpp

#include "src/Globals.h"
#include "src/DCSBIOSBridge.h"
#include "src/LABELS/DCSBIOSBridgeData.h"
#include "src/LEDControl.h"
#include "src/HIDManager.h"

#define DCSBIOS_DISABLE_SERVO
#define DCSBIOS_ESP32_CDC_SERIAL
#include "lib/DCS-BIOS/src/DcsBios.h"
#include "lib/DCS-BIOS/src/internal/Protocol.cpp"

#if !defined(ARDUINO_USB_CDC_ON_BOOT) || (ARDUINO_USB_CDC_ON_BOOT == 0)
// Get the USBCDC class (we don't need unless we DISABLE CDC ON BOOT)
USBCDC USBSerial;
#endif

// Same group send spacing enforcement
static uint32_t lastGroupSendUs[MAX_GROUPS] = {0};

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
        // initializePanels();

        if(isModeSelectorDCS()) {
            // flushBufferedDcsCommands();
        }
        else {
            // flushBufferedHidCommands();
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
            beginProfiling(PERF_REPLAY);
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
            endProfiling(PERF_REPLAY);
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
    uint32_t nowUs = micros();

    CommandHistoryEntry* groupLatest[MAX_GROUPS] = { nullptr };

    // Step 1: Find winner per group
    for (size_t i = 0; i < commandHistorySize; ++i) {
        CommandHistoryEntry& e = commandHistory[i];
        if (!e.hasPending || e.group == 0) continue;

        if (now - e.lastChangeTime >= SELECTOR_DWELL_MS) {
            uint16_t g = e.group;

            if (g >= MAX_GROUPS) {
                debugPrintf("❌ FATAL: group ID %u exceeds MAX_GROUPS (%u). Halting flush.\n", g, MAX_GROUPS);
                abort();  // Fail safe
            }

            if (!groupLatest[g] || e.lastChangeTime > groupLatest[g]->lastChangeTime) {
                groupLatest[g] = &e;
            }
        }
    }

    // Step 2: For each group, clear others and send winner (only if spacing OK)
    for (uint16_t g = 1; g < MAX_GROUPS; ++g) {
        CommandHistoryEntry* winner = groupLatest[g];
        if (!winner) continue;

        // Enforce spacing for this group
        nowUs = micros();
        if ((nowUs - lastGroupSendUs[g]) < DCS_GROUP_MIN_INTERVAL_US) {
            // debugPrintf("⚠️ [DCS] Group %u skipped: spacing\n", g);
            continue;
        }  

        // Clear other selectors in group
        for (size_t i = 0; i < commandHistorySize; ++i) {
            CommandHistoryEntry& e = commandHistory[i];
            if (e.group != g || &e == winner) continue;

            if (e.lastValue != 0) {
                char buf[10];
                snprintf(buf, sizeof(buf), "0");              

                // Send Command
                sendCommand(e.label,buf);

                e.lastValue = 0;
                e.lastSendTime = now;
            }

            e.hasPending = false;
        }

        // Send selected value
        if (winner->pendingValue != winner->lastValue) {
            char buf[10];
            snprintf(buf, sizeof(buf), "%u", winner->pendingValue);

            // Send command
            sendCommand(winner->label,buf);

            winner->lastValue    = winner->pendingValue;
            winner->lastSendTime = now;
        }

        winner->hasPending = false;
        lastGroupSendUs[g] = nowUs;
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

static volatile bool cdcTxReady = true;
static volatile bool cdcRxReady = true;

static void cdcConnectedHandler(void* arg, esp_event_base_t base, int32_t id, void* event_data) {
    debugPrintln("🔌 CDC Connected (DTR asserted)");
}

static void cdcDisconnectedHandler(void* arg, esp_event_base_t base, int32_t id, void* event_data) {
    debugPrintln("❌ CDC Disconnected (DTR deasserted)");
}

static void cdcLineStateHandler(void* arg, esp_event_base_t base, int32_t id, void* event_data) {
    auto* ev = (arduino_usb_cdc_event_data_t*)event_data;
    bool dtr = ev->line_state.dtr;
    bool rts = ev->line_state.rts;

    debugPrintf("📡 CDC Line State: DTR=%s, RTS=%s\n",
                dtr ? "ON" : "OFF",
                rts ? "ON" : "OFF");
}

static void cdcLineCodingHandler(void* arg, esp_event_base_t base, int32_t id, void* event_data) {
    auto* ev = (arduino_usb_cdc_event_data_t*)event_data;

    debugPrintf("🔧 CDC Line Coding: Baud=%u, StopBits=%u, Parity=%u, DataBits=%u\n",
                ev->line_coding.bit_rate,
                ev->line_coding.stop_bits,
                ev->line_coding.parity,
                ev->line_coding.data_bits);
}


static void cdcRxHandler(void* arg,
                         esp_event_base_t base,
                         int32_t id,
                         void* event_data)
{
    cdcRxReady = true;
    HIDManager_dispatchReport(true); // This will only send at HID_REPORT_RATE_HZ
    cdcRxReady = false;    
}

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
    debugPrintf("[CDC RX_OVERFLOW] ❌ dropped=%u\n", (unsigned)ev->rx_overflow.dropped_bytes);
}

void setupCDCEvents() {
    // Serial.onEvent(ARDUINO_USB_CDC_CONNECTED_EVENT, cdcConnectedHandler);
    // Serial.onEvent(ARDUINO_USB_CDC_DISCONNECTED_EVENT, cdcDisconnectedHandler);
    // Serial.onEvent(ARDUINO_USB_CDC_LINE_STATE_EVENT, cdcLineStateHandler);
    // Serial.onEvent(ARDUINO_USB_CDC_LINE_CODING_EVENT, cdcLineCodingHandler);       
    Serial.onEvent(ARDUINO_USB_CDC_RX_EVENT, cdcRxHandler);
    Serial.onEvent(ARDUINO_USB_CDC_TX_EVENT, cdcTxHandler);
    Serial.onEvent(ARDUINO_USB_CDC_RX_OVERFLOW_EVENT, cdcRxOvfHandler);    
}

bool cdcEnsureTxReady(uint32_t timeoutMs) {
    unsigned long start = millis();
    while (!cdcTxReady) {
        yield();
        if (millis() - start > timeoutMs) {
            return false;
        }
    }
    return true;
}

bool cdcEnsureRxReady(uint32_t timeoutMs) {
    unsigned long start = millis();
    while (!cdcRxReady) {
        yield();
        if (millis() - start > timeoutMs) {
            return false;
        }
    }
    return true;
}

void sendCommand(const char* msg, const char* arg) {
    if (!cdcEnsureRxReady()) {
        return;
    }

    if (!cdcEnsureTxReady()) {
        return;
    }

    cdcTxReady = false;
    if (DcsBios::sendDcsBiosMessage(msg, arg)) {
        debugPrintf("🛩️ [DCS] %s %s\n", msg, arg);
    } else {
        debugPrintf("❌ [DCS] Failed to send %s %s\n", msg, arg);
        return;
    }

/*
    unsigned long start = millis();
    while (!cdcTxReady) {
        yield();
        if (millis() - start > CDC_TIMEOUT_RX_TX) {
            debugPrintf("❌ [DCS] TX confirm timeout for %s %s\n", msg, arg);
            return;
        }
    }
*/

}

void DCSBIOS_keepAlive() {
    static unsigned long lastHeartbeat = 0;
    unsigned long now = millis();

    if (now - lastHeartbeat >= DCS_KEEP_ALIVE_MS) {
        lastHeartbeat = now;
        sendCommand("PING", "0");
    }
}

// sendDCSBIOSCommand: shared DCS command sender, with selector buffering & throttle.
void sendDCSBIOSCommand(const char* label, uint16_t value, bool force /*=false*/) {

    static char buf[10];
    snprintf(buf, sizeof(buf), "%u", value);
    // debugPrintf("🛩️ [DCS] ATTEMPING SEND: %s = %u%s\n", label, value, force ? " (forced)" : "");
    
    // Lookup history entry
    auto* e = findCmdEntry(label);
    if (!e) {
        debugPrintf("⚠️ [DCS] REJECTED untracked: %s = %u\n", label, value);
        return;
    }
    unsigned long now = millis();

    #if defined(SELECTOR_DWELL_MS) && (SELECTOR_DWELL_MS > 0)
    // Selector-group buffering (unchanged)
    if (!force && e->group > 0) {
        e->pendingValue   = value;
        e->lastChangeTime = now;
        e->hasPending     = true;
        // debugPrintf("🔁 [DCS] Buffer Selection for GroupID: %u - %s %u\n", e->group, label, e->pendingValue);
        return;
    }
    #endif

    // Apply unified throttle for non-zero
    if (!applyThrottle(*e, label, value, force)) {
        return;
    }          

    // Send Command
    sendCommand(label,buf);

    // 6) Update history
    e->lastValue    = value;
    e->lastSendTime = now;
}

void DcsBiosTask(void* param) {
    const TickType_t interval = pdMS_TO_TICKS(1000 / DCS_UPDATE_RATE_HZ);  // e.g., 30Hz
    while (true) {

        HIDManager_dispatchReport(true);

        #if DEBUG_PERFORMANCE
        beginProfiling(PERF_DCSBIOS);
        #endif

        cdcRxReady = false;
        while (Serial.available()) {
            DcsBios::parser.processChar(Serial.read());
        }

        // DcsBios::PollingInput::pollInputs();
        DcsBios::ExportStreamListener::loopAll();

        #if DEBUG_PERFORMANCE      
        endProfiling(PERF_DCSBIOS);
        #endif

        vTaskDelay(interval);
    }
}

void DCSBIOSBridge_setup() {

    // Descriptors for our device
    USB.VID(USB_VID);
    USB.PID(USB_PID);

    // Serial for DCS-BIOS
    setupCDCEvents(); // Load CDC Events
    Serial.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
    Serial.setTxTimeoutMs(SERIAL_TX_TIMEOUT);  // To avoid CDC getting stuck when SOCAT starts acting up
    // Serial.setTimeout(1); // New one.. test it
    // Serial.enableReboot(false); // Should be set to false for PRODUCTION, true for development
    // Serial.setDebugOutput(false);
    Serial.begin(250000);

    // Initialize DCSBIOS (Not needed as we already started serial above) 
    // DcsBios::setup();    
}

void DCSBIOSBridge_loop() {

    #if DEBUG_PERFORMANCE
    beginProfiling(PERF_DCSBIOS);
    #endif
/*
    while (Serial.available()) {
        DcsBios::parser.processChar(Serial.read());
    }
*/

    size_t avail = Serial.available();
    size_t chunk = (avail < DCSBIOS_SERIAL_CHUNK_SIZE) ? avail : DCSBIOS_SERIAL_CHUNK_SIZE;
    for (size_t i = 0; i < chunk; ++i) {
        DcsBios::parser.processChar(Serial.read());
    }

    // DcsBios::PollingInput::pollInputs();
    DcsBios::ExportStreamListener::loopAll();

    #if DEBUG_PERFORMANCE      
    endProfiling(PERF_DCSBIOS);
    #endif

    // Optional
    if (isModeSelectorDCS()) DCSBIOS_keepAlive();  // Still called only when in DCS mode   

    #if defined(SELECTOR_DWELL_MS) && (SELECTOR_DWELL_MS > 0)
    if (isModeSelectorDCS()) flushBufferedDcsCommands();
    #endif    

}