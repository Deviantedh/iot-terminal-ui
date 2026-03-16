#!/usr/bin/env bash
set -euo pipefail

detect_libraries_dir() {
  if [[ -n "${ARDUINO_LIBRARIES_DIR:-}" ]]; then
    printf '%s\n' "$ARDUINO_LIBRARIES_DIR"
    return
  fi
  if [[ -d "$HOME/Documents/Arduino/libraries" ]]; then
    printf '%s\n' "$HOME/Documents/Arduino/libraries"
    return
  fi
  if [[ -d "$HOME/Arduino/libraries" ]]; then
    printf '%s\n' "$HOME/Arduino/libraries"
    return
  fi
  printf '%s\n' "$HOME/Documents/Arduino/libraries"
}

detect_arduino_data_dir() {
  if [[ -n "${ARDUINO_DATA_DIR:-}" ]]; then
    printf '%s\n' "$ARDUINO_DATA_DIR"
    return
  fi
  if [[ -d "$HOME/Library/Arduino15" ]]; then
    printf '%s\n' "$HOME/Library/Arduino15"
    return
  fi
  if [[ -d "$HOME/.arduino15" ]]; then
    printf '%s\n' "$HOME/.arduino15"
    return
  fi
  printf '%s\n' "$HOME/Library/Arduino15"
}

LIB_DIR="${1:-$(detect_libraries_dir)}"
ARDUINO15_DIR="${2:-$(detect_arduino_data_dir)}"

echo "Project dependency check"
echo "Libraries dir: $LIB_DIR"
echo "Arduino core dir: $ARDUINO15_DIR"
echo

for lib in TFT_eSPI XPT2046_Touchscreen; do
  prop="$LIB_DIR/$lib/library.properties"
  echo "[$lib]"
  if [[ -f "$prop" ]]; then
    grep -E '^(name|version|author|maintainer)=' "$prop" || true
  else
    echo "missing: $prop"
  fi
  echo
done

core_glob="$ARDUINO15_DIR/packages/esp8266/hardware/esp8266"
if [[ -d "$core_glob" ]]; then
  echo "[ESP8266 core]"
  ls -1 "$core_glob" | sort
  echo
fi

if [[ -x "./tools/print_tft_spi_config.sh" ]]; then
  ./tools/print_tft_spi_config.sh
fi
