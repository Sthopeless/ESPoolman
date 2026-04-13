#include "OtaManager.h"

#include <Update.h>

#include "AppGlobals.h"

namespace {

const char *kOtaHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESPoolman OTA</title>
  <style>
    body {
      margin: 0;
      font-family: "Segoe UI", sans-serif;
      background: #111315;
      color: #ece7dd;
      display: grid;
      place-items: center;
      min-height: 100vh;
      padding: 20px;
    }
    .card {
      width: min(100%, 560px);
      background: #1b1f22;
      border: 1px solid #2d3338;
      border-radius: 18px;
      padding: 24px;
    }
    input, button {
      width: 100%;
      margin-top: 12px;
      border-radius: 12px;
      border: 1px solid #2d3338;
      padding: 12px;
      font-size: 14px;
      background: #121518;
      color: #ece7dd;
    }
    button {
      background: #d96b1d;
      border: none;
      cursor: pointer;
      font-weight: 600;
    }
    progress {
      width: 100%;
      margin-top: 16px;
      height: 18px;
    }
    .muted {
      color: #a89f92;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>OTA Update</h1>
    <p class="muted">Upload a compiled firmware <code>.bin</code> file to update the ESP directly from the browser.</p>
    <input type="file" id="firmware" accept=".bin">
    <button type="button" onclick="uploadFirmware()">Upload Firmware</button>
    <progress id="progress" value="0" max="100"></progress>
    <p id="status" class="muted">Waiting for file.</p>
  </div>

  <script>
    function uploadFirmware() {
      const fileInput = document.getElementById('firmware');
      const status = document.getElementById('status');
      const progress = document.getElementById('progress');
      if (!fileInput.files.length) {
        status.textContent = 'Choose a firmware .bin file first.';
        return;
      }

      const formData = new FormData();
      formData.append('update', fileInput.files[0]);

      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/update');

      xhr.upload.addEventListener('progress', event => {
        if (!event.lengthComputable) return;
        const value = Math.round((event.loaded / event.total) * 100);
        progress.value = value;
        status.textContent = 'Uploading: ' + value + '%';
      });

      xhr.onload = () => {
        status.textContent = xhr.status === 200 ? 'Update successful. Rebooting...' : 'Update failed: ' + xhr.responseText;
      };

      xhr.onerror = () => {
        status.textContent = 'Upload failed.';
      };

      xhr.send(formData);
    }
  </script>
</body>
</html>
)rawliteral";

}  // namespace

void setupOtaRoutes() {
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", kOtaHtml);
  });

  server.on(
    "/update",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {
      const bool hasError = Update.hasError();
      AsyncWebServerResponse *response = request->beginResponse(
        hasError ? 500 : 200,
        "text/plain",
        hasError ? "OTA update failed" : "OTA update complete. Rebooting.");
      response->addHeader("Connection", "close");
      request->send(response);

      if (!hasError) {
        logMessage("[INFO] OTA update completed successfully");
        scheduleRestart("ota update complete", 1000);
      } else {
        logMessage("[ERROR] OTA update failed");
      }
    },
    [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (index == 0) {
        logMessage("[INFO] OTA upload started: " + filename);
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      }

      if (len > 0) {
        if (Update.write(data, len) != len) {
          Update.printError(Serial);
        }
      }

      if (final) {
        if (Update.end(true)) {
          logMessage("[INFO] OTA upload finished");
        } else {
          Update.printError(Serial);
        }
      }
    });
}
