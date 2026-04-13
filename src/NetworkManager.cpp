#include "NetworkManager.h"
#include "AppGlobals.h"
#include "SettingsManager.h"
#include "WebRoutes.h"
#include "SpoolmanClient.h"
#include <WiFi.h>

static TaskHandle_t networkTaskHandle = nullptr;
static unsigned long lastPeriodicRefreshMs = 0;

static String wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_CONNECTED:
      return "Connected";
    case WL_NO_SSID_AVAIL:
      return "SSID not found";
    case WL_CONNECT_FAILED:
      return "Connect failed";
    case WL_CONNECTION_LOST:
      return "Connection lost";
    case WL_DISCONNECTED:
      return "Disconnected";
    case WL_IDLE_STATUS:
      return "Idle";
    default:
      return "Status " + String(static_cast<int>(status));
  }
}

void startAccessPoint() {
  wifiSettingMode = true;
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  bool apStarted = WiFi.softAP(AP_SSID);
  lastWifiStatus = apStarted ? "AP mode" : "AP start failed";
  if (apStarted) {
    logMessage("AP mode enabled, IP: " + WiFi.softAPIP().toString());
  } else {
    logMessage("[ERROR] AP mode failed to start");
  }
}

void connectWifi() {
  if (settings.wifiSsid.isEmpty()) {
    lastWifiStatus = "No credentials set";
    logMessage("[INFO] No WiFi credentials configured, starting AP mode for setup");
    startAccessPoint();
    return;
  }

  wifiSettingMode = false;
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.disconnect(false, false);
  delay(200);
  WiFi.begin(settings.wifiSsid.c_str(), settings.wifiPass.c_str());
  Serial.print("Connect STA");
  logMessage("Trying WiFi SSID: " + settings.wifiSsid);

  int tries = 40;
  while (WiFi.status() != WL_CONNECTED && tries-- > 0) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    lastWifiStatus = "Connected";
    logMessage("WiFi connected, IP: " + WiFi.localIP().toString());
    return;
  }

  lastWifiStatus = wifiStatusToString(WiFi.status());
  logMessage("[WARN] STA connection failed: " + lastWifiStatus + ". Enabling AP fallback");
  startAccessPoint();
}

String getCurrentIpAddress() {
  if (wifiSettingMode) {
    return WiFi.softAPIP().toString();
  }
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  return "0.0.0.0";
}

String getWifiMode() {
  return wifiSettingMode ? "AP" : "STA";
}

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

static void networkWorker(void *param) {
  while (true) {
    if (statusRequested.exchange(false)) {
      statusServer();
      buildSpoolsCache();
      notifyUiRefresh("status-updated");
    }

    if (millis() - lastPeriodicRefreshMs >= 60000UL) {
      lastPeriodicRefreshMs = millis();
      if (isWifiConnected()) {
        statusServer();
        buildSpoolsCache();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void initNetwork() {
  setupWebRoutes();
  if (digitalRead(SW_WIFI) == LOW) {
    logMessage("[INFO] AP mode forced by button");
    startAccessPoint();
  } else {
    connectWifi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    statusRequested = true;
  }

  server.begin();
  logMessage("[INFO] Web server started");

  if (networkTaskHandle == nullptr) {
    xTaskCreatePinnedToCore(networkWorker, "NetworkWorker", 4096, nullptr, 1, &networkTaskHandle, 0);
  }
}

void requestStatusRefresh() {
  statusRequested = true;
}

