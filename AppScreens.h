#ifndef APP_SCREENS_H
#define APP_SCREENS_H

#include <Arduino.h>
#include "SimpleUI.h"

enum ScreenState {
  SCREEN_MENU,
  SCREEN_GAME,
  SCREEN_BALANCE,
  SCREEN_SETTINGS
};

enum SettingsViewState : uint8_t {
  SETTINGS_VIEW_MAIN = 0,
  SETTINGS_VIEW_WIFI,
  SETTINGS_VIEW_TOUCH_DIAGNOSTICS
};

enum ButtonAnimTarget {
  BTN_ANIM_NONE = 0,
  BTN_ANIM_MENU_START,
  BTN_ANIM_MENU_BALANCE,
  BTN_ANIM_MENU_SETTINGS,
  BTN_ANIM_BACK
};

enum PendingAction {
  ACTION_NONE = 0,
  ACTION_GOTO_MENU,
  ACTION_GOTO_GAME,
  ACTION_GOTO_BALANCE,
  ACTION_GOTO_SETTINGS
};

enum DirtyRegion : uint8_t {
  DIRTY_NONE         = 0,
  DIRTY_BALANCE_CARD = 1 << 0,
  DIRTY_WINS_CARD    = 1 << 1,
  DIRTY_WIFI_CARD    = 1 << 2,
  DIRTY_SOUND_CARD   = 1 << 3,
  DIRTY_GAME_AREA    = 1 << 4,
  DIRTY_KEYBOARD_INPUT = 1 << 5,
  DIRTY_TOUCH_DIAG   = 1 << 6
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

  // Redraw control.
  bool fullRedrawRequested;
  uint8_t dirtyRegions;

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

#endif
