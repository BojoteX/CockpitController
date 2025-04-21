// Mappings.h
// Centralized management for mapping LED, Button/Actions & DCS Mappings

#ifndef MAPPINGS_H
#define MAPPINGS_H

#include <Arduino.h>
#include <map>
#include <unordered_map>
#include <string>
#include "LEDControl.h"

// All LED Mappings
#include "LEDMappings.h"

// Your Controller Assignments and Bitmasks (OLD Legacy)
// #include "ButtonMappings.h"

// Your Controller Assignments and Bitmasks (New System)
#include "InputMapping.h"

#endif // MAPPINGS_H