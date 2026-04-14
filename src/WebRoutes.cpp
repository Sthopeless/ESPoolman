#include "WebRoutes.h"
#include "AppGlobals.h"
#include "AppConfig.h"
#include "SettingsManager.h"
#include "WebUi.h"
#include "OtaManager.h"
#include "SpoolmanClient.h"
#include "NetworkManager.h"
#include "ScaleManager.h"
#include "MqttManager.h"
#include <WiFi.h>
#include <ArduinoJson.h>

bool authenticateRequest(AsyncWebServerRequest *request) {
  if (!settings.authEnabled) {
    return true;
  }
  if (request->authenticate(settings.authUser.c_str(), settings.authPass.c_str())) {
    return true;
  }
  request->requestAuthentication();
  return false;
}

static void readRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (index == 0) {
    auto *body = new String();
    body->reserve(total);
    request->_tempObject = body;
  }
  auto *body = reinterpret_cast<String *>(request->_tempObject);
  if (!body) {
    return;
  }
  for (size_t i = 0; i < len; ++i) {
    body->concat(static_cast<char>(data[i]));
  }
  if (index + len == total) {
    request->_tempObject = body;
  }
}

static String consumeRequestBody(AsyncWebServerRequest *request) {
  auto *body = reinterpret_cast<String *>(request->_tempObject);
  String result;
  if (body) {
    result = *body;
    delete body;
  }
  request->_tempObject = nullptr;
  return result;
}

static void sendJson(AsyncWebServerRequest *request, JsonDocument &doc) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  serializeJson(doc, *response);
  request->send(response);
}

static void populateDataResponse(JsonDocument &doc) {
  doc["wifiSsid"] = settings.wifiSsid;
  doc["wifiMode"] = getWifiMode();
  doc["wifiConnected"] = isWifiConnected();
  doc["ipAddress"] = getCurrentIpAddress();
  doc["wifiStatus"] = lastWifiStatus;
  doc["macAddress"] = WiFi.macAddress();
  doc["serverSpoolman"] = settings.serverSpoolman;
  doc["serverMoonraker"] = settings.serverMoonraker;
  doc["showWifi"] = settings.showWifi;
  doc["showServers"] = settings.showServers;
  doc["showCurrentSpool"] = settings.showCurrentSpool;
  doc["showNfcData"] = settings.showNfcData;
  doc["showLogger"] = settings.showLogger;
  doc["showNfcJson"] = settings.showNfcJson;
  doc["showTabDashboard"] = settings.showTabDashboard;
  doc["showTabNfc"] = settings.showTabNfc;
  doc["showTabWrite"] = settings.showTabWrite;
  doc["showTabOptions"] = settings.showTabOptions;
  doc["showTabLogs"] = settings.showTabLogs;
  doc["showTabSpools"] = settings.showTabSpools;
  doc["defaultTab"] = settings.defaultTab;
  doc["writeNfcEnabled"] = settings.writeNfcEnabled;
  doc["currentId"] = currentId;
  doc["currentName"] = filamentName;
  doc["nfcId"] = prefs.getInt(KEY_NFC_ID, 0);
  doc["nfcFilamentId"] = prefs.getInt(KEY_NFC_FILAMENT_ID, 0);
  doc["nfcName"] = prefs.getString(KEY_NFC_NAME);
  doc["nfcMaterial"] = prefs.getString(KEY_NFC_TYPE);
  doc["nfcBrand"] = prefs.getString(KEY_NFC_BRAND);
  doc["nfcColor"] = prefs.getString(KEY_NFC_COLOR);
  doc["nfcExtTemp"] = prefs.getString(KEY_NFC_MIN_TEMP);
  doc["nfcBedTemp"] = prefs.getString(KEY_NFC_MAX_TEMP);
  doc["nfcLocation"] = prefs.getString(KEY_NFC_LOCATION);
  doc["nfcRemaining"] = prefs.getString(KEY_NFC_REMAINING);
  doc["nfcWeight"] = prefs.getString(KEY_NFC_WEIGHT);
  doc["nfcDiameter"] = prefs.getString(KEY_NFC_DIAMETER);
  doc["nfcDensity"] = prefs.getString(KEY_NFC_DENSITY);
  doc["nfcFlowRatio"] = prefs.getString(KEY_NFC_FLOW_RATIO);
  doc["nfcMaxSpeed"] = prefs.getString(KEY_NFC_MAX_SPEED);
  doc["nfcSpoolWt"] = prefs.getString(KEY_NFC_SPOOL_WT);

  String raw = prefs.getString(KEY_NFC_JSON);
  String safe;
  safe.reserve(raw.length());
  for (size_t i = 0; i < raw.length(); ++i) {
    const byte b = static_cast<byte>(raw[i]);
    if (b == '\n' || b == '\r' || b == '\t' || (b >= 0x20 && b < 0x80)) {
      safe += raw[i];
    }
  }
  doc["nfcText"] = safe;
  doc["nfcTagType"] = prefs.getString(KEY_NFC_TAG_TYPE);

  doc["snfName"] = settings.snfName;
  doc["snfMaterial"] = settings.snfMaterial;
  doc["snfBrand"] = settings.snfBrand;
  doc["snfColor"] = settings.snfColor;
  doc["snfExtTemp"] = settings.snfExtTemp;
  doc["snfBedTemp"] = settings.snfBedTemp;
  doc["snfLocation"] = settings.snfLocation;
  doc["snfRemaining"] = settings.snfRemaining;
  doc["snfWeight"] = settings.snfWeight;
  doc["snfDiameter"] = settings.snfDiameter;
  doc["snfDensity"] = settings.snfDensity;
  doc["snfFlow"] = settings.snfFlow;
  doc["snfMaxSpeed"] = settings.snfMaxSpeed;
  doc["snfSpoolWt"] = settings.snfSpoolWt;
  doc["newNfcId"] = settings.newNfcId;
  doc["newNfcFilId"] = settings.newNfcFilId;
  doc["logger"] = logger.getContents();
  doc["moonraker"] = ledMoonraker;
  doc["spoolman"] = ledSpoolman;
  doc["nfc"] = ledNfc;
  doc["pendingWrite"] = static_cast<bool>(pendingNfcWrite);
  doc["pendingWriteStatus"] = pendingWriteStatus;
  doc["firstTimeSetup"] = settings.firstTimeSetup;
  doc["showFieldWifiSsid"] = settings.showFieldWifiSsid;
  doc["showFieldWifiPass"] = settings.showFieldWifiPass;
  doc["showFieldSpoolman"] = settings.showFieldSpoolman;
  doc["showFieldMoonraker"] = settings.showFieldMoonraker;
  doc["authEnabled"] = settings.authEnabled;
  doc["authUser"] = settings.authUser;

  doc["scaleEnabled"] = settings.scaleEnabled;
  doc["scaleDoutPin"] = settings.scaleDoutPin;
  doc["scaleSckPin"] = settings.scaleSckPin;
  doc["scaleCalibration"] = settings.scaleCalibration;
  doc["oledEnabled"] = settings.oledEnabled;
  doc["oledSdaPin"] = settings.oledSdaPin;
  doc["oledSclPin"] = settings.oledSclPin;
  doc["btnTarePin"] = settings.btnTarePin;
  doc["btnEnterPin"] = settings.btnEnterPin;
  doc["scaleWeight"] = lastScaleWeight;
  doc["scaleConnected"] = scaleConnected;

  doc["mqttEnabled"] = settings.mqttEnabled;
  doc["mqttHost"] = settings.mqttHost;
  doc["mqttPort"] = settings.mqttPort;
  doc["mqttUser"] = settings.mqttUser;
  doc["mqttTopic"] = settings.mqttTopic;
  doc["mqttConnected"] = mqttConnected;
}

static void saveConfig(AsyncWebServerRequest *request, const String &body) {
  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, body);
  if (error) {
    request->send(400, "text/plain", "Invalid JSON payload");
    return;
  }

  // Preserve existing passwords if none supplied in request
  const String existingAuthPass = settings.authPass;
  const String existingMqttPass = settings.mqttPass;
  settings.fromJson(doc);
  const String newAuthPass = doc["authPass"].is<const char*>() ? String((const char *)doc["authPass"]) : "";
  if (newAuthPass.isEmpty()) {
    settings.authPass = existingAuthPass;
  }
  const String newMqttPass = doc["mqttPass"].is<const char*>() ? String((const char *)doc["mqttPass"]) : "";
  if (newMqttPass.isEmpty()) {
    settings.mqttPass = existingMqttPass;
  }
  settings.firstTimeSetup = false;
  saveSettings();
  logMessage("[INFO] Configuration saved");
  request->send(200, "text/plain", "Configuration saved. Restarting.");
  scheduleRestart("apply saved config", 1500);
}

void setupWebRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    const char *html = getIndexHtml();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", reinterpret_cast<const uint8_t *>(html), strlen(html));
    request->send(response);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    populateDataResponse(doc);
    sendJson(request, doc);
  });

  events.onConnect([](AsyncEventSourceClient *client) {
    client->send("connected", "refresh", millis());
  });
  server.addHandler(&events);

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    statusRequested = true;
    request->send(200, "text/plain", "Status refresh queued");
  });

  server.on("/spools", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (spoolsJsonCache.isEmpty()) {
      request->send(503, "application/json", "[]");
    } else {
      request->send(200, "application/json", spoolsJsonCache);
    }
  });

  server.on("/save-config", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticateRequest(request)) {
      return;
    }
    const String body = consumeRequestBody(request);
    saveConfig(request, body);
  }, nullptr, readRequestBody);

  server.on("/write-nfc", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("id", true)) {
      request->send(400, "text/plain", "Missing spool id");
      return;
    }

    const bool writeEnabled = request->hasParam("enabled", true) && request->getParam("enabled", true)->value() == "1";
    if (!writeEnabled) {
      settings.writeNfcEnabled = false;
      saveSettings();
      request->send(400, "text/plain", "NFC writing is disabled — enable it first");
      return;
    }

    settings.newNfcId = request->getParam("id", true)->value();
    settings.newNfcFilId = request->hasParam("filamentId", true) ? request->getParam("filamentId", true)->value() : "0";
    settings.writeNfcEnabled = true;
    saveSettings();

    pendingNfcWrite = true;
    pendingWriteStartedAtMs = millis();
    pendingWriteStatus = "Waiting for next tag scan";
    notifyUiRefresh("write-armed");
    request->send(200, "text/plain", "NFC write armed. Scan the next tag to write.");
  });

  server.on("/cancel-write", HTTP_POST, [](AsyncWebServerRequest *request) {
    pendingNfcWrite = false;
    pendingWriteStartedAtMs = 0;
    pendingWriteStatus = "Cancelled";
    settings.writeNfcEnabled = false;
    saveSettings();
    notifyUiRefresh("write-cancelled");
    request->send(200, "text/plain", "Write cancelled");
  });

  server.on("/scale/tare", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticateRequest(request)) return;
    if (!settings.scaleEnabled || !scaleConnected) {
      request->send(400, "text/plain", "Scale not available");
      return;
    }
    scaleTare();
    notifyUiRefresh("scale-tared");
    request->send(200, "text/plain", "Scale tared");
  });

  server.on("/scale/measure", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticateRequest(request)) return;
    if (!settings.scaleEnabled || !scaleConnected) {
      request->send(400, "text/plain", "Scale not available");
      return;
    }
    if (currentId <= 0) {
      request->send(400, "text/plain", "No spool selected");
      return;
    }
    float weight = scaleGetWeight();
    if (weight < 0.5f) {
      request->send(400, "text/plain", "No weight on scale");
      return;
    }
    String error;
    if (measureSpoolWeight(currentId, weight, error)) {
      buildSpoolsCache();
      notifyUiRefresh("weight-updated");
      mqttPublishWeight(weight);
      request->send(200, "text/plain", "Weight " + String(weight, 1) + "g sent to spool #" + String(currentId));
    } else {
      request->send(500, "text/plain", "Failed: " + error);
    }
  });

  server.on("/scale/measure-spool", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticateRequest(request)) return;
    if (!settings.scaleEnabled || !scaleConnected) {
      request->send(400, "text/plain", "Scale not available");
      return;
    }
    const String body = consumeRequestBody(request);
    JsonDocument doc;
    if (deserializeJson(doc, body)) {
      request->send(400, "text/plain", "Invalid JSON");
      return;
    }
    int spoolId = doc["spoolId"] | 0;
    if (spoolId <= 0) {
      request->send(400, "text/plain", "Select a spool first");
      return;
    }
    float weight = scaleGetWeight();
    if (weight < 0.5f) {
      request->send(400, "text/plain", "No weight on scale");
      return;
    }
    String error;
    if (measureSpoolWeight(spoolId, weight, error)) {
      buildSpoolsCache();
      notifyUiRefresh("weight-updated");
      mqttPublishWeight(weight);
      request->send(200, "text/plain", "Weight " + String(weight, 1) + "g sent to spool #" + String(spoolId));
    } else {
      request->send(500, "text/plain", "Failed: " + error);
    }
  }, nullptr, readRequestBody);

  server.on("/scale/calibrate", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticateRequest(request)) return;
    const String body = consumeRequestBody(request);
    JsonDocument doc;
    if (deserializeJson(doc, body)) {
      request->send(400, "text/plain", "Invalid JSON");
      return;
    }
    float knownWeight = doc["knownWeight"] | 0.0f;
    if (knownWeight < 1.0f) {
      request->send(400, "text/plain", "Provide knownWeight >= 1g");
      return;
    }
    float rawReading = scaleGetWeight();
    if (rawReading < 0.5f) {
      request->send(400, "text/plain", "Place the known weight on the scale first");
      return;
    }
    float newCal = settings.scaleCalibration * (rawReading / knownWeight);
    settings.scaleCalibration = newCal;
    saveSettings();
    request->send(200, "text/plain", "Calibration updated to " + String(newCal, 2));
  }, nullptr, readRequestBody);

  setupOtaRoutes();
}
