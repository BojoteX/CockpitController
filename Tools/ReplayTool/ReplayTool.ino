// Replay & Live Debug Script by Bojote

#include <DcsBios.h>
#include "ReplayData/DCSBIOSBridgeData.h"
#include "Config.h" // use it to set IS_REPLAY or WIFI

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
class TeeDebugStream : public Print {
public:
  String buffer;

  size_t write(uint8_t c) override {
    Serial.write(c);  // still send to USB
    buffer += (char)c;
    if (c == '\n') {
      sendDebug(buffer.c_str());
      buffer = "";
    }
    return 1;
  }

  size_t write(const uint8_t* buf, size_t size) override {
    Serial.write(buf, size);  // still send to USB
    for (size_t i = 0; i < size; ++i) write(buf[i]);
    return size;
  }
};

TeeDebugStream debugStream;
#define Serial debugStream
#endif

// -- Serial Configuration --
#define BAUD_RATE 250000
#define SERIAL_STARTUP_DELAY 3000

void setLED(const char* label, bool state, uint8_t intensity = 100) {
    if (state && intensity != 100) {
        Serial.printf("[LED] %s → ON (%u%%)\n", label, intensity);
    } else if (state) {
        Serial.printf("[LED] %s → ON\n", label);
    } else {
        Serial.printf("[LED] %s → OFF\n", label);
    }
}

void onLedChange(const char* label, uint16_t value, uint16_t max_value) {
    if (max_value <= 1) {
        setLED(label, value > 0);
    } else {
        uint8_t intensity = (value * 100UL) / max_value;
        if (intensity < 10) {
            setLED(label, false, 0);
        } else {
            if (intensity > 90) intensity = 100;
            setLED(label, true, intensity);
        }
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

// Get MetaData to init cockpit state
DcsBios::StringBuffer<24> aicraftName(0x0000, onAircraftName);  // or whatever you're currently using
void onAircraftName(char* str) {
    Serial.printf("[AIRCRAFT] %s.", str);
}

#if IS_REPLAY
#include "ReplayData/DcsbiosReplayData.h"
void DcsbiosProtocolReplay() {
    Serial.println("\n[REPLAY PROTOCOL] Injecting binary data into DCS-BIOS parser...");
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
            delayMicroseconds(1);
            DcsBios::loop();
        }
    }
    Serial.println("[REPLAY PROTOCOL] Completed DCS-BIOS parser injection.\n");
}
#endif

void setup() {
    #if !DEBUG_USE_WIFI
        Serial.begin(BAUD_RATE); 
        unsigned long start = millis();
        while (!Serial && (millis() - start < SERIAL_STARTUP_DELAY)) delay(1);
        Serial.println("READY");
    #endif

    #if DEBUG_USE_WIFI
    wifi_setup();
    #endif

    DcsBios::setup();

    #if IS_REPLAY
    // Begin simulated loop
    DcsbiosProtocolReplay();
    #endif
}

void loop() {
    #if !IS_REPLAY
    DcsBios::loop();
    #endif
}