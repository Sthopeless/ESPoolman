#pragma once

#include <Arduino.h>

void initScale();
void loopScale();
void scaleTare();
float scaleGetWeight();
bool scaleIsReady();
void scaleUpdateDisplay();

extern float lastScaleWeight;
extern bool scaleConnected;
