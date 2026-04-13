#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <SPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <MFRC522Debug.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522v2.h>
#include <TimerMs.h>
#include <atomic>

#include "AppConfig.h"
#include "AppGlobals.h"
#include "NfcManager.h"
#include "OtaManager.h"
#include "SpoolmanClient.h"
#include "WebUi.h"
#include "NetworkManager.h"
#include "StatusLed.h"
#include "SettingsManager.h"

CRGB leds[NUMLEDS];

MFRC522DriverPinSimple ssPinDriver(SS_PIN);
MFRC522DriverSPI mfrc522Driver{ssPinDriver};
MFRC522 mfrc522{mfrc522Driver};

Preferences prefs;
AsyncWebServer server(80);
AsyncEventSource events("/events");

TimerMs ledWifiTimer(500, 1, 0);
TimerMs resetLedTimer(1000, 1, 0);
TimerMs rebootNfcTimer(60000, 1, 0);

CircularLogger logger;
String filamentName;
String lastDetectedTagType;
String lastWifiStatus = "Not started";
int currentId = 0;
bool ledSpoolman = false;
bool ledMoonraker = false;
bool ledNfc = false;
std::atomic<bool> wifiSettingMode(false);
std::atomic<bool> statusRequested(false);
bool ledOverrideActive = false;
std::atomic<bool> restartScheduled(false);
std::atomic<bool> pendingNfcWrite(false);
unsigned long restartAtMs = 0;
unsigned long pendingWriteStartedAtMs = 0;
String restartReason;
String pendingWriteStatus = "Idle";
String spoolsJsonCache;

void logMessage(const String &message) {
  logger.append(message);
  Serial.println(message);
}

void errorServer(const String &message) {
  logMessage("[ERROR] " + message);
  setLedColor(LED_HUE_RED);
}

String normalizeColorHex(String color) {
  color.trim();
  if (color.startsWith("#")) {
    color.remove(0, 1);
  }
  color.toUpperCase();
  if (color.length() == 3) {
    String expanded;
    for (size_t i = 0; i < 3; ++i) {
      expanded += color[i];
      expanded += color[i];
    }
    color = expanded;
  }
  if (color.length() != 6) {
    return "FFFFFF";
  }
  for (size_t i = 0; i < color.length(); ++i) {
    if (!isxdigit(static_cast<unsigned char>(color[i]))) {
      return "FFFFFF";
    }
  }
  return color;
}

void scheduleRestart(const String &reason, unsigned long delayMs) {
  restartReason = reason;
  restartAtMs = millis() + delayMs;
  restartScheduled = true;
  logMessage("[INFO] Restart scheduled: " + reason);
}

void notifyUiRefresh(const char *eventName) {
  events.send(eventName, "refresh", millis());
}

void cancelPendingNfcWrite(const String &reason) {
  pendingNfcWrite = false;
  pendingWriteStartedAtMs = 0;
  pendingWriteStatus = reason;
  setLedColor(LED_HUE_RED);
  logMessage("[INFO] NFC write queue cleared: " + reason);
  notifyUiRefresh("write-cancelled");
}

void buildSpoolsCache() {
  static bool lastSpoolmanOk = false;

  if (WiFi.status() != WL_CONNECTED) {
    lastSpoolmanOk = false;
    ledSpoolman = false;
    return;
  }

  DynamicJsonDocument sourceDoc(8192);
  String errorMessage;
  if (!fetchSpoolmanSpools(sourceDoc, errorMessage)) {
    errorServer(errorMessage);
    prefs.putString(KEY_FILAMENT, "");
    lastSpoolmanOk = false;
    ledSpoolman = false;
    return;
  }

  ledSpoolman = true;
  filamentName = "";

  DynamicJsonDocument responseDoc(8192);
  JsonArray responseArray = responseDoc.to<JsonArray>();

  for (JsonObject spool : sourceDoc.as<JsonArray>()) {
    if ((spool["id"] | 0) == currentId) {
      filamentName = String(spool["filament"]["name"] | "");
    }
    JsonObject item = responseArray.add<JsonObject>();
    const JsonVariantConst filament = spool["filament"];
    item["id"] = spool["id"] | 0;
    item["filamentId"] = filament["id"] | 0;
    item["name"] = String(filament["name"] | "");
    item["material"] = String(filament["material"] | "");
    item["brand"] = String(filament["vendor"]["name"] | filament["vendor"] | "");
    item["color"] = normalizeColorHex(String(filament["color_hex"] | spool["color_hex"] | "FFFFFF"));
    item["location"] = String(spool["location"] | "");
    item["extruderTemp"] = filament["settings_extruder_temp"] | 0;
    item["bedTemp"] = filament["settings_bed_temp"] | 0;
    item["remaining"] = spool["remaining_weight"] | 0.0;
    item["totalWeight"] = spool["initial_weight"] | 0.0;
    item["usedWeight"] = spool["used_weight"] | 0.0;
    item["diameter"] = filament["diameter"] | 0.0;
    item["density"] = filament["density"] | 0.0;
    item["flowRatio"] = String(filament["extra"]["flow_ratio"] | "");
    item["maxVolumetric"] = String(filament["extra"]["max_volumetric_speed"] | "");
    double sw = spool["spool_weight"] | 0.0;
    if (sw == 0.0) {
      sw = filament["spool_weight"] | 0.0;
    }
    item["spoolWeight"] = sw;
  }

  prefs.putString(KEY_FILAMENT, filamentName);
  serializeJson(responseDoc, spoolsJsonCache);
  if (!lastSpoolmanOk) {
    logMessage("Spoolman server is available");
    lastSpoolmanOk = true;
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);

  ledWifiTimer.setPeriodMode();
  resetLedTimer.setPeriodMode();
  rebootNfcTimer.setPeriodMode();

  prefs.begin(NVS_NAMESPACE, false);
  loadSettings();

  if (!prefs.isKey(KEY_NFC_ID))         prefs.putInt(KEY_NFC_ID, 0);
  if (!prefs.isKey(KEY_NFC_FILAMENT_ID))prefs.putInt(KEY_NFC_FILAMENT_ID, 0);
  if (!prefs.isKey(KEY_NFC_NAME))       prefs.putString(KEY_NFC_NAME, "");
  if (!prefs.isKey(KEY_NFC_BRAND))      prefs.putString(KEY_NFC_BRAND, "");
  if (!prefs.isKey(KEY_NFC_COLOR))      prefs.putString(KEY_NFC_COLOR, "");
  if (!prefs.isKey(KEY_NFC_JSON))       prefs.putString(KEY_NFC_JSON, "");
  if (!prefs.isKey(KEY_NFC_MIN_TEMP))   prefs.putString(KEY_NFC_MIN_TEMP, "");
  if (!prefs.isKey(KEY_NFC_MAX_TEMP))   prefs.putString(KEY_NFC_MAX_TEMP, "");
  if (!prefs.isKey(KEY_NFC_TYPE))       prefs.putString(KEY_NFC_TYPE, "");
  if (!prefs.isKey(KEY_NFC_LOCATION))   prefs.putString(KEY_NFC_LOCATION, "");
  if (!prefs.isKey(KEY_NFC_REMAINING))  prefs.putString(KEY_NFC_REMAINING, "");
  if (!prefs.isKey(KEY_NFC_WEIGHT))     prefs.putString(KEY_NFC_WEIGHT, "");
  if (!prefs.isKey(KEY_NFC_DIAMETER))   prefs.putString(KEY_NFC_DIAMETER, "");
  if (!prefs.isKey(KEY_NFC_DENSITY))    prefs.putString(KEY_NFC_DENSITY, "");
  if (!prefs.isKey(KEY_NFC_FLOW_RATIO)) prefs.putString(KEY_NFC_FLOW_RATIO, "");
  if (!prefs.isKey(KEY_NFC_MAX_SPEED))  prefs.putString(KEY_NFC_MAX_SPEED, "");
  if (!prefs.isKey(KEY_NFC_SPOOL_WT))   prefs.putString(KEY_NFC_SPOOL_WT, "");
  if (!prefs.isKey(KEY_NFC_TAG_TYPE))   prefs.putString(KEY_NFC_TAG_TYPE, "");

  filamentName = prefs.getString(KEY_FILAMENT, "");
  currentId = prefs.getInt(KEY_ID, 0);

  pinMode(SW_WIFI, INPUT_PULLUP);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);

  initStatusLed();
  initNetwork();

  ArduinoOTA.setHostname(APP_BRAND);
  ArduinoOTA.onStart([]() { logMessage("[INFO] ArduinoOTA start"); });
  ArduinoOTA.onEnd([]() { logMessage("[INFO] ArduinoOTA end"); });
  ArduinoOTA.onError([](ota_error_t error) {
    logMessage("[ERROR] ArduinoOTA error: " + String(static_cast<int>(error)));
  });
  ArduinoOTA.begin();
  logMessage("[INFO] ArduinoOTA listening");

  if (initializeNfcReader(true)) {
    logMessage("The reader was found RC522");
  } else {
    logMessage("[ERROR] Reader not found RC522");
  }
}

void loop() {
  ArduinoOTA.handle();

  if (restartScheduled && static_cast<long>(millis() - restartAtMs) >= 0) {
    restartScheduled = false;
    logMessage("[INFO] Restarting now");
    delay(100);
    ESP.restart();
  }

  ledWifiTimer.tick();
  resetLedTimer.tick();
  rebootNfcTimer.tick();

  static bool startupNotified = false;
  if (!startupNotified) {
    startupNotified = true;
    notifyUiRefresh("startup");
  }

  static bool prevWifiConnected = false;
  bool wifiNow = WiFi.status() == WL_CONNECTED;
  if (wifiNow && !prevWifiConnected && !wifiSettingMode) {
    lastWifiStatus = "Connected";
    logMessage("[INFO] WiFi (re)connected, IP: " + WiFi.localIP().toString());
    requestStatusRefresh();
    notifyUiRefresh("wifi-reconnect");
  }
  prevWifiConnected = wifiNow;

  if (pendingNfcWrite && static_cast<unsigned long>(millis() - pendingWriteStartedAtMs) > NFC_WRITE_TIMEOUT_MS) {
    cancelPendingNfcWrite("Write timed out");
  }

  if (rebootNfcTimer.ready()) {
    resetMFRC();
    requestStatusRefresh();
  }

  if (resetLedTimer.ready()) {
    ledOverrideActive = false;
  }

  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 25000UL) {
    lastHeartbeat = millis();
    events.send("ping", "heartbeat", millis());
  }

  readNFC();

  updateStatusLed(true, WiFi.status() == WL_CONNECTED, ledMoonraker && ledSpoolman && ledNfc, wifiSettingMode);

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "status") {
      requestStatusRefresh();
    } else if (!input.isEmpty()) {
      setSpool(input.toInt());
    }
  }
}
