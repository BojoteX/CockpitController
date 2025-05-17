// ************************************************************
// WS8212
//
//
//
// ************************************************************

#include "../CUtils.h"

CRGB leds[NUM_LEDS];
uint8_t brightness = 255;
#define WS2812B_PIN 35

void WS2812_init() {
  FastLED.addLeds<WS2812B, WS2812B_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255); // Max brightness by default
  WS2812_clearAll();
}

void WS2812_setLEDColor(uint8_t ledIndex, CRGB color) {

    // debugPrintf("[WS2812] ❌ Write / LED skipped. FORCED NOT PRESENT FOR DEBUG\n");
    // return;

  if (ledIndex < NUM_LEDS) {
    leds[ledIndex] = color;
    FastLED.show();
  }
}

void WS2812_clearAll() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// Venezuela Flag Colors (Yellow, Blue, Red)
enum VZColorKey { VZ_YELLOW, VZ_BLUE, VZ_RED };
const CRGB VZColors[] = {
  CRGB::Yellow,
  CRGB::Blue,
  CRGB::Red
};

void WS2812_allOn(CRGB color) {
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
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
    leds[i] = colors[i % count];
    FastLED.show();
    debugPrintf("🟢 LED %d ON with color: R=%d G=%d B=%d\n",
      i, colors[i % count].r, colors[i % count].g, colors[i % count].b);
    delay(400);
  }
  WS2812_clearAll();
  debugPrintln("✅ WS2812 Sweep complete.");
}

void WS2812_testPattern() {
  debugPrintln("🧪 WS2812 Test Pattern Start");
  WS2812_allOff();
  WS2812_sweep(VZColors, sizeof(VZColors) / sizeof(CRGB));
  WS2812_allOn(CRGB::Green);
  debugPrintln("✅ WS2812 Test Pattern Complete");
}

// WS2812 Helper Implementation
void WS2812_setAllLEDs(bool state) {
    if (state) WS2812_allOn(CRGB::Green);
    else WS2812_clearAll();
}