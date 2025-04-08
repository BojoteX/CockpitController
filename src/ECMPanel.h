#pragma once

// #ifndef ECMPANEL_H
// #define ECMPANEL_H

#include <Arduino.h>
#include <Wire.h>

#define ECM_PCA_ADDR 0x22

void ECM_init();
void ECM_loop();

// #endif