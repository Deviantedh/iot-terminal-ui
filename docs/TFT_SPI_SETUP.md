# TFT SPI Setup (ESP8266 + ST7789)

This project now uses a project-local `TFT_eSPI` configuration.

## Where the active setting comes from

1. Global build injection file:
`iot_terminal_ui.ino.globals.h`

2. The ESP8266 core automatically injects `iot_terminal_ui.ino.globals.h` into all translation units during build.

3. `TFT_eSPI` sees `USER_SETUP_LOADED` and uses the project-local defines instead of the library's global `User_Setup_Select.h`.

## Current pinned values

- `#define ST7789_DRIVER`
- `#define SPI_FREQUENCY 80000000`
- `#define SPI_TOUCH_FREQUENCY 2500000`

## Quick verification commands

Run from project root:

```bash
./tools/print_tft_spi_config.sh
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --verbose .
```

At runtime (Serial on boot), the sketch also prints:
- `TFT USER_SETUP_ID`
- `TFT SPI_FREQUENCY`
- `TFT SPI_TOUCH_FREQUENCY`

## Current working setup

This project currently uses `80000000` as the working display SPI clock.
If you need to retest this on different hardware, change only `SPI_FREQUENCY`
in `iot_terminal_ui.ino.globals.h`, rebuild, and verify the display and touch
behavior on the real device.

This change is now per-project, not machine-global.
