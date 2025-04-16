#define DCSBIOS_DEFAULT_SERIAL
#define DCSBIOS_DISABLE_SERVO
#include <DcsBios.h>
#include <Arduino.h>
#include "ReplayData/DcsbiosReplayData.h"
#include "ReplayData/DCSBIOSBridgeData.h"
#include <map>
#include <unordered_map>

// -- Serial Configuration --
#define BAUD_RATE 250000
#define SERIAL_STARTUP_DELAY 3000      // Delay (ms) allowing Serial Monitor to connect

// Tracks previous bit state per label
std::map<String, uint8_t> ledReplayState;

void onLedChange(const char* label, unsigned int value) {
    if (value == 1) {
        setLED(label, true, 100);
    } else if (value == 0) {
        setLED(label, false, 100);
    } else {
        uint8_t intensity = value * 100 / 65535;
        setLED(label, true, intensity);
    }
}

void onSelectorChange(const char* label, unsigned int value) {
    // HIDManager_setNamedButton(label, false, value);
}

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
                Serial.printf("[MATCH] Label = %s → value = %d\n", label, val);
                onLedChange(label, val);
            }
        }
    }
};

DcsBiosSniffer mySniffer;

void setLED(const char* label, unsigned int value, unsigned int intensity) {
    // Serial.printf("You debug");
}

void DcsbiosProtocolReplay() {
    Serial.println("\n[REPLAY PROTOCOL] Injecting binary data into DCS-BIOS parser...");
    delay(3000);

    for (size_t i = 0; i < replayFrameCount; ++i) {
        const auto& frame = replayFrames[i];
        delay((unsigned long)(frame.delay * 100));

        uint8_t buffer[10] = {
            0x55, 0x55, 0x55, 0x55,
            frame.address & 0xFF, (frame.address >> 8) & 0xFF,
            0x02, 0x00,
            frame.data & 0xFF, (frame.data >> 8) & 0xFF
        };

        for (uint8_t b : buffer) {
            DcsBios::parser.processChar(b);
            DcsBios::loop();
            delayMicroseconds(10);
        }

        DcsBios::loop();
    }

    Serial.println("[REPLAY PROTOCOL] Completed DCS-BIOS parser injection.\n");
}

void setup() {
    DcsBios::setup();

  // Initialize serial
  Serial.begin(BAUD_RATE); 
  unsigned long start = millis();
  while (!Serial && (millis() - start < SERIAL_STARTUP_DELAY)) delay(1);

  DcsbiosProtocolReplay();
}

void loop() {
   DcsBios::loop();
}