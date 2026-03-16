# iot_terminal_ui

UI platform for `ESP8266 NodeMCU v3 + ST7789 320x240 + XPT2046 touch`.

The project is organized as a responsive modular Arduino UI with:

- partial redraw / dirty regions
- optional profiling through `Serial`
- on-screen keyboard
- Wi-Fi scan / select / connect flow
- persistent Wi-Fi credentials in `LittleFS`
- `HOME`, `MENU`, `SETTINGS`, `PROFILE`, `TOP UP`, `BALANCE`, `GAME`
- NTP-backed time on `HOME`
- physical `MENU` / `POWER` buttons through `PCF8574`

The current goal of the codebase is to stay stable and portable as a base for
later game integration and server-side logic.

## Module layout

- `DisplayHAL`
  - display and touch hardware access
- `SimpleUI`
  - reusable UI primitives and lightweight control drawing
- `AppScreens`
  - app state, screen routing, redraw policy, profiling, Wi-Fi UX flow
- `OnScreenKeyboard`
  - touch keyboard for SSID/password entry
- `WiFiService`
  - non-blocking Wi-Fi scan/connect polling state machine
- `WiFiProfiles`
  - saved credentials in `LittleFS` and built-in fallback networks
- `TimeService`
  - lightweight NTP time sync for `HOME`
- `Pcf8574Buttons`
  - debounced physical button polling with short/long `POWER`
- `BuzzerService`
  - lightweight passive buzzer output on `D0 / GPIO16`
- `iot_terminal_ui.ino`
  - entry point, setup, main loop

## Current UX structure

- `HOME`
  - wake/start screen with current time, `PLAY`, `MENU`
- `MENU`
  - `PROFILE`, `TOP UP`, `SETTINGS`, `HOME`
- `PROFILE`
  - placeholder screen
- `TOP UP`
  - placeholder screen
- `GAME`
  - game placeholder plus top-right balance entry point
- `BALANCE`
  - separate screen opened from `GAME`
- `SETTINGS`
  - Wi-Fi, touch diagnostics, sound

Power behavior:

- short `POWER`: ST7789 display off + sleep-in, wake back to the same screen
- long `POWER`: pseudo-off standby, wake to `HOME` and restart non-blocking Wi-Fi auto-connect flow
- physical `MENU`: safe jump to `HOME`

## Build summary

Main build instructions are in `docs/BUILD.md`.

Short version:

- board: `ESP8266 NodeMCU v3`
- `arduino-cli` FQBN: `esp8266:esp8266:nodemcuv2`
- CPU frequency: `80 MHz`
- current TFT SPI clock: `40000000`

Required libraries:

- `ESP8266 core for Arduino` `3.1.2`
- `TFT_eSPI` `2.5.43`
- `XPT2046_Touchscreen` `1.4`

Generated local artifacts such as `build/` are not part of the source contract.

## Where TFT setup is defined

The active `TFT_eSPI` setup is project-local:

- `iot_terminal_ui.ino.globals.h`

This file is injected by the ESP8266 core into all translation units, so the
project does not require editing global `TFT_eSPI` library config files.

## Flashing

Arduino IDE:

1. Install the required board package and libraries.
2. Open `iot_terminal_ui.ino`.
3. Select `NodeMCU 1.0 (ESP-12E Module)`.
4. Keep CPU frequency at `80 MHz`.
5. Build and upload.

Hardware notes:

- touch stays on `TOUCH_CS = D2`, `TOUCH_IRQ = D1`
- `PCF8574` buttons use `SDA = GPIO3 (RX)`, `SCL = GPIO1 (TX)`, address `0x20`
- passive buzzer is assigned to `D0 / GPIO16`
- because `GPIO1/GPIO3` are reused for I2C, serial logging is disabled by default in `iot_terminal_ui.ino.globals.h`

`arduino-cli`:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 -p <serial-port> .
```

## Serial diagnostics

Serial logging is compile-time gated by `APP_SERIAL_LOGGING_ENABLED`.

If you temporarily enable it, use Serial Monitor at `115200`.

When enabled, the firmware prints:

- boot banner
- active TFT setup id
- `SPI_FREQUENCY`
- `SPI_TOUCH_FREQUENCY`

During runtime it can also print:

- tap coordinates
- UI profiling statistics
- Wi-Fi field update debug lines

With the current `PCF8574` wiring on `GPIO1/GPIO3`, leave serial logging off unless
you intentionally move the expander off those pins again.
If you still open Serial Monitor with this wiring, the USB-UART chip will see I2C traffic
on `TX/RX` as garbage characters. That is a wiring side effect, not firmware logging.

## Portability notes

- The project expects project-local TFT setup from `iot_terminal_ui.ino.globals.h`.
- `iot_terminal_ui.ino` contains compile-time guards that fail the build if that setup is not injected.
- `tools/print_project_dependencies.sh` supports autodetection and optional overrides through `ARDUINO_LIBRARIES_DIR` and `ARDUINO_DATA_DIR`.
- Review built-in Wi-Fi fallback entries in `WiFiProfiles.cpp` before handing the project to another participant.
- `GPIO16` works for simple buzzer output in the current implementation, but it still keeps the usual ESP8266 pin-16 limitations for other use cases such as interrupts/deep-sleep wake.

## Notes for the next integration stage

- `AppState` is the central application/UI state container.
- `WiFiService` owns non-blocking Wi-Fi transport state.
- `WiFiProfiles` owns persisted credentials and fallback network definitions.
- Future game logic should plug into `AppScreens` update/draw flow without
  changing the existing redraw contract.
- Future server logic should be attached as a separate service module and
  surfaced into `AppState`, not mixed directly into drawing code.
