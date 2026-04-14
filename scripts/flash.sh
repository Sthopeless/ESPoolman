#!/usr/bin/env bash
# =============================================================================
# ESPoolman build & flash helper
# Usage:
#   ./scripts/flash.sh                # clean → build → USB upload
#   ./scripts/flash.sh monitor        # build → upload → open serial monitor
#   ./scripts/flash.sh ota            # clean → build → OTA upload
#   ./scripts/flash.sh erase          # erase full flash (wipes NVS/WiFi creds!)
#   ./scripts/flash.sh erase-upload   # delete .pio + erase flash + build + upload
# =============================================================================

set -euo pipefail

cd "$(dirname "$0")/.."

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

MODE="${1:-}"
ENV_USB="esp32-c6-super-mini"
ENV_OTA="esp32-c6-super-mini-ota"

banner() { echo -e "\n${BOLD}${CYAN}▶ $*${RESET}\n"; }
ok()     { echo -e "${GREEN}✓ $*${RESET}"; }
warn()   { echo -e "${YELLOW}⚠ $*${RESET}"; }

# ── Erase full flash (clears NVS — WiFi creds and all settings will be lost) ──
erase_flash() {
  warn "This will ERASE the entire flash including WiFi credentials and NVS settings."
  read -rp "  Continue? [y/N] " confirm
  [[ "$confirm" =~ ^[Yy]$ ]] || { echo "Aborted."; exit 0; }
  banner "Erasing flash..."
  pio run --target erase -e "$ENV_USB"
  ok "Flash erased."
}

case "$MODE" in

  erase)
    erase_flash
    ;;

  erase-upload)
    erase_flash
    if [ -d ".pio" ]; then
      banner "Removing .pio build cache for a truly fresh build..."
      rm -rf .pio
      ok ".pio deleted."
    fi
    banner "Building..."
    pio run -e "$ENV_USB"
    banner "Uploading via USB..."
    pio run --target upload -e "$ENV_USB"
    ok "Done — fresh flash complete (NVS erased, .pio cleared, device will start in AP mode)."
    pio device monitor -e "$ENV_USB"
    ;;

  monitor)
    banner "Building..."
    pio run -e "$ENV_USB"
    banner "Uploading via USB..."
    pio run --target upload -e "$ENV_USB"
    ok "Upload complete — opening serial monitor (Ctrl+C to exit)."
    pio device monitor -e "$ENV_USB"
    ;;

  ota)
    banner "Cleaning build artifacts..."
    pio run --target clean -e "$ENV_OTA"
    banner "Building..."
    pio run -e "$ENV_OTA"
    banner "Uploading via OTA (${ENV_OTA})..."
    pio run --target upload -e "$ENV_OTA"
    ok "OTA upload complete."
    ;;

  "")
    banner "Cleaning build artifacts..."
    pio run --target clean -e "$ENV_USB"
    banner "Building..."
    pio run -e "$ENV_USB"
    banner "Uploading via USB..."
    pio run --target upload -e "$ENV_USB"
    ok "Upload complete — NVS preserved (WiFi/settings intact)."
    ;;

  *)
    echo "Unknown mode: $MODE"
    echo "Usage: $0 [monitor|ota|erase|erase-upload]"
    exit 1
    ;;
esac
