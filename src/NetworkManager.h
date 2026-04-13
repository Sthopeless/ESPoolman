#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void initNetwork();
void requestStatusRefresh();
String getCurrentIpAddress();
String getWifiMode();
bool isWifiConnected();
