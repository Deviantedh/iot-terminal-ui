# Integration Guide

This project already has a working UI shell, power model, Wi-Fi flow, event queue,
and hardware adapters. Future game/server code should plug into that surface, not
write directly into random `AppState` fields.

## Existing modules

- `DisplayHAL`
  - TFT, touch, display sleep/backlight hooks
- `SimpleUI`
  - lightweight drawing primitives
- `AppScreens`
  - screen routing, redraw policy, input handling, platform events
- `AppIntegration`
  - safe external-facing API for game/server code
- `WiFiService`
  - non-blocking Wi-Fi scan/connect state machine
- `WiFiProfiles`
  - saved credentials and built-in fallback networks
- `TimeService`
  - NTP-backed time for `HOME`
- `Pcf8574Buttons`
  - physical `MENU` / `POWER`
- `BuzzerService`
  - simple passive buzzer patterns
- `ServerApiService`
  - isolated server API client; current spin mode is strict network or fully local by config

## Do not touch directly

External code should avoid direct writes to:

- touch fields
- redraw flags / dirty regions
- button animation state
- keyboard internals
- Wi-Fi flow internals
- power mode fields

Direct `AppState` reads are acceptable for debugging, but normal integration
should go through `AppIntegration`.

## Use this API

Header:

- `AppIntegration.h`

Server API contract:

- `docs/server-api.openapi.yaml`

Main entry points:

- `appIntegrationSetBalance(app, value)`
- `appIntegrationSetGameState(app, stateValue, text)`
- `appIntegrationShowInfo(app, "text")`
- `appIntegrationShowError(app, "text")`
- `appIntegrationSetBusy(app, "SYNCING")`
- `appIntegrationClearBusy(app)`
- `appIntegrationOpenScreen(app, SCREEN_GAME)`
- `appIntegrationPostEvent(app, APP_EXTERNAL_EVENT_SHOW_NOTIFICATION, 0, "Ready")`
- `appIntegrationGetSnapshot(app)`
- `appIntegrationIsWifiConnected(app)`
- `appIntegrationGetGameStateText(app)`

## Event contract

Internal platform events:

- `APP_EVENT_WIFI_CONNECTED`
- `APP_EVENT_WIFI_DISCONNECTED`
- `APP_EVENT_BUTTON_MENU`
- `APP_EVENT_BUTTON_POWER_SHORT`
- `APP_EVENT_BUTTON_POWER_LONG`

External integration events:

- `APP_EVENT_BALANCE_UPDATED`
- `APP_EVENT_EXTERNAL_MESSAGE`
- `APP_EVENT_SHOW_NOTIFICATION`
- `APP_EVENT_SHOW_ERROR`
- `APP_EVENT_GAME_STATE_CHANGED`

Helpers:

- `appEventIsExternal(type)`
- `appEventIsInternal(type)`
- `appEventName(type)`

Recommended rule:

- platform code posts internal events
- game/server code posts only external events

## Top-level screen/state concepts

- `HOME`
- `MENU`
- `GAME`
- `BALANCE`
- `SETTINGS`
- `PROFILE`
- `TOPUP`
- `POWER_MODE_NORMAL`
- `POWER_MODE_SLEEP`
- `POWER_MODE_STANDBY`

## Minimal examples

Update balance after server response:

```cpp
appIntegrationSetBalance(app, 1250);
appIntegrationShowInfo(app, "Balance updated");
```

Show sync state:

```cpp
appIntegrationSetBusy(app, "SYNCING");
// ... async work ...
appIntegrationClearBusy(app);
```

Report game/server state through the queue:

```cpp
appIntegrationPostEvent(app, APP_EXTERNAL_EVENT_GAME_STATE_CHANGED, 2, "ROUND LIVE");
appIntegrationPostEvent(app, APP_EXTERNAL_EVENT_SHOW_NOTIFICATION, 0, "Server online");
```

Open balance safely:

```cpp
appIntegrationOpenScreen(app, SCREEN_BALANCE);
```

`SCREEN_BALANCE` keeps `balanceReturnScreen` aligned automatically, so future
game code can open it without manually patching return navigation.

## Server API mode rule

`ServerApiService` owns all HTTP requests. The current runtime mode is strict
network spin: the device waits for a server spin response before starting reel
animation. If the request fails, the UI returns to an idle spin button state and
shows the error without starting local fallback spin.

HTTP transport is handled through the ESP8266 Arduino core `ESP8266HTTPClient`.
Do not move request code into screen handlers or Wi-Fi connection code.

There is no recurring background server polling. After Wi-Fi time becomes valid,
the home screen may perform a one-shot bootstrap sequence: health, auth, balance.
After that, gameplay requests are only made by explicit spin actions.

Mode switches live in `ServerApiConfig.h`:

- `SERVER_API_NETWORK_ENABLED`
- `SERVER_API_REQUIRE_SERVER_FOR_SPIN`

Current setting is online-only for spin: network enabled and server-required spin
are both `true`. For fully local play, set `SERVER_API_NETWORK_ENABLED` to `false`.
