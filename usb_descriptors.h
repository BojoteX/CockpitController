#pragma once
#include <Arduino.h>
#include "tusb.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t const* tud_descriptor_device_cb(void);
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid);

#ifdef __cplusplus
}
#endif
