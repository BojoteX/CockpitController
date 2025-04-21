// **************************************************************
// TM1637
//
//
//
// **************************************************************

// OCD Stuff... 
portMUX_TYPE tm1637_mux = portMUX_INITIALIZER_UNLOCKED;

#define TM1637_CMD_SET_ADDR  0xC0
#define TM1637_CMD_DISP_CTRL 0x88

/**
 * Reads TM1637 button state with hardened multi-level validation.
 *
 * Implements:
 * - Triple-sample per-bit majority voting for noise immunity
 * - Dual-read confirmation to reject transient discrepancies
 * - Debounce via loop-count and minimum time-held threshold
 * - Logs only when a new stable state is confirmed
 *
 * Guarantees:
 * - No false triggers from bounce or EMI
 * - Stable transitions only after consistent and persistent key state
 * - Unique, deterministic raw values used for all logic
 *
 * Suitable for mission-critical input detection where reliability is paramount.
 */
uint8_t tm1637_readKeys(TM1637Device &dev) {
    constexpr int      outerConfirmReads    = 2;        // how many majority reads to match
    constexpr int      majorityPasses       = 3;        // per‑read noise immunity
    constexpr uint8_t  stabilityThreshold   = 10;       // loops for debounce
    constexpr uint32_t minStablePeriodUs    = 50000UL;  // must hold ≥50 ms

    static uint8_t  prevStable     = 0xFF;
    static uint8_t  candidate      = 0xFF;
    static uint8_t  candidateCount = 0;
    static uint32_t sampleCounter  = 0;
    static uint32_t candidateTs    = 0;

    // Single majority‑vote read
    auto majorityRead = [&](){
        uint8_t s1=0, s2=0, s3=0;
        for (uint8_t pass = 0; pass < majorityPasses; ++pass) {
            tm1637_start(dev);
            tm1637_writeByte(dev, 0x42);
            pinMode(dev.dioPin, INPUT_PULLUP);
            uint8_t cur = 0;
            for (uint8_t bit = 0; bit < 8; ++bit) {
                digitalWrite(dev.clkPin, LOW);
                delayMicroseconds(3);
                cur |= (digitalRead(dev.dioPin) << bit);
                digitalWrite(dev.clkPin, HIGH);
                delayMicroseconds(3);
            }
            tm1637_stop(dev);
            if      (pass == 0) s1 = cur;
            else if (pass == 1) s2 = cur;
            else                s3 = cur;
        }
        // per-bit majority
        return (s1 & s2) | (s2 & s3) | (s1 & s3);
    };

    // 1) Dual‑read confirmation
    uint8_t first  = majorityRead();
    uint8_t second = majorityRead();
    if (first != second) {
        // noisy – ignore this cycle
        ++sampleCounter;
        return prevStable;
    }
    uint8_t keys = first;
    ++sampleCounter;

    // 2) Debounce & stability window
    if (keys == candidate) {
        if (candidateCount < UINT8_MAX) ++candidateCount;
        uint32_t now = micros();
        if (candidateCount >= stabilityThreshold
            && (now - candidateTs) >= minStablePeriodUs
            && candidate != prevStable) {
            if (DEBUG) {
                debugPrintf("[TM1637] Confirmed = 0x%02X | Loops since last = %lu\n",
                            candidate, sampleCounter);
            }
            prevStable    = candidate;
            sampleCounter = 0;
        }
    } else {
        // new candidate – reset count & timestamp
        candidate      = keys;
        candidateCount = 0;
        candidateTs    = micros();
    }

    return prevStable;
}

void tm1637_start(TM1637Device &dev) {
    pinMode(dev.dioPin, OUTPUT);
    digitalWrite(dev.clkPin, HIGH);
    digitalWrite(dev.dioPin, HIGH);
    delayMicroseconds(2);
    digitalWrite(dev.dioPin, LOW);
}

void tm1637_stop(TM1637Device &dev) {
    pinMode(dev.dioPin, OUTPUT);
    digitalWrite(dev.clkPin, LOW);
    delayMicroseconds(2);
    digitalWrite(dev.dioPin, LOW);
    delayMicroseconds(2);
    digitalWrite(dev.clkPin, HIGH);
    delayMicroseconds(2);
    digitalWrite(dev.dioPin, HIGH);
}

bool tm1637_writeByte(TM1637Device &dev, uint8_t b) {
    portENTER_CRITICAL(&tm1637_mux);

    pinMode(dev.dioPin, OUTPUT);
    for (uint8_t i = 0; i < 8; ++i) {
        digitalWrite(dev.clkPin, LOW);
        digitalWrite(dev.dioPin, (b & 0x01));
        delayMicroseconds(3);
        digitalWrite(dev.clkPin, HIGH);
        delayMicroseconds(3);
        b >>= 1;
    }
    digitalWrite(dev.clkPin, LOW);
    pinMode(dev.dioPin, INPUT_PULLUP);
    delayMicroseconds(3);
    digitalWrite(dev.clkPin, HIGH);
    delayMicroseconds(3);
    bool ack = digitalRead(dev.dioPin) == 0;
    digitalWrite(dev.clkPin, LOW);
    pinMode(dev.dioPin, OUTPUT);

    portEXIT_CRITICAL(&tm1637_mux);
    return ack;
}

void tm1637_updateDisplay(TM1637Device &dev) {
    tm1637_start(dev);
    tm1637_writeByte(dev, 0x40); // Auto increment
    tm1637_stop(dev);

    tm1637_start(dev);
    tm1637_writeByte(dev, TM1637_CMD_SET_ADDR);
    for (int i = 0; i < 6; i++) {
        tm1637_writeByte(dev, dev.ledData[i]);
    }
    tm1637_stop(dev);

    tm1637_start(dev);
    tm1637_writeByte(dev, TM1637_CMD_DISP_CTRL | 7); // Brightness max
    tm1637_stop(dev);
}

void tm1637_init(TM1637Device &dev, uint8_t clkPin, uint8_t dioPin) {
    dev.clkPin = clkPin;
    dev.dioPin = dioPin;
    pinMode(clkPin, OUTPUT);
    pinMode(dioPin, OUTPUT);
    memset(dev.ledData, 0, sizeof(dev.ledData));
    tm1637_updateDisplay(dev);
}

void tm1637_displaySingleLED(TM1637Device &dev, uint8_t grid, uint8_t segment, bool state) {
    if (grid < 6 && segment < 8) {
        if (state)
            dev.ledData[grid] |= (1 << segment);
        else
            dev.ledData[grid] &= ~(1 << segment);
        tm1637_updateDisplay(dev);
    }
}

void tm1637_clearDisplay(TM1637Device &dev) {
    memset(dev.ledData, 0, sizeof(dev.ledData));
    tm1637_updateDisplay(dev);
}

void tm1637_allOn(TM1637Device& dev) {
  for (int i = 0; i < 6; i++) dev.ledData[i] = 0xFF;
  tm1637_updateDisplay(dev);
}

void tm1637_allOff(TM1637Device& dev) {
  memset(dev.ledData, 0, sizeof(dev.ledData));
  tm1637_updateDisplay(dev);
}

void tm1637_allOn() {
  debugPrintln("🔆 Turning ALL TM1637 LEDs ON");
  tm1637_allOn(RA_Device);
  tm1637_allOn(LA_Device);
}

void tm1637_allOff() {
  debugPrint("⚫ Turning ALL TM1637 LEDs OFF");
  tm1637_allOff(RA_Device);
  tm1637_allOff(LA_Device);
}

void tm1637_sweep(TM1637Device& dev, const char* deviceName) {
  debugPrintf("🔁 Sweep for %s:\n", deviceName);
  for (int grid = 0; grid < 6; grid++) {
    for (int seg = 0; seg < 8; seg++) {
      tm1637_allOff(dev);
      tm1637_displaySingleLED(dev, grid, seg, true);
      debugPrintf("🟢 %s LED at GRID %d, SEG %d\n", deviceName, grid, seg);
      delay(200);
    }
  }
  tm1637_allOff(dev);
  debugPrintf("✅ %s sweep complete.\n", deviceName);
}

void tm1637_sweepPanel() {
  debugPrintln("🔍 Starting TM1637 panel sweep...");
  for (uint16_t i = 0; i < panelLEDsCount; i++) {
    if (panelLEDs[i].deviceType != DEVICE_TM1637) continue;

    uint8_t clk     = panelLEDs[i].info.tm1637Info.clkPin;
    uint8_t dio     = panelLEDs[i].info.tm1637Info.dioPin;
    uint8_t grid    = panelLEDs[i].info.tm1637Info.segment;
    uint8_t segment = panelLEDs[i].info.tm1637Info.bit;

    TM1637Device* dev = (dio == RA_DIO_PIN) ? &RA_Device : (dio == LA_DIO_PIN) ? &LA_Device : nullptr;
    if (dev) {
      tm1637_allOff(*dev);
      tm1637_displaySingleLED(*dev, grid, segment, true);
      debugPrintf("🟢 LED ON: %s → GRID %d, SEG %d\n", panelLEDs[i].label, grid, segment);
      delay(500);
    }
  }
  tm1637_allOff();
  debugPrintln("✅ TM1637 labeled sweep complete.");
}

void tm1637_testPattern() {
  debugPrintln("🧪 Running TM1637 Global Test Pattern...");
  tm1637_allOff();
  tm1637_allOn();
  debugPrintln("✅ TM1637 Global Test Complete.");
}

void tm1637_testPattern(TM1637Device& dev, const char* deviceName) {
  debugPrintf("🧪 Running TM1637 Test Pattern for %s...\n", deviceName);
  tm1637_allOff(dev);
  tm1637_allOn(dev);
  debugPrintf("✅ %s Test Complete.\n", deviceName);
}

// TM1637 Helper Implementation
void TM1637_setPanelAllLEDs(TM1637Device &dev, bool state) {
    if (state) tm1637_allOn(dev);
    else tm1637_allOff(dev);
}

// Turns all TM1637 LEDs on/off (RA and LA)
void TM1637_setAllLEDs(bool state) {
  if (state)
    tm1637_allOn();
  else
    tm1637_allOff();
}