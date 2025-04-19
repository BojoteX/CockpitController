#include <DcsBios.h>
#include "src/DCSBIOSBridge.h"
#include "src/LABELS/DCSBIOSBridgeData.h"
#include "src/LEDControl.h"
#include "src/HIDManager.h"
#include "src/Globals.h"
#include "Config.h"
#include <unordered_map>

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#endif

class DcsBiosSniffer : public DcsBios::ExportStreamListener {
public:
    DcsBiosSniffer() : DcsBios::ExportStreamListener(0x7400, 0x77FF) {}

    inline uint32_t dcsHash(uint16_t addr, uint16_t mask, uint8_t shift) {
        return ((uint32_t)addr << 16) ^ (uint32_t)mask ^ ((uint32_t)shift << 1);
    }

    void onDcsBiosWrite(unsigned int addr, unsigned int value) override {
        static std::unordered_map<const char*, uint16_t> prev;

        auto it = addressToEntries.find(addr);
        if (it == addressToEntries.end()) return;

        for (const DcsOutputEntry* entry : it->second) {
            uint16_t val = (value & entry->mask) >> entry->shift;

            if (prev[entry->label] == val) continue;
            prev[entry->label] = val;

            onLedChange(entry->label, val, entry->max_value);
        }
    }
    
};
DcsBiosSniffer mySniffer;

void onLedChange(const char* label, uint16_t value, uint16_t max_value) {
    if (max_value <= 1) {
        setLED(label, value > 0);
        #if DEBUG_USE_WIFI
        char buf[128];
        snprintf(buf, sizeof(buf), "[LED] %s is set to %u", label, value);
        sendDebug(buf);        
        #endif
    } else {
        uint8_t intensity = (value * 100UL) / max_value;
        if (intensity < 3) {
            setLED(label, false, 0);  // treat as OFF
            #if DEBUG_USE_WIFI
            char buf[128];
            snprintf(buf, sizeof(buf), "[LED] %s Intensity was set to 0", label);
            sendDebug(buf);        
            #endif
        } else {
            if (intensity > 97) intensity = 100;
            setLED(label, true, intensity);
            #if DEBUG_USE_WIFI
            char buf[128];
            snprintf(buf, sizeof(buf), "[LED] %s Intensity %u\%.", label, value);
            sendDebug(buf);        
            #endif
        }
    }
}

void onSelectorChange(const char* label, unsigned int value) {
    // HIDManager_setNamedButton(label, false, value);
}

void DCSBIOS_init() {
    DcsBios::setup();

    #if DEBUG_USE_WIFI
    wifi_setup();
    #endif
}

void DCSBIOS_loop() {
    DcsBios::loop();
}

// Get MetaData to init cockpit state
DcsBios::StringBuffer<24> aicraftName(0x0000, onAircraftName); 
void onAircraftName(char* str) {

    #if DEBUG_USE_WIFI
    char buf[128];
    snprintf(buf, sizeof(buf), "[AIRCRAFT] %s.", str);
    sendDebug(buf);
    #else
    debugPrintf("[AIRCRAFT] %s.", str);
    #endif

    // Initialize panel switches
    if (hasIR) IRCool_init();
    if (hasECM) ECM_init();
    if (hasMasterARM) MasterARM_init();
}

void sendDCSBIOSCommand(const char* label, uint16_t value) {
    DcsBios::sendDcsBiosMessage(label, String(value).c_str());

    #if DEBUG_USE_WIFI
    char buf[128];
    snprintf(buf, sizeof(buf), "[DCS-MODE] %s %u", label, value);
    sendDebug(buf);
    #endif

}