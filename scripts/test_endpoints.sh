#!/usr/bin/env bash
# =============================================================================
# ESPoolman endpoint tester
# Usage:
#   ./scripts/test_endpoints.sh --device_ip=<ip> --moonraker=<ip:port> --spoolman=<ip:port>
#
# Example:
#   ./scripts/test_endpoints.sh --device_ip=192.168.1.100 --moonraker=192.168.1.50:7125 --spoolman=192.168.1.50:7912
# =============================================================================

DEVICE=""
MOONRAKER=""
SPOOLMAN=""

for arg in "$@"; do
  case "$arg" in
    --device_ip=*) DEVICE="${arg#*=}" ;;
    --moonraker=*) MOONRAKER="${arg#*=}" ;;
    --spoolman=*)  SPOOLMAN="${arg#*=}" ;;
  esac
done

if [[ -z "$DEVICE" || -z "$MOONRAKER" || -z "$SPOOLMAN" ]]; then
  echo "Usage: $0 --device_ip=<ip> --moonraker=<ip:port> --spoolman=<ip:port>"
  exit 1
fi
TIMEOUT=5

# ── Colours ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

pass() { echo -e "  ${GREEN}✓${RESET} $*"; }
fail() { echo -e "  ${RED}✗${RESET} $*"; }
info() { echo -e "  ${YELLOW}→${RESET} $*"; }
section() { echo -e "\n${BOLD}${CYAN}═══ $* ═══${RESET}"; }

# ── Helper: HTTP GET with status code ────────────────────────────────────────
# get <url> [--show-body] [--jq <filter>]
get() {
  local url="$1"; shift
  local show_body=0 jq_filter=""
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --show-body) show_body=1 ;;
      --jq)        jq_filter="$2"; shift ;;
    esac
    shift
  done

  local tmp; tmp=$(mktemp)
  local code
  code=$(curl -s -o "$tmp" -w "%{http_code}" --connect-timeout "$TIMEOUT" -m "$TIMEOUT" "$url" 2>/dev/null)
  local body; body=$(cat "$tmp"); rm -f "$tmp"

  if [[ "$code" =~ ^2 ]]; then
    pass "HTTP $code  $url"
    if [[ -n "$jq_filter" ]] && command -v jq &>/dev/null; then
      echo "$body" | jq -r "$jq_filter" 2>/dev/null | sed 's/^/       /'
    elif [[ $show_body -eq 1 ]]; then
      echo "$body" | head -c 400 | sed 's/^/       /'
      echo
    fi
  elif [[ "$code" == "000" ]]; then
    fail "UNREACHABLE  $url"
  else
    fail "HTTP $code  $url"
    if [[ $show_body -eq 1 ]]; then
      echo "$body" | head -c 200 | sed 's/^/       /'
    fi
  fi
  echo "$code"
}

# ── Helper: HTTP POST ─────────────────────────────────────────────────────────
post() {
  local url="$1" body="$2"
  local tmp; tmp=$(mktemp)
  local code
  code=$(curl -s -o "$tmp" -w "%{http_code}" --connect-timeout "$TIMEOUT" -m "$TIMEOUT" \
    -X POST -H "Content-Type: application/json" -d "$body" "$url" 2>/dev/null)
  local resp; resp=$(cat "$tmp"); rm -f "$tmp"
  if [[ "$code" =~ ^2 ]]; then
    pass "HTTP $code  POST $url  body=$body"
    echo "$resp" | head -c 200 | sed 's/^/       /'
  elif [[ "$code" == "000" ]]; then
    fail "UNREACHABLE  POST $url"
  else
    fail "HTTP $code  POST $url"
    echo "$resp" | head -c 200 | sed 's/^/       /'
  fi
}

# ── Validate JSON ─────────────────────────────────────────────────────────────
check_json() {
  local url="$1"
  local tmp; tmp=$(mktemp)
  curl -s -o "$tmp" --connect-timeout "$TIMEOUT" -m "$TIMEOUT" "$url" 2>/dev/null
  if command -v python3 &>/dev/null; then
    if python3 -c "import json,sys; json.load(open('$tmp'))" 2>/dev/null; then
      pass "JSON valid  $url"
    else
      fail "INVALID JSON  $url  (binary bytes or malformed response)"
      info "First 200 bytes (hex):"
      xxd "$tmp" 2>/dev/null | head -5 | sed 's/^/       /' || od -c "$tmp" | head -5 | sed 's/^/       /'
    fi
  elif command -v jq &>/dev/null; then
    if jq . "$tmp" &>/dev/null; then
      pass "JSON valid  $url"
    else
      fail "INVALID JSON  $url"
    fi
  else
    info "JSON validation skipped (install python3 or jq)"
  fi
  rm -f "$tmp"
}

# =============================================================================
echo -e "${BOLD}ESPoolman Endpoint Tester${RESET}"
echo "  Device   : http://$DEVICE"
echo "  Moonraker: http://$MOONRAKER"
echo "  Spoolman : http://$SPOOLMAN"

# ── 1. ESPoolman device ───────────────────────────────────────────────────────
section "ESPoolman device  ($DEVICE)"

info "Checking /data (WebUI data endpoint)..."
get "http://$DEVICE/data" --show-body

info "Validating /data JSON (binary bytes in nfcText corrupt the entire WebUI)..."
check_json "http://$DEVICE/data"

info "Checking /spools (Spoolman cache)..."
get "http://$DEVICE/spools" --show-body

info "Triggering /status (Moonraker status check)..."
get "http://$DEVICE/status"

# ── 2. Moonraker ─────────────────────────────────────────────────────────────
section "Moonraker  ($MOONRAKER)"

info "Server info..."
get "http://$MOONRAKER/server/info" \
    --jq '(.result.klippy_state // "n/a") as $k | "klippy_state=\($k)"'

info "Spoolman integration status..."
get "http://$MOONRAKER/server/spoolman/status" \
    --jq '(.result | "spoolman_connected=\(.spoolman_connected)  spool_id=\(.spool_id // "none")")'

info "Printer objects (check klippy is ready)..."
get "http://$MOONRAKER/printer/info" \
    --jq '(.result | "state=\(.state)  hostname=\(.hostname)")'

# ── 3. Spoolman ───────────────────────────────────────────────────────────────
section "Spoolman  ($SPOOLMAN)"

info "Health check..."
get "http://$SPOOLMAN/api/v1/health"

info "List all active spools..."
get "http://$SPOOLMAN/api/v1/spool?allow_archived=false" \
    --jq '[.[] | "#\(.id) \(.filament.name // "unnamed") (\(.filament.material // "?"))"] | .[]'

info "Fetch spool ID 1 (change to a real ID if needed)..."
get "http://$SPOOLMAN/api/v1/spool/1" \
    --jq '"#\(.id) \(.filament.name // "unnamed") \(.filament.material // "") brand=\(.filament.vendor.name // "?")"'

# ── 4. End-to-end: set spool via Moonraker ───────────────────────────────────
section "End-to-end: set spool via Moonraker"
info "Posting spool_id=1 to Moonraker (change ID as needed)..."
post "http://$MOONRAKER/server/spoolman/spool_id" '{"spool_id":1}'

echo -e "\n${BOLD}Done.${RESET}\n"
