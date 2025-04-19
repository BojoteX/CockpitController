#include <DcsBios.h>
#include "src/DCSBIOSBridge.h"
#include "src/LABELS/DCSBIOSBridgeData.h"
#include "src/LEDControl.h"
#include "src/HIDManager.h"
#include "src/debugPrint.h"

#include <unordered_map>

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
    } else {
        uint8_t intensity = (value * 100UL) / max_value;
        if (intensity < 10) {
            setLED(label, false);  // treat as OFF
        } else {
            if (intensity > 90) intensity = 100;
            setLED(label, true, intensity);
        }
    }
}

void onSelectorChange(const char* label, unsigned int value) {
    // HIDManager_setNamedButton(label, false, value);
}

void DCSBIOS_init() {
    DcsBios::setup();
}

void DCSBIOS_loop() {
    DcsBios::loop();
}

void sendDCSBIOSCommand(const char* label, uint16_t value) {
    DcsBios::sendDcsBiosMessage(label, String(value).c_str());
}