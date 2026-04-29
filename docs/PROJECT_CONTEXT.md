# Report Context for ChatGPT

This file summarizes the current final state of the `iot_terminal_ui` project.
It is intended to be copied into another ChatGPT conversation to help prepare
the final project report and defense presentation.

## Project Summary

The project is an interactive IoT gaming terminal based on:

- `ESP8266 NodeMCU v3`
- `ST7789 320x240` TFT display
- `XPT2046` resistive touch controller
- `PCF8574` I2C GPIO expander
- physical `POWER` and `MENU` buttons
- passive buzzer on `D0 / GPIO16`
- Wi-Fi connection and server API integration

The project started as a UI platform and evolved into a working embedded
application with:

- responsive TFT UI
- touch input
- physical service buttons
- Wi-Fi scan/connect flow
- saved Wi-Fi credentials
- sleep/standby behavior
- slot-machine game
- balance and account concepts
- server-backed spin results
- mock backend and OpenAPI contract
- integration API for future external logic

The project should be presented as:

> An interactive ESP8266-based IoT gaming terminal with a TFT touch interface,
> physical controls, Wi-Fi, slot-machine gameplay, account/balance logic, and a
> server API contour.

## Current Git / Build Status

- Working tree was clean during the latest analysis except for two untracked
  backend HTML files:
  - `backend/admin_panel.html`
  - `backend/user_panel.html`
- The project compiles with:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
```

- In plain `arduino-cli` balanced MMU mode, build output showed about:
  - RAM: `38076 / 80192 bytes` (`47%`)
  - IRAM: `62283 / 65536 bytes` (`95%`)
  - IROM/flash code: `402940 / 1048576 bytes` (`38%`)
- On the real Arduino IDE configuration with `16KB cache + 48KB IRAM`, the user
  observed about `70%` IRAM usage.
- The recommended Arduino IDE MMU mode is now:

```text
16KB cache + 48KB IRAM (IRAM)
```

Reason:

- IRAM dropped roughly from `94%` to `70%`
- no visible UI slowdown was observed on the real device

## Hardware Configuration

### Main Parts

- ESP8266 NodeMCU v3
- ST7789 320x240 TFT
- XPT2046 resistive touch
- PCF8574 I/O expander
- passive buzzer
- physical buttons:
  - `POWER`
  - `MENU`

### Pin / Wiring Notes

- Touch:
  - `TOUCH_CS = D2`
  - `TOUCH_IRQ = D1`
- PCF8574:
  - address: `0x20`
  - `SDA = GPIO3 (RX)`
  - `SCL = GPIO1 (TX)`
- Buzzer:
  - `D0 / GPIO16`
- TFT setup is project-local in:
  - `iot_terminal_ui.ino.globals.h`
- TFT SPI:
  - `SPI_FREQUENCY = 40000000`
  - `SPI_TOUCH_FREQUENCY = 2500000`

Important caveat:

- Because PCF8574 uses `GPIO1/GPIO3`, Serial Monitor is not used in normal
  working mode.
- If Serial Monitor is opened anyway, I2C traffic on TX/RX appears as garbage
  characters.
- `APP_SERIAL_LOGGING_ENABLED` is disabled by default.

## Firmware Architecture

### Main Modules

- `DisplayHAL`
  - TFT initialization
  - touch reading
  - ST7789 sleep/display commands
  - backlight hook
- `SimpleUI`
  - lightweight UI primitives
  - fast rectangular drawing style
- `AppScreens`
  - screen routing
  - `AppState`
  - dirty/full redraw
  - touch handling
  - Wi-Fi UX flow
  - power state
  - slot game UI orchestration
  - server bootstrap orchestration
- `OnScreenKeyboard`
  - SSID/password text entry
  - alpha/symbol pages
  - lowercase by default
- `WiFiService`
  - non-blocking scan/connect polling state machine
- `WiFiProfiles`
  - saved Wi-Fi credentials in LittleFS
  - built-in fallback networks
- `TimeService`
  - NTP-backed clock for HOME screen
- `Pcf8574Buttons`
  - PCF8574 polling
  - debounce
  - MENU button event
  - POWER short/long press events
- `BuzzerService`
  - non-blocking sound patterns
  - click/error/wake/boot/spin/win/lose/test sounds
- `SlotGame`
  - slot symbols
  - reel modes
  - paylines
  - outcome evaluation
- `ServerApiService`
  - HTTP client for backend API
  - accounts/auth/balance/spin/events
- `AppIntegration`
  - safe external-facing API for future game/server logic

### Current Technical Risk

`AppScreens.cpp` has grown to nearly `4000` lines. It still works, but it now
contains too much orchestration:

- screen drawing
- touch routing
- slot UI
- server polling
- power flow
- Wi-Fi flow
- dirty redraw handling

This is acceptable for the current defense, but a future refactor should split
it into smaller files.

## Screens / UX Structure

Current screens:

- `HOME`
  - NTP time
  - PLAY button
  - MENU button
  - server/player status area
- `MENU`
  - PROFILE
  - TOP UP
  - SETTINGS
  - HOME
- `MODE_SELECT`
  - choose slot mode:
    - Classic / 3 reels
    - Advanced / 5 reels + paylines
- `GAME`
  - slot-machine gameplay
  - top-right balance entry
  - bet controls
  - spin button
  - auto-play for 3-reel mode
- `BALANCE`
  - separate balance/wins screen
  - opened from GAME
- `PROFILE`
  - account picker / server account flow
- `TOPUP`
  - placeholder on the device
- `SETTINGS`
  - Wi-Fi settings
  - touch diagnostics
  - tests
  - sound toggle
- `TESTS`
  - sound test screen

## Game Logic

The game is a slot machine with two modes:

- Classic mode:
  - 3 reels
  - 1 row
  - simple 3-symbol match
- Advanced mode:
  - 5 reels
  - 3 rows
  - 20 paylines

Gameplay features:

- balance
- bet levels
- spin animation
- server-required spin mode
- local fallback architecture exists in code/config, but current mode is
  server-required for spin
- payout calculation
- paylines in 5-reel mode
- visual symbols/icons
- sounds:
  - spin start
  - reel stop
  - small win
  - jackpot
  - lose

Important configuration:

- `ServerApiConfig.h`
- Current settings:
  - `SERVER_API_NETWORK_ENABLED = true`
  - `SERVER_API_REQUIRE_SERVER_FOR_SPIN = true`

This means:

- device waits for server spin result
- if server request fails, game does not start a local fallback spin

## Server / Backend

### Device API

The API contract is documented in:

- `docs/server-api.openapi.yaml`

Main endpoints:

- `GET /api/v1/health`
- `POST /api/v1/devices/auth`
- `GET /api/v1/devices/{deviceId}/accounts`
- `GET /api/v1/devices/{deviceId}/balance`
- `POST /api/v1/games/slot/3-reels/spin`
- `POST /api/v1/games/slot/5-reels/spin`
- `POST /api/v1/devices/events`

### Mock Backend

Backend files:

- `backend/mock_server.py`
- `backend/requirements.txt`
- `backend/README.md`
- `backend/install_systemd_service.sh`
- `backend/iot-terminal-backend.service`

The mock backend:

- uses `aiohttp`
- returns JSON responses
- supports health/auth/accounts/balance/spin/events
- keeps in-memory accounts, balances and sessions
- can run as a Linux daemon through systemd

Run command:

```bash
python3 -m pip install -r backend/requirements.txt
python3 backend/mock_server.py --host 0.0.0.0 --port 8080
```

### Backend HTML Panels

Untracked files found:

- `backend/admin_panel.html`
- `backend/user_panel.html`

They appear to be:

- admin control panel
- user personal cabinet

Important caveat:

- These HTML files call routes like `/api/v1/backend/...`
- The local `backend/mock_server.py` currently does not define those routes
- Therefore, in the report they should be described carefully as prepared /
  additional backend panel files, unless the real VPS backend supports them

Safe wording:

> In addition to the device API and mock server, HTML prototypes for an admin
> panel and user cabinet are prepared in the backend folder. They are intended
> for the full backend environment.

## Wi-Fi / Network Flow

Implemented:

- real Wi-Fi scan
- network list with pagination
- SSID tap selection
- password input through on-screen keyboard
- saved credentials in LittleFS
- auto-connect on boot
- reconnect after standby
- built-in fallback networks
- non-blocking Wi-Fi service model

Important caveat:

- `WiFiProfiles.cpp` currently contains real built-in SSID/password entries.
- These should be reviewed before making the repository public or sharing it.

## Power / Sleep Behavior

Implemented:

- normal mode
- ordinary sleep
- standby / pseudo-off
- POWER short press:
  - enter ordinary sleep
  - wake back to previous screen
- POWER long press:
  - enter standby
  - wake to HOME
  - restart Wi-Fi auto-connect flow
- touch is fully ignored during sleep/standby
- MENU does not wake from sleep
- ST7789 display sleep hooks exist
- black frame fallback is used because fixed backlight may make controller-off
  state appear white

Important hardware limitation:

- real backlight control is not wired yet
- `BACKLIGHT_PIN = -1`
- true display darkening would require a transistor/PWM-controlled backlight
  circuit

## UI / Performance / Stability

Implemented:

- dirty regions / partial redraw
- full redraw when screen changes
- optimized fast drawing style
- reduced round-rect/shadow overdraw
- touch-safe hitboxes
- lower touch polling through IRQ gate
- NTP time redraw only on minute change
- debug/profiling hooks gated by `APP_SERIAL_LOGGING_ENABLED`

Display stability choices:

- `SPI_FREQUENCY = 40000000`
- previous `80000000` caused visual artifacts:
  - bottom stripes
  - white rectangular corruption near top title area
- ST7789 color inversion is explicitly disabled after init

## Integration API

External integration should use:

- `AppIntegration.h`

Main API:

- `appIntegrationSetBalance(app, value)`
- `appIntegrationSetGameState(app, stateValue, text)`
- `appIntegrationShowInfo(app, text)`
- `appIntegrationShowError(app, text)`
- `appIntegrationSetBusy(app, text)`
- `appIntegrationClearBusy(app)`
- `appIntegrationOpenScreen(app, screen)`
- `appIntegrationPostEvent(app, APP_EXTERNAL_EVENT_...)`
- `appIntegrationGetSnapshot(app)`
- `appIntegrationIsWifiConnected(app)`
- `appIntegrationGetGameStateText(app)`

Event contract:

- internal platform events:
  - `APP_EVENT_WIFI_CONNECTED`
  - `APP_EVENT_WIFI_DISCONNECTED`
  - `APP_EVENT_BUTTON_MENU`
  - `APP_EVENT_BUTTON_POWER_SHORT`
  - `APP_EVENT_BUTTON_POWER_LONG`
- external events:
  - `APP_EVENT_BALANCE_UPDATED`
  - `APP_EVENT_EXTERNAL_MESSAGE`
  - `APP_EVENT_SHOW_NOTIFICATION`
  - `APP_EVENT_SHOW_ERROR`
  - `APP_EVENT_GAME_STATE_CHANGED`

## Suggested Final Report Structure

1. Title page
   - project name
   - team members
   - course/group
   - date

2. Project goal
   - build an ESP8266-based interactive gaming terminal
   - combine embedded UI, hardware controls, Wi-Fi and server logic

3. Task statement
   - touch UI
   - Wi-Fi connectivity
   - game
   - account/balance model
   - backend API
   - power/sleep handling

4. Hardware architecture
   - ESP8266 NodeMCU v3
   - ST7789 display
   - XPT2046 touch
   - PCF8574 expander
   - buttons
   - buzzer
   - wiring diagram

5. Firmware architecture
   - module diagram
   - responsibilities of each module
   - main loop structure
   - AppState / event system / integration API

6. User interface
   - HOME
   - MENU
   - MODE SELECT
   - GAME
   - BALANCE
   - PROFILE
   - SETTINGS
   - TESTS
   - mention touch-friendly layout and safe hitboxes

7. Game implementation
   - slot machine
   - 3-reel mode
   - 5-reel mode
   - paylines
   - bet and payout
   - sounds
   - server-controlled spin result

8. Wi-Fi and persistence
   - scan
   - connect
   - saved credentials
   - auto-connect
   - reconnect
   - LittleFS

9. Backend integration
   - OpenAPI contract
   - device auth
   - account list
   - balance
   - spin result
   - device events
   - mock backend
   - systemd deployment
   - optional admin/user panel prototypes

10. Optimization and reliability
    - project-local TFT setup
    - SPI 40 MHz
    - MMU 16KB cache + 48KB IRAM
    - dirty redraw
    - fast theme
    - sleep/standby
    - Serial disabled due to RX/TX I2C

11. Testing / demonstration
    - upload/build
    - Wi-Fi connection
    - HOME time
    - account selection
    - game spin
    - balance update
    - settings screen
    - sleep/wake
    - server mock/backend

12. Problems and limitations
    - no hardware backlight control yet
    - Serial Monitor not used due to RX/TX I2C
    - `AppScreens.cpp` is large and should be split later
    - TOP UP on device is still placeholder
    - backend HTML panels may require full backend routes

13. Future work
    - final enclosure / soldering
    - real backlight power control
    - autonomous power supply
    - complete TOP UP flow
    - polish admin/user backend panels
    - split AppScreens into smaller modules
    - persistent backend database instead of in-memory mock

## Suggested Defense Flow

1. Show the physical device and wiring diagram.
2. Explain the hardware modules.
3. Show HOME screen.
4. Show MENU and navigation.
5. Show PROFILE/account flow if server is available.
6. Show MODE SELECT.
7. Show 3-reel game.
8. Show 5-reel game.
9. Show BALANCE.
10. Show SETTINGS / Wi-Fi.
11. Explain server API and backend mock.
12. Explain optimizations:
    - MMU mode
    - SPI 40 MHz
    - dirty redraw
    - sleep/standby
13. Finish with limitations and next steps.

## Strong Points to Emphasize

- This is not just an Arduino screen demo.
- It combines embedded UI, hardware input, networking, game logic and backend API.
- The UI is responsive despite ESP8266 limitations.
- The project has a clear module structure.
- The backend contract is documented with OpenAPI.
- The device can use server-provided spin results.
- The build is more reproducible because TFT setup is project-local.
- The project was optimized for real hardware issues:
  - SPI artifacts
  - IRAM pressure
  - touch noise
  - sleep behavior
  - UART pin conflict

## Known Caveats for Honest Reporting

- Deep sleep is not used.
- Backlight is not physically controlled yet.
- Some backend panels may rely on server routes not present in local mock.
- `TOP UP` is not fully implemented on the device UI.
- Current backend mock is in-memory, not persistent.
- `AppScreens.cpp` needs future decomposition.

