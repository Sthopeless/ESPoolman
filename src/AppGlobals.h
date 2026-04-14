#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <TimerMs.h>
#include <MFRC522v2.h>
#include <atomic>

#include "AppConfig.h"

struct DeviceSettings;

extern CRGB leds[NUMLEDS];

extern MFRC522 mfrc522;

extern Preferences prefs;
extern AsyncWebServer server;
extern AsyncEventSource events;

extern TimerMs ledWifiTimer;
extern TimerMs resetLedTimer;
extern TimerMs rebootNfcTimer;

class CircularLogger {
 public:
  static constexpr size_t Capacity = 4096;
  void append(const String &message) {
    portENTER_CRITICAL(&mux);
    for (size_t i = 0; i < message.length(); ++i) {
      pushChar(message[i]);
    }
    pushChar('\n');
    portEXIT_CRITICAL(&mux);
  }

  String getContents() {
    portENTER_CRITICAL(&mux);
    String output;
    output.reserve(length);
    for (size_t i = 0; i < length; ++i) {
      output += buffer[(head + i) % Capacity];
    }
    portEXIT_CRITICAL(&mux);
    return output;
  }

 private:
  void pushChar(char ch) {
    if (length < Capacity) {
      buffer[(head + length) % Capacity] = ch;
      ++length;
      return;
    }
    buffer[head] = ch;
    head = (head + 1) % Capacity;
  }

  char buffer[Capacity] = {};
  size_t head = 0;
  size_t length = 0;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};

extern CircularLogger logger;
extern String spoolsJsonCache;
extern String filamentName;
extern String lastDetectedTagType;
extern String lastWifiStatus;
extern int currentId;
extern bool ledSpoolman;
extern bool ledMoonraker;
extern bool ledNfc;
extern std::atomic<bool> wifiSettingMode;
extern std::atomic<bool> statusRequested;
extern bool ledOverrideActive;
extern std::atomic<bool> restartScheduled;
extern std::atomic<bool> pendingNfcWrite;
extern unsigned long restartAtMs;
extern unsigned long pendingWriteStartedAtMs;
extern String restartReason;
extern String pendingWriteStatus;

void buildSpoolsCache();
void logMessage(const String &message);
void setLedColor(int color);
void errorServer(const String &error);
void scheduleRestart(const String &reason, unsigned long delayMs = 1500);
String getCurrentIpAddress();
void notifyUiRefresh(const char *reason);
void cancelPendingNfcWrite(const String &reason);
String normalizeColorHex(String color);
