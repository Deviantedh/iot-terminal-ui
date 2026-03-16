#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SETUP_FILE="$PROJECT_DIR/iot_terminal_ui.ino.globals.h"
GLOBALS_FILE="$SETUP_FILE"

if [[ ! -f "$SETUP_FILE" ]]; then
  echo "Project-local TFT setup not found: $SETUP_FILE"
  exit 1
fi

echo "Project-local TFT_eSPI setup: $SETUP_FILE"
echo "Injected via globals header: $GLOBALS_FILE"

user_setup_line="$(grep -E '^[[:space:]]*#define[[:space:]]+USER_SETUP_ID[[:space:]]+[0-9]+' "$SETUP_FILE" || true)"
driver_line="$(grep -E '^[[:space:]]*#define[[:space:]]+ST7789_DRIVER' "$SETUP_FILE" || true)"
spi_line="$(grep -E '^[[:space:]]*#define[[:space:]]+SPI_FREQUENCY[[:space:]]+[0-9]+' "$SETUP_FILE" || true)"
touch_line="$(grep -E '^[[:space:]]*#define[[:space:]]+SPI_TOUCH_FREQUENCY[[:space:]]+[0-9]+' "$SETUP_FILE" || true)"

echo "USER_SETUP_ID: ${user_setup_line:-not found}"
echo "Driver define: ${driver_line:-not found}"
echo "SPI_FREQUENCY: ${spi_line:-not found}"
echo "SPI_TOUCH_FREQUENCY: ${touch_line:-not found}"
