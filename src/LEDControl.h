#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "LABELS/LEDMapping.h"

// Public API for LED Control
void initializeLEDs(const char* activePanels[], unsigned int panelCount);
void setLED(const char* label, bool state, uint8_t intensity = 100);

// Forward lookup declaration (findLED is auto-generated in LEDControl_Lookup.h)
struct LEDMapping;
const LEDMapping* findLED(const char* label);

