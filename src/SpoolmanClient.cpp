#include "SpoolmanClient.h"

#include <HTTPClient.h>
#include <WiFi.h>

#include "AppGlobals.h"
#include "SettingsManager.h"

void statusServer() {
  if (WiFi.status() != WL_CONNECTED) {
    ledMoonraker = false;
    ledSpoolman = false;
    return;
  }

  HTTPClient http;
  const String getUrl = "http://" + settings.serverMoonraker + "/server/spoolman/status";
  logMessage("[INFO] statusServer URL: " + getUrl);
  http.begin(getUrl);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.setTimeout(4000);
  http.addHeader("Content-Type", "application/json");

  const int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    errorServer("HTTP " + String(httpResponseCode));
    ledMoonraker = false;
    http.end();
    return;
  }

  const String response = http.getString();
  http.end();

  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, response);
  if (error) {
    errorServer(error.c_str());
    ledMoonraker = false;
    return;
  }

  const bool moonrakerStatus = doc["result"]["spoolman_connected"] | false;
  currentId = doc["result"]["spool_id"] | 0;
  prefs.putInt(KEY_ID, currentId);

  static bool lastMoonrakerOk = false;
  if (moonrakerStatus != lastMoonrakerOk) {
    logMessage(moonrakerStatus ? "Moonraker server is available"
                                : "[ERROR] Moonraker server is unavailable");
    lastMoonrakerOk = moonrakerStatus;
  }
  ledMoonraker = moonrakerStatus;
}


bool getIdSpool(int id) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  JsonDocument doc;
  String errorMessage;
  if (!fetchSpoolmanSpools(doc, errorMessage)) {
    errorServer(errorMessage);
    return false;
  }

  for (JsonObject spool : doc.as<JsonArray>()) {
    if ((spool["id"] | 0) == id) {
      return true;
    }
  }

  return false;
}

bool fetchSpoolmanSpools(JsonDocument &doc, String &errorMessage) {
  if (WiFi.status() != WL_CONNECTED) {
    errorMessage = "WiFi not connected";
    return false;
  }

  HTTPClient http;
  const String getUrl = "http://" + settings.serverSpoolman + "/api/v1/spool?allow_archived=false";
  http.begin(getUrl);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.setTimeout(4000);
  http.addHeader("Content-Type", "application/json");

  const int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    errorMessage = "HTTP " + String(httpResponseCode);
    http.end();
    return false;
  }

  const String payload = http.getString();
  http.end();

  const DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    errorMessage = error.c_str();
    return false;
  }

  return true;
}

bool fetchSpoolById(int id, JsonDocument &doc, String &errorMessage) {
  if (WiFi.status() != WL_CONNECTED) {
    errorMessage = "WiFi not connected";
    return false;
  }

  HTTPClient http;
  String url = "http://" + settings.serverSpoolman + "/api/v1/spool/" + String(id);
  http.begin(url);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

  http.setTimeout(4000);
  const int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    errorMessage = "HTTP error " + String(httpResponseCode);
    http.end();
    return false;
  }
  if (httpResponseCode != 200) {
    errorMessage = "HTTP " + String(httpResponseCode);
    http.end();
    return false;
  }

  const String payload = http.getString();
  http.end();

  const DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    errorMessage = "JSON: " + String(error.c_str());
    return false;
  }

  return true;
}

void setSpool(int setId, bool validate) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (validate && !getIdSpool(setId)) {
    char buf[128];
    snprintf(buf, sizeof(buf), "[ERROR] Spool with ID %d does not exist", setId);
    logMessage(buf);
    return;
  }

  HTTPClient http;
  const String getUrl = "http://" + settings.serverMoonraker + "/server/spoolman/spool_id";
  http.begin(getUrl);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.addHeader("Content-Type", "application/json");

  JsonDocument postDoc;
  postDoc["spool_id"] = setId;

  String requestBody;
  serializeJson(postDoc, requestBody);
  const int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode <= 0) {
    char buf[64];
    snprintf(buf, sizeof(buf), "HTTP %d", httpResponseCode);
    errorServer(buf);
    http.end();
    return;
  }

  const String response = http.getString();
  http.end();

  JsonDocument doc;
  const DeserializationError deError = deserializeJson(doc, response);
  if (deError) {
    errorServer(deError.c_str());
    return;
  }

  statusServer();
  if (currentId == setId) {
    char buf[128];
    snprintf(buf, sizeof(buf), "Spool with ID %d successfully installed", setId);
    logMessage(buf);
    buildSpoolsCache();
    setLedColor(LED_HUE_LIME);
  } else {
    char buf[128];
    snprintf(buf, sizeof(buf), "[ERROR] Installation error with spool ID %d", setId);
    logMessage(buf);
    setLedColor(LED_HUE_RED);
  }
}
