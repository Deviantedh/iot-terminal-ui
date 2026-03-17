#ifndef APP_INTEGRATION_H
#define APP_INTEGRATION_H

#include "AppScreens.h"

// Small external-facing event contract for game/server integration.
enum AppExternalEventType : uint8_t {
  APP_EXTERNAL_EVENT_BALANCE_UPDATED = 0,
  APP_EXTERNAL_EVENT_GAME_STATE_CHANGED,
  APP_EXTERNAL_EVENT_SERVER_MESSAGE,
  APP_EXTERNAL_EVENT_SHOW_NOTIFICATION,
  APP_EXTERNAL_EVENT_SHOW_ERROR
};

// Small read-only snapshot for external code.
struct AppIntegrationSnapshot {
  ScreenState currentScreen;
  PowerMode powerMode;
  WifiFlowState wifiFlowState;
  bool wifiConnected;
  bool busy;
  bool idle;
  bool keyboardActive;
  bool soundEnabled;
  int balanceValue;
  int32_t gameStateValue;
};

// Safe entry points for external logic. Prefer these over direct AppState edits.
void appIntegrationSetBalance(AppState& app, int balanceValue);
void appIntegrationSetGameState(AppState& app, int32_t stateValue, const char* text = nullptr);
void appIntegrationShowInfo(AppState& app, const char* text, unsigned long durationMs = 2000);
void appIntegrationShowError(AppState& app, const char* text, unsigned long durationMs = 2500);
void appIntegrationSetBusy(AppState& app, const char* text = "BUSY");
void appIntegrationClearBusy(AppState& app);
void appIntegrationOpenScreen(AppState& app, ScreenState screen);
bool appIntegrationPostEvent(AppState& app, AppExternalEventType type,
                             int32_t value = 0, const char* text = nullptr);

AppIntegrationSnapshot appIntegrationGetSnapshot(const AppState& app);
bool appIntegrationIsWifiConnected(const AppState& app);
const char* appIntegrationGetGameStateText(const AppState& app);

#endif
