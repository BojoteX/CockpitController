// #include <Arduino.h>
#include "esp32-hal-rmt.h"

CRGB leds[NUM_LEDS];
#define WS2812B_PIN 35

uint8_t pixels[NUM_LEDS * 3];
uint8_t brightness = 255;
uint32_t lastShowTime = 0;

const CRGB CRGB::Black  = {0,0,0};
const CRGB CRGB::Red    = {255,0,0};
const CRGB CRGB::Green  = {0,255,0};
const CRGB CRGB::Blue   = {0,0,255};
const CRGB CRGB::Yellow = {255,255,0};

void WS2812_show();

void WS2812_init() {
  pinMode(WS2812B_PIN, OUTPUT);
  digitalWrite(WS2812B_PIN, LOW);
  rmtInit(WS2812B_PIN, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);
  rmtSetEOT(WS2812B_PIN, 0);

  memset(pixels, 0, NUM_LEDS * 3);
  brightness = 255;
  WS2812_clearAll();
}

void WS2812_setLEDColor(uint8_t ledIndex, CRGB color) {
  if (ledIndex >= NUM_LEDS) return;

  pixels[ledIndex * 3 + 0] = color.g;
  pixels[ledIndex * 3 + 1] = color.r;
  pixels[ledIndex * 3 + 2] = color.b;

  WS2812_show();
}

void WS2812_clearAll() {
  memset(pixels, 0, NUM_LEDS * 3);
  WS2812_show();
}

void WS2812_show() {
  uint32_t now = micros();
  if (now - lastShowTime < 50) {
    delayMicroseconds(50 - (now - lastShowTime));
  }

  const uint8_t highTicks1 = 8;
  const uint8_t lowTicks1 = 4;
  const uint8_t highTicks0 = 4;
  const uint8_t lowTicks0 = 8;

  rmt_data_t rmtBuffer[NUM_LEDS * 24];
  int symbolIndex = 0;

  for (int i = 0; i < NUM_LEDS; ++i) {
    uint8_t g = (pixels[i * 3 + 0] * brightness) >> 8;
    uint8_t r = (pixels[i * 3 + 1] * brightness) >> 8;
    uint8_t b = (pixels[i * 3 + 2] * brightness) >> 8;

    uint32_t pixelData = (uint32_t)g << 16 | (uint32_t)r << 8 | b;

    for (int bit = 23; bit >= 0; --bit) {
      bool bitVal = pixelData & (1UL << bit);
      rmtBuffer[symbolIndex].level0 = 1;
      rmtBuffer[symbolIndex].duration0 = bitVal ? highTicks1 : highTicks0;
      rmtBuffer[symbolIndex].level1 = 0;
      rmtBuffer[symbolIndex].duration1 = bitVal ? lowTicks1 : lowTicks0;
      symbolIndex++;
    }
  }

  rmtWrite(WS2812B_PIN, rmtBuffer, symbolIndex, RMT_WAIT_FOR_EVER);
  lastShowTime = micros();
}

void WS2812_allOn(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++)
    WS2812_setLEDColor(i, color);
  debugPrintln("🔆 WS2812 All ON");
}

void WS2812_allOff() {
  WS2812_clearAll();
  debugPrintln("⚫ WS2812 All OFF");
}

void WS2812_sweep(const CRGB* colors, uint8_t count) {
  debugPrintln("🔁 WS2812 Sweep with custom colors:");
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    WS2812_clearAll();
    WS2812_setLEDColor(i, colors[i % count]);
    debugPrintf("🟢 LED %d ON with color: R=%d G=%d B=%d\n",
      i, colors[i % count].r, colors[i % count].g, colors[i % count].b);
    delay(400);
  }
  WS2812_clearAll();
  debugPrintln("✅ WS2812 Sweep complete.");
}

void WS2812_testPattern() {
  const CRGB VZColors[] = {CRGB::Yellow, CRGB::Blue, CRGB::Red};
  
  debugPrintln("🧪 WS2812 Test Pattern Start");
  WS2812_allOff();
  WS2812_sweep(VZColors, sizeof(VZColors)/sizeof(CRGB));
  WS2812_allOn(CRGB::Green);
  debugPrintln("✅ WS2812 Test Pattern Complete");
}

void WS2812_setAllLEDs(bool state) {
  WS2812_allOn(state ? CRGB::Green : CRGB::Black);
}