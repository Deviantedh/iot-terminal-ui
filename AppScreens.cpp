#include "AppScreens.h"
#include "OnScreenKeyboard.h"
#include "WiFiService.h"
#include "WiFiProfiles.h"
#include "TimeService.h"
#include <string.h>

#ifndef APP_SERIAL_LOGGING_ENABLED
#define APP_SERIAL_LOGGING_ENABLED 0
#endif

static constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

static const unsigned long TOUCH_DEBOUNCE = 180;
static const unsigned long BUTTON_ANIM_MS = 80;
static const unsigned long PROFILE_PRINT_INTERVAL_MS = 1000;
static const unsigned long WIFI_RECONNECT_INTERVAL_MS = 5000;
static const unsigned long WIFI_RECONNECT_POLL_MS = 250;
static const unsigned long WIFI_RECONNECT_POLL_IDLE_MS = 1000;
static const unsigned long UI_STATUS_DEFAULT_MS = 2000;
static const unsigned long IDLE_THRESHOLD_MS = 15000;
static const unsigned long DEBUG_PRINT_INTERVAL_MS = 5000;
static const unsigned long DEBUG_PRINT_INTERVAL_IDLE_MS = 15000;

static const int UI_OVERLAY_X = 116;
static const int UI_OVERLAY_Y = 220;
static const int UI_OVERLAY_W = 196;
static const int UI_OVERLAY_H = 16;
static const int IDLE_DIM_X = 0;
static const int IDLE_DIM_Y = 220;
static const int IDLE_DIM_W = 112;
static const int IDLE_DIM_H = 16;

// ---------------------------
// Visual palette
// ---------------------------
static const uint16_t UI_BG = rgb565(15, 18, 24);
static const uint16_t UI_PANEL = rgb565(28, 34, 43);
static const uint16_t UI_PANEL_ALT = rgb565(34, 41, 52);
static const uint16_t UI_BORDER = rgb565(92, 102, 116);
static const uint16_t UI_BORDER_SOFT = rgb565(56, 64, 76);
static const uint16_t UI_TEXT = rgb565(236, 238, 242);
static const uint16_t UI_TEXT_MUTED = rgb565(168, 177, 189);
static const uint16_t UI_ACCENT = rgb565(78, 178, 190);
static const uint16_t UI_ACCENT_PRESSED = rgb565(60, 156, 168);
static const uint16_t UI_SECONDARY = rgb565(109, 132, 170);
static const uint16_t UI_SECONDARY_PRESSED = rgb565(90, 112, 148);
static const uint16_t UI_SUCCESS = rgb565(118, 186, 138);
static const uint16_t UI_SUCCESS_PRESSED = rgb565(96, 164, 118);
static const uint16_t UI_WARNING = rgb565(220, 174, 92);
static const uint16_t UI_WARNING_PRESSED = rgb565(198, 152, 74);
static const uint16_t UI_ERROR = rgb565(210, 116, 122);
static const uint16_t UI_ERROR_PRESSED = rgb565(188, 96, 102);
static const uint16_t UI_INFO = rgb565(130, 170, 204);
static const uint16_t UI_INFO_SOFT = rgb565(74, 92, 122);
static const uint16_t UI_NEUTRAL = rgb565(72, 82, 96);
static const uint16_t UI_NEUTRAL_PRESSED = rgb565(92, 102, 118);
static const uint16_t UI_IDLE = rgb565(66, 72, 84);

// ---------------------------
// Layout
// ---------------------------
static const int SCREEN_W = 320;
static const int SCREEN_H = 240;
static const int TOUCH_SAFE_MARGIN_X = 24;
static const int TOUCH_SAFE_MARGIN_Y = 14;
static const int TOUCH_EDGE_BOOST_X = 10;
static const int TOUCH_EDGE_BOOST_Y = 8;

static const int CARD_X = 25;
static const int CARD_W = 270;
static const int CARD_H = 44;

static const int BACK_W = 72;
static const int BACK_H = 28;
static const int BACK_X = 12;
static const int BACK_Y = 4;

static const int SETTINGS_MAIN_WIFI_X = 28;
static const int SETTINGS_MAIN_WIFI_Y = 46;
static const int SETTINGS_MAIN_WIFI_W = 264;
static const int SETTINGS_MAIN_WIFI_H = 36;

static const int SETTINGS_MAIN_TOUCH_X = 28;
static const int SETTINGS_MAIN_TOUCH_Y = 88;
static const int SETTINGS_MAIN_TOUCH_W = 264;
static const int SETTINGS_MAIN_TOUCH_H = 36;

static const int SETTINGS_MAIN_TESTS_X = 28;
static const int SETTINGS_MAIN_TESTS_Y = 130;
static const int SETTINGS_MAIN_TESTS_W = 264;
static const int SETTINGS_MAIN_TESTS_H = 36;

static const int SETTINGS_MAIN_SOUND_X = 28;
static const int SETTINGS_MAIN_SOUND_Y = 172;
static const int SETTINGS_MAIN_SOUND_W = 264;
static const int SETTINGS_MAIN_SOUND_H = 36;

static const int SETTINGS_MAIN_BACK_X = 28;
static const int SETTINGS_MAIN_BACK_Y = 214;
static const int SETTINGS_MAIN_BACK_W = 264;
static const int SETTINGS_MAIN_BACK_H = 22;

static const int WIFI_FIELD_X = 28;
static const int WIFI_FIELD_W = 264;
static const int WIFI_FIELD_H = 40;
static const int WIFI_SSID_Y = 46;
static const int WIFI_PASS_Y = 94;

static const int WIFI_SCAN_X = 28;
static const int WIFI_SCAN_Y = 152;
static const int WIFI_SCAN_W = 122;
static const int WIFI_SCAN_H = 34;

static const int WIFI_CONNECT_X = 170;
static const int WIFI_CONNECT_Y = 152;
static const int WIFI_CONNECT_W = 122;
static const int WIFI_CONNECT_H = 34;

static const int WIFI_BACK_X = 28;
static const int WIFI_BACK_Y = 194;
static const int WIFI_BACK_W = 122;
static const int WIFI_BACK_H = 34;

static const int WIFI_STATUS_X = 170;
static const int WIFI_STATUS_Y = 194;
static const int WIFI_STATUS_W = 122;
static const int WIFI_STATUS_H = 34;

static const int LIST_X = 28;
static const int LIST_Y = 44;
static const int LIST_W = 264;
static const int LIST_ROW_H = 30;
static const int LIST_ROW_GAP = 4;
static const int LIST_PAGE_SIZE = 5;

static const int LIST_PREV_X = 28;
static const int LIST_PREV_Y = 190;
static const int LIST_PREV_W = 72;
static const int LIST_PREV_H = 36;

static const int LIST_NEXT_X = 114;
static const int LIST_NEXT_Y = 190;
static const int LIST_NEXT_W = 72;
static const int LIST_NEXT_H = 36;

static const int LIST_BACK_X = 200;
static const int LIST_BACK_Y = 190;
static const int LIST_BACK_W = 92;
static const int LIST_BACK_H = 36;

static const int TOUCH_DIAG_INFO_X = 28;
static const int TOUCH_DIAG_INFO_Y = 46;
static const int TOUCH_DIAG_INFO_W = 264;
static const int TOUCH_DIAG_INFO_H = 42;

static const int TOUCH_DIAG_TARGET_W = 64;
static const int TOUCH_DIAG_TARGET_H = 40;
static const int TOUCH_DIAG_TL_X = 34;
static const int TOUCH_DIAG_TR_X = SCREEN_W - 34 - TOUCH_DIAG_TARGET_W;
static const int TOUCH_DIAG_TOP_Y = 102;
static const int TOUCH_DIAG_BOTTOM_Y = 156;
static const int TOUCH_DIAG_CENTER_X = 108;
static const int TOUCH_DIAG_CENTER_Y = 129;
static const int TOUCH_DIAG_CENTER_W = 104;
static const int TOUCH_DIAG_CENTER_H = 44;
static const int TOUCH_DIAG_BACK_X = 92;
static const int TOUCH_DIAG_BACK_Y = 198;
static const int TOUCH_DIAG_BACK_W = 136;
static const int TOUCH_DIAG_BACK_H = 30;

static const int TESTS_PLAY_X = 44;
static const int TESTS_PLAY_Y = 102;
static const int TESTS_PLAY_W = 232;
static const int TESTS_PLAY_H = 42;

static const int HOME_TIME_Y = 56;
static const int HOME_SUBTITLE_Y = 102;
static const int HOME_PLAY_X = 18;
static const int HOME_PLAY_Y = 182;
static const int HOME_PLAY_W = 132;
static const int HOME_PLAY_H = 34;
static const int HOME_MENU_X = 170;
static const int HOME_MENU_Y = 182;
static const int HOME_MENU_W = 132;
static const int HOME_MENU_H = 34;
static const int HOME_TIME_X = 44;
static const int HOME_TIME_W = 232;
static const int HOME_TIME_H = 54;
static const int HOME_BG_X = 18;
static const int HOME_BG_Y = 18;
static const int HOME_BG_W = 284;
static const int HOME_BG_H = 136;

static const int MENU_BACK_X = 12;
static const int MENU_BACK_Y = 4;
static const int MENU_BACK_W = 72;
static const int MENU_BACK_H = 28;

static const int GAME_BALANCE_X = 210;
static const int GAME_BALANCE_Y = 4;
static const int GAME_BALANCE_W = 98;
static const int GAME_BALANCE_H = 28;

// ---------------------------
// Buttons
// ---------------------------
static UIButton btnHomePlay = {HOME_PLAY_X, HOME_PLAY_Y, HOME_PLAY_W, HOME_PLAY_H, UI_ACCENT, UI_ACCENT_PRESSED, "PLAY"};
static UIButton btnHomeMenu = {HOME_MENU_X, HOME_MENU_Y, HOME_MENU_W, HOME_MENU_H, UI_SECONDARY, UI_SECONDARY_PRESSED, "MENU"};
static UIButton btnMenuProfile = {42,  58, 236, 42, UI_PANEL_ALT, UI_SECONDARY_PRESSED, "PROFILE"};
static UIButton btnMenuTopUp   = {42, 112, 236, 42, UI_PANEL_ALT, UI_SECONDARY_PRESSED, "TOP UP"};
static UIButton btnMenuSettings = {42, 166, 236, 42, UI_PANEL_ALT, UI_SECONDARY_PRESSED, "SETTINGS"};
static UIButton btnMenuHome = {MENU_BACK_X, MENU_BACK_Y, MENU_BACK_W, MENU_BACK_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "HOME"};
static UIButton btnBack     = {BACK_X, BACK_Y, BACK_W, BACK_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "BACK"};
static UIButton btnSettingsMainBack = {SETTINGS_MAIN_BACK_X, SETTINGS_MAIN_BACK_Y, SETTINGS_MAIN_BACK_W, SETTINGS_MAIN_BACK_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "BACK"};
static UIButton btnSettingsTouchDiag = {SETTINGS_MAIN_TOUCH_X, SETTINGS_MAIN_TOUCH_Y, SETTINGS_MAIN_TOUCH_W, SETTINGS_MAIN_TOUCH_H, UI_PANEL_ALT, UI_SECONDARY_PRESSED, "TOUCH DIAGNOSTICS"};
static UIButton btnSettingsTests = {SETTINGS_MAIN_TESTS_X, SETTINGS_MAIN_TESTS_Y, SETTINGS_MAIN_TESTS_W, SETTINGS_MAIN_TESTS_H, UI_PANEL_ALT, UI_SECONDARY_PRESSED, "TESTS"};
static UIButton btnWifiScan = {WIFI_SCAN_X, WIFI_SCAN_Y, WIFI_SCAN_W, WIFI_SCAN_H, UI_PANEL_ALT, UI_SECONDARY_PRESSED, "SCAN NETWORKS"};
static UIButton btnWifiConnect = {WIFI_CONNECT_X, WIFI_CONNECT_Y, WIFI_CONNECT_W, WIFI_CONNECT_H, UI_SUCCESS, UI_SUCCESS_PRESSED, "CONNECT"};
static UIButton btnWifiBack = {WIFI_BACK_X, WIFI_BACK_Y, WIFI_BACK_W, WIFI_BACK_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "BACK"};
static UIButton btnListPrev = {LIST_PREV_X, LIST_PREV_Y, LIST_PREV_W, LIST_PREV_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "PREV"};
static UIButton btnListNext = {LIST_NEXT_X, LIST_NEXT_Y, LIST_NEXT_W, LIST_NEXT_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "NEXT"};
static UIButton btnListBack = {LIST_BACK_X, LIST_BACK_Y, LIST_BACK_W, LIST_BACK_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "BACK"};
static UIButton btnTouchDiagBack = {TOUCH_DIAG_BACK_X, TOUCH_DIAG_BACK_Y, TOUCH_DIAG_BACK_W, TOUCH_DIAG_BACK_H, UI_NEUTRAL, UI_NEUTRAL_PRESSED, "BACK"};
static UIButton btnTestsPlaySong = {TESTS_PLAY_X, TESTS_PLAY_Y, TESTS_PLAY_W, TESTS_PLAY_H, UI_ACCENT, UI_ACCENT_PRESSED, "PLAY TEST SONG"};

static OnScreenKeyboard* keyboard = nullptr;
static WiFiService wifiService;
static WiFiProfiles wifiProfiles;
static TimeService timeService;
static bool wifiServiceInit = false;
static bool wifiProfilesInit = false;
static bool startupAutoConnectDone = false;
static bool startupSavedAttempted = false;
static bool startupBuiltInScanStarted = false;
static bool startupBuiltInAttempted = false;
static bool wifiAutoReconnectEnabled = false;
static bool wifiReconnectPending = false;
static unsigned long wifiReconnectDueMs = 0;
static unsigned long wifiReconnectNextPollMs = 0;

static void fitTextWithEllipsis(TFT_eSPI& tft, const char* src, char* out, size_t outSize, int maxWidthPx);
static void restoreWifiFormState(AppState& app);
static void markDirtyRegions(AppState& app, uint16_t regions);
static void noteInteraction(AppState& app);
static bool appHasActiveWork(const AppState& app);
static void updateIdleState(AppState& app, unsigned long nowMs);
static void syncWifiPowerSave(const AppState& app);
static unsigned long debugPrintIntervalMs(const AppState& app);

static void configureWifiPrimaryButton(const AppState& app) {
  if (app.wifiFlowState == WIFI_FLOW_CONNECTED) {
    btnWifiConnect.color = UI_ERROR;
    btnWifiConnect.pressedColor = UI_ERROR_PRESSED;
    btnWifiConnect.text = "DISCONNECT";
    return;
  }

  if (app.wifiFlowState == WIFI_FLOW_ERROR || app.wifiFlowState == WIFI_FLOW_RETRY_WAIT) {
    btnWifiConnect.color = UI_WARNING;
    btnWifiConnect.pressedColor = UI_WARNING_PRESSED;
    btnWifiConnect.text = "RETRY";
    return;
  }

  if (app.wifiFlowState == WIFI_FLOW_CONNECTING) {
    btnWifiConnect.color = UI_NEUTRAL;
    btnWifiConnect.pressedColor = UI_NEUTRAL_PRESSED;
    btnWifiConnect.text = "WAIT";
    return;
  }

  btnWifiConnect.color = UI_SUCCESS;
  btnWifiConnect.pressedColor = UI_SUCCESS_PRESSED;
  btnWifiConnect.text = "CONNECT";
}

static bool startWifiConnect(AppState& app) {
  if (app.ssid[0] == '\0') {
    app.wifiFlowState = WIFI_FLOW_ERROR;
    wifiReconnectPending = false;
    markDirtyRegions(app, DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON);
    return false;
  }

  if (!wifiServiceInit) {
    wifiService.begin();
    wifiServiceInit = true;
  }

  wifiReconnectPending = false;
  wifiReconnectNextPollMs = 0;
  bool started = wifiService.beginConnect(app.ssid, app.password);
  app.wifiFlowState = started ? WIFI_FLOW_CONNECTING : WIFI_FLOW_ERROR;
  markDirtyRegions(app, DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON);
  return started;
}

static void armWifiReconnect(AppState& app) {
  if (app.ssid[0] == '\0' || !wifiAutoReconnectEnabled) {
    app.wifiFlowState = WIFI_FLOW_ERROR;
    wifiReconnectPending = false;
    return;
  }

  wifiReconnectPending = true;
  wifiReconnectDueMs = millis() + WIFI_RECONNECT_INTERVAL_MS;
  wifiReconnectNextPollMs = 0;
  app.wifiFlowState = WIFI_FLOW_RETRY_WAIT;
  markDirtyRegions(app, DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON);
}

static bool initWiFiProfilesIfNeeded() {
  if (wifiProfilesInit) {
    return true;
  }

  wifiProfilesInit = wifiProfiles.begin();
  return wifiProfilesInit;
}

static bool edgeAwareRectHit(SimpleUI& ui, int tx, int ty,
                             int x, int y, int w, int h,
                             int padX, int padY) {
  int left = x - padX;
  int top = y - padY;
  int width = w + padX * 2;
  int height = h + padY * 2;

  if (x <= TOUCH_SAFE_MARGIN_X + 4) {
    left -= TOUCH_EDGE_BOOST_X;
    width += TOUCH_EDGE_BOOST_X;
  }
  if (x + w >= SCREEN_W - TOUCH_SAFE_MARGIN_X - 4) {
    width += TOUCH_EDGE_BOOST_X;
  }
  if (y <= TOUCH_SAFE_MARGIN_Y + 4) {
    top -= TOUCH_EDGE_BOOST_Y;
    height += TOUCH_EDGE_BOOST_Y;
  }
  if (y + h >= SCREEN_H - TOUCH_SAFE_MARGIN_Y - 4) {
    height += TOUCH_EDGE_BOOST_Y;
  }

  return ui.inRect(tx, ty, left, top, width, height);
}

static bool edgeAwareButtonHit(SimpleUI& ui, const UIButton& btn, int tx, int ty, int padX, int padY) {
  return edgeAwareRectHit(ui, tx, ty, btn.x, btn.y, btn.w, btn.h, padX, padY);
}

static bool edgeAwareCardHit(SimpleUI& ui, int tx, int ty, int x, int y, int w, int h) {
  return edgeAwareRectHit(ui, tx, ty, x, y, w, h, 10, 8);
}

static void invalidateDynamicCaches(AppState& app) {
  app.balanceCacheValid = false;
  app.settingsCacheValid = false;
  app.networkListCacheValid = false;
  app.gameCacheValid = false;
}

static void markDirtyRegions(AppState& app, uint16_t regions) {
  app.dirtyRegions |= regions;
}

static uint16_t consumeDirtyRegions(AppState& app) {
  uint16_t regions = app.dirtyRegions;
  app.dirtyRegions = DIRTY_NONE;
  return regions;
}

static void noteInteraction(AppState& app) {
  const bool wasIdle = app.idleMode;
  app.lastInteractionMs = millis();
  if (wasIdle) {
    app.idleMode = false;
    app.idleSinceMs = 0;
    markDirtyRegions(app, DIRTY_UI_OVERLAY);
  }
}

static bool appHasActiveWork(const AppState& app) {
  if (app.powerMode != POWER_MODE_NORMAL) {
    return false;
  }
  if (app.keyboardActive) {
    return true;
  }

  switch (app.wifiFlowState) {
    case WIFI_FLOW_CONNECTING:
    case WIFI_FLOW_SCANNING:
    case WIFI_FLOW_RETRY_WAIT:
      return true;
    default:
      break;
  }

  return app.uiBusy;
}

static void updateIdleState(AppState& app, unsigned long nowMs) {
  if (app.powerMode != POWER_MODE_NORMAL) {
    if (!app.idleMode) {
      app.idleMode = true;
      app.idleSinceMs = nowMs;
      markDirtyRegions(app, DIRTY_UI_OVERLAY);
    }
    return;
  }

  const bool shouldBeIdle = !appHasActiveWork(app) &&
                            (long)(nowMs - app.lastInteractionMs) >= (long)IDLE_THRESHOLD_MS;
  if (shouldBeIdle == app.idleMode) {
    return;
  }

  app.idleMode = shouldBeIdle;
  app.idleSinceMs = shouldBeIdle ? nowMs : 0;
  markDirtyRegions(app, DIRTY_UI_OVERLAY);
}

static const char* screenStateName(ScreenState screen) {
  switch (screen) {
    case SCREEN_HOME: return "HOME";
    case SCREEN_MENU: return "MENU";
    case SCREEN_GAME: return "GAME";
    case SCREEN_BALANCE: return "BALANCE";
    case SCREEN_SETTINGS: return "SETTINGS";
    case SCREEN_TESTS: return "TESTS";
    case SCREEN_PROFILE: return "PROFILE";
    case SCREEN_TOPUP: return "TOPUP";
  }
  return "HOME";
}

static const char* settingsViewName(SettingsViewState view) {
  switch (view) {
    case SETTINGS_VIEW_MAIN: return "MAIN";
    case SETTINGS_VIEW_WIFI: return "WIFI";
    case SETTINGS_VIEW_TOUCH_DIAGNOSTICS: return "TOUCH";
    case SETTINGS_VIEW_TESTS: return "TESTS";
  }
  return "MAIN";
}

static uint16_t uiStatusLevelColor(AppUiStatusLevel level) {
  switch (level) {
    case APP_UI_STATUS_INFO: return UI_INFO;
    case APP_UI_STATUS_ERROR: return UI_ERROR;
    case APP_UI_STATUS_BUSY: return UI_WARNING;
  }
  return UI_INFO;
}

static const char* activeUiOverlayText(const AppState& app) {
  if (app.powerMode == POWER_MODE_STANDBY) {
    return "OFF";
  }
  if (app.powerMode == POWER_MODE_SLEEP) {
    return "SLEEP";
  }
  if (app.uiBusy && app.uiBusyText[0] != '\0') {
    return app.uiBusyText;
  }
  if (app.uiStatusVisible && app.uiStatusText[0] != '\0') {
    return app.uiStatusText;
  }
  return nullptr;
}

static uint16_t activeUiOverlayColor(const AppState& app) {
  if (app.powerMode == POWER_MODE_STANDBY) {
    return UI_WARNING;
  }
  if (app.powerMode == POWER_MODE_SLEEP) {
    return UI_TEXT_MUTED;
  }
  if (app.uiBusy) {
    return uiStatusLevelColor(APP_UI_STATUS_BUSY);
  }
  if (app.uiStatusVisible) {
    return uiStatusLevelColor(app.uiStatusLevel);
  }
  return UI_BG;
}

static void drawUiOverlay(TFT_eSPI& tft, const AppState& app) {
  tft.fillRect(IDLE_DIM_X, IDLE_DIM_Y, IDLE_DIM_W, IDLE_DIM_H, UI_BG);

  if (app.idleMode) {
    // Cheap idle indicator instead of a full-screen dimming pass.
    tft.fillRect(IDLE_DIM_X, IDLE_DIM_Y, IDLE_DIM_W, IDLE_DIM_H, UI_IDLE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(UI_TEXT_MUTED, UI_IDLE);
    tft.drawString("IDLE", IDLE_DIM_X + IDLE_DIM_W / 2, IDLE_DIM_Y + IDLE_DIM_H / 2, 2);
    tft.setTextDatum(TL_DATUM);
  }

  const char* text = activeUiOverlayText(app);
  if (text == nullptr || text[0] == '\0') {
    tft.fillRect(UI_OVERLAY_X, UI_OVERLAY_Y, UI_OVERLAY_W, UI_OVERLAY_H, UI_BG);
    return;
  }

  // The old top-right reserved box was the source of the visible rectangle artifact.
  // Status messages now live in a bottom strip so full-screen layouts keep their own backgrounds.
  tft.fillRect(UI_OVERLAY_X, UI_OVERLAY_Y, UI_OVERLAY_W, UI_OVERLAY_H, UI_BG);
  char overlayText[20];
  fitTextWithEllipsis(tft, text, overlayText, sizeof(overlayText), UI_OVERLAY_W - 8);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(activeUiOverlayColor(app), UI_BG);
  tft.setTextSize(1);
  tft.drawString(overlayText, UI_OVERLAY_X + 6, UI_OVERLAY_Y + UI_OVERLAY_H / 2, 2);
  tft.setTextDatum(TL_DATUM);
}

static void syncWifiPowerSave(const AppState& app) {
  const bool allowPowerSave =
    app.idleMode &&
    app.powerMode == POWER_MODE_NORMAL &&
    !app.keyboardActive &&
    wifiService.state() == WIFI_SERVICE_CONNECTED &&
    app.wifiFlowState == WIFI_FLOW_CONNECTED;
  wifiService.setPowerSaveEnabled(allowPowerSave);
}

static unsigned long debugPrintIntervalMs(const AppState& app) {
  return app.idleMode ? DEBUG_PRINT_INTERVAL_IDLE_MS : DEBUG_PRINT_INTERVAL_MS;
}

static OnScreenKeyboard& getKeyboard(TFT_eSPI& tft, SimpleUI& ui) {
  if (keyboard == nullptr) {
    static OnScreenKeyboard keyboardInstance(tft, ui);
    keyboard = &keyboardInstance;
  }
  return *keyboard;
}

static bool findMatchingBuiltInNetwork(WiFiCredentials& out) {
  if (!wifiProfiles.hasBuiltInNetworks()) {
    return false;
  }

  const int total = wifiService.networkCount();
  for (int builtInIndex = 0; builtInIndex < wifiProfiles.builtInCount(); builtInIndex++) {
    WiFiCredentials candidate;
    if (!wifiProfiles.builtInAt(builtInIndex, candidate)) {
      continue;
    }

    for (int i = 0; i < total; i++) {
      const WiFiNetworkInfo* n = wifiService.networkAt(i);
      if (n == nullptr) {
        continue;
      }
      if (strcmp(candidate.ssid, n->ssid) == 0) {
        out = candidate;
        return true;
      }
    }
  }

  return false;
}

static void resetStartupAutoConnectState() {
  startupAutoConnectDone = false;
  startupSavedAttempted = false;
  startupBuiltInScanStarted = false;
  startupBuiltInAttempted = false;
  wifiAutoReconnectEnabled = false;
  wifiReconnectPending = false;
  wifiReconnectDueMs = 0;
  wifiReconnectNextPollMs = 0;
}

static void beginStartupBuiltInScan(AppState& app) {
  if (!wifiProfiles.hasBuiltInNetworks() || startupBuiltInScanStarted) {
    startupAutoConnectDone = true;
    restoreWifiFormState(app);
    return;
  }

  if (wifiService.startScan()) {
    startupBuiltInScanStarted = true;
    app.wifiFlowState = WIFI_FLOW_SCANNING;
    markDirtyRegions(app, DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON);
  } else {
    startupAutoConnectDone = true;
    app.wifiFlowState = WIFI_FLOW_ERROR;
    markDirtyRegions(app, DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON);
  }
}

static bool isWifiOverlayState(const AppState& app) {
  return app.wifiFlowState == WIFI_FLOW_SCANNING ||
         app.wifiFlowState == WIFI_FLOW_NETWORK_LIST ||
         app.wifiFlowState == WIFI_FLOW_NO_NETWORKS ||
         app.wifiFlowState == WIFI_FLOW_SCAN_ERROR;
}

static void restoreWifiFormState(AppState& app) {
  if (wifiService.state() == WIFI_SERVICE_CONNECTED) {
    app.wifiFlowState = WIFI_FLOW_CONNECTED;
    return;
  }
  if (wifiService.state() == WIFI_SERVICE_CONNECTING) {
    app.wifiFlowState = WIFI_FLOW_CONNECTING;
    return;
  }
  app.wifiFlowState = (app.ssid[0] != '\0') ? WIFI_FLOW_READY_TO_CONNECT : WIFI_FLOW_IDLE;
}

static const char* getSettingsWifiSummary(const AppState& app) {
  if (app.ssid[0] != '\0') {
    return app.ssid;
  }

  if (app.wifiFlowState == WIFI_FLOW_CONNECTED) {
    return "Connected";
  }

  return "Tap to configure";
}

static const char* getWifiStatusText(const AppState& app) {
  switch (app.wifiFlowState) {
    case WIFI_FLOW_IDLE:
      return "IDLE";
    case WIFI_FLOW_DISCONNECTED:
      return "DISCONNECTED";
    case WIFI_FLOW_EDITING_SSID:
      return "EDIT SSID";
    case WIFI_FLOW_EDITING_PASSWORD:
      return "EDIT PASSWORD";
    case WIFI_FLOW_READY_TO_CONNECT:
      return "READY";
    case WIFI_FLOW_SCANNING:
      return "SCANNING";
    case WIFI_FLOW_NETWORK_LIST:
      return "LIST";
    case WIFI_FLOW_NO_NETWORKS:
      return "NO NETWORKS";
    case WIFI_FLOW_SCAN_ERROR:
      return "SCAN ERROR";
    case WIFI_FLOW_RETRY_WAIT:
      return "RETRY WAIT";
    case WIFI_FLOW_CONNECTING:
      return "CONNECTING";
    case WIFI_FLOW_CONNECTED:
      return "CONNECTED";
    case WIFI_FLOW_ERROR:
      return "ERROR";
  }
  return "IDLE";
}

static uint16_t getWifiStatusColor(const AppState& app) {
  switch (app.wifiFlowState) {
    case WIFI_FLOW_IDLE:
      return UI_NEUTRAL;
    case WIFI_FLOW_DISCONNECTED:
      return UI_NEUTRAL;
    case WIFI_FLOW_EDITING_SSID:
    case WIFI_FLOW_EDITING_PASSWORD:
      return UI_INFO_SOFT;
    case WIFI_FLOW_READY_TO_CONNECT:
      return UI_WARNING;
    case WIFI_FLOW_SCANNING:
      return UI_ACCENT;
    case WIFI_FLOW_NETWORK_LIST:
      return UI_INFO_SOFT;
    case WIFI_FLOW_NO_NETWORKS:
      return UI_NEUTRAL;
    case WIFI_FLOW_SCAN_ERROR:
      return UI_ERROR;
    case WIFI_FLOW_RETRY_WAIT:
      return UI_WARNING;
    case WIFI_FLOW_CONNECTING:
      return UI_ACCENT;
    case WIFI_FLOW_CONNECTED:
      return UI_SUCCESS;
    case WIFI_FLOW_ERROR:
      return UI_ERROR;
  }
  return UI_NEUTRAL;
}

static int networkPageCount() {
  int count = wifiService.networkCount();
  if (count <= 0) {
    return 1;
  }
  return (count + LIST_PAGE_SIZE - 1) / LIST_PAGE_SIZE;
}

static void drawNetworkPageLabel(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  const int pages = networkPageCount();
  if (app.networkListCacheValid && app.lastDrawnNetworkPage == app.networkPage) {
    return;
  }
  tft.fillRect(106, 170, 108, 14, UI_BG);
  char pageTxt[24];
  snprintf(pageTxt, sizeof(pageTxt), "Page %d/%d", app.networkPage + 1, pages);
  ui.drawCenteredText(pageTxt, 160, 178, UI_TEXT_MUTED, UI_BG, 1);
  app.lastDrawnNetworkPage = app.networkPage;
}

static void drawNetworkListBody(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool drawPageLabel) {
  tft.fillRect(0, LIST_Y, SCREEN_W, 144, UI_BG);
  ui.drawHeaderBar("SELECT NETWORK", UI_ACCENT);

  int total = wifiService.networkCount();
  int pages = networkPageCount();
  if (app.networkPage >= (uint8_t)pages) {
    app.networkPage = pages - 1;
  }

  int start = app.networkPage * LIST_PAGE_SIZE;
  for (int i = 0; i < LIST_PAGE_SIZE; i++) {
    int idx = start + i;
    int y = LIST_Y + i * (LIST_ROW_H + LIST_ROW_GAP);

    if (idx >= total) {
      tft.drawRect(LIST_X, y, LIST_W, LIST_ROW_H, UI_BORDER_SOFT);
      continue;
    }

    const WiFiNetworkInfo* n = wifiService.networkAt(idx);
    if (n == nullptr) {
      continue;
    }

    tft.fillRect(LIST_X, y, LIST_W, LIST_ROW_H, UI_PANEL);
    tft.drawRect(LIST_X, y, LIST_W, LIST_ROW_H, UI_BORDER);

    char rssiText[16];
    snprintf(rssiText, sizeof(rssiText), "%ddBm", (int)n->rssi);

    const int rowMidY = y + LIST_ROW_H / 2;
    const int leftPad = 10;
    const int rightPad = 10;
    const int secureMarkWidth = n->secured ? tft.textWidth("*", 2) + 6 : 0;
    const int rssiWidth = tft.textWidth(rssiText, 2);
    const int ssidMaxWidth = LIST_W - leftPad - rightPad - secureMarkWidth - rssiWidth - 12;

    char ssidText[33];
    fitTextWithEllipsis(tft, n->ssid, ssidText, sizeof(ssidText), ssidMaxWidth);

    tft.setTextSize(1);
    tft.setTextDatum(ML_DATUM);

    int textX = LIST_X + leftPad;
    if (n->secured) {
      tft.setTextColor(UI_ACCENT, UI_PANEL);
      tft.drawString("*", textX, rowMidY, 2);
      textX += secureMarkWidth;
    }

    tft.setTextColor(UI_TEXT, UI_PANEL);
    tft.drawString(ssidText, textX, rowMidY, 2);

    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(UI_TEXT_MUTED, UI_PANEL);
    tft.drawString(rssiText, LIST_X + LIST_W - rightPad, rowMidY, 2);
    tft.setTextDatum(TL_DATUM);
  }

  if (drawPageLabel) {
    drawNetworkPageLabel(tft, ui, app);
  }
  app.networkListCacheValid = true;
}

static bool handleNetworkListTap(SimpleUI& ui, AppState& app, int tx, int ty) {
  if (edgeAwareButtonHit(ui, btnListBack, tx, ty, 8, 8)) {
    app.settingsView = SETTINGS_VIEW_WIFI;
    restoreWifiFormState(app);
    app.fullRedrawRequested = true;
    return true;
  }

  int pages = networkPageCount();
  if (edgeAwareButtonHit(ui, btnListPrev, tx, ty, 8, 8) && app.networkPage > 0) {
    app.networkPage--;
    markDirtyRegions(app, DIRTY_WIFI_CARD | DIRTY_NETWORK_PAGE_LABEL);
    return true;
  }
  if (edgeAwareButtonHit(ui, btnListNext, tx, ty, 8, 8) && app.networkPage + 1 < pages) {
    app.networkPage++;
    markDirtyRegions(app, DIRTY_WIFI_CARD | DIRTY_NETWORK_PAGE_LABEL);
    return true;
  }

  int start = app.networkPage * LIST_PAGE_SIZE;
  for (int i = 0; i < LIST_PAGE_SIZE; i++) {
    int y = LIST_Y + i * (LIST_ROW_H + LIST_ROW_GAP);
    if (!edgeAwareRectHit(ui, tx, ty, LIST_X, y, LIST_W, LIST_ROW_H, 4, 5)) {
      continue;
    }

    const WiFiNetworkInfo* n = wifiService.networkAt(start + i);
    if (n == nullptr || n->ssid[0] == '\0') {
      return true;
    }

    strncpy(app.ssid, n->ssid, sizeof(app.ssid) - 1);
    app.ssid[sizeof(app.ssid) - 1] = '\0';
    app.wifiFlowState = WIFI_FLOW_READY_TO_CONNECT;
    app.settingsView = SETTINGS_VIEW_WIFI;
    app.fullRedrawRequested = true;
    markDirtyRegions(app, DIRTY_WIFI_CARD);
    return true;
  }

  return false;
}

static void maskPassword(const char* password, char* out, size_t outSize) {
  if (outSize == 0) {
    return;
  }
  size_t len = strlen(password);
  if (len >= outSize) {
    len = outSize - 1;
  }
  for (size_t i = 0; i < len; i++) {
    out[i] = '*';
  }
  out[len] = '\0';
}

static void fitTextWithEllipsis(TFT_eSPI& tft, const char* src, char* out, size_t outSize, int maxWidthPx) {
  if (outSize == 0) {
    return;
  }

  out[0] = '\0';
  if (src == nullptr || src[0] == '\0') {
    return;
  }

  strncpy(out, src, outSize - 1);
  out[outSize - 1] = '\0';
  if (tft.textWidth(out, 2) <= maxWidthPx) {
    return;
  }

  const char* ellipsis = "...";
  const int ellipsisWidth = tft.textWidth(ellipsis, 2);
  size_t len = strlen(out);
  while (len > 0) {
    out[--len] = '\0';
    if (tft.textWidth(out, 2) + ellipsisWidth <= maxWidthPx) {
      strncat(out, ellipsis, outSize - strlen(out) - 1);
      return;
    }
  }

  strncpy(out, ellipsis, outSize - 1);
  out[outSize - 1] = '\0';
}

static void queueScreenAction(AppState& app, PendingAction action, ButtonAnimTarget target) {
  app.buttonAnimActive = true;
  app.buttonAnimPressedDrawn = false;
  app.buttonAnimTarget = target;
  app.buttonAnimStartMs = millis();
  app.pendingAction = action;
}

static void applyPendingAction(AppState& app) {
  switch (app.pendingAction) {
    case ACTION_GOTO_HOME:
      app.currentScreen = SCREEN_HOME;
      break;
    case ACTION_GOTO_MENU:
      app.currentScreen = SCREEN_MENU;
      break;
    case ACTION_GOTO_GAME:
      app.currentScreen = SCREEN_GAME;
      break;
    case ACTION_GOTO_BALANCE:
      app.currentScreen = SCREEN_BALANCE;
      break;
    case ACTION_GOTO_SETTINGS:
      app.currentScreen = SCREEN_SETTINGS;
      app.settingsView = SETTINGS_VIEW_MAIN;
      break;
    case ACTION_GOTO_TESTS:
      app.currentScreen = SCREEN_TESTS;
      break;
    case ACTION_GOTO_PROFILE:
      app.currentScreen = SCREEN_PROFILE;
      break;
    case ACTION_GOTO_TOPUP:
      app.currentScreen = SCREEN_TOPUP;
      break;
    case ACTION_NONE:
      break;
  }

  app.pendingAction = ACTION_NONE;
}

void appUiShowMessage(AppState& app, const char* text, uint16_t color, unsigned long durationMs) {
  if (text == nullptr) {
    text = "";
  }
  strncpy(app.uiStatusText, text, sizeof(app.uiStatusText) - 1);
  app.uiStatusText[sizeof(app.uiStatusText) - 1] = '\0';
  app.uiStatusLevel = (color == UI_ERROR) ? APP_UI_STATUS_ERROR : APP_UI_STATUS_INFO;
  app.uiStatusVisible = (app.uiStatusText[0] != '\0');
  app.uiStatusUntilMs = millis() + ((durationMs > 0) ? durationMs : UI_STATUS_DEFAULT_MS);
  markDirtyRegions(app, DIRTY_UI_OVERLAY);
  noteInteraction(app);
}

void appUiShowError(AppState& app, const char* text, unsigned long durationMs) {
  if (text == nullptr) {
    text = "";
  }
  strncpy(app.uiStatusText, text, sizeof(app.uiStatusText) - 1);
  app.uiStatusText[sizeof(app.uiStatusText) - 1] = '\0';
  app.uiStatusLevel = APP_UI_STATUS_ERROR;
  app.uiStatusVisible = (app.uiStatusText[0] != '\0');
  app.uiStatusUntilMs = millis() + durationMs;
  markDirtyRegions(app, DIRTY_UI_OVERLAY);
  noteInteraction(app);
}

void appUiSetBusy(AppState& app, bool busy, const char* text) {
  app.uiBusy = busy;
  if (text == nullptr) {
    text = "BUSY";
  }
  strncpy(app.uiBusyText, text, sizeof(app.uiBusyText) - 1);
  app.uiBusyText[sizeof(app.uiBusyText) - 1] = '\0';
  markDirtyRegions(app, DIRTY_UI_OVERLAY);
  noteInteraction(app);
}

void appUiSwitchScreen(AppState& app, ScreenState screen) {
  app.currentScreen = screen;
  if (screen == SCREEN_SETTINGS) {
    app.settingsView = SETTINGS_VIEW_MAIN;
  }
  app.fullRedrawRequested = true;
  noteInteraction(app);
}

bool appPostEvent(AppState& app, AppEventType type, int32_t value, const char* text) {
  const uint8_t nextTail = (uint8_t)((app.eventTail + 1) % APP_EVENT_QUEUE_CAPACITY);
  if (nextTail == app.eventHead) {
    return false;
  }

  AppEvent& slot = app.eventQueue[app.eventTail];
  slot.type = type;
  slot.value = value;
  slot.text[0] = '\0';
  if (text != nullptr) {
    strncpy(slot.text, text, sizeof(slot.text) - 1);
    slot.text[sizeof(slot.text) - 1] = '\0';
  }
  app.eventTail = nextTail;
  markDirtyRegions(app, DIRTY_UI_OVERLAY);
  return true;
}

bool appConsumeEvent(AppState& app, AppEvent& out) {
  if (app.eventHead == app.eventTail) {
    return false;
  }

  out = app.eventQueue[app.eventHead];
  app.eventHead = (uint8_t)((app.eventHead + 1) % APP_EVENT_QUEUE_CAPACITY);
  return true;
}

bool appEventIsExternal(AppEventType type) {
  switch (type) {
    case APP_EVENT_BALANCE_UPDATED:
    case APP_EVENT_EXTERNAL_MESSAGE:
    case APP_EVENT_SHOW_NOTIFICATION:
    case APP_EVENT_SHOW_ERROR:
    case APP_EVENT_GAME_STATE_CHANGED:
      return true;
    default:
      return false;
  }
}

bool appEventIsInternal(AppEventType type) {
  return type != APP_EVENT_NONE && !appEventIsExternal(type);
}

const char* appEventName(AppEventType type) {
  switch (type) {
    case APP_EVENT_NONE: return "NONE";
    case APP_EVENT_WIFI_CONNECTED: return "WIFI_CONNECTED";
    case APP_EVENT_WIFI_DISCONNECTED: return "WIFI_DISCONNECTED";
    case APP_EVENT_BUTTON_MENU: return "BUTTON_MENU";
    case APP_EVENT_BUTTON_POWER_SHORT: return "BUTTON_POWER_SHORT";
    case APP_EVENT_BUTTON_POWER_LONG: return "BUTTON_POWER_LONG";
    case APP_EVENT_BALANCE_UPDATED: return "BALANCE_UPDATED";
    case APP_EVENT_EXTERNAL_MESSAGE: return "EXTERNAL_MESSAGE";
    case APP_EVENT_SHOW_NOTIFICATION: return "SHOW_NOTIFICATION";
    case APP_EVENT_SHOW_ERROR: return "SHOW_ERROR";
    case APP_EVENT_GAME_STATE_CHANGED: return "GAME_STATE_CHANGED";
  }

  return "UNKNOWN";
}

void appSetDebugMode(AppState& app, bool enabled) {
  app.debugMode = enabled;
}

bool appIsDebugMode(const AppState& app) {
  return app.debugMode;
}

bool appIsIdleMode(const AppState& app) {
  return app.idleMode;
}

bool appIsSoftStandby(const AppState& app) {
  return app.powerMode == POWER_MODE_STANDBY;
}

bool appIsSleeping(const AppState& app) {
  return app.powerMode != POWER_MODE_NORMAL;
}

uint8_t appLoopDelayMs(const AppState& app) {
  if (app.powerMode == POWER_MODE_STANDBY) {
    return 18;
  }
  if (app.powerMode == POWER_MODE_SLEEP) {
    return 12;
  }
  if (app.keyboardActive) {
    return 1;
  }
  if (app.wifiFlowState == WIFI_FLOW_CONNECTING ||
      app.wifiFlowState == WIFI_FLOW_SCANNING ||
      app.wifiFlowState == WIFI_FLOW_RETRY_WAIT) {
    return 1;
  }
  return app.idleMode ? 8 : 1;
}

static void forceHomeState(AppState& app) {
  app.keyboardActive = false;
  app.keyboardNeedsInit = true;
  app.keyboardTarget = INPUT_TARGET_NONE;
  app.buttonAnimActive = false;
  app.buttonAnimPressedDrawn = false;
  app.buttonAnimTarget = BTN_ANIM_NONE;
  app.pendingAction = ACTION_NONE;
  app.uiBusy = false;
  app.uiStatusVisible = false;
  app.uiStatusText[0] = '\0';
  app.settingsView = SETTINGS_VIEW_MAIN;
  app.fullRedrawRequested = true;
  appUiSwitchScreen(app, SCREEN_HOME);
}

static void enterSleepMode(AppState& app) {
  if (app.powerMode != POWER_MODE_NORMAL) {
    return;
  }
  app.powerMode = POWER_MODE_SLEEP;
  app.buttonAnimActive = false;
  app.buttonAnimPressedDrawn = false;
  app.buttonAnimTarget = BTN_ANIM_NONE;
  app.pendingAction = ACTION_NONE;
  app.uiBusy = false;
  app.uiStatusVisible = false;
  app.uiStatusText[0] = '\0';
  app.idleMode = true;
  app.idleSinceMs = millis();
  app.fullRedrawRequested = true;
  markDirtyRegions(app, DIRTY_UI_OVERLAY);
}

static void wakeFromSleepMode(AppState& app) {
  if (app.powerMode != POWER_MODE_SLEEP) {
    return;
  }
  app.powerMode = POWER_MODE_NORMAL;
  noteInteraction(app);
  app.fullRedrawRequested = true;
}

static void enterStandbyMode(AppState& app) {
  app.powerMode = POWER_MODE_STANDBY;
  app.buttonAnimActive = false;
  app.buttonAnimPressedDrawn = false;
  app.buttonAnimTarget = BTN_ANIM_NONE;
  app.pendingAction = ACTION_NONE;
  app.keyboardActive = false;
  app.keyboardNeedsInit = true;
  app.keyboardTarget = INPUT_TARGET_NONE;
  app.uiBusy = false;
  app.uiStatusVisible = false;
  app.uiStatusText[0] = '\0';
  app.idleMode = true;
  app.idleSinceMs = millis();
  wifiReconnectPending = false;
  wifiAutoReconnectEnabled = false;
  wifiReconnectNextPollMs = 0;
  wifiService.disconnect();
  if (app.ssid[0] != '\0') {
    app.wifiFlowState = WIFI_FLOW_DISCONNECTED;
  } else {
    app.wifiFlowState = WIFI_FLOW_IDLE;
  }
  app.fullRedrawRequested = true;
  markDirtyRegions(app, DIRTY_UI_OVERLAY);
}

static void wakeFromStandbyMode(AppState& app) {
  if (app.powerMode != POWER_MODE_STANDBY) {
    return;
  }
  app.powerMode = POWER_MODE_NORMAL;
  app.settingsView = SETTINGS_VIEW_MAIN;
  app.keyboardActive = false;
  app.keyboardNeedsInit = false;
  app.keyboardTarget = INPUT_TARGET_NONE;
  app.currentScreen = SCREEN_HOME;
  app.balanceReturnScreen = SCREEN_GAME;
  resetStartupAutoConnectState();
  restoreWifiFormState(app);
  noteInteraction(app);
  app.fullRedrawRequested = true;
}

static void handlePostedEvent(AppState& app, const AppEvent& event) {
  switch (event.type) {
    case APP_EVENT_WIFI_CONNECTED:
      appUiShowMessage(app, (event.text[0] != '\0') ? event.text : "WiFi connected", UI_INFO, 1800);
      break;
    case APP_EVENT_WIFI_DISCONNECTED:
      appUiShowError(app, (event.text[0] != '\0') ? event.text : "WiFi disconnected", 2200);
      break;
    case APP_EVENT_BUTTON_MENU:
      if (app.powerMode == POWER_MODE_NORMAL) {
        forceHomeState(app);
      }
      break;
    case APP_EVENT_BUTTON_POWER_SHORT:
      if (app.powerMode == POWER_MODE_NORMAL) {
        enterSleepMode(app);
      } else if (app.powerMode == POWER_MODE_SLEEP) {
        wakeFromSleepMode(app);
      } else {
        wakeFromStandbyMode(app);
      }
      break;
    case APP_EVENT_BUTTON_POWER_LONG:
      if (app.powerMode == POWER_MODE_STANDBY) {
        wakeFromStandbyMode(app);
      } else {
        enterStandbyMode(app);
      }
      break;
    case APP_EVENT_BALANCE_UPDATED:
      app.balanceValue = (int)event.value;
      markDirtyRegions(app, DIRTY_BALANCE_CARD);
      break;
    case APP_EVENT_EXTERNAL_MESSAGE:
    case APP_EVENT_SHOW_NOTIFICATION:
      appUiShowMessage(app, event.text, UI_INFO, UI_STATUS_DEFAULT_MS);
      break;
    case APP_EVENT_SHOW_ERROR:
      appUiShowError(app, event.text, 2500);
      break;
    case APP_EVENT_GAME_STATE_CHANGED:
      app.gameStateValue = event.value;
      strncpy(app.gameStateText, event.text, sizeof(app.gameStateText) - 1);
      app.gameStateText[sizeof(app.gameStateText) - 1] = '\0';
      noteInteraction(app);
      break;
    case APP_EVENT_NONE:
      break;
  }
}

static void drawAnimatedButton(SimpleUI& ui, ButtonAnimTarget target, bool pressed) {
  switch (target) {
    case BTN_ANIM_HOME_PLAY:
      ui.drawButton(btnHomePlay, pressed ? btnHomePlay.pressedColor : btnHomePlay.color);
      break;
    case BTN_ANIM_HOME_MENU:
      ui.drawButton(btnHomeMenu, pressed ? btnHomeMenu.pressedColor : btnHomeMenu.color);
      break;
    case BTN_ANIM_MENU_PROFILE:
      ui.drawButton(btnMenuProfile, pressed ? btnMenuProfile.pressedColor : btnMenuProfile.color);
      break;
    case BTN_ANIM_MENU_TOPUP:
      ui.drawButton(btnMenuTopUp, pressed ? btnMenuTopUp.pressedColor : btnMenuTopUp.color);
      break;
    case BTN_ANIM_MENU_SETTINGS:
      ui.drawButton(btnMenuSettings, pressed ? btnMenuSettings.pressedColor : btnMenuSettings.color);
      break;
    case BTN_ANIM_MENU_HOME:
      ui.drawButton(btnMenuHome, pressed ? btnMenuHome.pressedColor : btnMenuHome.color);
      break;
    case BTN_ANIM_BACK:
      ui.drawButton(btnBack, pressed ? btnBack.pressedColor : btnBack.color);
      break;
    case BTN_ANIM_NONE:
      break;
  }
}

static bool isBackOnlyScreen(ScreenState screen) {
  return screen == SCREEN_GAME ||
         screen == SCREEN_BALANCE ||
         screen == SCREEN_PROFILE ||
         screen == SCREEN_TOPUP ||
         screen == SCREEN_TESTS;
}

static bool isAnimTargetVisible(ScreenState screen, ButtonAnimTarget target) {
  if (screen == SCREEN_HOME) {
    return (target == BTN_ANIM_HOME_PLAY || target == BTN_ANIM_HOME_MENU);
  }
  if (screen == SCREEN_MENU) {
    return (target == BTN_ANIM_MENU_PROFILE || target == BTN_ANIM_MENU_TOPUP ||
            target == BTN_ANIM_MENU_SETTINGS || target == BTN_ANIM_MENU_HOME);
  }
  return isBackOnlyScreen(screen) &&
         target == BTN_ANIM_BACK;
}

static bool processButtonAnimation(SimpleUI& ui, AppState& app) {
  if (!app.buttonAnimActive) {
    return false;
  }

  if (!isAnimTargetVisible(app.currentScreen, app.buttonAnimTarget)) {
    app.buttonAnimActive = false;
    app.buttonAnimPressedDrawn = false;
    app.buttonAnimTarget = BTN_ANIM_NONE;
    app.pendingAction = ACTION_NONE;
    return false;
  }

  const unsigned long elapsed = millis() - app.buttonAnimStartMs;
  if (elapsed == 0) {
    return false;
  }

  if (elapsed < BUTTON_ANIM_MS) {
    if (!app.buttonAnimPressedDrawn) {
      drawAnimatedButton(ui, app.buttonAnimTarget, true);
      app.buttonAnimPressedDrawn = true;
      return true;
    }
    return false;
  }

  drawAnimatedButton(ui, app.buttonAnimTarget, false);
  app.buttonAnimActive = false;
  app.buttonAnimPressedDrawn = false;
  app.buttonAnimTarget = BTN_ANIM_NONE;
  applyPendingAction(app);
  app.fullRedrawRequested = true;
  return true;
}

// ---------------------------
// Init
// ---------------------------
void initAppState(AppState& app) {
  initWiFiProfilesIfNeeded();

  app.currentScreen = SCREEN_HOME;
  app.lastDrawnScreen = SCREEN_HOME;

  app.balanceValue = 1000;
  app.winsCount = 0;
  app.soundEnabled = true;
  app.gameStateValue = 0;
  app.ssid[0] = '\0';
  app.password[0] = '\0';
  app.keyboardDraft[0] = '\0';
  app.gameStateText[0] = '\0';
  app.keyboardTarget = INPUT_TARGET_NONE;
  app.wifiFlowState = WIFI_FLOW_IDLE;
  app.settingsView = SETTINGS_VIEW_MAIN;
  app.touchScreenX = 0;
  app.touchScreenY = 0;
  app.touchPressed = false;
  app.keyboardActive = false;
  app.keyboardNeedsInit = false;
  app.networkPage = 0;

  app.lastTouchTime = 0;
  app.lastInteractionMs = millis();

  app.fullRedrawRequested = true;
  app.dirtyRegions = DIRTY_BALANCE_CARD | DIRTY_WINS_CARD | DIRTY_WIFI_CARD | DIRTY_SOUND_CARD | DIRTY_GAME_AREA;

  app.buttonAnimActive = false;
  app.buttonAnimPressedDrawn = false;
  app.buttonAnimTarget = BTN_ANIM_NONE;
  app.buttonAnimStartMs = 0;
  app.pendingAction = ACTION_NONE;

  app.balanceCacheValid = false;
  app.lastDrawnBalanceValue = 0;
  app.lastDrawnWinsCount = 0;

  app.settingsCacheValid = false;
  app.lastDrawnSoundEnabled = false;
  app.lastDrawnSsid[0] = '\0';
  app.lastDrawnPasswordMasked[0] = '\0';
  app.lastDrawnWifiFlowState = WIFI_FLOW_IDLE;
  app.lastDrawnWifiBadgeText[0] = '\0';
  app.lastDrawnWifiBadgeColor = UI_BG;
  app.lastDrawnWifiActionText[0] = '\0';
  app.lastDrawnWifiActionColor = UI_BG;
  app.lastDrawnNetworkPage = 0;
  app.networkListCacheValid = false;
  app.lastDrawnGameBalanceValue = INT32_MIN;
  app.lastDrawnHomeTime[0] = '\0';
  app.homeCacheValid = false;

  app.uiStatusVisible = false;
  app.uiStatusLevel = APP_UI_STATUS_INFO;
  app.uiStatusText[0] = '\0';
  app.uiStatusUntilMs = 0;
  app.uiBusy = false;
  strncpy(app.uiBusyText, "BUSY", sizeof(app.uiBusyText) - 1);
  app.uiBusyText[sizeof(app.uiBusyText) - 1] = '\0';

  app.eventHead = 0;
  app.eventTail = 0;

  app.debugMode = false;
  app.lastDebugPrintMs = 0;
  app.fullRedrawCount = 0;
  app.partialRedrawCount = 0;
  app.idleLoopCount = 0;
  app.idleMode = false;
  app.idleSinceMs = 0;
  app.powerMode = POWER_MODE_NORMAL;
  app.balanceReturnScreen = SCREEN_GAME;
  app.testSongRequested = false;

  app.gameCacheValid = false;

  resetStartupAutoConnectState();

  WiFiCredentials saved;
  if (wifiProfilesInit && wifiProfiles.loadSaved(saved)) {
    strncpy(app.ssid, saved.ssid, sizeof(app.ssid) - 1);
    app.ssid[sizeof(app.ssid) - 1] = '\0';
    strncpy(app.password, saved.password, sizeof(app.password) - 1);
    app.password[sizeof(app.password) - 1] = '\0';
    app.wifiFlowState = WIFI_FLOW_READY_TO_CONNECT;
  }
}

// ---------------------------
// Base drawing
// ---------------------------
static void drawScreenBase(TFT_eSPI& tft) {
  tft.fillScreen(UI_BG);
  yield();
}

static void drawCard(SimpleUI& ui, int x, int y, int w, int h) {
  ui.drawPanel(x, y, w, h, UI_PANEL, UI_BORDER);
}

static void clearValueArea(TFT_eSPI& tft, int x, int y, int w, int h) {
  tft.fillRect(x, y, w, h, UI_PANEL);
}

static void drawGameBalanceEntry(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  if (!force && app.lastDrawnGameBalanceValue == app.balanceValue) {
    return;
  }

  char valueText[20];
  snprintf(valueText, sizeof(valueText), "$ %d", app.balanceValue);
  tft.fillRect(GAME_BALANCE_X, GAME_BALANCE_Y, GAME_BALANCE_W, GAME_BALANCE_H, UI_PANEL_ALT);
  tft.drawRect(GAME_BALANCE_X, GAME_BALANCE_Y, GAME_BALANCE_W, GAME_BALANCE_H, UI_BORDER);
  tft.drawFastVLine(GAME_BALANCE_X + 34, GAME_BALANCE_Y + 3, GAME_BALANCE_H - 6, UI_BORDER_SOFT);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(UI_TEXT_MUTED, UI_PANEL_ALT);
  tft.setTextSize(1);
  tft.drawString("BAL", GAME_BALANCE_X + 17, GAME_BALANCE_Y + GAME_BALANCE_H / 2, 2);
  tft.setTextColor(UI_INFO, UI_PANEL_ALT);
  tft.drawString(valueText, GAME_BALANCE_X + 66, GAME_BALANCE_Y + GAME_BALANCE_H / 2, 2);
  tft.setTextDatum(TL_DATUM);

  app.lastDrawnGameBalanceValue = app.balanceValue;
}

static void drawHomeTimeSection(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  char timeText[8];
  timeService.formatTime(timeText, sizeof(timeText));
  if (!force && app.homeCacheValid && strcmp(app.lastDrawnHomeTime, timeText) == 0) {
    return;
  }

  const int cardY = HOME_TIME_Y - HOME_TIME_H / 2;
  tft.fillRect(HOME_TIME_X, cardY, HOME_TIME_W, HOME_TIME_H, UI_PANEL_ALT);
  tft.drawRect(HOME_TIME_X, cardY, HOME_TIME_W, HOME_TIME_H, UI_BORDER);
  tft.drawFastHLine(HOME_TIME_X + 2, cardY + 2, HOME_TIME_W - 4, UI_BORDER_SOFT);
  ui.drawCenteredText(timeText, SCREEN_W / 2, HOME_TIME_Y, UI_TEXT, UI_PANEL_ALT, 4);
  ui.drawCenteredText(timeService.isTimeValid() ? "network time" : "waiting for NTP",
                      SCREEN_W / 2, HOME_SUBTITLE_Y, UI_TEXT_MUTED, UI_PANEL_ALT, 1);
  strncpy(app.lastDrawnHomeTime, timeText, sizeof(app.lastDrawnHomeTime) - 1);
  app.lastDrawnHomeTime[sizeof(app.lastDrawnHomeTime) - 1] = '\0';
  app.homeCacheValid = true;
}

// ---------------------------
// Dynamic updates
// ---------------------------
static void updateBalanceValueCard(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  char buffer[32];
  UIValueCard card = {
    CARD_X, 60, CARD_W, CARD_H,
    "BALANCE", "",
    UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
  };

  if (force || !app.balanceCacheValid || app.balanceValue != app.lastDrawnBalanceValue) {
    snprintf(buffer, sizeof(buffer), "$ %d", app.balanceValue);
    if (force) {
      card.value = buffer;
      ui.drawValueCard(card);
    } else {
      ui.drawValueCardValue(card, buffer);
    }
    app.lastDrawnBalanceValue = app.balanceValue;
  }
}

static void updateWinsCard(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  char buffer[32];
  UIValueCard card = {
    CARD_X, 118, CARD_W, CARD_H,
    "WINS", "",
    UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
  };

  if (force || !app.balanceCacheValid || app.winsCount != app.lastDrawnWinsCount) {
    snprintf(buffer, sizeof(buffer), "%d", app.winsCount);
    if (force) {
      card.value = buffer;
      ui.drawValueCard(card);
    } else {
      ui.drawValueCardValue(card, buffer);
    }
    app.lastDrawnWinsCount = app.winsCount;
  }
}

static void updateBalanceRegions(TFT_eSPI& tft, SimpleUI& ui, AppState& app, uint16_t regions, bool force) {
  if (force || (regions & DIRTY_BALANCE_CARD)) {
    updateBalanceValueCard(tft, ui, app, force);
  }
  if (force || (regions & DIRTY_WINS_CARD)) {
    updateWinsCard(tft, ui, app, force);
  }
  app.balanceCacheValid = true;
}

static void updateMainSettingsWifiCard(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  if (!force && app.settingsCacheValid &&
      strcmp(app.ssid, app.lastDrawnSsid) == 0 &&
      app.wifiFlowState == app.lastDrawnWifiFlowState) {
    return;
  }

  tft.fillRect(SETTINGS_MAIN_WIFI_X, SETTINGS_MAIN_WIFI_Y, SETTINGS_MAIN_WIFI_W, SETTINGS_MAIN_WIFI_H, UI_BG);

  UIValueCard wifiCard = {
    SETTINGS_MAIN_WIFI_X, SETTINGS_MAIN_WIFI_Y, SETTINGS_MAIN_WIFI_W, SETTINGS_MAIN_WIFI_H,
    "WiFi Settings", getSettingsWifiSummary(app),
    UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
  };
  ui.drawValueCard(wifiCard);

  strncpy(app.lastDrawnSsid, app.ssid, sizeof(app.lastDrawnSsid) - 1);
  app.lastDrawnSsid[sizeof(app.lastDrawnSsid) - 1] = '\0';
  app.lastDrawnWifiFlowState = app.wifiFlowState;
}

static void updateMainSettingsTouchCard(TFT_eSPI& tft, SimpleUI& ui) {
  tft.fillRect(SETTINGS_MAIN_TOUCH_X, SETTINGS_MAIN_TOUCH_Y, SETTINGS_MAIN_TOUCH_W, SETTINGS_MAIN_TOUCH_H, UI_BG);

  UIValueCard touchCard = {
    SETTINGS_MAIN_TOUCH_X, SETTINGS_MAIN_TOUCH_Y, SETTINGS_MAIN_TOUCH_W, SETTINGS_MAIN_TOUCH_H,
    "Touch Diagnostics", "Open service screen",
    UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
  };
  ui.drawValueCard(touchCard);
}

static void updateMainSettingsTestsCard(TFT_eSPI& tft, SimpleUI& ui) {
  tft.fillRect(SETTINGS_MAIN_TESTS_X, SETTINGS_MAIN_TESTS_Y, SETTINGS_MAIN_TESTS_W, SETTINGS_MAIN_TESTS_H, UI_BG);

  UIValueCard testsCard = {
    SETTINGS_MAIN_TESTS_X, SETTINGS_MAIN_TESTS_Y, SETTINGS_MAIN_TESTS_W, SETTINGS_MAIN_TESTS_H,
    "Tests", "Open sound test tools",
    UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
  };
  ui.drawValueCard(testsCard);
}

static void drawWifiStatusBadge(SimpleUI& ui, const AppState& app) {
  UIStatusBadge wifiBadge = {
    WIFI_STATUS_X, WIFI_STATUS_Y, WIFI_STATUS_W, WIFI_STATUS_H,
    getWifiStatusText(app),
    getWifiStatusColor(app), UI_BORDER, UI_TEXT
  };
  ui.drawStatusBadge(wifiBadge);
}

static bool wifiBadgeChanged(const AppState& app) {
  return strcmp(getWifiStatusText(app), app.lastDrawnWifiBadgeText) != 0 ||
         getWifiStatusColor(app) != app.lastDrawnWifiBadgeColor;
}

static bool wifiActionButtonChanged(const AppState& app) {
  configureWifiPrimaryButton(app);
  return strcmp(btnWifiConnect.text, app.lastDrawnWifiActionText) != 0 ||
         btnWifiConnect.color != app.lastDrawnWifiActionColor;
}

static void updateWifiActionButton(SimpleUI& ui, AppState& app) {
  ui.drawButton(btnWifiConnect);
  strncpy(app.lastDrawnWifiActionText, btnWifiConnect.text, sizeof(app.lastDrawnWifiActionText) - 1);
  app.lastDrawnWifiActionText[sizeof(app.lastDrawnWifiActionText) - 1] = '\0';
  app.lastDrawnWifiActionColor = btnWifiConnect.color;
}

static void updateWifiBadge(SimpleUI& ui, AppState& app) {
  drawWifiStatusBadge(ui, app);
  strncpy(app.lastDrawnWifiBadgeText, getWifiStatusText(app), sizeof(app.lastDrawnWifiBadgeText) - 1);
  app.lastDrawnWifiBadgeText[sizeof(app.lastDrawnWifiBadgeText) - 1] = '\0';
  app.lastDrawnWifiBadgeColor = getWifiStatusColor(app);
}

static void updateWifiForm(TFT_eSPI& tft, SimpleUI& ui, AppState& app, uint16_t regions, bool force) {
  char maskedPassword[65];
  maskPassword(app.password, maskedPassword, sizeof(maskedPassword));
  const bool credentialsChanged =
    !app.settingsCacheValid ||
    strcmp(app.ssid, app.lastDrawnSsid) != 0 ||
    strcmp(maskedPassword, app.lastDrawnPasswordMasked) != 0;
  const bool flowChanged =
    !app.settingsCacheValid || app.wifiFlowState != app.lastDrawnWifiFlowState;
  const bool badgeNeedsUpdate = force || flowChanged || (regions & DIRTY_WIFI_BADGE) || wifiBadgeChanged(app);
  const bool actionNeedsUpdate = force || flowChanged || (regions & DIRTY_WIFI_ACTION_BUTTON) || wifiActionButtonChanged(app);

  if (!force && !credentialsChanged && !flowChanged &&
      !(regions & (DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON))) {
    return;
  }

  if (force || credentialsChanged) {
    tft.fillRect(WIFI_FIELD_X, WIFI_SSID_Y, WIFI_FIELD_W, 94, UI_BG);

    UIValueCard ssidCard = {
      WIFI_FIELD_X, WIFI_SSID_Y, WIFI_FIELD_W, WIFI_FIELD_H,
      "SSID", (app.ssid[0] != '\0') ? app.ssid : "<tap to edit>",
      UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
    };
    ui.drawValueCard(ssidCard);

    UIValueCard passCard = {
      WIFI_FIELD_X, WIFI_PASS_Y, WIFI_FIELD_W, WIFI_FIELD_H,
      "Password", (maskedPassword[0] != '\0') ? maskedPassword : "<tap to edit>",
      UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
    };
    ui.drawValueCard(passCard);
  }

  if (force) {
    btnWifiScan.text = "SCAN NETWORKS";
    ui.drawButton(btnWifiScan);
    ui.drawButton(btnWifiBack);
  }
  if (actionNeedsUpdate) {
    // Hot path: status changes should not force redraw of the whole Wi-Fi form.
    updateWifiActionButton(ui, app);
  }
  if (badgeNeedsUpdate) {
    updateWifiBadge(ui, app);
  }

  strncpy(app.lastDrawnSsid, app.ssid, sizeof(app.lastDrawnSsid) - 1);
  app.lastDrawnSsid[sizeof(app.lastDrawnSsid) - 1] = '\0';
  strncpy(app.lastDrawnPasswordMasked, maskedPassword, sizeof(app.lastDrawnPasswordMasked) - 1);
  app.lastDrawnPasswordMasked[sizeof(app.lastDrawnPasswordMasked) - 1] = '\0';
  app.lastDrawnWifiFlowState = app.wifiFlowState;
}

static void updateSoundCard(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  if (!force && app.settingsCacheValid && app.soundEnabled == app.lastDrawnSoundEnabled) {
    return;
  }

  UIToggle soundToggle = {
    SETTINGS_MAIN_SOUND_X, SETTINGS_MAIN_SOUND_Y, SETTINGS_MAIN_SOUND_W, SETTINGS_MAIN_SOUND_H,
    "Sound", app.soundEnabled,
    UI_PANEL, UI_BORDER, UI_TEXT, UI_SUCCESS, UI_NEUTRAL
  };
  if (force || !app.settingsCacheValid) {
    tft.fillRect(SETTINGS_MAIN_SOUND_X, SETTINGS_MAIN_SOUND_Y,
                 SETTINGS_MAIN_SOUND_W, SETTINGS_MAIN_SOUND_H, UI_BG);
    ui.drawToggle(soundToggle);
  } else {
    // Hot path: only the switch value changes, the card label stays constant.
    ui.drawToggleValue(soundToggle, app.soundEnabled);
  }

  app.lastDrawnSoundEnabled = app.soundEnabled;
}

static bool touchInsideRect(const AppState& app, int x, int y, int w, int h) {
  if (!app.touchPressed) {
    return false;
  }
  return app.touchScreenX >= x && app.touchScreenX <= x + w &&
         app.touchScreenY >= y && app.touchScreenY <= y + h;
}

static void drawTouchDiagnosticsTarget(TFT_eSPI& tft, const char* label,
                                       int x, int y, int w, int h, bool active) {
  const uint16_t fill = active ? UI_SUCCESS : UI_PANEL_ALT;
  const uint16_t border = active ? UI_ACCENT : UI_BORDER;

  tft.fillRect(x, y, w, h, fill);
  tft.drawRect(x, y, w, h, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(UI_TEXT, fill);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
  tft.setTextDatum(TL_DATUM);
}

static void drawTouchDiagnosticsScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  tft.fillRect(0, 36, SCREEN_W, SCREEN_H - 36, UI_BG);

  char coordText[32];
  if (app.touchPressed) {
    snprintf(coordText, sizeof(coordText), "X:%03d  Y:%03d", app.touchScreenX, app.touchScreenY);
  } else {
    snprintf(coordText, sizeof(coordText), "X:---  Y:---");
  }

  UIValueCard coordsCard = {
    TOUCH_DIAG_INFO_X, TOUCH_DIAG_INFO_Y, TOUCH_DIAG_INFO_W, TOUCH_DIAG_INFO_H,
    "Touch Coordinates", coordText,
    UI_PANEL, UI_BORDER, UI_TEXT_MUTED, UI_TEXT
  };
  ui.drawValueCard(coordsCard);

  tft.drawRect(TOUCH_SAFE_MARGIN_X, 96, SCREEN_W - TOUCH_SAFE_MARGIN_X * 2, 88, UI_BORDER_SOFT);
  drawTouchDiagnosticsTarget(tft, "TL", TOUCH_DIAG_TL_X, TOUCH_DIAG_TOP_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H,
                             touchInsideRect(app, TOUCH_DIAG_TL_X, TOUCH_DIAG_TOP_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H));
  drawTouchDiagnosticsTarget(tft, "TR", TOUCH_DIAG_TR_X, TOUCH_DIAG_TOP_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H,
                             touchInsideRect(app, TOUCH_DIAG_TR_X, TOUCH_DIAG_TOP_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H));
  drawTouchDiagnosticsTarget(tft, "BL", TOUCH_DIAG_TL_X, TOUCH_DIAG_BOTTOM_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H,
                             touchInsideRect(app, TOUCH_DIAG_TL_X, TOUCH_DIAG_BOTTOM_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H));
  drawTouchDiagnosticsTarget(tft, "BR", TOUCH_DIAG_TR_X, TOUCH_DIAG_BOTTOM_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H,
                             touchInsideRect(app, TOUCH_DIAG_TR_X, TOUCH_DIAG_BOTTOM_Y, TOUCH_DIAG_TARGET_W, TOUCH_DIAG_TARGET_H));
  drawTouchDiagnosticsTarget(tft, "CENTER", TOUCH_DIAG_CENTER_X, TOUCH_DIAG_CENTER_Y,
                             TOUCH_DIAG_CENTER_W, TOUCH_DIAG_CENTER_H,
                             touchInsideRect(app, TOUCH_DIAG_CENTER_X, TOUCH_DIAG_CENTER_Y,
                                             TOUCH_DIAG_CENTER_W, TOUCH_DIAG_CENTER_H));

  if (app.touchPressed) {
    tft.drawFastVLine(app.touchScreenX, 96, 88, UI_ACCENT);
    tft.drawFastHLine(TOUCH_SAFE_MARGIN_X, app.touchScreenY, SCREEN_W - TOUCH_SAFE_MARGIN_X * 2, UI_ACCENT);
    tft.fillCircle(app.touchScreenX, app.touchScreenY, 3, UI_WARNING);
  }

  ui.drawButton(btnTouchDiagBack);
}

static void drawTestsScreen(TFT_eSPI& tft, SimpleUI& ui) {
  tft.fillRect(0, 36, SCREEN_W, SCREEN_H - 36, UI_BG);
  drawCard(ui, 28, 60, 264, 108);
  ui.drawCenteredText("Sound test area", SCREEN_W / 2, 84, UI_TEXT, UI_PANEL, 2);
  ui.drawCenteredText("Run the loaded MIDI sequence", SCREEN_W / 2, 110, UI_TEXT_MUTED, UI_PANEL, 1);
  ui.drawButton(btnTestsPlaySong);
  ui.drawButton(btnBack);
}

static void updateSettingsRegions(TFT_eSPI& tft, SimpleUI& ui, AppState& app, uint16_t regions, bool force) {
  if (app.keyboardActive) {
    if (keyboard != nullptr) {
      if (force || (regions & DIRTY_KEYBOARD_INPUT)) {
        getKeyboard(tft, ui).drawInputOnly();
      }
      if (force || (regions & DIRTY_WIFI_CARD)) {
        getKeyboard(tft, ui).drawKeysOnly();
      }
    }
    return;
  }

  if (app.settingsView == SETTINGS_VIEW_WIFI && app.wifiFlowState == WIFI_FLOW_NETWORK_LIST) {
    if (force || (regions & DIRTY_WIFI_CARD)) {
      drawNetworkListBody(tft, ui, app, true);
    } else if (regions & DIRTY_NETWORK_PAGE_LABEL) {
      drawNetworkPageLabel(tft, ui, app);
    }
    return;
  }

  if (app.settingsView == SETTINGS_VIEW_WIFI && isWifiOverlayState(app)) {
    return;
  }

  if (app.settingsView == SETTINGS_VIEW_TOUCH_DIAGNOSTICS) {
    if (force || (regions & DIRTY_TOUCH_DIAG)) {
      drawTouchDiagnosticsScreen(tft, ui, app);
    }
    app.settingsCacheValid = true;
    return;
  }

  if (app.settingsView == SETTINGS_VIEW_TESTS) {
    if (force) {
      drawTestsScreen(tft, ui);
    }
    app.settingsCacheValid = true;
    return;
  }

  if (app.settingsView == SETTINGS_VIEW_MAIN) {
    if (force || (regions & (DIRTY_WIFI_CARD | DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON))) {
      updateMainSettingsWifiCard(tft, ui, app, force);
    }
    if (force) {
      updateMainSettingsTouchCard(tft, ui);
      updateMainSettingsTestsCard(tft, ui);
    }
    if (force || (regions & DIRTY_SOUND_CARD)) {
      updateSoundCard(tft, ui, app, force);
    }
  } else {
    if (force || (regions & (DIRTY_WIFI_CARD | DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON))) {
      updateWifiForm(tft, ui, app, regions, force);
    }
  }
  app.settingsCacheValid = true;
}

static void updateGameArea(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  if (!force && app.gameCacheValid) {
    return;
  }

  clearValueArea(tft, 32, 70, 256, 66);
  ui.drawCenteredText("Game area", SCREEN_W / 2, 92, UI_TEXT, UI_PANEL, 2);
  ui.drawCenteredText("core mechanics unchanged", SCREEN_W / 2, 116, UI_TEXT_MUTED, UI_PANEL, 1);
  app.gameCacheValid = true;
}

static void drawHomeScreen(TFT_eSPI& tft, SimpleUI& ui) {
  drawScreenBase(tft);
  tft.fillRect(HOME_BG_X, HOME_BG_Y, HOME_BG_W, HOME_BG_H, UI_PANEL);
  tft.drawRect(HOME_BG_X, HOME_BG_Y, HOME_BG_W, HOME_BG_H, UI_BORDER);
  tft.drawRect(HOME_BG_X + 5, HOME_BG_Y + 5, HOME_BG_W - 10, HOME_BG_H - 10, UI_BORDER_SOFT);
  tft.drawFastHLine(HOME_BG_X + 16, HOME_BG_Y + 28, HOME_BG_W - 32, UI_BORDER_SOFT);
  ui.drawCenteredText("HOME", SCREEN_W / 2, 34, UI_TEXT_MUTED, UI_PANEL, 1);
  ui.drawCenteredText("device home screen", SCREEN_W / 2, 122, UI_TEXT_MUTED, UI_PANEL, 1);
  ui.drawButton(btnHomePlay);
  ui.drawButton(btnHomeMenu);
}

// ---------------------------
// Full screen draw
// ---------------------------
static void drawMenuScreen(TFT_eSPI& tft, SimpleUI& ui) {
  drawScreenBase(tft);
  ui.drawHeaderBar("MENU", UI_TEXT);
  yield();

  ui.drawButton(btnMenuProfile);
  yield();
  ui.drawButton(btnMenuTopUp);
  yield();
  ui.drawButton(btnMenuSettings);
  yield();
  ui.drawButton(btnMenuHome);
  yield();
}

static void drawGameScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  drawScreenBase(tft);
  ui.drawHeaderBar("GAME", UI_TEXT);
  yield();

  drawGameBalanceEntry(tft, ui, app, true);
  yield();

  drawCard(ui, 30, 68, 260, 70);
  yield();
  updateGameArea(tft, ui, app, true);
  yield();

  ui.drawButton(btnBack);
  yield();
}

static void drawBalanceScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  drawScreenBase(tft);
  ui.drawHeaderBar("BALANCE", UI_INFO);
  yield();

  updateBalanceRegions(tft, ui, app, DIRTY_BALANCE_CARD | DIRTY_WINS_CARD, true);
  yield();

  ui.drawButton(btnBack);
  yield();
}

static void drawPlaceholderScreen(TFT_eSPI& tft, SimpleUI& ui, const char* title, const char* line1, const char* line2, uint16_t accent) {
  drawScreenBase(tft);
  ui.drawHeaderBar(title, accent);
  yield();
  drawCard(ui, 28, 72, 264, 82);
  tft.drawFastHLine(44, 98, 232, UI_BORDER_SOFT);
  yield();
  ui.drawCenteredText(line1, SCREEN_W / 2, 98, UI_TEXT, UI_PANEL, 2);
  ui.drawCenteredText(line2, SCREEN_W / 2, 126, UI_TEXT_MUTED, UI_PANEL, 1);
  yield();
  ui.drawButton(btnBack);
  yield();
}

static void drawSettingsScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  if (app.keyboardActive) {
    OnScreenKeyboard& kb = getKeyboard(tft, ui);
    if (app.keyboardNeedsInit) {
      kb.setText(app.keyboardDraft);
      kb.resetLayout();
      app.keyboardNeedsInit = false;
    }
    kb.draw();
    yield();
    return;
  }

  if (app.settingsView == SETTINGS_VIEW_WIFI && app.wifiFlowState == WIFI_FLOW_SCANNING) {
    drawScreenBase(tft);
    ui.drawHeaderBar("WIFI SETTINGS", UI_ACCENT);
    ui.drawCenteredText("Scanning networks...", 160, 108, UI_TEXT, UI_BG, 2);
    ui.drawButton(btnListBack);
    yield();
    return;
  }

  if (app.settingsView == SETTINGS_VIEW_WIFI &&
      (app.wifiFlowState == WIFI_FLOW_NETWORK_LIST ||
       app.wifiFlowState == WIFI_FLOW_NO_NETWORKS ||
       app.wifiFlowState == WIFI_FLOW_SCAN_ERROR)) {
    drawScreenBase(tft);
    if (app.wifiFlowState == WIFI_FLOW_NETWORK_LIST) {
      drawNetworkListBody(tft, ui, app, true);
      ui.drawButton(btnListPrev);
      ui.drawButton(btnListNext);
      ui.drawButton(btnListBack);
    } else if (app.wifiFlowState == WIFI_FLOW_NO_NETWORKS) {
      ui.drawHeaderBar("WIFI SETTINGS", UI_ACCENT);
      ui.drawCenteredText("No networks found", 160, 104, UI_TEXT, UI_BG, 2);
      ui.drawButton(btnWifiScan);
      ui.drawButton(btnListBack);
    } else {
      ui.drawHeaderBar("WIFI SETTINGS", UI_ACCENT);
      ui.drawCenteredText("Scan failed", 160, 104, UI_TEXT, UI_BG, 2);
      ui.drawButton(btnWifiScan);
      ui.drawButton(btnListBack);
    }
    yield();
    return;
  }

  drawScreenBase(tft);
  const char* title = "SETTINGS";
  uint16_t titleColor = UI_TEXT;
  if (app.settingsView == SETTINGS_VIEW_WIFI) {
    title = "WIFI SETTINGS";
    titleColor = UI_ACCENT;
  } else if (app.settingsView == SETTINGS_VIEW_TOUCH_DIAGNOSTICS) {
    title = "TOUCH DIAGNOSTICS";
    titleColor = UI_WARNING;
  } else if (app.settingsView == SETTINGS_VIEW_TESTS) {
    title = "TESTS";
    titleColor = UI_ACCENT;
  }
  ui.drawHeaderBar(title, titleColor);
  yield();

  updateSettingsRegions(tft, ui, app, DIRTY_WIFI_CARD | DIRTY_SOUND_CARD | DIRTY_TOUCH_DIAG, true);
  yield();

  if (app.settingsView == SETTINGS_VIEW_MAIN) {
    ui.drawButton(btnSettingsMainBack);
  }
  yield();
}

void drawCurrentScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  if (app.powerMode != POWER_MODE_NORMAL) {
    app.homeCacheValid = false;
    tft.fillScreen(TFT_BLACK);
    return;
  }

  switch (app.currentScreen) {
    case SCREEN_HOME:
      drawHomeScreen(tft, ui);
      drawHomeTimeSection(tft, ui, app, true);
      break;
    case SCREEN_MENU:
      drawMenuScreen(tft, ui);
      break;

    case SCREEN_GAME:
      drawGameScreen(tft, ui, app);
      break;

    case SCREEN_BALANCE:
      drawBalanceScreen(tft, ui, app);
      break;

    case SCREEN_SETTINGS:
      drawSettingsScreen(tft, ui, app);
      break;
    case SCREEN_TESTS:
      drawScreenBase(tft);
      ui.drawHeaderBar("TESTS", UI_ACCENT);
      drawTestsScreen(tft, ui);
      break;
    case SCREEN_PROFILE:
      drawPlaceholderScreen(tft, ui, "PROFILE", "Profile area", "account details coming later", UI_SECONDARY);
      break;
    case SCREEN_TOPUP:
      drawPlaceholderScreen(tft, ui, "TOP UP", "Top up area", "payment flow coming later", UI_ACCENT);
      break;
  }
}

void updateCurrentScreenData(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  if (app.powerMode != POWER_MODE_NORMAL) {
    return;
  }
  const uint16_t regions = app.dirtyRegions;
  switch (app.currentScreen) {
    case SCREEN_HOME:
      if (regions & DIRTY_HOME_TIME) {
        drawHomeTimeSection(tft, ui, app, false);
      }
      break;
    case SCREEN_MENU:
    case SCREEN_PROFILE:
    case SCREEN_TOPUP:
      break;

    case SCREEN_GAME:
      if (regions & DIRTY_BALANCE_CARD) {
        drawGameBalanceEntry(tft, ui, app, false);
      }
      if (regions & DIRTY_GAME_AREA) {
        updateGameArea(tft, ui, app, false);
      }
      break;

    case SCREEN_BALANCE:
      updateBalanceRegions(tft, ui, app, regions, false);
      break;

    case SCREEN_SETTINGS:
      updateSettingsRegions(tft, ui, app, regions, false);
      break;
    case SCREEN_TESTS:
      break;
  }
}

void updateTouchState(AppState& app, bool touched, int tx, int ty) {
  bool changed = (app.touchPressed != touched);
  if (touched) {
    if (app.powerMode == POWER_MODE_NORMAL) {
      noteInteraction(app);
    }
    if (app.touchScreenX != tx || app.touchScreenY != ty) {
      changed = true;
    }
    app.touchScreenX = tx;
    app.touchScreenY = ty;
  }
  app.touchPressed = touched;

  if (changed && app.currentScreen == SCREEN_SETTINGS &&
      app.settingsView == SETTINGS_VIEW_TOUCH_DIAGNOSTICS &&
      !app.keyboardActive) {
    markDirtyRegions(app, DIRTY_TOUCH_DIAG);
  }
}

// ---------------------------
// UI update processor
// ---------------------------
void processUiUpdates(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  const uint32_t frameStartUs = micros();
  bool drewSomething = false;
  const unsigned long nowMs = millis();

  if (!wifiServiceInit) {
    wifiService.begin();
    wifiServiceInit = true;
  }
  initWiFiProfilesIfNeeded();
  timeService.tick(wifiService.state() == WIFI_SERVICE_CONNECTED);
  if (timeService.consumeMinuteChanged()) {
    markDirtyRegions(app, DIRTY_HOME_TIME);
  }
  updateIdleState(app, nowMs);
  syncWifiPowerSave(app);

  if (app.uiStatusVisible && !app.uiBusy && nowMs >= app.uiStatusUntilMs) {
    app.uiStatusVisible = false;
    app.uiStatusText[0] = '\0';
    markDirtyRegions(app, DIRTY_UI_OVERLAY);
  }

  if (app.powerMode == POWER_MODE_NORMAL && !startupAutoConnectDone && !app.keyboardActive) {
    if (!startupSavedAttempted && app.ssid[0] != '\0') {
      startupSavedAttempted = true;
      if (!startWifiConnect(app)) {
        beginStartupBuiltInScan(app);
      }
    } else if (!startupSavedAttempted && wifiProfiles.hasBuiltInNetworks()) {
      startupSavedAttempted = true;
      beginStartupBuiltInScan(app);
    } else if (!startupSavedAttempted) {
      startupSavedAttempted = true;
      startupAutoConnectDone = true;
      restoreWifiFormState(app);
    }
  }

  if (app.powerMode == POWER_MODE_NORMAL &&
      wifiReconnectPending &&
      (wifiService.state() == WIFI_SERVICE_IDLE || wifiService.state() == WIFI_SERVICE_ERROR) &&
      (long)(nowMs - wifiReconnectNextPollMs) >= 0) {
    wifiReconnectNextPollMs = nowMs + (app.idleMode ? WIFI_RECONNECT_POLL_IDLE_MS : WIFI_RECONNECT_POLL_MS);
    if ((long)(nowMs - wifiReconnectDueMs) >= 0) {
      if (!startWifiConnect(app) && wifiAutoReconnectEnabled) {
        armWifiReconnect(app);
      }
    }
  }

  if (wifiService.tick()) {
    WiFiServiceState s = wifiService.state();
    if (s == WIFI_SERVICE_CONNECTING) {
      app.wifiFlowState = WIFI_FLOW_CONNECTING;
    } else if (s == WIFI_SERVICE_CONNECTED) {
      app.wifiFlowState = WIFI_FLOW_CONNECTED;
      wifiAutoReconnectEnabled = true;
      wifiReconnectPending = false;
      appPostEvent(app, APP_EVENT_WIFI_CONNECTED, 0, "WiFi connected");
      if (wifiProfilesInit && app.ssid[0] != '\0') {
        wifiProfiles.saveSaved(app.ssid, app.password);
      }
      startupAutoConnectDone = true;
    } else if (s == WIFI_SERVICE_ERROR) {
      if (wifiAutoReconnectEnabled && startupAutoConnectDone) {
        armWifiReconnect(app);
      } else if (!startupAutoConnectDone && startupSavedAttempted && !startupBuiltInScanStarted &&
          !startupBuiltInAttempted && wifiProfiles.hasBuiltInNetworks()) {
        beginStartupBuiltInScan(app);
      } else {
        app.wifiFlowState = WIFI_FLOW_ERROR;
        startupAutoConnectDone = true;
      }
    } else if (s == WIFI_SERVICE_IDLE &&
               (app.wifiFlowState == WIFI_FLOW_CONNECTING || app.wifiFlowState == WIFI_FLOW_CONNECTED ||
                app.wifiFlowState == WIFI_FLOW_RETRY_WAIT)) {
      const bool wasConnected = (app.wifiFlowState == WIFI_FLOW_CONNECTED);
      if (wifiAutoReconnectEnabled && startupAutoConnectDone) {
        armWifiReconnect(app);
      } else {
        restoreWifiFormState(app);
      }
      if (wasConnected) {
        appPostEvent(app, APP_EVENT_WIFI_DISCONNECTED, 0, "WiFi disconnected");
      }
    }
    markDirtyRegions(app, DIRTY_WIFI_BADGE | DIRTY_WIFI_ACTION_BUTTON);
  }
  syncWifiPowerSave(app);

  if (wifiService.consumeScanStateChanged()) {
    WiFiScanState ss = wifiService.scanState();
    if (ss == WIFI_SCAN_STATE_RUNNING) {
      app.wifiFlowState = WIFI_FLOW_SCANNING;
    } else if (ss == WIFI_SCAN_STATE_DONE) {
      if (!startupAutoConnectDone && startupBuiltInScanStarted && !startupBuiltInAttempted) {
        WiFiCredentials candidate;
        if (findMatchingBuiltInNetwork(candidate)) {
          strncpy(app.ssid, candidate.ssid, sizeof(app.ssid) - 1);
          app.ssid[sizeof(app.ssid) - 1] = '\0';
          strncpy(app.password, candidate.password, sizeof(app.password) - 1);
          app.password[sizeof(app.password) - 1] = '\0';
          startupBuiltInAttempted = true;
          if (!startWifiConnect(app)) {
            app.wifiFlowState = WIFI_FLOW_ERROR;
            startupAutoConnectDone = true;
          }
        } else {
          app.wifiFlowState = WIFI_FLOW_ERROR;
          startupAutoConnectDone = true;
        }
      } else {
        app.wifiFlowState = WIFI_FLOW_NETWORK_LIST;
        app.networkPage = 0;
      }
    } else if (ss == WIFI_SCAN_STATE_NO_RESULTS) {
      if (!startupAutoConnectDone && startupBuiltInScanStarted && !startupBuiltInAttempted) {
        app.wifiFlowState = WIFI_FLOW_ERROR;
        startupAutoConnectDone = true;
      } else {
        app.wifiFlowState = WIFI_FLOW_NO_NETWORKS;
      }
    } else if (ss == WIFI_SCAN_STATE_ERROR) {
      if (!startupAutoConnectDone && startupBuiltInScanStarted && !startupBuiltInAttempted) {
        app.wifiFlowState = WIFI_FLOW_ERROR;
        startupAutoConnectDone = true;
      } else {
        app.wifiFlowState = WIFI_FLOW_SCAN_ERROR;
      }
    }
    app.fullRedrawRequested = true;
    markDirtyRegions(app, DIRTY_WIFI_CARD);
  }

  AppEvent event;
  while (appConsumeEvent(app, event)) {
    handlePostedEvent(app, event);
  }

  if (processButtonAnimation(ui, app)) {
    drewSomething = true;
  }

  bool screenChanged = (app.currentScreen != app.lastDrawnScreen);
  if (screenChanged) {
    app.fullRedrawRequested = true;
  }

  static uint16_t lastDirtyMask = DIRTY_NONE;
  static uint16_t dirtyStreak = 0;
  const uint16_t pendingDirtyBeforeConsume = app.dirtyRegions;
#if APP_SERIAL_LOGGING_ENABLED
  if (pendingDirtyBeforeConsume != DIRTY_NONE) {
    if (pendingDirtyBeforeConsume == lastDirtyMask) {
      dirtyStreak++;
    } else {
      lastDirtyMask = pendingDirtyBeforeConsume;
      dirtyStreak = 1;
    }
    if (dirtyStreak == 50) {
      Serial.print("WARN: dirtyRegions repeats, mask=");
      Serial.println(pendingDirtyBeforeConsume, HEX);
    }
  } else {
    lastDirtyMask = DIRTY_NONE;
    dirtyStreak = 0;
  }
#else
  (void)lastDirtyMask;
  (void)dirtyStreak;
  (void)pendingDirtyBeforeConsume;
#endif

  static uint16_t fullRedrawStreak = 0;
  if (app.fullRedrawRequested) {
#if APP_SERIAL_LOGGING_ENABLED
    fullRedrawStreak++;
    if (fullRedrawStreak == 20) {
      Serial.println("WARN: full redraw requested for 20 frames");
    }
#else
    (void)fullRedrawStreak;
#endif

    invalidateDynamicCaches(app);
    drawCurrentScreen(tft, ui, app);
    if (app.powerMode == POWER_MODE_NORMAL && app.currentScreen != SCREEN_GAME && !app.keyboardActive) {
      drawUiOverlay(tft, app);
    }
    app.lastDrawnScreen = app.currentScreen;
    drewSomething = true;
    app.fullRedrawCount++;

    app.fullRedrawRequested = false;
    app.dirtyRegions = DIRTY_NONE;
  } else {
    fullRedrawStreak = 0;

    const uint16_t regions = consumeDirtyRegions(app);
    if (regions != DIRTY_NONE) {
      if (app.powerMode != POWER_MODE_NORMAL) {
        // Sleep/standby keep the panel black and avoid partial UI churn.
      } else {
      switch (app.currentScreen) {
        case SCREEN_HOME:
          if (regions & DIRTY_HOME_TIME) {
            drawHomeTimeSection(tft, ui, app, false);
            drewSomething = true;
          }
          if (regions & DIRTY_UI_OVERLAY) {
            drawUiOverlay(tft, app);
            drewSomething = true;
          }
          break;

        case SCREEN_MENU:
          if (regions & DIRTY_UI_OVERLAY) {
            drawUiOverlay(tft, app);
            drewSomething = true;
          }
          break;

        case SCREEN_GAME:
          if (regions & DIRTY_BALANCE_CARD) {
            drawGameBalanceEntry(tft, ui, app, false);
            drewSomething = true;
          }
          if (regions & DIRTY_GAME_AREA) {
            updateGameArea(tft, ui, app, false);
            drewSomething = true;
          }
          break;

        case SCREEN_BALANCE:
          updateBalanceRegions(tft, ui, app, regions, false);
          if (regions & DIRTY_UI_OVERLAY) {
            drawUiOverlay(tft, app);
          }
          drewSomething = true;
          break;

        case SCREEN_SETTINGS:
          if (app.keyboardActive) {
            if (regions & (DIRTY_KEYBOARD_INPUT | DIRTY_WIFI_CARD)) {
              updateSettingsRegions(tft, ui, app, regions, false);
              drewSomething = true;
            }
          } else {
            updateSettingsRegions(tft, ui, app, regions, false);
            if (regions & DIRTY_UI_OVERLAY) {
              drawUiOverlay(tft, app);
            }
            drewSomething = true;
          }
          break;

        case SCREEN_PROFILE:
        case SCREEN_TOPUP:
          if (regions & DIRTY_UI_OVERLAY) {
            drawUiOverlay(tft, app);
            drewSomething = true;
          }
          break;
      }
      }
      if (drewSomething) {
        app.partialRedrawCount++;
      }
    }
  }

  if (drewSomething) {
    yield();
  } else if (app.idleMode) {
    app.idleLoopCount++;
  }

  const uint32_t frameTimeUs = micros() - frameStartUs;
  static uint32_t frameSumUs = 0;
  static uint32_t frameMaxUs = 0;
  static uint16_t frameCount = 0;
  static unsigned long lastPrintMs = 0;

  frameSumUs += frameTimeUs;
  frameCount++;
  if (frameTimeUs > frameMaxUs) {
    frameMaxUs = frameTimeUs;
  }

  if (nowMs - lastPrintMs >= PROFILE_PRINT_INTERVAL_MS) {
#if APP_SERIAL_LOGGING_ENABLED
    const uint32_t avgUs = (frameCount > 0) ? (frameSumUs / frameCount) : 0;
    Serial.print("UI frame us avg=");
    Serial.print(avgUs);
    Serial.print(" max=");
    Serial.print(frameMaxUs);
    Serial.print(" samples=");
    Serial.println(frameCount);

    frameSumUs = 0;
    frameMaxUs = 0;
    frameCount = 0;
    lastPrintMs = nowMs;
#else
    frameSumUs = 0;
    frameMaxUs = 0;
    frameCount = 0;
    lastPrintMs = nowMs;
#endif
  }

#if APP_SERIAL_LOGGING_ENABLED
  if (app.debugMode && nowMs - app.lastDebugPrintMs >= debugPrintIntervalMs(app)) {
    Serial.print("DBG up=");
    Serial.print(nowMs / 1000);
    Serial.print("s heap=");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" rst=");
    Serial.print(ESP.getResetReason());
    Serial.print(" scr=");
    Serial.print(screenStateName(app.currentScreen));
    if (app.currentScreen == SCREEN_SETTINGS) {
      Serial.print("/");
      Serial.print(settingsViewName(app.settingsView));
    }
    Serial.print(" wifi=");
    Serial.print(getWifiStatusText(app));
    Serial.print(" ps=");
    Serial.print(wifiService.powerSaveEnabled() ? "on" : "off");
    Serial.print(" rec=");
    Serial.print(wifiReconnectPending ? "pending" : "off");
    Serial.print(" idle=");
    Serial.print(app.idleMode ? "1" : "0");
    Serial.print(" pwr=");
    Serial.print((int)app.powerMode);
    Serial.print(" redraw=");
    Serial.print(app.fullRedrawCount);
    Serial.print("/");
    Serial.print(app.partialRedrawCount);
    Serial.print(" idleLoops=");
    Serial.println(app.idleLoopCount);
    app.lastDebugPrintMs = nowMs;
  }
#endif
}

// ---------------------------
// Touch handling
// ---------------------------
void handleAppTouch(SimpleUI& ui, AppState& app, int tx, int ty) {
  if (app.powerMode != POWER_MODE_NORMAL) {
    return;
  }
  if (app.buttonAnimActive) {
    return;
  }

  if (millis() - app.lastTouchTime < TOUCH_DEBOUNCE) {
    return;
  }

  app.lastTouchTime = millis();
  noteInteraction(app);

  switch (app.currentScreen) {
    case SCREEN_HOME:
      if (edgeAwareButtonHit(ui, btnHomePlay, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_GAME, BTN_ANIM_HOME_PLAY);
        return;
      }

      if (edgeAwareButtonHit(ui, btnHomeMenu, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_MENU, BTN_ANIM_HOME_MENU);
        return;
      }
      break;

    case SCREEN_MENU:
      if (edgeAwareButtonHit(ui, btnMenuProfile, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_PROFILE, BTN_ANIM_MENU_PROFILE);
        return;
      }

      if (edgeAwareButtonHit(ui, btnMenuTopUp, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_TOPUP, BTN_ANIM_MENU_TOPUP);
        return;
      }

      if (edgeAwareButtonHit(ui, btnMenuSettings, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_SETTINGS, BTN_ANIM_MENU_SETTINGS);
        return;
      }

      if (edgeAwareButtonHit(ui, btnMenuHome, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_HOME, BTN_ANIM_MENU_HOME);
        return;
      }
      break;

    case SCREEN_GAME:
      if (edgeAwareRectHit(ui, tx, ty, GAME_BALANCE_X, GAME_BALANCE_Y, GAME_BALANCE_W, GAME_BALANCE_H, 8, 6)) {
        app.balanceReturnScreen = SCREEN_GAME;
        app.currentScreen = SCREEN_BALANCE;
        app.fullRedrawRequested = true;
        return;
      }
      if (edgeAwareButtonHit(ui, btnBack, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_HOME, BTN_ANIM_BACK);
        return;
      }
      break;

    case SCREEN_BALANCE:
      if (edgeAwareButtonHit(ui, btnBack, tx, ty, 10, 8)) {
        app.currentScreen = app.balanceReturnScreen;
        app.fullRedrawRequested = true;
        return;
      }

      // Тестовая оптимизация: тап по верхней карточке увеличивает баланс
      if (ui.inRect(tx, ty, CARD_X, 60, CARD_W, CARD_H)) {
        app.balanceValue += 100;
        markDirtyRegions(app, DIRTY_BALANCE_CARD);
        return;
      }

      // Тап по нижней карточке увеличивает wins
      if (ui.inRect(tx, ty, CARD_X, 118, CARD_W, CARD_H)) {
        app.winsCount += 1;
        markDirtyRegions(app, DIRTY_WINS_CARD);
        return;
      }
      break;

    case SCREEN_TESTS:
      if (edgeAwareButtonHit(ui, btnBack, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_SETTINGS, BTN_ANIM_BACK);
        return;
      }
      if (edgeAwareButtonHit(ui, btnTestsPlaySong, tx, ty, 10, 8)) {
        app.testSongRequested = true;
        appUiShowMessage(app, "Playing test song", UI_INFO, 1800);
        return;
      }
      break;

    case SCREEN_PROFILE:
    case SCREEN_TOPUP:
      if (edgeAwareButtonHit(ui, btnBack, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_MENU, BTN_ANIM_BACK);
        return;
      }
      break;

    case SCREEN_SETTINGS:
      if (app.keyboardActive) {
        if (keyboard == nullptr) {
          return;
        }
        OnScreenKeyboard& kb = *keyboard;
        KeyboardAction action = kb.handleTap(tx, ty);

        if (action == KEYBOARD_ACTION_INPUT_CHANGED) {
          strncpy(app.keyboardDraft, kb.getText(), sizeof(app.keyboardDraft) - 1);
          app.keyboardDraft[sizeof(app.keyboardDraft) - 1] = '\0';
          markDirtyRegions(app, DIRTY_KEYBOARD_INPUT);
          return;
        }

        if (action == KEYBOARD_ACTION_LAYOUT_CHANGED) {
          strncpy(app.keyboardDraft, kb.getText(), sizeof(app.keyboardDraft) - 1);
          app.keyboardDraft[sizeof(app.keyboardDraft) - 1] = '\0';
          markDirtyRegions(app, DIRTY_WIFI_CARD);
          return;
        }

        if (action == KEYBOARD_ACTION_DONE) {
          if (app.keyboardTarget == INPUT_TARGET_SSID) {
            strncpy(app.ssid, kb.getText(), sizeof(app.ssid) - 1);
            app.ssid[sizeof(app.ssid) - 1] = '\0';
          } else if (app.keyboardTarget == INPUT_TARGET_PASSWORD) {
            strncpy(app.password, kb.getText(), sizeof(app.password) - 1);
            app.password[sizeof(app.password) - 1] = '\0';
          }

          restoreWifiFormState(app);
          app.keyboardActive = false;
          app.keyboardNeedsInit = false;
          app.keyboardTarget = INPUT_TARGET_NONE;
          app.fullRedrawRequested = true;
          markDirtyRegions(app, DIRTY_WIFI_CARD);

#if APP_SERIAL_LOGGING_ENABLED
          Serial.print("WiFi fields updated. SSID=");
          Serial.print(app.ssid);
          Serial.print(" pass_len=");
          Serial.println(strlen(app.password));
#endif
          return;
        }

        if (action == KEYBOARD_ACTION_EXIT) {
          app.keyboardActive = false;
          app.keyboardNeedsInit = false;
          app.keyboardTarget = INPUT_TARGET_NONE;
          restoreWifiFormState(app);
          app.fullRedrawRequested = true;
          return;
        }
        return;
      }

      if (app.settingsView == SETTINGS_VIEW_WIFI && app.wifiFlowState == WIFI_FLOW_SCANNING) {
        if (edgeAwareButtonHit(ui, btnListBack, tx, ty, 8, 8)) {
          restoreWifiFormState(app);
          app.fullRedrawRequested = true;
        }
        return;
      }

      if (app.settingsView == SETTINGS_VIEW_WIFI && app.wifiFlowState == WIFI_FLOW_NETWORK_LIST) {
        handleNetworkListTap(ui, app, tx, ty);
        return;
      }

      if (app.settingsView == SETTINGS_VIEW_WIFI &&
          (app.wifiFlowState == WIFI_FLOW_NO_NETWORKS || app.wifiFlowState == WIFI_FLOW_SCAN_ERROR)) {
        if (edgeAwareButtonHit(ui, btnWifiScan, tx, ty, 8, 8)) {
          startupAutoConnectDone = true;
          wifiReconnectPending = false;
          wifiAutoReconnectEnabled = false;
          app.networkPage = 0;
          app.wifiFlowState = WIFI_FLOW_SCANNING;
          wifiService.startScan();
          app.fullRedrawRequested = true;
          return;
        }
        if (edgeAwareButtonHit(ui, btnListBack, tx, ty, 8, 8)) {
          restoreWifiFormState(app);
          app.fullRedrawRequested = true;
          return;
        }
        return;
      }

      if (app.settingsView == SETTINGS_VIEW_MAIN) {
        if (edgeAwareButtonHit(ui, btnSettingsMainBack, tx, ty, 8, 8)) {
          app.currentScreen = SCREEN_MENU;
          app.fullRedrawRequested = true;
          return;
        }

        if (edgeAwareCardHit(ui, tx, ty, SETTINGS_MAIN_WIFI_X, SETTINGS_MAIN_WIFI_Y, SETTINGS_MAIN_WIFI_W, SETTINGS_MAIN_WIFI_H)) {
          app.settingsView = SETTINGS_VIEW_WIFI;
          app.fullRedrawRequested = true;
          return;
        }

        if (edgeAwareButtonHit(ui, btnSettingsTouchDiag, tx, ty, 8, 8)) {
          app.settingsView = SETTINGS_VIEW_TOUCH_DIAGNOSTICS;
          app.fullRedrawRequested = true;
          return;
        }

        if (edgeAwareButtonHit(ui, btnSettingsTests, tx, ty, 8, 8)) {
          app.settingsView = SETTINGS_VIEW_TESTS;
          app.fullRedrawRequested = true;
          return;
        }

        UIToggle soundToggle = {
          SETTINGS_MAIN_SOUND_X, SETTINGS_MAIN_SOUND_Y, SETTINGS_MAIN_SOUND_W, SETTINGS_MAIN_SOUND_H,
          "Sound", app.soundEnabled,
          UI_PANEL, UI_BORDER, UI_TEXT, UI_SUCCESS, UI_NEUTRAL
        };
        if (edgeAwareRectHit(ui, tx, ty, soundToggle.x, soundToggle.y, soundToggle.w, soundToggle.h, 8, 8)) {
          app.soundEnabled = !app.soundEnabled;
          markDirtyRegions(app, DIRTY_SOUND_CARD);
          return;
        }

        return;
      }

      if (app.settingsView == SETTINGS_VIEW_TOUCH_DIAGNOSTICS) {
        if (edgeAwareButtonHit(ui, btnTouchDiagBack, tx, ty, 8, 8)) {
          app.settingsView = SETTINGS_VIEW_MAIN;
          app.fullRedrawRequested = true;
        }
        return;
      }

      if (app.settingsView == SETTINGS_VIEW_TESTS) {
        if (edgeAwareButtonHit(ui, btnBack, tx, ty, 10, 8)) {
          app.settingsView = SETTINGS_VIEW_MAIN;
          app.fullRedrawRequested = true;
          return;
        }
        if (edgeAwareButtonHit(ui, btnTestsPlaySong, tx, ty, 10, 8)) {
          app.testSongRequested = true;
          appUiShowMessage(app, "Playing test song", UI_INFO, 1800);
          return;
        }
        return;
      }

      if (edgeAwareButtonHit(ui, btnWifiBack, tx, ty, 8, 8)) {
        app.settingsView = SETTINGS_VIEW_MAIN;
        restoreWifiFormState(app);
        app.fullRedrawRequested = true;
        return;
      }

      if (edgeAwareCardHit(ui, tx, ty, WIFI_FIELD_X, WIFI_SSID_Y, WIFI_FIELD_W, WIFI_FIELD_H)) {
        wifiReconnectPending = false;
        wifiAutoReconnectEnabled = false;
        strncpy(app.keyboardDraft, app.ssid, sizeof(app.keyboardDraft) - 1);
        app.keyboardDraft[sizeof(app.keyboardDraft) - 1] = '\0';
        app.keyboardActive = true;
        app.keyboardNeedsInit = true;
        app.keyboardTarget = INPUT_TARGET_SSID;
        app.wifiFlowState = WIFI_FLOW_EDITING_SSID;
        app.fullRedrawRequested = true;
        return;
      }

      if (edgeAwareCardHit(ui, tx, ty, WIFI_FIELD_X, WIFI_PASS_Y, WIFI_FIELD_W, WIFI_FIELD_H)) {
        wifiReconnectPending = false;
        wifiAutoReconnectEnabled = false;
        strncpy(app.keyboardDraft, app.password, sizeof(app.keyboardDraft) - 1);
        app.keyboardDraft[sizeof(app.keyboardDraft) - 1] = '\0';
        app.keyboardActive = true;
        app.keyboardNeedsInit = true;
        app.keyboardTarget = INPUT_TARGET_PASSWORD;
        app.wifiFlowState = WIFI_FLOW_EDITING_PASSWORD;
        app.fullRedrawRequested = true;
        return;
      }

      if (edgeAwareButtonHit(ui, btnWifiConnect, tx, ty, 8, 8)) {
        startupAutoConnectDone = true;
        if (app.wifiFlowState == WIFI_FLOW_CONNECTED) {
          wifiAutoReconnectEnabled = false;
          wifiReconnectPending = false;
          wifiService.disconnect();
          app.wifiFlowState = (app.ssid[0] != '\0') ? WIFI_FLOW_DISCONNECTED : WIFI_FLOW_IDLE;
          markDirtyRegions(app, DIRTY_WIFI_CARD);
          return;
        }

        if (app.wifiFlowState == WIFI_FLOW_CONNECTING) {
          return;
        }

        startWifiConnect(app);
        return;
      }

      if (edgeAwareButtonHit(ui, btnWifiScan, tx, ty, 8, 8)) {
        startupAutoConnectDone = true;
        wifiReconnectPending = false;
        wifiAutoReconnectEnabled = false;
        app.networkPage = 0;
        app.wifiFlowState = WIFI_FLOW_SCANNING;
        wifiService.startScan();
        app.fullRedrawRequested = true;
        return;
      }
      break;
  }
}
