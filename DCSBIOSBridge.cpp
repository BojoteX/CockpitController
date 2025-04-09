#include "src/DCSBIOSBridge.h"
#include "src/LEDControl.h"
#include "src/Mappings.h"
#include "src/Globals.h"

// ------------------ Output Mapping Lookup ------------------
static std::unordered_map<uint16_t, std::vector<const DcsOutputMapping*>> outputMap;

// ------------------ Parser State Machine ------------------
enum ParserState {
  WAIT_FOR_SYNC,
  ADDRESS_LOW, ADDRESS_HIGH,
  COUNT_LOW, COUNT_HIGH,
  DATA_LOW, DATA_HIGH
};

static ParserState parserState = WAIT_FOR_SYNC;
static uint16_t currentAddress = 0;
static uint16_t remainingCount = 0;
static uint16_t currentData = 0;
static uint8_t syncCount = 0;

// ------------------ Frame Sync Hook ------------------
void onDcsBiosFrameSync() {
  // Optional: Add any end-of-frame logic here
}

// ------------------ Output Handler ------------------
void handleDcsBiosWrite(uint16_t addr, uint16_t value) {
  auto it = outputMap.find(addr);
  if (it == outputMap.end()) return;
  for (const DcsOutputMapping* entry : it->second) {
    bool on = (value & entry->mask) != 0;
    setLED(entry->ledLabel, on);
  }
}

// ------------------ Byte Parser ------------------
void processDcsBiosByte(uint8_t b) {
  switch (parserState) {
    case WAIT_FOR_SYNC: break;
    case ADDRESS_LOW: currentAddress = b; parserState = ADDRESS_HIGH; break;
    case ADDRESS_HIGH:
      currentAddress |= b << 8;
      if (currentAddress == 0x5555) {
        parserState = WAIT_FOR_SYNC;
      } else {
        parserState = COUNT_LOW;
      }
      break;
    case COUNT_LOW: remainingCount = b; parserState = COUNT_HIGH; break;
    case COUNT_HIGH: remainingCount |= b << 8; parserState = DATA_LOW; break;
    case DATA_LOW: currentData = b; remainingCount--; parserState = DATA_HIGH; break;
    case DATA_HIGH:
      currentData |= b << 8; remainingCount--;
      handleDcsBiosWrite(currentAddress, currentData);
      currentAddress += 2;
      parserState = (remainingCount > 0) ? DATA_LOW : ADDRESS_LOW;
      break;
  }

  if (b == 0x55) syncCount++;
  else syncCount = 0;

  if (syncCount == 4) {
    syncCount = 0;
    parserState = ADDRESS_LOW;
    onDcsBiosFrameSync();
  }
}

// ------------------ Input Sender ------------------
void DCSBIOS_sendCommandByLabel(const String& name, bool pressed) {
  for (size_t i = 0; i < numDCSInputs; ++i) {
    if (strcmp(DCSCommands[i].inputLabel, name.c_str()) == 0) {
      Serial.write(DCSCommands[i].command);
      Serial.write(" ");
      Serial.write(pressed ? DCSCommands[i].onValue : DCSCommands[i].offValue);
      Serial.write("\n");

      debugPrint("📡 [DCS-BIOS MODE] ");
      debugPrint(DCSCommands[i].command);
      debugPrint(" ");
      debugPrintln(pressed ? DCSCommands[i].onValue : DCSCommands[i].offValue);
      return;
    }
  }

  debugPrint("⚠️ [DCS-BIOS MODE] Unknown DCS input: ");
  debugPrintln(name);
}

// ------------------ Init ------------------
void DCSBIOSBridge_init() {
  Serial.begin(250000);
  while (!Serial) delay(1);

  for (size_t i = 0; i < numDCSOutputs; ++i) {
    const DcsOutputMapping* e = &DCSOutputs[i];
    outputMap[e->address].push_back(e);
  }
}

// ------------------ Main Loop ------------------
void DCSBIOSBridge_loop() {
  while (Serial.available()) {
    uint8_t b = Serial.read();
    processDcsBiosByte(b);
  }
}