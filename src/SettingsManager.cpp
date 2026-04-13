#include "SettingsManager.h"
#include "AppGlobals.h"
#include <ArduinoJson.h>

DeviceSettings settings;

void DeviceSettings::setDefaults() {
  wifiSsid = "";
  wifiPass = "";
  serverSpoolman = "";
  serverMoonraker = "";
  firstTimeSetup = true;
  showWifi = true;
  showServers = true;
  showCurrentSpool = true;
  showNfcData = true;
  showLogger = true;
  showNfcJson = true;
  showTabDashboard = true;
  showTabNfc = true;
  showTabWrite = true;
  showTabOptions = true;
  showTabLogs = true;
  showTabSpools = true;
  defaultTab = "dashboard";
  writeNfcEnabled = false;
  newNfcId = "0";
  newNfcFilId = "0";
  snfName = true;
  snfMaterial = true;
  snfBrand = true;
  snfColor = true;
  snfExtTemp = true;
  snfBedTemp = true;
  snfLocation = true;
  snfRemaining = true;
  snfWeight = true;
  snfDiameter = true;
  snfDensity = true;
  snfFlow = true;
  snfMaxSpeed = true;
  snfSpoolWt = true;
  showFieldWifiSsid = true;
  showFieldWifiPass = true;
  showFieldSpoolman = true;
  showFieldMoonraker = true;
  authEnabled = false;
  authUser = "espoolman";
  authPass = "espoolman";
}

void DeviceSettings::toJson(JsonDocument &doc) const {
  doc["wifiSsid"] = wifiSsid;
  doc["wifiPass"] = wifiPass;
  doc["serverSpoolman"] = serverSpoolman;
  doc["serverMoonraker"] = serverMoonraker;
  doc["firstTimeSetup"] = firstTimeSetup;
  doc["showWifi"] = showWifi;
  doc["showServers"] = showServers;
  doc["showCurrentSpool"] = showCurrentSpool;
  doc["showNfcData"] = showNfcData;
  doc["showLogger"] = showLogger;
  doc["showNfcJson"] = showNfcJson;
  doc["showTabDashboard"] = showTabDashboard;
  doc["showTabNfc"] = showTabNfc;
  doc["showTabWrite"] = showTabWrite;
  doc["showTabOptions"] = showTabOptions;
  doc["showTabLogs"] = showTabLogs;
  doc["showTabSpools"] = showTabSpools;
  doc["defaultTab"] = defaultTab;
  doc["writeNfcEnabled"] = writeNfcEnabled;
  doc["newNfcId"] = newNfcId;
  doc["newNfcFilId"] = newNfcFilId;
  doc["snfName"] = snfName;
  doc["snfMaterial"] = snfMaterial;
  doc["snfBrand"] = snfBrand;
  doc["snfColor"] = snfColor;
  doc["snfExtTemp"] = snfExtTemp;
  doc["snfBedTemp"] = snfBedTemp;
  doc["snfLocation"] = snfLocation;
  doc["snfRemaining"] = snfRemaining;
  doc["snfWeight"] = snfWeight;
  doc["snfDiameter"] = snfDiameter;
  doc["snfDensity"] = snfDensity;
  doc["snfFlow"] = snfFlow;
  doc["snfMaxSpeed"] = snfMaxSpeed;
  doc["snfSpoolWt"] = snfSpoolWt;
  doc["showFieldWifiSsid"] = showFieldWifiSsid;
  doc["showFieldWifiPass"] = showFieldWifiPass;
  doc["showFieldSpoolman"] = showFieldSpoolman;
  doc["showFieldMoonraker"] = showFieldMoonraker;
  doc["authEnabled"] = authEnabled;
  doc["authUser"] = authUser;
  doc["authPass"] = authPass;
}

bool DeviceSettings::fromJson(const JsonDocument &doc) {
  setDefaults();
  wifiSsid = String(doc["wifiSsid"] | wifiSsid);
  wifiPass = String(doc["wifiPass"] | wifiPass);
  serverSpoolman = String(doc["serverSpoolman"] | serverSpoolman);
  serverMoonraker = String(doc["serverMoonraker"] | serverMoonraker);
  firstTimeSetup = doc["firstTimeSetup"] | firstTimeSetup;
  showWifi = doc["showWifi"] | showWifi;
  showServers = doc["showServers"] | showServers;
  showCurrentSpool = doc["showCurrentSpool"] | showCurrentSpool;
  showNfcData = doc["showNfcData"] | showNfcData;
  showLogger = doc["showLogger"] | showLogger;
  showNfcJson = doc["showNfcJson"] | showNfcJson;
  showTabDashboard = doc["showTabDashboard"] | showTabDashboard;
  showTabNfc = doc["showTabNfc"] | showTabNfc;
  showTabWrite = doc["showTabWrite"] | showTabWrite;
  showTabOptions = doc["showTabOptions"] | showTabOptions;
  showTabLogs = doc["showTabLogs"] | showTabLogs;
  showTabSpools = doc["showTabSpools"] | showTabSpools;
  defaultTab = String(doc["defaultTab"] | defaultTab);
  writeNfcEnabled = doc["writeNfcEnabled"] | writeNfcEnabled;
  newNfcId = String(doc["newNfcId"] | newNfcId);
  newNfcFilId = String(doc["newNfcFilId"] | newNfcFilId);
  snfName = doc["snfName"] | snfName;
  snfMaterial = doc["snfMaterial"] | snfMaterial;
  snfBrand = doc["snfBrand"] | snfBrand;
  snfColor = doc["snfColor"] | snfColor;
  snfExtTemp = doc["snfExtTemp"] | snfExtTemp;
  snfBedTemp = doc["snfBedTemp"] | snfBedTemp;
  snfLocation = doc["snfLocation"] | snfLocation;
  snfRemaining = doc["snfRemaining"] | snfRemaining;
  snfWeight = doc["snfWeight"] | snfWeight;
  snfDiameter = doc["snfDiameter"] | snfDiameter;
  snfDensity = doc["snfDensity"] | snfDensity;
  snfFlow = doc["snfFlow"] | snfFlow;
  snfMaxSpeed = doc["snfMaxSpeed"] | snfMaxSpeed;
  snfSpoolWt = doc["snfSpoolWt"] | snfSpoolWt;
  showFieldWifiSsid = doc["showFieldWifiSsid"] | showFieldWifiSsid;
  showFieldWifiPass = doc["showFieldWifiPass"] | showFieldWifiPass;
  showFieldSpoolman = doc["showFieldSpoolman"] | showFieldSpoolman;
  showFieldMoonraker = doc["showFieldMoonraker"] | showFieldMoonraker;
  authEnabled = doc["authEnabled"] | authEnabled;
  authUser = String(doc["authUser"] | authUser.c_str());
  authPass = String(doc["authPass"] | authPass.c_str());
  return true;
}

String DeviceSettings::serialize() const {
  JsonDocument doc;
  toJson(doc);
  String json;
  serializeJson(doc, json);
  return json;
}

bool DeviceSettings::deserialize(const String &json) {
  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, json);
  if (error) {
    return false;
  }
  return fromJson(doc);
}

static void applyBuildFlagDefaults() {
#if defined(CONFIG_WIFI_SSID) && defined(CONFIG_WIFI_PASS) && defined(CONFIG_SPOOLMAN_URL) && defined(CONFIG_MOONRAKER_URL)
  if (settings.wifiSsid.isEmpty()        && strlen(CONFIG_WIFI_SSID)     > 0) settings.wifiSsid        = CONFIG_WIFI_SSID;
  if (settings.wifiPass.isEmpty()        && strlen(CONFIG_WIFI_PASS)     > 0) settings.wifiPass        = CONFIG_WIFI_PASS;
  if (settings.serverSpoolman.isEmpty()  && strlen(CONFIG_SPOOLMAN_URL)  > 0) settings.serverSpoolman  = CONFIG_SPOOLMAN_URL;
  if (settings.serverMoonraker.isEmpty() && strlen(CONFIG_MOONRAKER_URL) > 0) settings.serverMoonraker = CONFIG_MOONRAKER_URL;
#endif
  // Any device that already has WiFi credentials doesn't need first-time setup
  if (!settings.wifiSsid.isEmpty()) settings.firstTimeSetup = false;
}

void loadSettings() {
  if (prefs.isKey(KEY_DEVICE_CONFIG)) {
    const String stored = prefs.getString(KEY_DEVICE_CONFIG, "");
    if (!stored.isEmpty() && settings.deserialize(stored)) {
      const bool wasFts = settings.firstTimeSetup;
      applyBuildFlagDefaults();
      if (wasFts && !settings.firstTimeSetup) saveSettings(); // persist the correction
      return;
    }
  }
  settings.setDefaults();
  applyBuildFlagDefaults();
  saveSettings();
}

void saveSettings() {
  prefs.putString(KEY_DEVICE_CONFIG, settings.serialize());
}
