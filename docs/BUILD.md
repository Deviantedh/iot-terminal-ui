# Build Guide

## Target hardware

- Board: `ESP8266 NodeMCU v3`
- FQBN for `arduino-cli`: `esp8266:esp8266:nodemcuv2`
- CPU frequency: `80 MHz`
- Display: `ST7789 320x240`
- Touch: `XPT2046`

## Board package and libraries

Required external dependencies:

- `ESP8266 core for Arduino` `3.1.2`
- `TFT_eSPI` `2.5.43`
- `XPT2046_Touchscreen` `1.4`
- `ESP8266WiFi` and `LittleFS` come from the ESP8266 core package
- time sync uses the ESP8266 core SNTP support via standard libc time functions

## Where TFT setup is defined

This project uses a project-local `TFT_eSPI` setup in:

- `iot_terminal_ui.ino.globals.h`

The ESP8266 core injects this file into every translation unit during build, so
the project does not rely on editing `TFT_eSPI` inside the Arduino libraries folder.

Current working values:

- `USER_SETUP_ID = 1801`
- `SPI_FREQUENCY = 40000000`
- `SPI_TOUCH_FREQUENCY = 2500000`

## Verify dependency versions

From project root:

```bash
./tools/print_project_dependencies.sh
./tools/print_tft_spi_config.sh
arduino-cli version
```

If your Arduino libraries or Arduino data directory live outside the default
location, you can override autodetection:

```bash
ARDUINO_LIBRARIES_DIR=/path/to/libraries ARDUINO_DATA_DIR=/path/to/Arduino15 ./tools/print_project_dependencies.sh
```

## Build with arduino-cli

From project root:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
```

Verbose build:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --verbose .
```

## Build with Arduino IDE

1. Install `ESP8266 by ESP8266 Community`.
2. Install libraries `TFT_eSPI` and `XPT2046_Touchscreen`.
3. Open the project folder or `iot_terminal_ui.ino`.
4. Select board `NodeMCU 1.0 (ESP-12E Module)`.
5. Keep CPU frequency at `80 MHz`.
6. Build or upload normally.

## Project-specific notes

- Do not edit global `TFT_eSPI/User_Setup_Select.h` for this project.
- The active display config is the project file `iot_terminal_ui.ino.globals.h`.
- `SPI_FREQUENCY = 40000000` is the current stable working value used by the UI code and docs.
- Touch wiring stays on `D2` (`TOUCH_CS`) and `D1` (`TOUCH_IRQ`).
- `PCF8574` buttons are on software I2C lines `GPIO3` (`SDA`) and `GPIO1` (`SCL`) at address `0x20`.
- Because `GPIO1/GPIO3` are used by I2C, `APP_SERIAL_LOGGING_ENABLED` defaults to `0`; enable serial logging only if you are not using those pins for the expander.
- Current navigation model:
  - `HOME` with NTP time and `PLAY` / `MENU`
  - `MENU` with `PROFILE` / `TOP UP` / `SETTINGS` / `HOME`
  - `BALANCE` remains a separate screen opened from `GAME`
- Current power behavior:
  - short `POWER` -> black-screen sleep -> wake back to previous screen
  - long `POWER` -> pseudo-off standby -> wake to `HOME`
  - physical `MENU` -> safe jump to `HOME`
- Wi-Fi credentials are stored in LittleFS at `/wifi_credentials.txt`.
- Built-in fallback Wi-Fi networks are defined in `WiFiProfiles.cpp` and should be reviewed per device/project handoff.
- `build/` is a generated local artifact and should not be treated as source.
- The UI uses partial redraw and profiling already; avoid large redraw refactors unless necessary.
- `GAME` content is still intentionally placeholder-only; only the balance entry point is wired.
