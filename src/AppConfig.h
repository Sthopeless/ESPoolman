#pragma once

#include <Arduino.h>

#define VER "2.1"

// PINs
#define SW_WIFI 9  // GPIO9 = BOOT button on ESP32-C6 Super Mini (GPIO12/13 are USB D-/D+)

// WS2812
#define NUMLEDS 1
#define PIN_DATA 8 // onboard LED on ESP32-C6 supermini
#define BRIGHTNESS 100
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
inline constexpr uint8_t LED_HUE_RED = 0;
inline constexpr uint8_t LED_HUE_BLUE = 150;
inline constexpr uint8_t LED_HUE_GREEN = 100;
inline constexpr uint8_t LED_HUE_YELLOW = 30;
inline constexpr uint8_t LED_HUE_LIME = 60;

// RC522
#define SS_PIN 7
#define RST_PIN 6
#define SCK_PIN 4
#define MOSI_PIN 5
#define MISO_PIN 2

inline constexpr size_t LOGGER_LIMIT = 6000;
inline constexpr size_t NFC_READ_BUFFER_LIMIT = 768;
inline constexpr size_t NFC_CLASSIC_MAX_BLOCK = 63;
inline constexpr byte NFC_ULTRALIGHT_START_PAGE = 4;
inline constexpr byte NFC_ULTRALIGHT_MAX_PAGE = 39;
inline constexpr byte NFC_CLASSIC_START_BLOCK = 4;
inline constexpr const char APP_BRAND[] = "ESPoolman";
inline constexpr const char AP_SSID[] = "ESPoolman";
inline constexpr const char NVS_NAMESPACE[] = "espoolman";
inline constexpr const char KEY_DEVICE_CONFIG[] = "device_config";
inline constexpr unsigned long NFC_WRITE_TIMEOUT_MS = 30000;

inline constexpr const char *KEY_SERVER_SPOOLMAN = "serverSpoolman";
inline constexpr const char *KEY_SERVER_MOONRAKER = "serverMoonraker";
inline constexpr const char *KEY_NFC_ID = "nfc_id";
inline constexpr const char *KEY_NFC_NAME = "nfc_name";
inline constexpr const char *KEY_NFC_BRAND = "nfc_brand";
inline constexpr const char *KEY_NFC_COLOR = "nfc_color";
inline constexpr const char *KEY_NFC_JSON = "nfc_json";
inline constexpr const char *KEY_NFC_MIN_TEMP = "nfc_min_temp";
inline constexpr const char *KEY_NFC_MAX_TEMP = "nfc_max_temp";
inline constexpr const char *KEY_NFC_TYPE = "nfc_type";
inline constexpr const char *KEY_NFC_LOCATION = "nfc_location";
inline constexpr const char *KEY_NFC_FILAMENT_ID = "nfc_fil_id";     // 10 chars ok
inline constexpr const char *KEY_NFC_REMAINING   = "nfc_remaining";  // 13 chars ok
inline constexpr const char *KEY_NFC_WEIGHT      = "nfc_weight";     // 10 chars ok
inline constexpr const char *KEY_NFC_DIAMETER    = "nfc_diameter";   // 12 chars ok
inline constexpr const char *KEY_NFC_DENSITY     = "nfc_density";    // 11 chars ok
inline constexpr const char *KEY_NFC_FLOW_RATIO  = "nfc_flow_ratio"; // 14 chars ok
inline constexpr const char *KEY_NFC_MAX_SPEED   = "nfc_max_speed";  // 13 chars ok
inline constexpr const char *KEY_NFC_SPOOL_WT    = "nfc_spool_wt";   // 12 chars ok
inline constexpr const char *KEY_ID = "id";
inline constexpr const char *KEY_FILAMENT = "filament";
inline constexpr const char *KEY_NFC_TAG_TYPE = "nfc_tag_type";
inline constexpr const char *KEY_WIFI_SSID = "wifi_ssid";
inline constexpr const char *KEY_WIFI_PASS = "wifi_pass";
inline constexpr const char *KEY_FIRST_TIME_SETUP = "first_time_setup";
inline constexpr const char *KEY_WRITE_NFC_ENABLED = "write_nfc_enabled";
inline constexpr const char *KEY_NEW_NFC_ID = "new_nfc_id";
inline constexpr const char *KEY_NEW_NFC_FIL_ID = "new_nfc_fil_id";
inline constexpr const char *KEY_DEFAULT_TAB = "default_tab";
inline constexpr const char *KEY_SHOW_WIFI = "show_wifi";
inline constexpr const char *KEY_SHOW_SERVERS = "show_servers";
inline constexpr const char *KEY_SHOW_CURRENT_SPOOL = "show_current_spool";
inline constexpr const char *KEY_SHOW_NFC_DATA = "show_nfc_data";
inline constexpr const char *KEY_SHOW_LOGGER = "show_logger";
inline constexpr const char *KEY_SHOW_NFC_JSON = "show_nfc_json";
inline constexpr const char *KEY_SHOW_TAB_DASHBOARD = "show_tab_dashboard";
inline constexpr const char *KEY_SHOW_TAB_NFC = "show_tab_nfc";
inline constexpr const char *KEY_SHOW_TAB_WRITE = "show_tab_write";
inline constexpr const char *KEY_SHOW_TAB_OPTIONS = "show_tab_options";
inline constexpr const char *KEY_SHOW_TAB_LOGS = "show_tab_logs";
inline constexpr const char *KEY_SHOW_TAB_SPOOLS = "show_tab_spools";
inline constexpr const char *BASIC_AUTH_USER = "espoolman";
inline constexpr const char *BASIC_AUTH_PASS = "espoolman";

// HX711 scale defaults
inline constexpr int DEFAULT_HX711_DOUT_PIN = 3;
inline constexpr int DEFAULT_HX711_SCK_PIN = 10;
inline constexpr float DEFAULT_HX711_CALIBRATION = 420.0f;

// SSD1306 OLED defaults (I2C)
inline constexpr int DEFAULT_OLED_SDA_PIN = 18;
inline constexpr int DEFAULT_OLED_SCL_PIN = 19;
inline constexpr uint8_t DEFAULT_OLED_ADDR = 0x3C;

// Scale buttons
inline constexpr int DEFAULT_BTN_TARE_PIN = 0;
inline constexpr int DEFAULT_BTN_ENTER_PIN = 1;

// MQTT defaults
inline constexpr int DEFAULT_MQTT_PORT = 1883;

// NFC dashboard field show/hide (all under 15 chars)
inline constexpr const char *KEY_SHOW_NFC_FIELD_NAME     = "snf_name";   // 8
inline constexpr const char *KEY_SHOW_NFC_FIELD_MATERIAL = "snf_mat";    // 7
inline constexpr const char *KEY_SHOW_NFC_FIELD_BRAND    = "snf_brand";  // 9
inline constexpr const char *KEY_SHOW_NFC_FIELD_COLOR    = "snf_color";  // 9
inline constexpr const char *KEY_SHOW_NFC_FIELD_EXT_TEMP = "snf_ext_t";  // 9
inline constexpr const char *KEY_SHOW_NFC_FIELD_BED_TEMP = "snf_bed_t";  // 9
inline constexpr const char *KEY_SHOW_NFC_FIELD_LOCATION = "snf_loc";    // 7
inline constexpr const char *KEY_SHOW_NFC_FIELD_REMAINING= "snf_rem";    // 7
inline constexpr const char *KEY_SHOW_NFC_FIELD_WEIGHT   = "snf_wt";     // 6
inline constexpr const char *KEY_SHOW_NFC_FIELD_DIAMETER = "snf_diam";   // 8
inline constexpr const char *KEY_SHOW_NFC_FIELD_DENSITY  = "snf_den";    // 7
inline constexpr const char *KEY_SHOW_NFC_FIELD_FLOW     = "snf_flow";   // 8
inline constexpr const char *KEY_SHOW_NFC_FIELD_MAX_SPEED= "snf_speed";  // 9
inline constexpr const char *KEY_SHOW_NFC_FIELD_SPOOL_WT = "snf_spwt";   // 8
