#include "NfcManager.h"

#include <MFRC522Debug.h>

#include "AppGlobals.h"
#include "SettingsManager.h"
#include "SpoolmanClient.h"

namespace {

struct UltralightLayout {
  byte startPage;
  byte lastUserPage;
  size_t userBytes;
  const char *source;
};

// Builds a single NDEF Well-Known Text record (TNF=0x01, type="T", UTF-8, lang="en").
// SR=1 (1-byte payload length) for payloads < 256 bytes; SR=0 (4-byte) otherwise.
String buildNdefTextRecord(const String &text) {
  // Payload: 0x02 (UTF-8, lang_len=2) + "en" + text
  String textPayload;
  textPayload.reserve(3 + text.length());
  textPayload += static_cast<char>(0x02);
  textPayload += 'e';
  textPayload += 'n';
  textPayload += text;

  const size_t payloadLen = textPayload.length();
  String record;
  if (payloadLen < 256) {
    record.reserve(4 + payloadLen);
    record += static_cast<char>(0xD1);  // MB=1, ME=1, SR=1, TNF=0x01
    record += static_cast<char>(0x01);  // type length = 1
    record += static_cast<char>(static_cast<byte>(payloadLen));
    record += 'T';
  } else {
    record.reserve(7 + payloadLen);
    record += static_cast<char>(0xC1);  // MB=1, ME=1, SR=0, TNF=0x01
    record += static_cast<char>(0x01);
    record += static_cast<char>((payloadLen >> 24) & 0xFF);
    record += static_cast<char>((payloadLen >> 16) & 0xFF);
    record += static_cast<char>((payloadLen >> 8) & 0xFF);
    record += static_cast<char>(payloadLen & 0xFF);
    record += 'T';
  }
  record += textPayload;
  return record;
}

// Wraps an NDEF record in a TLV envelope required by NTAG/Ultralight and MIFARE.
// Format: 0x03 <len> <ndef_record> 0xFE
String buildNdefTlv(const String &ndefRecord) {
  const size_t len = ndefRecord.length();
  String tlv;

  if (len < 0xFF) {
    tlv.reserve(2 + len + 1);
  } else {
    tlv.reserve(4 + len + 1);
  }

  tlv += static_cast<char>(0x03);  // NDEF Message TLV
  if (len < 0xFF) {
    tlv += static_cast<char>(static_cast<byte>(len));
  } else {
    tlv += static_cast<char>(0xFF);
    tlv += static_cast<char>((len >> 8) & 0xFF);
    tlv += static_cast<char>(len & 0xFF);
  }

  tlv += ndefRecord;
  tlv += static_cast<char>(0xFE);  // Terminator TLV
  return tlv;
}

// Finds SPOOL:X and FILAMENT:Y tokens in rawData.
// Returns true if a valid (> 0) spool ID was found.
bool parseNfcText(const String &raw, int &spoolId, int &filamentId) {
  spoolId = 0;
  filamentId = 0;
  const int si = raw.indexOf("SPOOL:");
  if (si >= 0) spoolId = raw.substring(si + 6).toInt();
  const int fi = raw.indexOf("FILAMENT:");
  if (fi >= 0) filamentId = raw.substring(fi + 9).toInt();
  return spoolId > 0;
}

bool isClassicTrailerBlock(byte block) {
  return (block + 1) % 4 == 0;
}

bool authenticateClassicBlock(byte block) {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; ++i) {
    key.keyByte[i] = 0xFF;
  }

  return mfrc522.PCD_Authenticate(
           MFRC522Constants::PICC_CMD_MF_AUTH_KEY_A,
           block,
           &key,
           &(mfrc522.uid)) == MFRC522::StatusCode::STATUS_OK;
}

String detectTagTypeName() {
  const MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  return String(MFRC522Debug::PICC_GetTypeName(piccType));
}


bool getUltralightLayout(UltralightLayout &layout, String &errorMessage) {
  layout.startPage = NFC_ULTRALIGHT_START_PAGE;
  // Conservative default for plain MIFARE Ultralight (pages 4-15, 48 bytes).
  // If CC detection succeeds we upgrade to the actual size.
  layout.lastUserPage = 15;
  layout.userBytes = (15 - NFC_ULTRALIGHT_START_PAGE + 1) * 4;
  layout.source = "default-ul";

  // Read Capability Container at page 3 (NTAG2xx only).
  // MIFARE_Read returns 16 bytes (pages 3-6) in one shot.
  byte ccBuffer[18] = {0};
  byte ccSize = sizeof(ccBuffer);
  const MFRC522::StatusCode ccStatus = mfrc522.MIFARE_Read(3, ccBuffer, &ccSize);

  char ccLog[64];
  snprintf(ccLog, sizeof(ccLog), "[INFO] UL CC: status=%d b0=0x%02X b1=0x%02X b2=0x%02X",
           (int)ccStatus, ccBuffer[0], ccBuffer[1], ccBuffer[2]);
  logMessage(ccLog);

  if (ccStatus == MFRC522::StatusCode::STATUS_OK && ccSize >= 4
      && ccBuffer[0] == 0xE1 && ccBuffer[2] > 0) {
    // NTAG2xx / MIFARE Ultralight C: CC byte 2 is mem-size in units of 8 bytes
    layout.userBytes = static_cast<size_t>(ccBuffer[2]) * 8;
    layout.lastUserPage = static_cast<byte>(4 + (layout.userBytes / 4) - 1);
    layout.source = "CC";
  } else {
    logMessage("[WARN] No valid CC (0xE1) at page 3 — using MIFARE Ultralight default (pages 4-15)");
  }

  logMessage("[INFO] UL layout: pages " + String(layout.startPage) + "-"
             + String(layout.lastUserPage) + " src=" + String(layout.source));

  if (layout.lastUserPage < layout.startPage) {
    errorMessage = "Tag user memory ends before application start page";
    return false;
  }

  return true;
}

bool readClassicTag(String &rawData) {
  byte buffer[18];
  byte byteCount = sizeof(buffer);

  for (byte block = NFC_CLASSIC_START_BLOCK; block <= NFC_CLASSIC_MAX_BLOCK; ++block) {
    if (isClassicTrailerBlock(block)) {
      continue;
    }

    if (!authenticateClassicBlock(block)) {
      return false;
    }

    byteCount = sizeof(buffer);
    const MFRC522::StatusCode status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::StatusCode::STATUS_OK) {
      return false;
    }

    for (byte i = 0; i < 16 && rawData.length() < NFC_READ_BUFFER_LIMIT; ++i) {
      if (buffer[i] == '\0') {
        return true;
      }
      rawData += static_cast<char>(buffer[i]);
    }
  }

  return !rawData.isEmpty();
}

bool writeClassicTag(const String &payload, String &errorMessage) {
  String data = payload;
  while (data.length() % 16 != 0) {
    data += '\0';
  }

  byte buffer[16];
  size_t offset = 0;
  byte nextBlock = NFC_CLASSIC_MAX_BLOCK + 1;

  for (byte block = NFC_CLASSIC_START_BLOCK; block <= NFC_CLASSIC_MAX_BLOCK && offset < data.length(); ++block) {
    if (isClassicTrailerBlock(block)) {
      continue;
    }

    if (!authenticateClassicBlock(block)) {
      errorMessage = "MIFARE Classic authentication failed";
      return false;
    }

    memset(buffer, 0, sizeof(buffer));
    for (byte i = 0; i < 16 && offset < data.length(); ++i, ++offset) {
      buffer[i] = static_cast<byte>(data[offset]);
    }

    const MFRC522::StatusCode status = mfrc522.MIFARE_Write(block, buffer, sizeof(buffer));
    if (status != MFRC522::StatusCode::STATUS_OK) {
      errorMessage = "Write failed at block " + String(block);
      return false;
    }

    nextBlock = block + 1;
  }

  if (offset < data.length()) {
    errorMessage = "Payload is too large for this MIFARE Classic tag";
    return false;
  }

  while (nextBlock <= NFC_CLASSIC_MAX_BLOCK && isClassicTrailerBlock(nextBlock)) {
    ++nextBlock;
  }

  if (nextBlock <= NFC_CLASSIC_MAX_BLOCK) {
    memset(buffer, 0, sizeof(buffer));
    if (!authenticateClassicBlock(nextBlock)) {
      errorMessage = "Failed to write terminator block";
      return false;
    }
    const MFRC522::StatusCode status = mfrc522.MIFARE_Write(nextBlock, buffer, sizeof(buffer));
    if (status != MFRC522::StatusCode::STATUS_OK) {
      errorMessage = "Failed to write terminator block " + String(nextBlock);
      return false;
    }
  }

  return true;
}

bool readUltralightTag(String &rawData) {
  UltralightLayout layout;
  String errorMessage;
  if (!getUltralightLayout(layout, errorMessage)) {
    logMessage("[ERROR] " + errorMessage);
    return false;
  }

  byte buffer[18];
  byte byteCount;
  bool foundTerminator = false;

  for (byte page = layout.startPage; page <= layout.lastUserPage && !foundTerminator; page += 4) {
    byteCount = sizeof(buffer);
    const MFRC522::StatusCode status = mfrc522.MIFARE_Read(page, buffer, &byteCount);
    if (status != MFRC522::StatusCode::STATUS_OK) {
      // Tag NAKed — this is normal at the end of MIFARE Ultralight (page 16+).
      // If we already collected data, treat this as end-of-memory, not an error.
      logMessage("[INFO] UL page " + String(page) + " read status=" + String((int)status)
                 + " rawData=" + String(rawData.length()) + "B — stopping");
      break;
    }

    for (byte i = 0; i < 16 && rawData.length() < NFC_READ_BUFFER_LIMIT; ++i) {
      if (buffer[i] == 0xFE) {
        // NDEF Terminator TLV — all useful data has been read
        foundTerminator = true;
        break;
      }
      if (buffer[i] != '\0') {
        rawData += static_cast<char>(buffer[i]);
      }
    }
  }

  logMessage("[INFO] UL read done: rawData=" + String(rawData.length())
             + "B terminator=" + String(foundTerminator ? "yes" : "no"));
  return !rawData.isEmpty();
}

bool writeUltralightTag(const String &payload, String &errorMessage) {
  UltralightLayout layout;
  if (!getUltralightLayout(layout, errorMessage)) {
    return false;
  }

  // Write Capability Container to page 3 if it was absent (no valid 0xE1 magic).
  // This initialises a blank or mis-formatted tag so readers can find the NDEF message.
  if (strcmp(layout.source, "default-ul") == 0) {
    byte cc[4] = {0xE1, 0x10, static_cast<byte>(layout.userBytes / 8), 0x00};
    const MFRC522::StatusCode ccStatus = mfrc522.MIFARE_Ultralight_Write(3, cc, sizeof(cc));
    if (ccStatus != MFRC522::StatusCode::STATUS_OK) {
      logMessage("[WARN] Could not write CC to page 3 — tag may not be NDEF-formatted after write");
    } else {
      logMessage("[INFO] CC written to page 3");
    }
  }

  String data = payload;
  while (data.length() % 4 != 0) {
    data += '\0';
  }

  const size_t requiredPages = data.length() / 4;
  const size_t availablePages = (layout.lastUserPage - layout.startPage) + 1;
  if (requiredPages > availablePages) {
    errorMessage = "Payload is too large for this Ultralight/NTAG tag";
    return false;
  }

  byte buffer[4];
  size_t offset = 0;
  byte nextPage = layout.lastUserPage + 1;

  for (byte page = layout.startPage; page <= layout.lastUserPage && offset < data.length(); ++page) {
    memset(buffer, 0, sizeof(buffer));
    for (byte i = 0; i < 4 && offset < data.length(); ++i, ++offset) {
      buffer[i] = static_cast<byte>(data[offset]);
    }

    const MFRC522::StatusCode status = mfrc522.MIFARE_Ultralight_Write(page, buffer, sizeof(buffer));
    if (status != MFRC522::StatusCode::STATUS_OK) {
      errorMessage = "Write failed at page " + String(page);
      return false;
    }

    nextPage = page + 1;
  }

  if (nextPage <= layout.lastUserPage) {
    memset(buffer, 0, sizeof(buffer));
    const MFRC522::StatusCode status = mfrc522.MIFARE_Ultralight_Write(nextPage, buffer, sizeof(buffer));
    if (status != MFRC522::StatusCode::STATUS_OK) {
      errorMessage = "Failed to write terminator page " + String(nextPage);
      return false;
    }
  }

  return true;
}

}  // namespace

bool initializeNfcReader(bool logDetails) {
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(MFRC522Constants::PCD_RxGain::RxGain_max);
  mfrc522.PCD_AntennaOff();
  mfrc522.PCD_AntennaOn();

  if (logDetails) {
    MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);
    delay(100);
  }

  const bool selfTestOk = mfrc522.PCD_PerformSelfTest();

  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(MFRC522Constants::PCD_RxGain::RxGain_max);
  mfrc522.PCD_AntennaOff();
  mfrc522.PCD_AntennaOn();

  ledNfc = selfTestOk;
  return selfTestOk;
}

void resetMFRC() {
  digitalWrite(RST_PIN, LOW);
  delayMicroseconds(100);
  digitalWrite(RST_PIN, HIGH);

  if (initializeNfcReader(false)) {
    logMessage("RC522 reset successful");
  } else {
    logMessage("[ERROR] RC522 self-test failed after reset");
  }
}

bool writeNFC(String &errorMessage) {
  if (!settings.writeNfcEnabled) {
    errorMessage = "NFC writing is disabled";
    pendingWriteStatus = errorMessage;
    return false;
  }

  const int spoolId = settings.newNfcId.toInt();
  const int filamentId = settings.newNfcFilId.toInt();
  logMessage("[INFO] Write NFC armed: spoolId=" + String(spoolId) + " filamentId=" + String(filamentId));

  if (spoolId <= 0) {
    errorMessage = "No spool selected for writing";
    pendingWriteStatus = errorMessage;
    logMessage("[ERROR] Write aborted: " + errorMessage);
    return false;
  }

  const String nfcText = "SPOOL:" + String(spoolId) + "\nFILAMENT:" + String(filamentId);
  const String ndefRecord = buildNdefTextRecord(nfcText);
  const String writePayload = buildNdefTlv(ndefRecord);
  logMessage("[INFO] Write NFC text: \"" + nfcText + "\"");
  logMessage("[INFO] Write payload: ndefRecord=" + String(ndefRecord.length()) + "B tlv=" + String(writePayload.length()) + "B tag=[" + lastDetectedTagType + "]");

  const MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  bool writeOk = false;

  switch (piccType) {
    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_1K:
    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_4K:
    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_MINI:
      writeOk = writeClassicTag(writePayload, errorMessage);
      break;

    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_UL:
      writeOk = writeUltralightTag(writePayload, errorMessage);
      break;

    default:
      errorMessage = "Unsupported tag type: " + lastDetectedTagType;
      break;
  }

  if (!writeOk) {
    logMessage("[ERROR] " + errorMessage);
    setLedColor(LED_HUE_RED);
    pendingWriteStatus = errorMessage;
    return false;
  }

  prefs.putString(KEY_NFC_JSON, nfcText);
  logMessage("Write successful on " + lastDetectedTagType);
  pendingWriteStatus = "Write successful on " + lastDetectedTagType;
  setLedColor(LED_HUE_GREEN);
  return true;
}

void readNFC() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  ledNfc = true;
  logMessage("[INFO] NFC tag detected");

  lastDetectedTagType = detectTagTypeName();
  prefs.putString(KEY_NFC_TAG_TYPE, lastDetectedTagType);
  logMessage("[INFO] NFC tag type: " + lastDetectedTagType);

  const MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  String rawData;
  rawData.reserve(NFC_READ_BUFFER_LIMIT);
  bool readOk = false;

  if (pendingNfcWrite) {
    String errorMessage;
    if (writeNFC(errorMessage)) {
      pendingNfcWrite = false;
      pendingWriteStartedAtMs = 0;
      notifyUiRefresh("write-complete");
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      resetMFRC();
      return;
    }

    cancelPendingNfcWrite(errorMessage);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    resetMFRC();
    return;
  }

  switch (piccType) {
    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_1K:
    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_4K:
    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_MINI:
      readOk = readClassicTag(rawData);
      break;

    case MFRC522::PICC_Type::PICC_TYPE_MIFARE_UL:
      readOk = readUltralightTag(rawData);
      break;

    default:
      logMessage("[ERROR] Unsupported tag type for NFC spool data: " + lastDetectedTagType);
      setLedColor(LED_HUE_RED);
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
  }

  if (!readOk) {
    logMessage("[ERROR] Failed to read NFC payload from " + lastDetectedTagType);
    setLedColor(LED_HUE_RED);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  // Log raw data for diagnostics
  logMessage("[INFO] Raw NFC length=" + String(rawData.length()) + "B");
  {
    String hexDump;
    hexDump.reserve(60);
    for (size_t i = 0; i < rawData.length() && i < 20; ++i) {
      char h[4];
      snprintf(h, sizeof(h), "%02X ", (uint8_t)rawData[i]);
      hexDump += h;
    }
    logMessage("[INFO] Raw hex (first 20B): " + hexDump);
    String printable;
    printable.reserve(64);
    for (size_t i = 0; i < rawData.length() && printable.length() < 64; ++i) {
      const char c = rawData[i];
      printable += (c >= 0x20 && c < 0x7F) ? c : '.';
    }
    logMessage("[INFO] Raw printable: " + printable);
  }

  int spoolId = 0, filamentId = 0;
  if (!parseNfcText(rawData, spoolId, filamentId)) {
    logMessage("[ERROR] No SPOOL: token found in NFC data");
    setLedColor(LED_HUE_RED);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  logMessage("[INFO] Parsed: SPOOL=" + String(spoolId) + " FILAMENT=" + String(filamentId));

  // Store only clean ASCII text — rawData contains binary NDEF header bytes that
  // corrupt the /data JSON response and silently break the entire WebUI.
  prefs.putString(KEY_NFC_JSON, "SPOOL:" + String(spoolId) + "\nFILAMENT:" + String(filamentId));
  prefs.putInt(KEY_NFC_ID, spoolId);
  prefs.putInt(KEY_NFC_FILAMENT_ID, filamentId);
  // Clear stale Spoolman fields now so the UI shows a clean slate immediately
  prefs.putString(KEY_NFC_NAME,       "");
  prefs.putString(KEY_NFC_BRAND,      "");
  prefs.putString(KEY_NFC_COLOR,      "");
  prefs.putString(KEY_NFC_TYPE,       "");
  prefs.putString(KEY_NFC_MIN_TEMP,   "");
  prefs.putString(KEY_NFC_MAX_TEMP,   "");
  prefs.putString(KEY_NFC_LOCATION,   "");
  prefs.putString(KEY_NFC_REMAINING,  "");
  prefs.putString(KEY_NFC_WEIGHT,     "");
  prefs.putString(KEY_NFC_DIAMETER,   "");
  prefs.putString(KEY_NFC_DENSITY,    "");
  prefs.putString(KEY_NFC_FLOW_RATIO, "");
  prefs.putString(KEY_NFC_MAX_SPEED,  "");
  prefs.putString(KEY_NFC_SPOOL_WT,   "");

  // Halt tag before making HTTP calls — tag is no longer needed
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Notify UI immediately — spool ID / filament ID appear at once, before the Spoolman HTTP call
  setLedColor(LED_HUE_YELLOW);
  notifyUiRefresh("nfc-read");

  // Fetch full spool data from Spoolman
  logMessage("[INFO] Fetching spool " + String(spoolId) + " from Spoolman...");
  JsonDocument spoolDoc;
  String fetchError;
  if (fetchSpoolById(spoolId, spoolDoc, fetchError)) {
    const JsonVariantConst filament = spoolDoc["filament"];
    const JsonVariantConst extra    = filament["extra"];
    char buf[16];

    const String nfcName      = String(filament["name"] | "");
    const String nfcBrand     = String(filament["vendor"]["name"] | "");
    const String nfcColor     = normalizeColorHex(String(filament["color_hex"] | "FFFFFF"));
    const String nfcMaterial  = String(filament["material"] | "");
    const int    extTemp      = (int)(filament["settings_extruder_temp"] | 0);
    const int    bedTemp      = (int)(filament["settings_bed_temp"] | 0);
    const String location     = String(spoolDoc["location"] | "");
    const double remaining    = (double)(spoolDoc["remaining_weight"] | 0.0);
    const double weight       = (double)(spoolDoc["initial_weight"] | 0.0);
    const double diameter     = (double)(filament["diameter"] | 0.0);
    const double density      = (double)(filament["density"] | 0.0);
    const String flowRatio    = String(extra["flow_ratio"] | "");
    const String maxSpeed     = String(extra["max_volumetric_speed"] | "");
    // spool_weight may appear at spool level or filament level
    double spoolWt = (double)(spoolDoc["spool_weight"] | 0.0);
    if (spoolWt == 0.0) spoolWt = (double)(filament["spool_weight"] | 0.0);

    prefs.putString(KEY_NFC_NAME,      nfcName);
    prefs.putString(KEY_NFC_BRAND,     nfcBrand);
    prefs.putString(KEY_NFC_COLOR,     nfcColor);
    prefs.putString(KEY_NFC_TYPE,      nfcMaterial);
    prefs.putString(KEY_NFC_LOCATION,  location);
    snprintf(buf, sizeof(buf), "%d", extTemp);
    prefs.putString(KEY_NFC_MIN_TEMP,  buf);
    snprintf(buf, sizeof(buf), "%d", bedTemp);
    prefs.putString(KEY_NFC_MAX_TEMP,  buf);
    snprintf(buf, sizeof(buf), "%.1f", remaining);
    prefs.putString(KEY_NFC_REMAINING, buf);
    snprintf(buf, sizeof(buf), "%.1f", weight);
    prefs.putString(KEY_NFC_WEIGHT,    buf);
    snprintf(buf, sizeof(buf), "%.2f", diameter);
    prefs.putString(KEY_NFC_DIAMETER,  buf);
    snprintf(buf, sizeof(buf), "%.2f", density);
    prefs.putString(KEY_NFC_DENSITY,   buf);
    prefs.putString(KEY_NFC_FLOW_RATIO, flowRatio);
    prefs.putString(KEY_NFC_MAX_SPEED,  maxSpeed);
    snprintf(buf, sizeof(buf), "%.1f", spoolWt);
    prefs.putString(KEY_NFC_SPOOL_WT,  buf);

    logMessage("[INFO] Spoolman OK: \"" + nfcName + "\" " + nfcMaterial
               + " brand=\"" + nfcBrand + "\""
               + " color=#" + nfcColor
               + " ext=" + String(extTemp) + "C bed=" + String(bedTemp) + "C"
               + " diam=" + String(diameter, 2) + "mm dens=" + String(density, 2)
               + " flow=" + flowRatio + " speed=" + maxSpeed
               + " rem=" + String(remaining, 1) + "g loc=\"" + location + "\"");
    logMessage("Read tag [" + lastDetectedTagType + "] SPOOL:" + String(spoolId) +
               " FILAMENT:" + String(filamentId) + " \"" + nfcName + "\"");
  } else {
    logMessage("[WARN] Spoolman fetch failed for spool " + String(spoolId) + ": " + fetchError);
    logMessage("Read tag [" + lastDetectedTagType + "] SPOOL:" + String(spoolId) + " (no Spoolman data)");
  }

  // Second push — updates Spoolman-derived fields (or shows them as empty if fetch failed)
  notifyUiRefresh("nfc-spoolman");

  // validate=false: we already fetched this spool successfully above, skip the redundant fetchSpoolmanSpools() call
  setSpool(spoolId, false);
  notifyUiRefresh("spool-updated");
}
