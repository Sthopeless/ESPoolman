#include "MqttManager.h"
#include "AppGlobals.h"
#include "SettingsManager.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient mqttWifiClient;
static PubSubClient mqtt(mqttWifiClient);

bool mqttConnected = false;

static unsigned long lastReconnectAttempt = 0;
static String topicPrefix;
static bool mqttInitialized = false;

static String topic(const char *suffix) {
  return topicPrefix + "/" + suffix;
}

static bool mqttReconnect() {
  String clientId = "espoolman-" + WiFi.macAddress();
  clientId.replace(":", "");

  bool ok;
  if (settings.mqttUser.isEmpty()) {
    ok = mqtt.connect(clientId.c_str());
  } else {
    ok = mqtt.connect(clientId.c_str(), settings.mqttUser.c_str(), settings.mqttPass.c_str());
  }

  if (ok) {
    logMessage("[INFO] MQTT connected to " + settings.mqttHost);
    mqtt.publish(topic("status").c_str(), "online", true);
  }
  return ok;
}

void initMqtt() {
  if (!settings.mqttEnabled || settings.mqttHost.isEmpty()) return;

  topicPrefix = settings.mqttTopic;
  mqttWifiClient.setTimeout(3);  // 3 second TCP timeout (not 15)
  mqtt.setServer(settings.mqttHost.c_str(), settings.mqttPort);
  mqtt.setBufferSize(512);
  mqtt.setSocketTimeout(3);      // 3 second socket timeout
  mqtt.setKeepAlive(60);
  mqttInitialized = true;
  logMessage("[INFO] MQTT configured: " + settings.mqttHost + ":" + String(settings.mqttPort));
}

void loopMqtt() {
  if (!mqttInitialized) return;
  if (WiFi.status() != WL_CONNECTED) {
    mqttConnected = false;
    return;
  }

  if (!mqtt.connected()) {
    mqttConnected = false;
    if (millis() - lastReconnectAttempt >= 30000) {
      lastReconnectAttempt = millis();
      mqttReconnect();
    }
  } else {
    mqttConnected = true;
  }

  mqtt.loop();
}

void mqttPublishWeight(float weight) {
  if (!mqttConnected) return;
  char payload[32];
  snprintf(payload, sizeof(payload), "%.1f", weight);
  mqtt.publish(topic("weight").c_str(), payload);
}

void mqttPublishSpoolChange(int spoolId, const String &name) {
  if (!mqttConnected) return;
  JsonDocument doc;
  doc["spool_id"] = spoolId;
  doc["name"] = name;
  String payload;
  serializeJson(doc, payload);
  mqtt.publish(topic("spool").c_str(), payload.c_str());
}

void mqttPublishNfcEvent(int spoolId, const String &tagType) {
  if (!mqttConnected) return;
  JsonDocument doc;
  doc["spool_id"] = spoolId;
  doc["tag_type"] = tagType;
  doc["timestamp"] = millis();
  String payload;
  serializeJson(doc, payload);
  mqtt.publish(topic("nfc").c_str(), payload.c_str());
}
