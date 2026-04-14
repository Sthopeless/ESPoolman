#include "ScaleManager.h"
#include "AppGlobals.h"
#include "AppConfig.h"
#include "SettingsManager.h"
#include "SpoolmanClient.h"
#include "MqttManager.h"

#include <HX711.h>
#include <U8g2lib.h>
#include <Wire.h>

static HX711 scale;
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C *oled = nullptr;

float lastScaleWeight = 0.0f;
bool scaleConnected = false;

static unsigned long lastReadMs = 0;
static unsigned long lastBtnCheckMs = 0;
static bool lastTareState = HIGH;
static bool lastEnterState = HIGH;

static void drawOled(float weight) {
  if (!oled) return;
  oled->clearBuffer();

  oled->setFont(u8g2_font_helvB14_tr);
  oled->drawStr(0, 16, "Weight");

  char buf[32];
  snprintf(buf, sizeof(buf), "%.1f g", weight);
  oled->setFont(u8g2_font_helvB24_tr);
  int w = oled->getStrWidth(buf);
  oled->drawStr((128 - w) / 2, 50, buf);

  oled->setFont(u8g2_font_helvR08_tr);
  if (currentId > 0) {
    snprintf(buf, sizeof(buf), "Spool #%d", currentId);
    oled->drawStr(0, 64, buf);
  } else {
    oled->drawStr(0, 64, "No spool selected");
  }

  oled->sendBuffer();
}

void initScale() {
  if (!settings.scaleEnabled) return;

  scale.begin(settings.scaleDoutPin, settings.scaleSckPin);
  delay(100);

  if (scale.wait_ready_timeout(1000)) {
    scaleConnected = true;
    scale.set_scale(settings.scaleCalibration);
    if (settings.scaleOffset != 0) {
      scale.set_offset(settings.scaleOffset);
    } else {
      scale.tare(10);
      settings.scaleOffset = scale.get_offset();
      saveSettings();
    }
    logMessage("[INFO] HX711 scale initialized");
  } else {
    logMessage("[ERROR] HX711 not found");
    scaleConnected = false;
  }

  if (settings.oledEnabled) {
    Wire.begin(settings.oledSdaPin, settings.oledSclPin);
    oled = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
    oled->begin();
    oled->setContrast(200);
    drawOled(0.0f);
    logMessage("[INFO] SSD1306 OLED initialized");
  }

  pinMode(settings.btnTarePin, INPUT_PULLUP);
  pinMode(settings.btnEnterPin, INPUT_PULLUP);
}

void scaleTare() {
  if (!scaleConnected) return;
  scale.tare(10);
  settings.scaleOffset = scale.get_offset();
  saveSettings();
  lastScaleWeight = 0.0f;
  logMessage("[INFO] Scale tared");
  if (oled) drawOled(0.0f);
}

float scaleGetWeight() {
  return lastScaleWeight;
}

bool scaleIsReady() {
  return scaleConnected && scale.wait_ready_timeout(50);
}

void scaleUpdateDisplay() {
  if (oled) drawOled(lastScaleWeight);
}

void loopScale() {
  if (!settings.scaleEnabled || !scaleConnected) return;

  // Read weight every 250ms
  if (millis() - lastReadMs >= 250) {
    lastReadMs = millis();
    if (scale.wait_ready_timeout(50)) {
      float raw = scale.get_units(3);
      if (raw < 0.5f) raw = 0.0f;
      lastScaleWeight = raw;
      if (oled) drawOled(lastScaleWeight);
    }
  }

  // Check buttons every 50ms (with debounce)
  if (millis() - lastBtnCheckMs >= 50) {
    lastBtnCheckMs = millis();

    bool tareNow = digitalRead(settings.btnTarePin);
    if (lastTareState == HIGH && tareNow == LOW) {
      scaleTare();
    }
    lastTareState = tareNow;

    bool enterNow = digitalRead(settings.btnEnterPin);
    if (lastEnterState == HIGH && enterNow == LOW) {
      if (currentId > 0 && lastScaleWeight > 0.5f) {
        String error;
        if (measureSpoolWeight(currentId, lastScaleWeight, error)) {
          logMessage("[INFO] Weight " + String(lastScaleWeight, 1) + "g sent to Spoolman spool #" + String(currentId));
          buildSpoolsCache();
          notifyUiRefresh("weight-updated");
          mqttPublishWeight(lastScaleWeight);
        } else {
          logMessage("[ERROR] Measure failed: " + error);
        }
      } else if (currentId <= 0) {
        logMessage("[WARN] No spool selected, weight not sent");
      }
    }
    lastEnterState = enterNow;
  }
}
