#include "WebUi.h"

const char *getIndexHtml() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
  <title>ESPoolman</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; }

    :root {
      color-scheme: dark;
      --bg: #0f1316;
      --panel: #161b1f;
      --surface: #1d2429;
      --line: #273038;
      --text: #eef2f2;
      --muted: #8a98a6;
      --accent: #ff8a3d;
      --good: #3acc7a;
      --bad: #ff5f6d;
      --shadow: 0 24px 50px rgba(0, 0, 0, 0.24);
      --border-radius: 22px;
    }

    body {
      margin: 0;
      min-height: 100vh;
      background: radial-gradient(circle at top left, rgba(255,138,61,0.15), transparent 24%),
                  linear-gradient(180deg, #111416 0%, #0c1013 100%);
      color: var(--text);
      font-family: "Inter", system-ui, sans-serif;
      -webkit-font-smoothing: antialiased;
      padding-bottom: 88px;
    }

    .page {
      max-width: 1200px;
      margin: 0 auto;
      padding: 24px 20px 16px;
    }

    h1, h2, h3 {
      margin: 0;
      font-weight: 700;
    }

    .pill {
      padding: 8px 14px;
      border-radius: 999px;
      background: rgba(255,255,255,0.04);
      border: 1px solid rgba(255,255,255,0.06);
      color: var(--muted);
      font-size: 13px;
      display: inline-flex;
      align-items: center;
      gap: 7px;
      flex-shrink: 0;
      white-space: nowrap;
    }

    .pill span {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      display: inline-block;
      flex-shrink: 0;
    }

    .grid {
      display: grid;
      gap: 18px;
    }

    .card {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: var(--border-radius);
      box-shadow: var(--shadow);
      padding: 20px;
    }

    .card header {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      align-items: center;
      margin-bottom: 18px;
    }

    .card header > *:first-child {
      flex: 1;
      min-width: 0;
    }

    .card header > *:not(:first-child) {
      flex-shrink: 0;
    }

    .card header h2 {
      font-size: 16px;
    }

    .card small {
      color: var(--muted);
      line-height: 1.5;
    }

    .status-row {
      display: grid;
      gap: 10px;
      grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
    }

    .status-block {
      padding: 14px;
      border-radius: 18px;
      background: rgba(255,255,255,0.03);
      border: 1px solid rgba(255,255,255,0.05);
    }

    .status-block strong {
      display: block;
      font-size: 12px;
      color: var(--muted);
      margin-bottom: 8px;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }

    .status-block span {
      font-size: 18px;
      font-weight: 700;
      word-break: break-word;
    }

    .metric {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      margin-top: 22px;
      flex-wrap: wrap;
    }

    .metric-card {
      flex: 1 1 160px;
      min-width: 160px;
      padding: 16px;
      background: rgba(255,255,255,0.03);
      border-radius: 18px;
      border: 1px solid rgba(255,255,255,0.05);
    }

    .metric-card strong {
      font-size: 12px;
      color: var(--muted);
      letter-spacing: 0.08em;
      text-transform: uppercase;
      display: block;
      margin-bottom: 8px;
    }

    .metric-card p {
      margin: 0;
      font-size: 20px;
      font-weight: 700;
    }

    .gauge {
      position: relative;
      height: 12px;
      border-radius: 999px;
      background: rgba(255,255,255,0.08);
      overflow: hidden;
      margin-top: 12px;
    }

    .gauge-fill {
      display: block;
      height: 100%;
      border-radius: 999px;
      transition: width 0.3s ease;
    }

    .color-chip {
      width: 16px;
      height: 16px;
      border-radius: 50%;
      border: 1px solid rgba(255,255,255,0.15);
      display: inline-block;
      vertical-align: middle;
      margin-right: 8px;
      flex-shrink: 0;
    }

    .color-swatch {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 5px;
    }

    .color-swatch-block {
      width: 52px;
      height: 36px;
      border-radius: 10px;
      border: 1px solid rgba(255,255,255,0.18);
      box-shadow: 0 4px 12px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.15);
    }

    .color-swatch small {
      font-size: 11px;
      color: var(--muted);
      font-family: "Fira Mono", monospace;
      letter-spacing: 0.05em;
    }

    .button,
    button {
      border: none;
      border-radius: 14px;
      padding: 14px 18px;
      background: linear-gradient(135deg, var(--accent), #ffb575);
      color: #111;
      font-weight: 700;
      cursor: pointer;
      transition: transform 0.2s ease, opacity 0.2s ease;
    }

    .button.secondary,
    button.secondary {
      background: rgba(255,255,255,0.06);
      color: var(--text);
    }

    button:hover,
    .button:hover {
      transform: translateY(-1px);
    }

    .input,
    .select,
    textarea {
      width: 100%;
      border-radius: 14px;
      border: 1px solid rgba(255,255,255,0.08);
      background: var(--surface);
      color: var(--text);
      padding: 14px 16px;
      font-size: 14px;
      outline: none;
      resize: vertical;
    }

    .input::placeholder,
    textarea::placeholder {
      color: var(--muted);
    }

    input[type="search"] {
      appearance: none;
    }

    .field {
      display: grid;
      gap: 10px;
      margin-bottom: 16px;
    }

    .field label {
      color: var(--muted);
      font-size: 13px;
    }

    .checkbox-row {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
      gap: 8px;
    }

    .checkbox-toggle {
      display: flex;
      gap: 10px;
      align-items: center;
      padding: 10px 12px;
      border-radius: 12px;
      background: rgba(255,255,255,0.02);
      border: 1px solid rgba(255,255,255,0.05);
      font-size: 13px;
      cursor: pointer;
    }

    .checkbox-toggle:hover {
      background: rgba(255,255,255,0.04);
    }

    .checkbox-toggle input {
      accent-color: var(--accent);
      width: 16px;
      height: 16px;
      flex-shrink: 0;
    }

    .settings-divider {
      border: none;
      border-top: 1px solid var(--line);
      margin: 20px 0;
    }

    .bottom-nav {
      position: fixed;
      left: 0;
      right: 0;
      bottom: 0;
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 1px;
      background: rgba(15,19,22,0.98);
      border-top: 1px solid rgba(255,255,255,0.08);
      z-index: 10;
      padding: 8px;
      backdrop-filter: blur(12px);
    }

    .bottom-nav button {
      background: var(--panel);
      color: var(--text);
      font-size: 13px;
      padding: 12px 0;
      border-radius: 16px;
      display: flex;
      flex-direction: column;
      gap: 6px;
      align-items: center;
      justify-content: center;
    }

    .bottom-nav button.active {
      background: linear-gradient(135deg, var(--accent), #ffb575);
      color: #111;
    }

    .hidden { display: none !important; }

    .toast-container {
      position: fixed;
      right: 18px;
      top: 18px;
      display: grid;
      gap: 12px;
      z-index: 20;
      width: min(320px, calc(100vw - 40px));
    }

    .toast {
      padding: 16px 18px;
      border-radius: 16px;
      background: rgba(19, 29, 40, 0.96);
      border: 1px solid rgba(255,255,255,0.08);
      box-shadow: var(--shadow);
      animation: toastIn 0.25s ease;
    }

    .toast.success { border-color: rgba(60,204,122,0.3); }
    .toast.warning { border-color: rgba(255,138,61,0.25); }
    .toast.error { border-color: rgba(255,95,109,0.25); }

    @keyframes toastIn {
      from { opacity: 0; transform: translateY(-12px); }
      to { opacity: 1; transform: translateY(0); }
    }

    .pulse {
      animation: pulse 1.8s ease-in-out infinite;
    }

    @keyframes pulse {
      0%, 100% { box-shadow: 0 0 0 0 rgba(255,138,61,0.25); }
      50% { box-shadow: 0 0 0 16px rgba(255,138,61,0.02); }
    }

    .view-toggle {
      display: flex;
      gap: 6px;
    }

    .view-toggle button {
      padding: 8px 12px;
      font-size: 15px;
      border-radius: 12px;
      background: rgba(255,255,255,0.05);
      color: var(--muted);
      line-height: 1;
    }

    .view-toggle button.active {
      background: rgba(255,138,61,0.2);
      color: var(--accent);
    }

    .spool-list {
      display: grid;
      gap: 16px;
    }

    .spool-list.view-grid {
      grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
    }

    .spool-list.view-list {
      grid-template-columns: 1fr;
    }

    .spool-card {
      border-radius: 18px;
      overflow: hidden;
      border: 1px solid rgba(255,255,255,0.07);
      background: var(--panel);
      box-shadow: 0 8px 24px rgba(0,0,0,0.2);
      transition: border-color 0.2s, box-shadow 0.2s;
    }

    .spool-card:hover {
      box-shadow: 0 12px 32px rgba(0,0,0,0.3);
    }

    .spool-color-bar {
      height: 10px;
      width: 100%;
    }

    .spool-stats {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 6px;
      margin-bottom: 12px;
    }

    .spool-stat {
      background: rgba(255,255,255,0.04);
      border-radius: 8px;
      padding: 6px 9px;
      text-align: center;
    }

    .spool-stat span {
      display: block;
      font-size: 10px;
      color: var(--muted);
      text-transform: uppercase;
      letter-spacing: 0.05em;
      margin-bottom: 2px;
    }

    .spool-stat strong {
      display: block;
      font-size: 13px;
      color: var(--text);
    }

    .spool-card .card-body {
      padding: 0 18px 18px;
      display: grid;
      gap: 14px;
    }

    /* Table view */
    .spool-table {
      width: 100%;
      border-collapse: collapse;
      font-size: 14px;
    }

    .spool-table th {
      text-align: left;
      padding: 10px 12px;
      color: var(--muted);
      font-size: 11px;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      border-bottom: 1px solid var(--line);
    }

    .spool-table td {
      padding: 10px 12px;
      border-bottom: 1px solid rgba(255,255,255,0.04);
      vertical-align: middle;
    }

    .spool-table tr:last-child td { border-bottom: none; }

    .spool-table tr:hover td { background: rgba(255,255,255,0.02); }

    .spool-meta {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      gap: 12px;
    }

    .spool-meta span {
      display: block;
      color: var(--muted);
      font-size: 12px;
    }

    .spool-meta strong {
      display: block;
      margin-top: 4px;
      font-size: 15px;
    }

    .spool-actions {
      display: flex;
      gap: 12px;
      flex-wrap: wrap;
    }

    .spool-badge {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      background: rgba(255,255,255,0.06);
      padding: 8px 12px;
      border-radius: 999px;
      color: var(--muted);
      font-size: 13px;
      flex-shrink: 0;
    }

    .visibility-group-header {
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.12em;
      color: var(--muted);
      padding: 4px 0 6px;
      margin-top: 16px;
      border-bottom: 1px solid var(--line);
    }

    .visibility-group-header:first-child { margin-top: 0; }

    .console {
      width: 100%;
      height: 420px;
      background: #0a0d0f;
      color: #b8ccd4;
      font-family: "Fira Mono", "Consolas", monospace;
      font-size: 12px;
      line-height: 1.6;
      border: 1px solid rgba(255,255,255,0.08);
      border-radius: 14px;
      padding: 14px;
      resize: vertical;
      overflow-y: scroll;
      white-space: pre-wrap;
      word-break: break-all;
      box-sizing: border-box;
      outline: none;
    }
    @media (max-width: 860px) {
      .metric { grid-template-columns: 1fr; }
      .status-row { grid-template-columns: 1fr 1fr; }
      .spool-meta { grid-template-columns: 1fr 1fr; }
    }

    @media (max-width: 640px) {
      .page { padding: 18px 14px 108px; }
      .bottom-nav { grid-template-columns: repeat(3, 1fr); }
    }
  </style>
</head>
<body>
  <div class="toast-container" id="toastContainer"></div>
  <div class="page">
    <div class="grid" id="dashboardSection">
      <section class="card">
        <header>
          <div>
            <h2><span id="statusLight" style="display:inline-block;width:9px;height:9px;border-radius:50%;background:var(--bad);margin-right:9px;vertical-align:middle;flex-shrink:0"></span>Connectivity</h2>
            <small>WiFi and server health</small>
          </div>
          <button class="button secondary" style="padding:8px 14px;font-size:13px" onclick="checkStatus()">↻ Refresh</button>
        </header>
        <div class="status-row">
          <div class="status-block"><strong>Status</strong><span id="connectionLabel" style="font-size:16px">Offline</span></div>
          <div class="status-block"><strong>WiFi</strong><span id="wifiStatus">-</span></div>
          <div class="status-block"><strong>IP</strong><span id="wifiIp" style="font-size:14px;word-break:break-all">-</span></div>
          <div class="status-block"><strong>Spoolman</strong><span id="spoolmanStatus">-</span></div>
          <div class="status-block"><strong>Moonraker</strong><span id="moonrakerStatus">-</span></div>
          <div class="status-block"><strong>RFID</strong><span id="rfidStatus">-</span></div>
        </div>
      </section>

      <section class="card" id="currentSpoolCard">
        <header>
          <div>
            <h2>Active Spool</h2>
            <small>Current spool installed in Moonraker</small>
          </div>
          <div class="color-swatch">
            <div id="currentSpoolColor" class="color-swatch-block" style="background:#FFFFFF"></div>
            <small id="currentSpoolHex">#FFFFFF</small>
          </div>
        </header>
        <div class="metric">
          <div class="metric-card"><strong>Spool ID</strong><p id="currentId">-</p></div>
          <div class="metric-card"><strong>Name</strong><p id="currentName">-</p></div>
          <div class="metric-card"><strong>Remaining</strong><p id="currentRemaining">-</p></div>
        </div>
        <div class="gauge" style="margin-top:14px"><span id="currentGauge" class="gauge-fill" style="width:0%"></span></div>
      </section>

      <section class="card" id="nfcStatusCard">
        <header>
          <div>
            <h2><span id="nfcHealthDot" style="display:inline-block;width:9px;height:9px;border-radius:50%;background:var(--muted);margin-right:9px;vertical-align:middle;flex-shrink:0"></span>NFC Reader</h2>
            <small>Latest tag and write queue status</small>
          </div>
        </header>
        <div class="status-row">
          <div class="status-block"><strong>Status</strong><span id="nfcHealth" style="font-size:16px">-</span></div>
          <div class="status-block"><strong>Tag Type</strong><span id="nfcTagType">-</span></div>
          <div class="status-block"><strong>Spool ID</strong><span id="nfcId">-</span></div>
          <div class="status-block"><strong>Write Queue</strong><span id="pendingWriteStatus">Idle</span></div>
        </div>
      </section>

      <section class="card">
        <header>
          <div>
            <h2>Last Scanned Tag</h2>
            <small id="nfcSummaryText">Scan a tag to see the filament profile.</small>
          </div>
          <div class="color-swatch">
            <div id="nfcColorChip" class="color-swatch-block" style="background:#FFFFFF"></div>
            <small id="nfcColorDisplay">#FFFFFF</small>
          </div>
        </header>
        <div class="status-row">
          <div class="status-block"><strong>Name</strong><span id="nfcName">-</span></div>
          <div class="status-block"><strong>Material</strong><span id="nfcMaterial">-</span></div>
          <div class="status-block"><strong>Brand</strong><span id="nfcBrand">-</span></div>
          <div class="status-block"><strong>Ext Temp</strong><span id="nfcExtTemp">-</span></div>
          <div class="status-block"><strong>Bed Temp</strong><span id="nfcBedTemp">-</span></div>
        </div>
        <div class="status-row" style="margin-top:10px">
          <div class="status-block"><strong>Remaining</strong><span id="nfcRemaining">-</span></div>
          <div class="status-block"><strong>Total Weight</strong><span id="nfcWeight">-</span></div>
          <div class="status-block"><strong>Diameter</strong><span id="nfcDiameter">-</span></div>
          <div class="status-block"><strong>Location</strong><span id="nfcLocation">-</span></div>
        </div>
      </section>

      <section class="card">
        <header>
          <div>
            <h2>Device Console</h2>
            <small>Live log output from the device</small>
          </div>
          <div style="display:flex;gap:8px">
            <small id="logLineCount" style="align-self:center;color:var(--muted)">0 lines</small>
            <button class="button secondary" style="padding:8px 12px;font-size:13px" onclick="clearConsole()">Clear</button>
            <button class="button secondary" style="padding:8px 12px;font-size:13px" onclick="copyConsole()">Copy</button>
          </div>
        </header>
        <div id="console" class="console"></div>
      </section>
    </div>

    <section id="writeSection" class="card hidden">
      <header style="flex-wrap:wrap;gap:12px">
        <div style="flex:1;min-width:0">
          <h2>Spool Inventory</h2>
          <small>Click <strong>Write to Tag</strong> on a spool, then scan any NFC tag.</small>
        </div>
        <div style="display:flex;gap:8px;align-items:center;flex-wrap:wrap">
          <div class="view-toggle">
            <button id="viewGrid" class="active" title="Grid" onclick="setViewMode('grid')">⊞</button>
            <button id="viewList" title="List" onclick="setViewMode('list')">☰</button>
            <button id="viewTable" title="Table" onclick="setViewMode('table')">⊟</button>
          </div>
          <button class="button secondary" style="padding:10px 14px;font-size:13px" onclick="refreshSpools()">↻ Refresh</button>
        </div>
      </header>

      <div id="writeArmedBanner" class="hidden" style="margin-bottom:18px;padding:14px 16px;border-radius:14px;background:rgba(58,204,122,0.10);border:1px solid rgba(58,204,122,0.35);display:flex;align-items:center;gap:12px;">
        <span style="width:10px;height:10px;border-radius:50%;background:var(--good);flex-shrink:0;animation:pulse 1.8s ease-in-out infinite"></span>
        <span style="flex:1">Armed — scan a tag to write spool <strong id="writeArmedSpoolName">-</strong></span>
        <button class="button secondary" style="padding:8px 14px;font-size:12px" onclick="cancelWrite()">Cancel</button>
      </div>

      <div class="field" style="margin-bottom:18px">
        <input id="spoolSearch" class="input" type="search" placeholder="Search by name, brand, material, or ID…" oninput="filterSpools()">
      </div>
      <div class="spool-list" id="spoolCards"></div>
    </section>

    <section id="settingsSection" class="grid hidden">
      <section class="card">
        <header>
          <div>
            <h2>Network & Server</h2>
            <small>WiFi credentials, server URLs and access control</small>
          </div>
        </header>
        <div id="fieldWifiSsid" class="field">
          <label for="wifiSsid">WiFi SSID</label>
          <input id="wifiSsid" class="input" type="text" autocomplete="off" placeholder="Network name" oninput="markDirty('wifiSsid')">
        </div>
        <div id="fieldWifiPass" class="field">
          <label for="wifiPass">WiFi Password</label>
          <input id="wifiPass" class="input" type="password" autocomplete="new-password" placeholder="Leave blank to keep existing" oninput="markDirty('wifiPass')">
        </div>
        <div id="fieldSpoolman" class="field">
          <label for="serverSpoolman">Spoolman URL <small style="color:var(--muted)">(IP:Port — no http://)</small></label>
          <input id="serverSpoolman" class="input" type="text" placeholder="10.0.0.X:7912" oninput="markDirty('serverSpoolman')">
        </div>
        <div id="fieldMoonraker" class="field">
          <label for="serverMoonraker">Moonraker URL <small style="color:var(--muted)">(IP:Port — no http://)</small></label>
          <input id="serverMoonraker" class="input" type="text" placeholder="10.0.0.X:7125" oninput="markDirty('serverMoonraker')">
        </div>
        <hr class="settings-divider">
        <div style="margin-bottom:14px">
          <strong style="font-size:14px">Access Control</strong>
          <small style="display:block;color:var(--muted);margin-top:4px">Require login to save configuration</small>
        </div>
        <div class="field">
          <label class="checkbox-toggle" style="width:fit-content">
            <input type="checkbox" id="authEnabled" onchange="updateAuthVisibility()">
            Enable HTTP authentication
          </label>
        </div>
        <div id="authFields" class="hidden">
          <div class="field">
            <label for="authUser">Username</label>
            <input id="authUser" class="input" type="text" autocomplete="off" placeholder="admin" oninput="markDirty('authUser')">
          </div>
          <div class="field">
            <label for="authPass">New Password</label>
            <input id="authPass" class="input" type="password" autocomplete="new-password" placeholder="Leave blank to keep existing" oninput="markDirty('authPass')">
          </div>
        </div>
        <button class="button" onclick="saveConfig()">Save Configuration</button>
      </section>

      <section class="card">
        <header>
          <div>
            <h2>Visibility</h2>
            <small>Configure which sections and fields are shown</small>
          </div>
        </header>
        <div id="visibilityCheckboxes"></div>
        <button class="button secondary" style="margin-top:18px" onclick="saveConfig()">Save Visibility</button>
      </section>
    </section>

  </div>

  <nav class="bottom-nav">
    <button id="navDashboard" class="active" onclick="showPanel('dashboard')">Dashboard</button>
    <button id="navWrite" onclick="showPanel('write')">Spools</button>
    <button id="navSettings" onclick="showPanel('settings')">Settings</button>
  </nav>

  <script>
    const uiState = {
      spoolData: [],
      armedSpoolId: null,
      armedSpoolName: null,
      currentPanel: 'dashboard',
      viewMode: 'grid',
      settings: {}
    };

    const dirtyFields = new Set();
    function markDirty(id) { dirtyFields.add(id); }
    function clearDirty() { dirtyFields.clear(); }

    const visibilityConfig = [
      { key: 'showWifi',         label: 'WiFi info',       group: 'Dashboard Sections' },
      { key: 'showServers',      label: 'Server status',   group: 'Dashboard Sections' },
      { key: 'showCurrentSpool', label: 'Active Spool',    group: 'Dashboard Sections' },
      { key: 'showNfcData',      label: 'NFC data',        group: 'Dashboard Sections' },
      { key: 'showLogger',       label: 'Log output',      group: 'Dashboard Sections' },
      { key: 'showNfcJson',      label: 'NFC raw JSON',    group: 'Dashboard Sections' },
      { key: 'showTabDashboard', label: 'Dashboard',       group: 'Tab Visibility' },
      { key: 'showTabNfc',       label: 'NFC',             group: 'Tab Visibility' },
      { key: 'showTabWrite',     label: 'Spools',          group: 'Tab Visibility' },
      { key: 'showTabSpools',    label: 'Spool list',      group: 'Tab Visibility' },
      { key: 'showTabOptions',   label: 'Options',         group: 'Tab Visibility' },
      { key: 'showTabLogs',      label: 'Logs',            group: 'Tab Visibility' },
      { key: 'showFieldWifiSsid',  label: 'WiFi SSID',     group: 'Settings Fields' },
      { key: 'showFieldWifiPass',  label: 'WiFi Password', group: 'Settings Fields' },
      { key: 'showFieldSpoolman',  label: 'Spoolman URL',  group: 'Settings Fields' },
      { key: 'showFieldMoonraker', label: 'Moonraker URL', group: 'Settings Fields' }
    ];

    const fieldConfig = [
      { key: 'snfName', label: 'Name' },
      { key: 'snfMaterial', label: 'Material' },
      { key: 'snfBrand', label: 'Manufacturer' },
      { key: 'snfColor', label: 'Color' },
      { key: 'snfExtTemp', label: 'Extruder Temp' },
      { key: 'snfBedTemp', label: 'Bed Temp' },
      { key: 'snfLocation', label: 'Location' },
      { key: 'snfRemaining', label: 'Remaining' },
      { key: 'snfWeight', label: 'Weight' },
      { key: 'snfDiameter', label: 'Diameter' },
      { key: 'snfDensity', label: 'Density' },
      { key: 'snfFlow', label: 'Flow Ratio' },
      { key: 'snfMaxSpeed', label: 'Max Volumetric' },
      { key: 'snfSpoolWt', label: 'Spool Weight' }
    ];

    function showPanel(panel) {
      if (panel === 'logs') panel = 'dashboard';
      uiState.currentPanel = panel;
      document.getElementById('dashboardSection').classList.toggle('hidden', panel !== 'dashboard');
      document.getElementById('writeSection').classList.toggle('hidden', panel !== 'write');
      document.getElementById('settingsSection').classList.toggle('hidden', panel !== 'settings');
      document.getElementById('navDashboard').classList.toggle('active', panel === 'dashboard');
      document.getElementById('navWrite').classList.toggle('active', panel === 'write');
      document.getElementById('navSettings').classList.toggle('active', panel === 'settings');
      if (panel === 'write') {
        loadSpools();
      }
    }

    function updateAuthVisibility() {
      const enabled = document.getElementById('authEnabled').checked;
      document.getElementById('authFields').classList.toggle('hidden', !enabled);
    }

    let consoleAutoScroll = true;
    function updateConsole(text) {
      const el = document.getElementById('console');
      const atBottom = el.scrollHeight - el.scrollTop - el.clientHeight < 40;
      el.textContent = text || '';
      const lines = (text || '').split('\n').filter(l => l.length).length;
      document.getElementById('logLineCount').textContent = lines + ' lines';
      if (atBottom || consoleAutoScroll) {
        el.scrollTop = el.scrollHeight;
      }
    }

    function clearConsole() {
      document.getElementById('console').textContent = '';
      document.getElementById('logLineCount').textContent = '0 lines';
    }

    function copyConsole() {
      const text = document.getElementById('console').textContent;
      if (!text) { showToast('Nothing to copy', 'warning'); return; }
      if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(text)
          .then(() => showToast('Copied to clipboard', 'success'))
          .catch(() => execCopy(text));
      } else {
        execCopy(text);
      }
    }

    function execCopy(text) {
      const ta = document.createElement('textarea');
      ta.value = text;
      ta.style.cssText = 'position:fixed;top:0;left:0;opacity:0;pointer-events:none';
      document.body.appendChild(ta);
      ta.focus();
      ta.select();
      try {
        document.execCommand('copy');
        showToast('Copied to clipboard', 'success');
      } catch (e) {
        showToast('Copy not supported in this browser', 'error');
      }
      document.body.removeChild(ta);
    }

    function setServiceStatus(id, ok, okLabel, failLabel) {
      const el = document.getElementById(id);
      el.textContent = ok ? okLabel : failLabel;
      el.style.color = ok ? 'var(--good)' : 'var(--bad)';
    }

    function showToast(message, type = 'success') {
      const container = document.getElementById('toastContainer');
      const toast = document.createElement('div');
      toast.className = `toast ${type}`;
      toast.textContent = message;
      container.appendChild(toast);
      setTimeout(() => toast.remove(), 3500);
    }

    function setConnectionStatus(connected) {
      document.getElementById('statusLight').style.background = connected ? 'var(--good)' : 'var(--bad)';
      const lbl = document.getElementById('connectionLabel');
      lbl.textContent = connected ? 'Online' : 'Offline';
      lbl.style.color = connected ? 'var(--good)' : 'var(--bad)';
    }

    function buildVisibilityForm(settings) {
      renderVisibilityOptions(settings);
    }

    function loadSpools(force = false) {
      if (uiState.spoolData.length > 0 && !force) {
        return Promise.resolve();
      }
      return fetch('/spools')
        .then(response => response.ok || response.status === 503 ? response.json() : Promise.reject('HTTP ' + response.status))
        .then(data => {
          uiState.spoolData = Array.isArray(data) ? data : [];
          renderSpools(uiState.spoolData);
          return uiState.spoolData;
        })
        .catch(err => {
          renderSpools([]);
          console.warn('Spools load failed', err);
        });
    }

    function renderSpools(spools) {
      const container = document.getElementById('spoolCards');
      container.innerHTML = '';
      const search = (document.getElementById('spoolSearch').value || '').trim().toLowerCase();
      const items = spools.filter(spool => {
        if (!search) return true;
        const text = `${spool.name} ${spool.material} ${spool.brand} ${spool.location}`.toLowerCase();
        return text.includes(search) || String(spool.id) === search;
      });

      if (!items.length) {
        container.style.display = 'block';
        container.innerHTML = '<p style="margin:0;color:var(--muted);text-align:center;padding:24px">No spools match your search.</p>';
        return;
      }

      const mode = uiState.viewMode || 'grid';

      if (mode === 'table') {
        container.style.display = 'block';
        const table = document.createElement('table');
        table.className = 'spool-table';
        table.innerHTML = `<thead><tr>
          <th>#</th><th>Name</th><th>Material</th><th>Brand</th>
          <th>Remaining</th><th>Location</th><th></th>
        </tr></thead>`;
        const tbody = document.createElement('tbody');
        items.forEach(spool => {
          const remaining = parseFloat(spool.remaining) || 0;
          const total = parseFloat(spool.totalWeight) || 0;
          const percent = total ? Math.max(0, Math.min(100, Math.round((remaining / total) * 100))) : 0;
          const color = normalizeHex(spool.color || 'FFFFFF');
          const isArmed = uiState.armedSpoolId === spool.id;
          const safeName = (spool.name||'Spool #'+spool.id).replace(/'/g,"\\'");
          const tr = document.createElement('tr');
          tr.innerHTML = `
            <td><span class="spool-badge" style="padding:4px 8px">#${spool.id}</span></td>
            <td><span class="color-chip" style="background:#${color}"></span>${spool.name || '-'}</td>
            <td>${spool.material || '-'}</td>
            <td>${spool.brand || '-'}</td>
            <td>
              <span>${remaining.toFixed(0)} g · ${percent}%</span>
              <div class="gauge" style="height:6px;margin-top:4px"><span class="gauge-fill" style="width:${percent}%;background:#${color}"></span></div>
            </td>
            <td>${spool.location || '-'}</td>
            <td>
              <button class="${isArmed ? 'button' : 'button secondary'}" style="padding:8px 12px;font-size:12px;white-space:nowrap"
                onclick="writeSpoolDirect(${spool.id},${spool.filamentId||0},'${safeName}')">
                ${isArmed ? '⚡ Armed' : '✎ Write'}
              </button>
            </td>`;
          tbody.appendChild(tr);
        });
        table.appendChild(tbody);
        container.appendChild(table);
        return;
      }

      // Grid or List mode
      container.style.display = '';
      container.className = 'spool-list view-' + mode;
      items.forEach(spool => {
        const remaining = parseFloat(spool.remaining) || 0;
        const total = parseFloat(spool.totalWeight) || 0;
        const usedWeight = parseFloat(spool.usedWeight) || (total - remaining);
        const usedPercent = total ? Math.max(0, Math.min(100, Math.round((usedWeight / total) * 100))) : 0;
        const percent = 100 - usedPercent;
        const color = normalizeHex(spool.color || 'FFFFFF');
        const isArmed = uiState.armedSpoolId === spool.id;
        const safeName = (spool.name||'Spool #'+spool.id).replace(/'/g,"\\'");
        const card = document.createElement('article');
        card.className = 'spool-card' + (isArmed ? ' pulse' : '');
        card.style.borderColor = isArmed ? 'rgba(58,204,122,0.5)' : 'rgba(255,255,255,0.07)';
        const snf = uiState.settings || {};
        const stats = [];
        if (snf.snfExtTemp  && spool.extruderTemp)  stats.push(['Extruder', spool.extruderTemp + ' °C']);
        if (snf.snfBedTemp  && spool.bedTemp)        stats.push(['Bed', spool.bedTemp + ' °C']);
        if (snf.snfDiameter && spool.diameter)       stats.push(['Diameter', spool.diameter.toFixed(2) + ' mm']);
        if (snf.snfWeight   && spool.totalWeight)    stats.push(['Net Weight', spool.totalWeight.toFixed(0) + ' g']);
        if (snf.snfSpoolWt  && spool.spoolWeight)    stats.push(['Spool Wt', spool.spoolWeight.toFixed(0) + ' g']);
        if (snf.snfDensity  && spool.density)        stats.push(['Density', spool.density.toFixed(2) + ' g/cm³']);
        if (snf.snfFlow     && spool.flowRatio)      stats.push(['Flow', spool.flowRatio]);
        if (snf.snfMaxSpeed && spool.maxVolumetric)  stats.push(['Max Vol', spool.maxVolumetric + ' mm³/s']);
        const statsHtml = stats.length ? `<div class="spool-stats">${stats.map(([l,v]) =>
          `<div class="spool-stat"><span>${l}</span><strong>${v}</strong></div>`).join('')}</div>` : '';

        card.innerHTML = `
          <div class="spool-color-bar" style="background:#${color};opacity:${color === 'FFFFFF' ? 0.15 : 1}"></div>
          <div class="card-body" style="padding:16px">
            <div style="display:flex;justify-content:space-between;align-items:flex-start;gap:10px;margin-bottom:10px">
              <div style="min-width:0">
                <h3 style="font-size:15px;display:flex;align-items:center;gap:8px;margin-bottom:4px">
                  <span class="color-chip" style="background:#${color};width:12px;height:12px;flex-shrink:0"></span>
                  <span style="overflow:hidden;text-overflow:ellipsis;white-space:nowrap">${spool.name || 'Unnamed'}</span>
                </h3>
                <small style="color:var(--muted)">${[spool.material, spool.brand].filter(Boolean).join(' · ') || '-'}</small>
              </div>
              <span class="spool-badge" style="padding:5px 10px;font-size:12px;flex-shrink:0">#${spool.id}</span>
            </div>
            ${snf.snfRemaining !== false ? `
            <div class="gauge" style="margin-bottom:6px"><span class="gauge-fill" style="width:${percent}%;background:${gaugeColor(color)}"></span></div>
            <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:12px">
              <span style="font-size:13px;color:var(--muted)">${remaining.toFixed(0)} g remaining</span>
              <span style="font-size:13px;font-weight:700;color:var(--text)">${percent}% left</span>
            </div>` : ''}
            ${statsHtml}
            ${snf.snfLocation !== false && spool.location ? `<div style="font-size:12px;color:var(--muted);margin-bottom:12px">📍 ${spool.location}</div>` : ''}
            <button class="${isArmed ? 'button' : 'button secondary'}" style="width:100%;justify-content:center"
              onclick="writeSpoolDirect(${spool.id},${spool.filamentId||0},'${safeName}')">
              ${isArmed ? '⚡ Armed — tap a tag' : '✎ Write to Tag'}
            </button>
          </div>`;
        container.appendChild(card);
      });
    }

    function normalizeHex(value) {
      let hex = (value || '').trim().replace(/^#/, '').toUpperCase();
      if (hex.length === 3) {
        hex = hex.split('').map(ch => ch + ch).join('');
      }
      return /^[0-9A-F]{6}$/.test(hex) ? hex : 'FFFFFF';
    }

    // Returns a CSS color safe to use as a gauge fill — brightens very dark colors
    // so they remain visible on a dark panel background.
    function gaugeColor(hex) {
      const r = parseInt(hex.slice(0,2), 16);
      const g = parseInt(hex.slice(2,4), 16);
      const b = parseInt(hex.slice(4,6), 16);
      const lum = 0.299*r + 0.587*g + 0.114*b;
      if (lum < 40) {
        const boost = Math.round((40 - lum) * 2.5);
        return `rgb(${Math.min(255,r+boost)},${Math.min(255,g+boost)},${Math.min(255,b+boost)})`;
      }
      return `#${hex}`;
    }

    function writeSpoolDirect(id, filamentId, name) {
      const params = new URLSearchParams();
      params.append('id', String(id));
      params.append('filamentId', String(filamentId || 0));
      params.append('enabled', '1');

      fetch('/write-nfc', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: params.toString()
      }).then(r => r.text()).then(text => {
        uiState.armedSpoolId = id;
        uiState.armedSpoolName = name || ('Spool #' + id);
        const banner = document.getElementById('writeArmedBanner');
        banner.classList.remove('hidden');
        document.getElementById('writeArmedSpoolName').textContent = uiState.armedSpoolName;
        renderSpools(uiState.spoolData);
        showToast(text, 'success');
      }).catch(() => showToast('Unable to arm NFC write', 'error'));
    }

    function cancelWrite() {
      fetch('/cancel-write', { method: 'POST' })
        .then(() => {
          uiState.armedSpoolId = null;
          uiState.armedSpoolName = null;
          document.getElementById('writeArmedBanner').classList.add('hidden');
          renderSpools(uiState.spoolData);
          showToast('Write cancelled', 'warning');
        }).catch(() => {});
    }

    function refreshSpools() {
      loadSpools(true).then(() => showToast('Spools refreshed', 'success'));
    }

    function filterSpools() {
      renderSpools(uiState.spoolData);
    }

    function setViewMode(mode) {
      uiState.viewMode = mode;
      ['grid', 'list', 'table'].forEach(m => {
        const btn = document.getElementById('view' + m.charAt(0).toUpperCase() + m.slice(1));
        if (btn) btn.classList.toggle('active', m === mode);
      });
      renderSpools(uiState.spoolData);
    }

    function saveConfig() {
      const authPassEl = document.getElementById('authPass');
      const body = {
        wifiSsid: document.getElementById('wifiSsid').value,
        wifiPass: document.getElementById('wifiPass').value,
        serverSpoolman: document.getElementById('serverSpoolman').value,
        serverMoonraker: document.getElementById('serverMoonraker').value,
        authEnabled: document.getElementById('authEnabled').checked,
        authUser: document.getElementById('authUser').value,
        authPass: authPassEl ? authPassEl.value : '',
        showWifi: document.getElementById('showWifi').checked,
        showServers: document.getElementById('showServers').checked,
        showCurrentSpool: document.getElementById('showCurrentSpool').checked,
        showNfcData: document.getElementById('showNfcData').checked,
        showLogger: document.getElementById('showLogger').checked,
        showNfcJson: document.getElementById('showNfcJson').checked,
        showTabDashboard: document.getElementById('showTabDashboard').checked,
        showTabNfc: document.getElementById('showTabNfc').checked,
        showTabWrite: document.getElementById('showTabWrite').checked,
        showTabOptions: document.getElementById('showTabOptions').checked,
        showTabLogs: document.getElementById('showTabLogs').checked,
        showTabSpools: document.getElementById('showTabSpools').checked,
        defaultTab: 'dashboard',
        snfName: document.getElementById('snfName').checked,
        snfMaterial: document.getElementById('snfMaterial').checked,
        snfBrand: document.getElementById('snfBrand').checked,
        snfColor: document.getElementById('snfColor').checked,
        snfExtTemp: document.getElementById('snfExtTemp').checked,
        snfBedTemp: document.getElementById('snfBedTemp').checked,
        snfLocation: document.getElementById('snfLocation').checked,
        snfRemaining: document.getElementById('snfRemaining').checked,
        snfWeight: document.getElementById('snfWeight').checked,
        snfDiameter: document.getElementById('snfDiameter').checked,
        snfDensity: document.getElementById('snfDensity').checked,
        snfFlow: document.getElementById('snfFlow').checked,
        snfMaxSpeed: document.getElementById('snfMaxSpeed').checked,
        snfSpoolWt: document.getElementById('snfSpoolWt').checked,
        showFieldWifiSsid: document.getElementById('showFieldWifiSsid').checked,
        showFieldWifiPass: document.getElementById('showFieldWifiPass').checked,
        showFieldSpoolman: document.getElementById('showFieldSpoolman').checked,
        showFieldMoonraker: document.getElementById('showFieldMoonraker').checked
      };

      fetch('/save-config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body)
      }).then(response => response.text())
        .then(text => {
          clearDirty();
          showToast(text, 'success');
          setTimeout(() => location.reload(), 2500);
        })
        .catch(() => showToast('Failed to save config', 'error'));
    }

    function checkStatus() {
      fetch('/status').then(() => showToast('Status refresh requested', 'success'));
    }

    function setField(id, value) {
      if (dirtyFields.has(id)) return;
      const el = document.getElementById(id);
      if (el) el.value = value || '';
    }

    function applyFieldVisibility(id, visible) {
      const el = document.getElementById(id);
      if (el) el.classList.toggle('hidden', !visible);
    }

    function updateData() {
      fetch('/data').then(response => response.json()).then(data => {
        setConnectionStatus(data.wifiConnected);
        document.getElementById('wifiStatus').textContent = data.wifiStatus || '-';
        document.getElementById('wifiIp').textContent = data.ipAddress || '-';
        setServiceStatus('spoolmanStatus', data.spoolman, 'Connected', 'Offline');
        setServiceStatus('moonrakerStatus', data.moonraker, 'Connected', 'Offline');
        setServiceStatus('rfidStatus', data.nfc, 'OK', 'Error');

        // Active spool — look up full data from spoolData cache for remaining/color/gauge
        const cSpool = uiState.spoolData.find(s => Number(s.id) === Number(data.currentId));
        const cColor = normalizeHex(cSpool ? cSpool.color : 'FFFFFF');
        const cRem = cSpool ? parseFloat(cSpool.remaining) || 0 : 0;
        const cTotal = cSpool ? parseFloat(cSpool.totalWeight) || 0 : 0;
        const cPct = cTotal ? Math.max(0, Math.min(100, Math.round((cRem / cTotal) * 100))) : 0;
        document.getElementById('currentId').textContent = data.currentId || '-';
        document.getElementById('currentName').textContent = data.currentName || '-';
        document.getElementById('currentRemaining').textContent = cSpool ? cRem.toFixed(0) + ' g (' + cPct + '%)' : '-';
        document.getElementById('currentSpoolColor').style.background = '#' + cColor;
        document.getElementById('currentSpoolHex').textContent = '#' + cColor;
        document.getElementById('currentGauge').style.width = cPct + '%';
        document.getElementById('currentGauge').style.background = '#' + cColor;

        // NFC reader card
        document.getElementById('nfcHealthDot').style.background = data.nfc ? 'var(--good)' : 'var(--muted)';
        document.getElementById('nfcHealth').textContent = data.nfc ? 'Active' : 'Idle';
        document.getElementById('nfcHealth').style.color = data.nfc ? 'var(--good)' : 'var(--muted)';
        document.getElementById('nfcTagType').textContent = data.nfcTagType || '-';
        document.getElementById('nfcId').textContent = data.nfcId || '-';
        document.getElementById('pendingWriteStatus').textContent = data.pendingWriteStatus || 'Idle';
        document.getElementById('nfcStatusCard').classList.toggle('pulse', !!data.pendingWrite);

        // Last scanned tag
        const nfcColor = normalizeHex(data.nfcColor || 'FFFFFF');
        document.getElementById('nfcName').textContent = data.nfcName || '-';
        document.getElementById('nfcMaterial').textContent = data.nfcMaterial || '-';
        document.getElementById('nfcBrand').textContent = data.nfcBrand || '-';
        document.getElementById('nfcColorChip').style.background = '#' + nfcColor;
        document.getElementById('nfcColorDisplay').textContent = '#' + nfcColor;
        document.getElementById('nfcExtTemp').textContent = data.nfcExtTemp && data.nfcExtTemp !== '0' ? data.nfcExtTemp + ' °C' : '-';
        document.getElementById('nfcBedTemp').textContent = data.nfcBedTemp && data.nfcBedTemp !== '0' ? data.nfcBedTemp + ' °C' : '-';
        document.getElementById('nfcRemaining').textContent = data.nfcRemaining ? data.nfcRemaining + ' g' : '-';
        document.getElementById('nfcWeight').textContent = data.nfcWeight ? data.nfcWeight + ' g' : '-';
        document.getElementById('nfcDiameter').textContent = data.nfcDiameter ? data.nfcDiameter + ' mm' : '-';
        document.getElementById('nfcLocation').textContent = data.nfcLocation || '-';
        if (data.nfcName) document.getElementById('nfcSummaryText').textContent = 'Last scan: ' + data.nfcName;

        // Write tab armed banner sync
        if (data.pendingWrite && data.newNfcId && data.newNfcId !== '0') {
          document.getElementById('writeArmedBanner').classList.remove('hidden');
          if (!uiState.armedSpoolId) uiState.armedSpoolId = Number(data.newNfcId);
          const armName = uiState.armedSpoolName || ('Spool #' + data.newNfcId);
          document.getElementById('writeArmedSpoolName').textContent = armName;
        } else if (!data.pendingWrite && uiState.armedSpoolId) {
          uiState.armedSpoolId = null;
          uiState.armedSpoolName = null;
          document.getElementById('writeArmedBanner').classList.add('hidden');
          renderSpools(uiState.spoolData);
        }

        // Only update settings fields when they are not focused (avoids wiping user input)
        setField('wifiSsid', data.wifiSsid);
        setField('serverSpoolman', data.serverSpoolman);
        setField('serverMoonraker', data.serverMoonraker);
        // wifiPass is never prefilled from server

        // Apply settings field visibility
        applyFieldVisibility('fieldWifiSsid', data.showFieldWifiSsid !== false);
        applyFieldVisibility('fieldWifiPass', data.showFieldWifiPass !== false);
        applyFieldVisibility('fieldSpoolman', data.showFieldSpoolman !== false);
        applyFieldVisibility('fieldMoonraker', data.showFieldMoonraker !== false);

        // Auth settings — only update when not focused
        const authEnabledEl = document.getElementById('authEnabled');
        if (document.activeElement !== authEnabledEl) {
          authEnabledEl.checked = data.authEnabled === true;
          document.getElementById('authFields').classList.toggle('hidden', !data.authEnabled);
        }
        setField('authUser', data.authUser);
        // authPass is never prefilled

        visibilityConfig.forEach(item => {
          const checkbox = document.getElementById(item.key);
          if (checkbox) checkbox.checked = data[item.key] !== false;
        });

        fieldConfig.forEach(item => {
          const checkbox = document.getElementById(item.key);
          if (checkbox) checkbox.checked = data[item.key] !== false;
        });

        // Cache snf* flags so renderSpools can respect them
        uiState.settings = {};
        fieldConfig.forEach(item => { uiState.settings[item.key] = data[item.key] !== false; });

        updateConsole(data.logger);

        showPanel(data.firstTimeSetup ? 'settings' : uiState.currentPanel);
      }).catch(() => {
        setConnectionStatus(false);
      });
    }

    function renderVisibilityOptions(settings) {
      const container = document.getElementById('visibilityCheckboxes');
      container.innerHTML = '';

      // Render grouped visibilityConfig
      let lastGroup = null;
      visibilityConfig.forEach(item => {
        if (item.group !== lastGroup) {
          lastGroup = item.group;
          const hdr = document.createElement('div');
          hdr.className = 'visibility-group-header';
          hdr.textContent = item.group;
          container.appendChild(hdr);
          const row = document.createElement('div');
          row.className = 'checkbox-row';
          row.id = 'group_' + item.group.replace(/\s+/g, '_');
          container.appendChild(row);
        }
        const row = container.querySelector('#group_' + item.group.replace(/\s+/g, '_'));
        const label = document.createElement('label');
        label.className = 'checkbox-toggle';
        label.innerHTML = `<input type="checkbox" id="${item.key}">${item.label}`;
        row.appendChild(label);
        document.getElementById(item.key).checked = settings[item.key] !== false;
      });

      // NFC write fields group
      const nfcHdr = document.createElement('div');
      nfcHdr.className = 'visibility-group-header';
      nfcHdr.textContent = 'NFC Write Fields';
      container.appendChild(nfcHdr);
      const nfcRow = document.createElement('div');
      nfcRow.className = 'checkbox-row';
      container.appendChild(nfcRow);
      fieldConfig.forEach(item => {
        const label = document.createElement('label');
        label.className = 'checkbox-toggle';
        label.innerHTML = `<input type="checkbox" id="${item.key}">${item.label}`;
        nfcRow.appendChild(label);
        document.getElementById(item.key).checked = settings[item.key] !== false;
      });
    }

    function init() {
      const settings = {
        showWifi: true,
        showServers: true,
        showCurrentSpool: true,
        showNfcData: true,
        showLogger: true,
        showNfcJson: false,
        showTabDashboard: true,
        showTabNfc: true,
        showTabWrite: true,
        showTabOptions: true,
        showTabLogs: true,
        showTabSpools: true,
        snfName: true,
        snfMaterial: true,
        snfBrand: true,
        snfColor: true,
        snfExtTemp: true,
        snfBedTemp: true,
        snfLocation: true,
        snfRemaining: true,
        snfWeight: true,
        snfDiameter: true,
        snfDensity: true,
        snfFlow: true,
        snfMaxSpeed: true,
        snfSpoolWt: true,
        showFieldWifiSsid: true,
        showFieldWifiPass: true,
        showFieldSpoolman: true,
        showFieldMoonraker: true
      };
      renderVisibilityOptions(settings);
      showPanel('dashboard');
      updateData();
      loadSpools();
      setInterval(updateData, 5000);

      const eventSource = new EventSource('/events');
      eventSource.addEventListener('refresh', event => {
        const reason = event.data || '';
        if (reason === 'write-armed') {
          showToast('NFC write armed. Scan a tag to complete.', 'success');
        }
        if (reason === 'write-complete') {
          showToast('NFC write complete!', 'success');
        }
        if (reason === 'wifi-reconnect') {
          showToast('WiFi reconnected.', 'success');
        }
        updateData();
      });
      eventSource.onerror = () => { setConnectionStatus(false); };
    }

    init();
  </script>
</body>
</html>
  )rawliteral";
}
