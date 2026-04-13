#pragma once

#include <Arduino.h>

void resetMFRC();
void readNFC();
bool writeNFC(String &errorMessage);
bool initializeNfcReader(bool logDetails);
