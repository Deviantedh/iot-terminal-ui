# Project Dependencies

## Runtime dependencies

- `ESP8266 core for Arduino` (tested with `3.1.2`)
- `TFT_eSPI` (detected `2.5.43`)
- `XPT2046_Touchscreen` (detected `1.4`)
- `ESP8266WiFi` (comes from ESP8266 core package)

## Why these are external (recommended)

- `ESP8266 core`: must stay external because it provides toolchain, linker scripts, and board support.
- `TFT_eSPI`: keep external to avoid carrying a large vendor library in repo. Only the project-local setup is versioned here.
- `XPT2046_Touchscreen`: small but still better as external Arduino dependency for easier upgrades and compatibility.

## What to localize inside project (recommended)

- Local app modules and adapters only:
  - `DisplayHAL`, `SimpleUI`, `AppScreens`, `OnScreenKeyboard`, `WiFiService`, `TimeService`, `Pcf8574Buttons`, `BuzzerService`
- Project-local display config:
  - `iot_terminal_ui.ino.globals.h`
- Project scripts/docs for reproducibility:
  - `tools/print_tft_spi_config.sh`
  - `tools/print_project_dependencies.sh`
  - `docs/BUILD.md`
  - `docs/TFT_SPI_SETUP.md`
  - this file

## Reproducibility workflow

1. Install/verify required libraries and ESP8266 core.
2. Run:

```bash
./tools/print_project_dependencies.sh
```

3. Confirm project-local TFT setup and SPI frequency.
   Current stable display SPI clock is `40000000`.
4. Review deployment-specific Wi-Fi built-ins in `WiFiProfiles.cpp`.
5. Build/upload with the same board core version and external library versions.

## Risks by approach

- Vendor libraries inside repo:
  - Pros: fully pinned source.
  - Cons: heavy repository, harder updates, frequent merge noise.
- External libraries + pinned versions + project-local setup (current approach):
  - Pros: cleaner repo, easier maintenance.
  - Cons: still depends on installed library versions.

For this project, external libraries + strict version notes is the best balance.

## Board-specific wiring notes

- Touch uses `D2` for `TOUCH_CS` and `D1` for `TOUCH_IRQ`.
- `PCF8574` buttons use `GPIO3` (`SDA`) and `GPIO1` (`SCL`) at address `0x20`.
- Passive buzzer is assigned to `D0` / `GPIO16`.
- With that wiring, UART pins are repurposed for I2C, so serial logging is disabled by default.
- If Serial Monitor is opened anyway, I2C traffic on `GPIO1/GPIO3` appears as garbage characters in the terminal.
