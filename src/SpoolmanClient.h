#pragma once

#include <ArduinoJson.h>

void statusServer();
bool getIdSpool(int id);
bool fetchSpoolmanSpools(JsonDocument &doc, String &errorMessage);
bool fetchSpoolById(int id, JsonDocument &doc, String &errorMessage);
void setSpool(int setId, bool validate = true);
