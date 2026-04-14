#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "AppConfig.h"

struct DeviceSettings {
  String wifiSsid;
  String wifiPass;
  String serverSpoolman;
  String serverMoonraker;
  bool firstTimeSetup = true;
  bool showWifi = true;
  bool showServers = true;
  bool showCurrentSpool = true;
  bool showNfcData = true;
  bool showLogger = true;
  bool showNfcJson = false;
  bool showTabDashboard = true;
  bool showTabNfc = true;
  bool showTabWrite = true;
  bool showTabOptions = true;
  bool showTabLogs = true;
  bool showTabSpools = true;
  String defaultTab = "dashboard";
  bool writeNfcEnabled = false;
  String newNfcId = "0";
  String newNfcFilId = "0";
  bool snfName = true;
  bool snfMaterial = true;
  bool snfBrand = true;
  bool snfColor = true;
  bool snfExtTemp = true;
  bool snfBedTemp = true;
  bool snfLocation = true;
  bool snfRemaining = true;
  bool snfWeight = true;
  bool snfDiameter = true;
  bool snfDensity = true;
  bool snfFlow = true;
  bool snfMaxSpeed = true;
  bool snfSpoolWt = true;
  bool showFieldWifiSsid = true;
  bool showFieldWifiPass = true;
  bool showFieldSpoolman = true;
  bool showFieldMoonraker = true;
  bool authEnabled = false;
  String authUser = "admin";
  String authPass = "espoolman";

  // HX711 Scale
  bool scaleEnabled = false;
  int scaleDoutPin = DEFAULT_HX711_DOUT_PIN;
  int scaleSckPin = DEFAULT_HX711_SCK_PIN;
  float scaleCalibration = DEFAULT_HX711_CALIBRATION;
  long scaleOffset = 0;

  // SSD1306 OLED
  bool oledEnabled = false;
  int oledSdaPin = DEFAULT_OLED_SDA_PIN;
  int oledSclPin = DEFAULT_OLED_SCL_PIN;

  // Scale buttons
  int btnTarePin = DEFAULT_BTN_TARE_PIN;
  int btnEnterPin = DEFAULT_BTN_ENTER_PIN;

  // Dashboard card order (comma-separated IDs)
  String dashCardOrder;

  // MQTT
  bool mqttEnabled = false;
  String mqttHost;
  int mqttPort = DEFAULT_MQTT_PORT;
  String mqttUser;
  String mqttPass;
  String mqttTopic = "espoolman";

  void setDefaults();
  void toJson(JsonDocument &doc) const;
  bool fromJson(const JsonDocument &doc);
  String serialize() const;
  bool deserialize(const String &json);
};

extern DeviceSettings settings;

void loadSettings();
void saveSettings();
