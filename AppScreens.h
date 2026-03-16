#ifndef APP_SCREENS_H
#define APP_SCREENS_H

#include <Arduino.h>
#include "SimpleUI.h"

enum ScreenState {
  SCREEN_HOME,
  SCREEN_MENU,
  SCREEN_GAME,
  SCREEN_BALANCE,
  SCREEN_SETTINGS,
  SCREEN_PROFILE,
  SCREEN_TOPUP
};

enum SettingsViewState : uint8_t {
  SETTINGS_VIEW_MAIN = 0,
  SETTINGS_VIEW_WIFI,
  SETTINGS_VIEW_TOUCH_DIAGNOSTICS
};

enum ButtonAnimTarget {
  BTN_ANIM_NONE = 0,
  BTN_ANIM_HOME_PLAY,
  BTN_ANIM_HOME_MENU,
  BTN_ANIM_MENU_PROFILE,
  BTN_ANIM_MENU_TOPUP,
  BTN_ANIM_MENU_SETTINGS,
  BTN_ANIM_MENU_HOME,
  BTN_ANIM_BACK
};

enum PendingAction {
  ACTION_NONE = 0,
  ACTION_GOTO_HOME,
  ACTION_GOTO_MENU,
  ACTION_GOTO_GAME,
  ACTION_GOTO_BALANCE,
  ACTION_GOTO_SETTINGS,
  ACTION_GOTO_PROFILE,
  ACTION_GOTO_TOPUP
};

enum DirtyRegion : uint16_t {
  DIRTY_NONE               = 0,
  DIRTY_BALANCE_CARD       = 1 << 0,
  DIRTY_WINS_CARD          = 1 << 1,
  DIRTY_WIFI_CARD          = 1 << 2,
  DIRTY_SOUND_CARD         = 1 << 3,
  DIRTY_GAME_AREA          = 1 << 4,
  DIRTY_KEYBOARD_INPUT     = 1 << 5,
  DIRTY_TOUCH_DIAG         = 1 << 6,
  DIRTY_WIFI_BADGE         = 1 << 7,
  DIRTY_WIFI_ACTION_BUTTON = 1 << 8,
  DIRTY_NETWORK_PAGE_LABEL = 1 << 9,
  DIRTY_UI_OVERLAY         = 1 << 10,
  DIRTY_HOME_TIME          = 1 << 11
};

enum KeyboardInputTarget : uint8_t {
  INPUT_TARGET_NONE = 0,
  INPUT_TARGET_SSID,
  INPUT_TARGET_PASSWORD
};

enum WifiFlowState : uint8_t {
  WIFI_FLOW_IDLE = 0,
  WIFI_FLOW_DISCONNECTED,
  WIFI_FLOW_EDITING_SSID,
  WIFI_FLOW_EDITING_PASSWORD,
  WIFI_FLOW_READY_TO_CONNECT,
  WIFI_FLOW_SCANNING,
  WIFI_FLOW_NETWORK_LIST,
  WIFI_FLOW_NO_NETWORKS,
  WIFI_FLOW_SCAN_ERROR,
  WIFI_FLOW_RETRY_WAIT,
  WIFI_FLOW_CONNECTING,
  WIFI_FLOW_CONNECTED,
  WIFI_FLOW_ERROR
};

enum AppUiStatusLevel : uint8_t {
  APP_UI_STATUS_INFO = 0,
  APP_UI_STATUS_ERROR,
  APP_UI_STATUS_BUSY
};

enum AppEventType : uint8_t {
  APP_EVENT_NONE = 0,
  APP_EVENT_WIFI_CONNECTED,
  APP_EVENT_WIFI_DISCONNECTED,
  APP_EVENT_BUTTON_MENU,
  APP_EVENT_BUTTON_POWER_SHORT,
  APP_EVENT_BUTTON_POWER_LONG,
  APP_EVENT_BALANCE_UPDATED,
  APP_EVENT_EXTERNAL_MESSAGE,
  APP_EVENT_SHOW_NOTIFICATION,
  APP_EVENT_SHOW_ERROR,
  APP_EVENT_GAME_STATE_CHANGED
};

enum PowerMode : uint8_t {
  POWER_MODE_NORMAL = 0,
  POWER_MODE_SLEEP,
  POWER_MODE_STANDBY
};

struct AppEvent {
  AppEventType type;
  int32_t value;
  char text[33];
};

struct AppState {
  // Top-level app navigation state.
  ScreenState currentScreen;
  ScreenState lastDrawnScreen;

  // Example business data used by existing non-game screens.
  int balanceValue;
  int winsCount;
  bool soundEnabled;

  // Wi-Fi credentials and editor draft buffer.
  char ssid[33];
  char password[65];
  char keyboardDraft[65];

  // Wi-Fi and settings sub-flow state.
  KeyboardInputTarget keyboardTarget;
  WifiFlowState wifiFlowState;
  SettingsViewState settingsView;

  // Live touch info used by diagnostics and input handling.
  int touchScreenX;
  int touchScreenY;
  bool touchPressed;

  // Keyboard and Wi-Fi list transient UI state.
  bool keyboardActive;
  bool keyboardNeedsInit;
  uint8_t networkPage;

  unsigned long lastTouchTime;
  unsigned long lastInteractionMs;

  // Redraw control.
  bool fullRedrawRequested;
  uint16_t dirtyRegions;

  // Button press animation state.
  bool buttonAnimActive;
  bool buttonAnimPressedDrawn;
  ButtonAnimTarget buttonAnimTarget;
  unsigned long buttonAnimStartMs;
  PendingAction pendingAction;

  // Cached values for partial redraw.
  bool balanceCacheValid;
  int lastDrawnBalanceValue;
  int lastDrawnWinsCount;

  bool settingsCacheValid;
  bool lastDrawnSoundEnabled;
  char lastDrawnSsid[33];
  char lastDrawnPasswordMasked[65];
  WifiFlowState lastDrawnWifiFlowState;
  char lastDrawnWifiBadgeText[16];
  uint16_t lastDrawnWifiBadgeColor;
  char lastDrawnWifiActionText[16];
  uint16_t lastDrawnWifiActionColor;
  uint8_t lastDrawnNetworkPage;
  bool networkListCacheValid;
  int lastDrawnGameBalanceValue;
  char lastDrawnHomeTime[6];
  bool homeCacheValid;

  // Lightweight global UI feedback for future game/server integration.
  bool uiStatusVisible;
  AppUiStatusLevel uiStatusLevel;
  char uiStatusText[33];
  unsigned long uiStatusUntilMs;
  bool uiBusy;
  char uiBusyText[17];

  // Fixed-size event queue to avoid dynamic allocation.
  AppEvent eventQueue[6];
  uint8_t eventHead;
  uint8_t eventTail;

  // Runtime diagnostics and energy hooks.
  bool debugMode;
  unsigned long lastDebugPrintMs;
  uint32_t fullRedrawCount;
  uint32_t partialRedrawCount;
  uint32_t idleLoopCount;
  bool idleMode;
  unsigned long idleSinceMs;
  PowerMode powerMode;
  ScreenState balanceReturnScreen;

  bool gameCacheValid;
};

// AppState is the main integration point for future app/game/server state.
// Keep transport services outside this struct and expose only UI-relevant state here.

void initAppState(AppState& app);

// Полная отрисовка текущего экрана
void drawCurrentScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app);

// Частичные обновления
void updateCurrentScreenData(TFT_eSPI& tft, SimpleUI& ui, AppState& app);

// Обработка touch — только изменение состояния и dirty flags
void handleAppTouch(SimpleUI& ui, AppState& app, int tx, int ty);

// Обновление live touch state для diagnostics screen
void updateTouchState(AppState& app, bool touched, int tx, int ty);

// Выполнение запрошенных обновлений экрана
void processUiUpdates(TFT_eSPI& tft, SimpleUI& ui, AppState& app);

// Lightweight UI API for future game/server logic.
void appUiShowMessage(AppState& app, const char* text,
                      uint16_t color = TFT_CYAN, unsigned long durationMs = 2000);
void appUiShowError(AppState& app, const char* text, unsigned long durationMs = 2500);
void appUiSetBusy(AppState& app, bool busy, const char* text = "BUSY");
void appUiSwitchScreen(AppState& app, ScreenState screen);

// Lightweight event queue for future integration.
bool appPostEvent(AppState& app, AppEventType type, int32_t value = 0, const char* text = nullptr);
bool appConsumeEvent(AppState& app, AppEvent& out);

// Runtime diagnostics / idle helpers.
void appSetDebugMode(AppState& app, bool enabled);
bool appIsDebugMode(const AppState& app);
bool appIsIdleMode(const AppState& app);
bool appIsSoftStandby(const AppState& app);
bool appIsSleeping(const AppState& app);
uint8_t appLoopDelayMs(const AppState& app);

#endif
