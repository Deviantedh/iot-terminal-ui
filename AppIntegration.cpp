#include "AppIntegration.h"

#include <string.h>

static AppEventType toInternalEvent(AppExternalEventType type) {
  switch (type) {
    case APP_EXTERNAL_EVENT_BALANCE_UPDATED:
      return APP_EVENT_BALANCE_UPDATED;
    case APP_EXTERNAL_EVENT_GAME_STATE_CHANGED:
      return APP_EVENT_GAME_STATE_CHANGED;
    case APP_EXTERNAL_EVENT_SERVER_MESSAGE:
      return APP_EVENT_EXTERNAL_MESSAGE;
    case APP_EXTERNAL_EVENT_SHOW_NOTIFICATION:
      return APP_EVENT_SHOW_NOTIFICATION;
    case APP_EXTERNAL_EVENT_SHOW_ERROR:
      return APP_EVENT_SHOW_ERROR;
  }

  return APP_EVENT_NONE;
}

void appIntegrationSetBalance(AppState& app, int balanceValue) {
  appPostEvent(app, APP_EVENT_BALANCE_UPDATED, balanceValue);
}

void appIntegrationSetGameState(AppState& app, int32_t stateValue, const char* text) {
  appPostEvent(app, APP_EVENT_GAME_STATE_CHANGED, stateValue, text);
}

void appIntegrationShowInfo(AppState& app, const char* text, unsigned long durationMs) {
  appUiShowMessage(app, text, TFT_CYAN, durationMs);
}

void appIntegrationShowError(AppState& app, const char* text, unsigned long durationMs) {
  appUiShowError(app, text, durationMs);
}

void appIntegrationSetBusy(AppState& app, const char* text) {
  appUiSetBusy(app, true, text);
}

void appIntegrationClearBusy(AppState& app) {
  appUiSetBusy(app, false, "");
}

void appIntegrationOpenScreen(AppState& app, ScreenState screen) {
  if (screen == SCREEN_BALANCE && app.currentScreen != SCREEN_BALANCE) {
    app.balanceReturnScreen = app.currentScreen;
  }
  appUiSwitchScreen(app, screen);
}

bool appIntegrationPostEvent(AppState& app, AppExternalEventType type,
                             int32_t value, const char* text) {
  const AppEventType internalType = toInternalEvent(type);
  if (!appEventIsExternal(internalType)) {
    return false;
  }
  return appPostEvent(app, internalType, value, text);
}

AppIntegrationSnapshot appIntegrationGetSnapshot(const AppState& app) {
  AppIntegrationSnapshot snapshot;
  snapshot.currentScreen = app.currentScreen;
  snapshot.powerMode = app.powerMode;
  snapshot.wifiFlowState = app.wifiFlowState;
  snapshot.wifiConnected = (app.wifiFlowState == WIFI_FLOW_CONNECTED);
  snapshot.busy = app.uiBusy;
  snapshot.idle = app.idleMode;
  snapshot.keyboardActive = app.keyboardActive;
  snapshot.soundEnabled = app.soundEnabled;
  snapshot.balanceValue = app.balanceValue;
  snapshot.gameStateValue = app.gameStateValue;
  return snapshot;
}

bool appIntegrationIsWifiConnected(const AppState& app) {
  return app.wifiFlowState == WIFI_FLOW_CONNECTED;
}

const char* appIntegrationGetGameStateText(const AppState& app) {
  return app.gameStateText;
}
