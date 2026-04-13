#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void setupWebRoutes();
bool authenticateRequest(AsyncWebServerRequest *request);
