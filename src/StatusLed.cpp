#include "StatusLed.h"
#include "AppGlobals.h"

void initStatusLed() {
  FastLED.addLeds<LED_TYPE, PIN_DATA, COLOR_ORDER>(leds, NUMLEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}

void setLedColor(int color) {
  leds[0].setHue(color);
  FastLED.show();
  ledOverrideActive = true;
}

void updateStatusLed(bool enabled, bool wifiConnected, bool systemHealthy, bool wifiModeActive) {
  if (wifiModeActive) {
    if (ledWifiTimer.ready()) {
      leds[0].setHue(LED_HUE_BLUE);
      FastLED.show();
    } else {
      FastLED.clear();
      FastLED.show();
    }
    return;
  }

  if (!ledOverrideActive) {
    if (wifiConnected) {
      leds[0].setHue(systemHealthy ? LED_HUE_GREEN : LED_HUE_BLUE);
    } else {
      leds[0].setHue(LED_HUE_RED);
    }
    FastLED.show();
  }
}
