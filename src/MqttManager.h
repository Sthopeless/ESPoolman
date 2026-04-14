#pragma once

#include <Arduino.h>

void initMqtt();
void loopMqtt();
void mqttPublishWeight(float weight);
void mqttPublishSpoolChange(int spoolId, const String &name);
void mqttPublishNfcEvent(int spoolId, const String &tagType);

extern bool mqttConnected;
