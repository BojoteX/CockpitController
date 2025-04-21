// Mutex for PCA9555 I²C access
portMUX_TYPE pca9555_mux = portMUX_INITIALIZER_UNLOCKED;

bool loggingEnabled = false;

// static const char* resolveInputLabel(uint8_t addr, uint8_t port, uint8_t bit);
static const char* resolveInputLabel(uint8_t addr, uint8_t port, uint8_t bit) {
  char deviceName[20];
  snprintf(deviceName, sizeof(deviceName), "PCA_0x%02X", addr);
  for (size_t i = 0; i < InputMappingSize; ++i) {
    const InputMapping& m = InputMappings[i];
    if (!m.source) continue;
    if (strcmp(m.source, deviceName) == 0 && m.port == port && m.bit == bit) {
      return m.label;
    }
  }
  return nullptr;
}

static bool isInputBitMapped(uint8_t addr, uint8_t port, uint8_t bit) {
  return resolveInputLabel(addr, port, bit) != nullptr;
}

void PCA9555_setAllLEDs(bool state) {
  for (int i = 0; i < panelLEDsCount; i++) {
    if (panelLEDs[i].deviceType == DEVICE_PCA9555) {
      bool writeState = panelLEDs[i].activeLow ? !state : state;
      PCA9555_write(panelLEDs[i].info.pcaInfo.address,
                    panelLEDs[i].info.pcaInfo.port,
                    panelLEDs[i].info.pcaInfo.bit,
                    writeState);
    }
  }
}

// PCA9555 Helper Implementation
void PCA9555_allLEDsByAddress(uint8_t addr, bool state) {
    for (int i = 0; i < panelLEDsCount; i++) {
        if (panelLEDs[i].deviceType == DEVICE_PCA9555 &&
            panelLEDs[i].info.pcaInfo.address == addr) {
            bool writeState = panelLEDs[i].activeLow ? !state : state;
            PCA9555_write(addr,
                          panelLEDs[i].info.pcaInfo.port,
                          panelLEDs[i].info.pcaInfo.bit,
                          writeState);
        }
    }
}

// Turn ON all PCA9555 LEDs at a specific address
void PCA9555_allOn(uint8_t addr) {
  debugPrintf("🔆 PCA9555 (0x%02X) Turning ALL LEDs ON\n", addr);
  for (int i = 0; i < panelLEDsCount; i++) {
    if (panelLEDs[i].deviceType == DEVICE_PCA9555 && panelLEDs[i].info.pcaInfo.address == addr) {
      setLED(panelLEDs[i].label, true, 100);
    }
  }
}

// Turn OFF all PCA9555 LEDs at a specific address
void PCA9555_allOff(uint8_t addr) {
  debugPrintf("⚫ PCA9555 (0x%02X) Turning ALL LEDs OFF\n", addr);
  for (int i = 0; i < panelLEDsCount; i++) {
    if (panelLEDs[i].deviceType == DEVICE_PCA9555 && panelLEDs[i].info.pcaInfo.address == addr) {
      setLED(panelLEDs[i].label, false, 0);
    }
  }
}

// Sweep through all PCA9555 LEDs at a specific address
void PCA9555_sweep(uint8_t addr) {
  debugPrintf("🔍 PCA9555 (0x%02X) LED Sweep Start\n", addr);
  for (int i = 0; i < panelLEDsCount; i++) {
    if (panelLEDs[i].deviceType == DEVICE_PCA9555 && panelLEDs[i].info.pcaInfo.address == addr) {
      debugPrint("🟢 Sweeping LED: ");
      debugPrintln(panelLEDs[i].label);
      setLED(panelLEDs[i].label, true, 100);
      delay(500);
      setLED(panelLEDs[i].label, false, 0);
    }
  }
  debugPrintf("✅ PCA9555 (0x%02X) LED Sweep Complete\n", addr);
}

// Run full PCA9555 test pattern for a single device (All OFF → Sweep → All ON → OFF)
void PCA9555_patternTesting(uint8_t addr) {
  debugPrintf("🧪 PCA9555 (0x%02X) Test Pattern Start\n", addr);
  PCA9555_allOff(addr);
  PCA9555_allOn(addr);
  delay(3000);
  PCA9555_allOff(addr);
  debugPrintf("✅ PCA9555 (0x%02X) Test Pattern Complete\n", addr);
}

// Máximo 8 PCA9555 conectados
static byte prevPort0Cache[8];
static byte prevPort1Cache[8];
static uint8_t addrCache[8];
static uint8_t cacheSize = 0;

void PCA9555_autoInitFromLEDMap(uint8_t addr) {
    uint8_t configPort0 = 0xFF; // Initially, all INPUTS
    uint8_t configPort1 = 0xFF; // Initially, all INPUTS
    uint8_t outputPort0 = 0xFF; // Initially all HIGH (OFF state)
    uint8_t outputPort1 = 0xFF; // Initially all HIGH (OFF state)

    for (int i = 0; i < panelLEDsCount; i++) {
        if (panelLEDs[i].deviceType == DEVICE_PCA9555 && panelLEDs[i].info.pcaInfo.address == addr) {
            if (panelLEDs[i].info.pcaInfo.port == 0) {
                configPort0 &= ~(1 << panelLEDs[i].info.pcaInfo.bit); // set as OUTPUT
                outputPort0 |= (1 << panelLEDs[i].info.pcaInfo.bit);  // explicitly HIGH (LED OFF initially)
            } else {
                configPort1 &= ~(1 << panelLEDs[i].info.pcaInfo.bit); // set as OUTPUT
                outputPort1 |= (1 << panelLEDs[i].info.pcaInfo.bit);  // explicitly HIGH (LED OFF initially)
            }
        }
    }

    // Set initial OUTPUT STATE FIRST
    Wire.beginTransmission(addr);
    Wire.write(0x02); // Output port register PORT0
    Wire.write(outputPort0); // Port0 initially HIGH
    Wire.write(outputPort1); // Port1 initially HIGH
    Wire.endTransmission();

    // Now set pins as OUTPUT
    Wire.beginTransmission(addr);
    Wire.write(0x06); // Configuration register PORT0
    Wire.write(configPort0); // set directions for PORT0
    Wire.write(configPort1); // set directions for PORT1
    Wire.endTransmission();
}

void PCA9555_write(uint8_t addr, uint8_t port, uint8_t bit, bool state) {
    uint8_t dataToSend;

    // ONLY protect the cache access
    portENTER_CRITICAL(&pca9555_mux);
    if (state)
        PCA9555_cachedPortStates[addr][port] |= (1 << bit);
    else
        PCA9555_cachedPortStates[addr][port] &= ~(1 << bit);

    dataToSend = PCA9555_cachedPortStates[addr][port];
    portEXIT_CRITICAL(&pca9555_mux);

    // Perform I²C operation outside the critical section
    uint8_t reg = (port == 0) ? 0x02 : 0x03;

    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(dataToSend);
    Wire.endTransmission();
}

void initPCA9555AsInput(uint8_t addr) {
    uint8_t configPort0 = 0xFF; // All inputs initially
    uint8_t configPort1 = 0xFF; // All inputs initially

    // EXCLUDE any LED pin clearly (set as outputs later)
    for (int i = 0; i < panelLEDsCount; i++) {
        if (panelLEDs[i].deviceType == DEVICE_PCA9555 && panelLEDs[i].info.pcaInfo.address == addr) {
            if (panelLEDs[i].info.pcaInfo.port == 0) {
                configPort0 &= ~(1 << panelLEDs[i].info.pcaInfo.bit); // exclude LED pins (output)
            } else {
                configPort1 &= ~(1 << panelLEDs[i].info.pcaInfo.bit); // exclude LED pins (output)
            }
        }
    }

    Wire.beginTransmission(addr);
    Wire.write(0x06); // Configuration register PORT0
    Wire.write(configPort0); // Set directions for PORT0
    Wire.write(configPort1); // Set directions for PORT1
    Wire.endTransmission();
}

int getCacheIndex(uint8_t address) {
  for (uint8_t i = 0; i < cacheSize; i++) {
    if (addrCache[i] == address) return i;
  }
  return -1;
}

bool shouldLogChange(uint8_t address, byte port0, byte port1) {
  int idx = getCacheIndex(address);
  if (idx >= 0) {
    return (prevPort0Cache[idx] != port0 || prevPort1Cache[idx] != port1);
  }

  // Nuevo PCA detectado
  if (cacheSize < 8) {
    addrCache[cacheSize] = address;
    prevPort0Cache[cacheSize] = port0;
    prevPort1Cache[cacheSize] = port1;
    cacheSize++;
    return true;
  }

  return false;
}

// Corrected readPCA9555 function
bool readPCA9555(uint8_t address, byte &port0, byte &port1) {
    byte tmpPort0, tmpPort1;

    Wire.beginTransmission(address);
    Wire.write(0x00);  // Port 0 input register
    if (Wire.endTransmission(false) == 0 && Wire.requestFrom(address, 2) == 2) {
        tmpPort0 = Wire.read();
        tmpPort1 = Wire.read();

        port0 = tmpPort0;
        port1 = tmpPort1;

        // NOW CALL LOGGER IF CHANGED
        if (isPCA9555LoggingEnabled() && shouldLogChange(address, port0, port1)) {
            logPCA9555State(address, port0, port1);
        }

        return true;
    }
    return false;
}

inline const char* resolvePanelName(const char* source) {
  if (strstr(source, "0x22")) return "ECM";
  if (strstr(source, "0x26")) return "IR COOL";
  if (strstr(source, "0x5B")) return "Master ARM";
  return "UNKNOWN_PANEL";
}

void enablePCA9555Logging(bool enable) {
  loggingEnabled = enable;
}

bool isPCA9555LoggingEnabled() {
  return loggingEnabled;
}

void logExpanderState(uint8_t p0, uint8_t p1) {
  Serial.print(F(" → [ p0:"));
  for (int8_t b = 6; b >= 0; --b) {
    Serial.print((p0 >> b) & 1);
  }
  Serial.print(F(" | p1:"));
  for (int8_t b = 6; b >= 0; --b) {
    Serial.print((p1 >> b) & 1);
  }
  Serial.print(" ]");
}

void logPCA9555State(uint8_t address, byte port0, byte port1) {
  int idx = getCacheIndex(address);
  if (idx < 0) return;

  byte prev0 = prevPort0Cache[idx];
  byte prev1 = prevPort1Cache[idx];

  bool printedNewLine = false;

  for (int b = 0; b < 8; b++) {
    if (bitRead(prev0, b) != bitRead(port0, b)) {
      const char* label = resolveInputLabel(address, 0, b);

      debugPrint("⚡PCA 0x");
      debugPrint(address, HEX);
      logExpanderState(port0, port1);
      debugPrint(" ");

      if (!label) {
        debugPrint("→ Port0 Bit ");
        debugPrint(b);
        debugPrint(" ");
        debugPrint(bitRead(port0, b) ? "HIGH → " : "LOW → ");
        debugPrint("❌ Not mapped");
      } else {
        debugPrint("⚠️ CRITICAL LOGIC ERROR: Bit ");
        debugPrint(b);
        debugPrint(" → ");
        debugPrint(label);
        debugPrint(" on PCA 0x");
        debugPrint(address, HEX);
        debugPrint(" is mapped but triggered no action 🤖💥");
      }

      printedNewLine = true;
    }
  }

  for (int b = 0; b < 8; b++) {
    if (bitRead(prev1, b) != bitRead(port1, b)) {
      const char* label = resolveInputLabel(address, 1, b);

      if (printedNewLine) debugPrintln("");

      debugPrint("⚡PCA 0x");
      debugPrint(address, HEX);
      logExpanderState(port0, port1);
      debugPrint(" ");

      if (!label) {
        debugPrint("→ Port1 Bit ");
        debugPrint(b);
        debugPrint(" ");
        debugPrint(bitRead(port1, b) ? "HIGH → " : "LOW → ");
        debugPrint("❌ Not mapped");
      } else {
        debugPrint("⚠️ CRITICAL LOGIC ERROR: Bit ");
        debugPrint(b);
        debugPrint(" → ");
        debugPrint(label);
        debugPrint(" on PCA 0x");
        debugPrint(address, HEX);
        debugPrint(" is mapped but triggered no action 🤖💥");
      }

      printedNewLine = true;
    }
  }

  debugPrintln("");

  // 🔄 Update cache
  prevPort0Cache[idx] = port0;
  prevPort1Cache[idx] = port1;
}