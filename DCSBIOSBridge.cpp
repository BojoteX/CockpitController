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
        static std::unordered_map<uint32_t, uint8_t> prev;

        for (uint8_t shift = 0; shift < 16; ++shift) {
            uint16_t mask = (1 << shift);
            uint8_t val = (value & mask) ? 1 : 0;

            uint32_t key = dcsHash(addr, mask, shift);
            auto it = DcsOutputHashTable.find(key);
            if (it == DcsOutputHashTable.end()) continue;

            if (prev[key] == val) continue;
            prev[key] = val;

            for (const char* label : it->second) {
                debugPrintf("[MATCH] Label = %s → value = %d\n", label, val);
                onLedChange(label, val);
            }
        }
    }
};

DcsBiosSniffer mySniffer;

void onLedChange(const char* label, unsigned int value) {
    if (value == 1) {
        setLED(label, true);
    } else if (value == 0) {
        setLED(label, false);
    } else {
        uint8_t intensity = value * 100 / 65535;
        setLED(label, true, intensity);
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