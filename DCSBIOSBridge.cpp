#define DCSBIOS_DISABLE_SERVO
#define DCSBIOS_ESP32_CDC_SERIAL
#include <DcsBios.h>
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

// Used to track cockpit cover states for the hornet.
namespace cover {
    volatile bool gain_switch      = false;
    volatile bool gen_tie          = false;
    volatile bool left_eng_fire    = false;
    volatile bool right_eng_fire   = false;
    volatile bool spin_recovery    = false;
}

struct PendingUpdate {
    const char* label;
    uint16_t value;
    uint16_t max_value;
};

// ——— Configuration ———
#define MAX_PENDING_UPDATES 220

class DcsBiosSniffer : public DcsBios::ExportStreamListener {
public:
    DcsBiosSniffer() : DcsBios::ExportStreamListener(0x7400, 0x77FF), pendingUpdateCount(0), pendingUpdateOverflow(0) {}

    inline uint32_t dcsHash(uint16_t addr, uint16_t mask, uint8_t shift) {
        return ((uint32_t)addr << 16) ^ (uint32_t)mask ^ ((uint32_t)shift << 1);
    }

    void onDcsBiosWrite(unsigned int addr, unsigned int value) override {
        #if DEBUG_PERFORMANCE
        beginProfiling("Listener");
        #endif

        static std::unordered_map<const char*, uint16_t> prev;
        auto it = addressToEntries.find(addr);
        if (it == addressToEntries.end()) {
            #if DEBUG_PERFORMANCE
            endProfiling("Listener");
            #endif
            return;
        }

        for (const DcsOutputEntry* entry : it->second) {
            uint16_t val = (value & entry->mask) >> entry->shift;
            if (prev[entry->label] == val) continue;
            prev[entry->label] = val;

            if (pendingUpdateCount < MAX_PENDING_UPDATES) {
                pendingUpdates[pendingUpdateCount++] = {entry->label, val, entry->max_value};
            } else {
                pendingUpdateOverflow++; // 🔥 Log overflow event
            }
        }

        #if DEBUG_PERFORMANCE
        endProfiling("Listener");
        #endif
    }

    void onConsistentData() override {
        for (uint16_t i = 0; i < pendingUpdateCount; ++i) {
            const PendingUpdate& u = pendingUpdates[i];
            onLedChange(u.label, u.value, u.max_value);
        }
        pendingUpdateCount = 0; // Clear buffer for next frame

        // Optional: Alert if we dropped any pending updates
        if (pendingUpdateOverflow > 0) {
            char buf[128];
            snprintf(buf, sizeof(buf), "[WARNING] ⚠️ %lu pending updates were DROPPED due to overflow!", pendingUpdateOverflow);
            #if VERBOSE_PERFORMANCE_ONLY
                wifiDebugPrint(buf);
            #else
                debugPrint(buf);
            #endif
            pendingUpdateOverflow = 0; // reset counter after reporting
        }
    }

private:
    PendingUpdate pendingUpdates[MAX_PENDING_UPDATES];
    uint16_t pendingUpdateCount;
    uint32_t pendingUpdateOverflow;
};
DcsBiosSniffer mySniffer;

/*
class DcsBiosSniffer : public DcsBios::ExportStreamListener {
public:
    DcsBiosSniffer() : DcsBios::ExportStreamListener(0x7400, 0x77FF) {
        pendingUpdates.reserve(180);  // Safe headroom for maximum DCS-BIOS frame load
    }

    inline uint32_t dcsHash(uint16_t addr, uint16_t mask, uint8_t shift) {
        return ((uint32_t)addr << 16) ^ (uint32_t)mask ^ ((uint32_t)shift << 1);
    }

    void onDcsBiosWrite(unsigned int addr, unsigned int value) override {

        #if DEBUG_PERFORMANCE
        beginProfiling("Listener");
        #endif

        static std::unordered_map<const char*, uint16_t> prev;
        auto it = addressToEntries.find(addr);
        if (it == addressToEntries.end()) {
            #if DEBUG_PERFORMANCE
            endProfiling("Listener");
            #endif
            return;
        }

        for (const DcsOutputEntry* entry : it->second) {
            uint16_t val = (value & entry->mask) >> entry->shift;

            if (prev[entry->label] == val) continue;
            prev[entry->label] = val;

            // Instead of immediate onLedChange, we queue
            pendingUpdates.push_back({entry->label, val, entry->max_value});
        }

        #if DEBUG_PERFORMANCE
        endProfiling("Listener");
        #endif
    }

    // Queue updates and update in one go.
    void onConsistentData() override {
        for (const PendingUpdate& u : pendingUpdates) {
            onLedChange(u.label, u.value, u.max_value);
        }
        pendingUpdates.clear();
    }

private:
    std::vector<PendingUpdate> pendingUpdates;
};
DcsBiosSniffer mySniffer;
*/

/*
class DcsBiosSniffer : public DcsBios::ExportStreamListener {
public:
    DcsBiosSniffer() : DcsBios::ExportStreamListener(0x7400, 0x77FF) {}

    inline uint32_t dcsHash(uint16_t addr, uint16_t mask, uint8_t shift) {
        return ((uint32_t)addr << 16) ^ (uint32_t)mask ^ ((uint32_t)shift << 1);
    }

    void onDcsBiosWrite(unsigned int addr, unsigned int value) override {

        #if DEBUG_PERFORMANCE
        beginProfiling("onDcsBiosWrite");
        #endif

        static std::unordered_map<const char*, uint16_t> prev;

        auto it = addressToEntries.find(addr);
        if (it == addressToEntries.end()) return;

        for (const DcsOutputEntry* entry : it->second) {
            uint16_t val = (value & entry->mask) >> entry->shift;

            if (prev[entry->label] == val) continue;
            prev[entry->label] = val;

            onLedChange(entry->label, val, entry->max_value);
        }

        #if DEBUG_PERFORMANCE
        endProfiling("onDcsBiosWrite");
        #endif

    }
    
};
DcsBiosSniffer mySniffer;
*/

// This is to determine mission start and properly initialize cockpit and syncronize with our hardware.
void onAircraftName(char* str) {

    char buf[128];
    snprintf(buf, sizeof(buf), "[AIRCRAFT] %s.\n", str);
    debugPrintf("[AIRCRAFT] %s.\n", str);

    // Initialize panel switches
    if (hasIR) IRCool_init();
    if (hasECM) ECM_init();
    if (hasMasterARM) MasterARM_init();
}
// Get MetaData to init cockpit state
DcsBios::StringBuffer<24> aicraftName(0x0000, onAircraftName); // Its safe to do StringBuffer and Integer buffer for addresses OUTSIDE our listener, we do so for MetaData

/*
// Cover state callbacks (bitmask logic made explicit and consistent)
void onGainSwitchCover(unsigned int val)     { cover::gain_switch     = (val & 0x8000) != 0; }
void onGenTieCover(unsigned int val)         { cover::gen_tie         = (val & 0x1000) != 0; }
void onLeftFireCover(unsigned int val)       { cover::left_eng_fire   = (val & 0x0100) != 0; }
void onRightFireCover(unsigned int val)      { cover::right_eng_fire  = (val & 0x0040) != 0; }
void onSpinRecoveryCover(unsigned int val)   { cover::spin_recovery   = (val & 0x1000) != 0; }

// IntegerBuffers (addr, mask, shift, callback)
DcsBios::IntegerBuffer gainSwitchCoverBuf    (0x74F4, 0x8000, 15, onGainSwitchCover);
DcsBios::IntegerBuffer genTieCoverBuf        (0x74F2, 0x1000, 12, onGenTieCover);
DcsBios::IntegerBuffer leftFireCoverBuf      (0x73F8, 0x0100,  8, onLeftFireCover);
DcsBios::IntegerBuffer rightFireCoverBuf     (0x73FC, 0x0040,  6, onRightFireCover);
DcsBios::IntegerBuffer spinRecoveryCoverBuf  (0x74FA, 0x1000, 12, onSpinRecoveryCover);
*/

//////////////////////////////////////////////////////////////////////////////////////
//
//          Example to check cover state
//
//          gain_switch
//          gen_tie
//          left_eng_fire
//          right_eng_fire
//          spin_recovery
//
//          if (cover::left_eng_fire) {
//              sendDCSBIOSCommand("FIRE_EXT", 1);
//          } else {
//              debugPrintln("🔥 BLOCKED: FIRE_EXT — cover closed");
//          }
//
/////////////////////////////////////////////////////////////////////////////////////

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
            debugPrintln(buf);
        } else {
            if (intensity > 97) intensity = 100;
            setLED(label, true, intensity);
            char buf[128];
            snprintf(buf, sizeof(buf), "[LED] %s Intensity %u\%.", label, value);
            debugPrintln(buf);
        }
    }
}

void onSelectorChange(const char* label, unsigned int value) {
    // HIDManager_setNamedButton(label, false, value);
}

#if IS_REPLAY
#include "ReplayData/DcsbiosReplayData.h"
void DcsbiosProtocolReplay() {
    debugPrintln("\n[REPLAY PROTOCOL] 🔁 Playing from PROGMEM binary blob...");

    const uint8_t* ptr = dcsbiosReplayData;
    const uint8_t* end = dcsbiosReplayData + dcsbiosReplayLength;

    while (ptr < end) {

        float frameDelay;
        memcpy_P(&frameDelay, ptr, sizeof(float));
        ptr += sizeof(float);

        uint16_t len = pgm_read_byte(ptr) | (pgm_read_byte(ptr + 1) << 8);
        ptr += 2;

        for (uint16_t i = 0; i < len; i++) {
            uint8_t b = pgm_read_byte(ptr + i);
            DcsBios::parser.processChar(b);
            DcsBios::loop();               // ✅ Loop after each byte
            delayMicroseconds(1);          // simulate serial pace
        }
        ptr += len;

        DcsBios::loop();                   // catch final updates
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
    debugPrintln("\nDCSBIOS Library Initialization Complete.");

    #if IS_REPLAY
    // Begin simulated loop
    DcsbiosProtocolReplay();
    #endif
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
    // Any conditional logic before sending a command to DCS should go here (e.g. cover checks)

    char valueStr[10];
    snprintf(valueStr, sizeof(valueStr), "%u", value); // safely format value into a string

    // DcsBios::tryToSendDcsBiosMessage(label, valueStr);
    DcsBios::sendDcsBiosMessage(label, valueStr);

    #if VERBOSE_PERFORMANCE_ONLY
        wifiDebugPrintf("🛩️ DCS Command Sent: %s %s\n", label, valueStr);
    #else
        debugPrintf("🛩️ DCS Command Sent: %s %s\n", label, valueStr);
    #endif
}