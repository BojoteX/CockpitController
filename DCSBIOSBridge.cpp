#define DCSBIOS_DISABLE_SERVO
#define DCSBIOS_ESP32_CDC_SERIAL
#include "lib/DCS-BIOS/src/DcsBios.h"
#include "src/Globals.h"
#include "src/DCSBIOSBridge.h"
#include "src/LABELS/DCSBIOSBridgeData.h"
#include "src/LEDControl.h"
#include "src/HIDManager.h"
#if DEBUG_PERFORMANCE
#include "src/PerfMonitor.h"
#endif

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#endif

// Force compilation of all DCS-BIOS internals (Arduino won't compile .cpp's inside lib automatically)
#include "lib/DCS-BIOS/src/internal/Protocol.cpp"

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

int trackedIndexFor(const char* label) {
    for (size_t i = 0; i < trackedStatesCount; ++i) {
        if (trackedStates[i].label == label || strcmp(trackedStates[i].label, label) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool getTrackedState(const char* label) {
    int index = trackedIndexFor(label);
    return (index >= 0) ? trackedStates[index].value : false;
}

void setTrackedState(const char* label, bool value) {
    int index = trackedIndexFor(label);
    if (index >= 0) trackedStates[index].value = value;
}

// ——— Corrected Mission Handler ———
void onAircraftName(char* str) {
    if (str != nullptr && str[0] != '\0') {
        char buf[128];
        snprintf(buf, sizeof(buf), "[MISSION START] %s.\n", str);
        debugPrintf("%s", buf);

        if (mySniffer.isStreamAlive()) initializePanels();
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
    setTrackedState(label, value > 0);

    const char* stateStr;
    if (strstr(label, "_COVER")) {
        stateStr = (value > 0) ? "OPEN" : "CLOSED";
    } else {
        stateStr = (value > 0) ? "ON" : "OFF";
    }

    debugPrintf("[STATE UPDATE] %s = %s\n", label, stateStr);
}

#if IS_REPLAY
#include "ReplayData/DcsbiosReplayData.h"
void DcsbiosProtocolReplay() {
    debugPrintln("\n[REPLAY PROTOCOL] 🔁 Playing from PROGMEM binary blob...");

    const uint8_t* ptr = dcsbiosReplayData;
    const uint8_t* end = dcsbiosReplayData + dcsbiosReplayLength;

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
            DcsBios::loop();               // ✅ Loop after each byte
            // delayMicroseconds(1);          // simulate serial pace
        }
        ptr += len;

        DcsBios::loop();                   // catch final updates

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

void DCSBIOS_init() {

    #if DEBUG_PERFORMANCE
    initPerfMonitor(); // this is used for profiling, see debugPrint for details
    #endif

    DcsBios::setup();
}

void DCSBIOS_loop() {
  #if DEBUG_PERFORMANCE
    beginProfiling("DCS-BIOS Loop");
  #endif

  DcsBios::loop();

  #if DEBUG_PERFORMANCE
    endProfiling("DCS-BIOS Loop");
  #endif

  #if DEBUG_PERFORMANCE
    perfMonitorUpdate();
  #endif
}

void sendDCSBIOSCommand(const char* label, uint16_t value) {

    char valueStr[10];
    snprintf(valueStr, sizeof(valueStr), "%u", value);

    // If DTR was asserted + terminal is connected we send the DCS Command. Also if the stream is Alive we send commands
    // The idea is that we'll NEVER call DcsBios::sendDcsBiosMessage without making sure it can receive our data
    if (tud_cdc_connected() || mySniffer.isStreamAlive()) {
        DcsBios::sendDcsBiosMessage(label, valueStr);
        delay(10); // Experiment to see which value allows crazy transitions.
        debugPrintf("🛩️ DCS Command Sent: %s %s\n", label, valueStr);
        return;
    }

    #if VERBOSE_PERFORMANCE_ONLY
        wifiDebugPrintf("⚠️ [NOT CONNECTED] Command suppressed: %s %s\n", label, valueStr);
    #else
        debugPrintf("⚠️ [NOT CONNECTED] Command suppressed: %s %s\n", label, valueStr);
    #endif
}