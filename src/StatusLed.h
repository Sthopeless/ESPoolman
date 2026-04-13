#pragma once

#include <Arduino.h>
#include <FastLED.h>

void initStatusLed();
void setLedColor(int color);
void updateStatusLed(bool enabled, bool wifiConnected, bool systemHealthy, bool wifiModeActive);
