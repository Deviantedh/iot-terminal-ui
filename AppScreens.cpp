#include "AppScreens.h"
#include "OnScreenKeyboard.h"
#include "WiFiService.h"
#include "WiFiProfiles.h"
#include <string.h>

static const unsigned long TOUCH_DEBOUNCE = 180;
static const unsigned long BUTTON_ANIM_MS = 80;
static const unsigned long PROFILE_PRINT_INTERVAL_MS = 1000;
static const unsigned long WIFI_RECONNECT_INTERVAL_MS = 5000;

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

static const int BACK_W = 120;
static const int BACK_H = 46;
static const int BACK_X = 20;
static const int BACK_Y = 182;

static const int SETTINGS_MAIN_WIFI_X = 28;
static const int SETTINGS_MAIN_WIFI_Y = 46;
static const int SETTINGS_MAIN_WIFI_W = 264;
static const int SETTINGS_MAIN_WIFI_H = 42;

static const int SETTINGS_MAIN_TOUCH_X = 28;
static const int SETTINGS_MAIN_TOUCH_Y = 94;
static const int SETTINGS_MAIN_TOUCH_W = 264;
static const int SETTINGS_MAIN_TOUCH_H = 42;

static const int SETTINGS_MAIN_SOUND_X = 28;
static const int SETTINGS_MAIN_SOUND_Y = 142;
static const int SETTINGS_MAIN_SOUND_W = 264;
static const int SETTINGS_MAIN_SOUND_H = 40;

static const int SETTINGS_MAIN_BACK_X = 28;
static const int SETTINGS_MAIN_BACK_Y = 190;
static const int SETTINGS_MAIN_BACK_W = 264;
static const int SETTINGS_MAIN_BACK_H = 34;

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

// ---------------------------
// Buttons
// ---------------------------
static UIButton btnStart    = {50,  58, 220, 42, TFT_DARKGREEN, TFT_GREEN,     "START"};
static UIButton btnBalance  = {50, 112, 220, 42, TFT_NAVY,      TFT_BLUE,      "BALANCE"};
static UIButton btnSettings = {50, 166, 220, 42, TFT_MAROON,    TFT_RED,       "SETTINGS"};
static UIButton btnBack     = {BACK_X, BACK_Y, BACK_W, BACK_H, TFT_DARKGREY, TFT_LIGHTGREY, "BACK"};
static UIButton btnSettingsMainBack = {SETTINGS_MAIN_BACK_X, SETTINGS_MAIN_BACK_Y, SETTINGS_MAIN_BACK_W, SETTINGS_MAIN_BACK_H, TFT_DARKGREY, TFT_LIGHTGREY, "BACK"};
static UIButton btnSettingsTouchDiag = {SETTINGS_MAIN_TOUCH_X, SETTINGS_MAIN_TOUCH_Y, SETTINGS_MAIN_TOUCH_W, SETTINGS_MAIN_TOUCH_H, TFT_DARKCYAN, TFT_CYAN, "TOUCH DIAGNOSTICS"};
static UIButton btnWifiScan = {WIFI_SCAN_X, WIFI_SCAN_Y, WIFI_SCAN_W, WIFI_SCAN_H, TFT_DARKCYAN, TFT_CYAN, "SCAN NETWORKS"};
static UIButton btnWifiConnect = {WIFI_CONNECT_X, WIFI_CONNECT_Y, WIFI_CONNECT_W, WIFI_CONNECT_H, TFT_DARKGREEN, TFT_GREEN, "CONNECT"};
static UIButton btnWifiBack = {WIFI_BACK_X, WIFI_BACK_Y, WIFI_BACK_W, WIFI_BACK_H, TFT_DARKGREY, TFT_LIGHTGREY, "BACK"};
static UIButton btnListPrev = {LIST_PREV_X, LIST_PREV_Y, LIST_PREV_W, LIST_PREV_H, TFT_DARKGREY, TFT_LIGHTGREY, "PREV"};
static UIButton btnListNext = {LIST_NEXT_X, LIST_NEXT_Y, LIST_NEXT_W, LIST_NEXT_H, TFT_DARKGREY, TFT_LIGHTGREY, "NEXT"};
static UIButton btnListBack = {LIST_BACK_X, LIST_BACK_Y, LIST_BACK_W, LIST_BACK_H, TFT_DARKGREY, TFT_LIGHTGREY, "BACK"};
static UIButton btnTouchDiagBack = {TOUCH_DIAG_BACK_X, TOUCH_DIAG_BACK_Y, TOUCH_DIAG_BACK_W, TOUCH_DIAG_BACK_H, TFT_DARKGREY, TFT_LIGHTGREY, "BACK"};

static OnScreenKeyboard* keyboard = nullptr;
static WiFiService wifiService;
static WiFiProfiles wifiProfiles;
static bool wifiServiceInit = false;
static bool wifiProfilesInit = false;
static bool startupAutoConnectDone = false;
static bool startupSavedAttempted = false;
static bool startupBuiltInScanStarted = false;
static bool startupBuiltInAttempted = false;
static bool wifiAutoReconnectEnabled = false;
static bool wifiReconnectPending = false;
static unsigned long wifiReconnectDueMs = 0;

static void fitTextWithEllipsis(TFT_eSPI& tft, const char* src, char* out, size_t outSize, int maxWidthPx);
static void restoreWifiFormState(AppState& app);
static void markDirtyRegions(AppState& app, uint8_t regions);

static void configureWifiPrimaryButton(const AppState& app) {
  if (app.wifiFlowState == WIFI_FLOW_CONNECTED) {
    btnWifiConnect.color = TFT_MAROON;
    btnWifiConnect.pressedColor = TFT_RED;
    btnWifiConnect.text = "DISCONNECT";
    return;
  }

  if (app.wifiFlowState == WIFI_FLOW_ERROR || app.wifiFlowState == WIFI_FLOW_RETRY_WAIT) {
    btnWifiConnect.color = TFT_BROWN;
    btnWifiConnect.pressedColor = TFT_ORANGE;
    btnWifiConnect.text = "RETRY";
    return;
  }

  if (app.wifiFlowState == WIFI_FLOW_CONNECTING) {
    btnWifiConnect.color = TFT_DARKGREY;
    btnWifiConnect.pressedColor = TFT_LIGHTGREY;
    btnWifiConnect.text = "WAIT";
    return;
  }

  btnWifiConnect.color = TFT_DARKGREEN;
  btnWifiConnect.pressedColor = TFT_GREEN;
  btnWifiConnect.text = "CONNECT";
}

static bool startWifiConnect(AppState& app) {
  if (app.ssid[0] == '\0') {
    app.wifiFlowState = WIFI_FLOW_ERROR;
    wifiReconnectPending = false;
    markDirtyRegions(app, DIRTY_WIFI_CARD);
    return false;
  }

  if (!wifiServiceInit) {
    wifiService.begin();
    wifiServiceInit = true;
  }

  wifiReconnectPending = false;
  bool started = wifiService.beginConnect(app.ssid, app.password);
  app.wifiFlowState = started ? WIFI_FLOW_CONNECTING : WIFI_FLOW_ERROR;
  markDirtyRegions(app, DIRTY_WIFI_CARD);
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
  app.wifiFlowState = WIFI_FLOW_RETRY_WAIT;
  markDirtyRegions(app, DIRTY_WIFI_CARD);
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
  app.gameCacheValid = false;
}

static void markDirtyRegions(AppState& app, uint8_t regions) {
  app.dirtyRegions |= regions;
}

static uint8_t consumeDirtyRegions(AppState& app) {
  uint8_t regions = app.dirtyRegions;
  app.dirtyRegions = DIRTY_NONE;
  return regions;
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

static void beginStartupBuiltInScan(AppState& app) {
  if (!wifiProfiles.hasBuiltInNetworks() || startupBuiltInScanStarted) {
    startupAutoConnectDone = true;
    restoreWifiFormState(app);
    return;
  }

  if (wifiService.startScan()) {
    startupBuiltInScanStarted = true;
    app.wifiFlowState = WIFI_FLOW_SCANNING;
    markDirtyRegions(app, DIRTY_WIFI_CARD);
  } else {
    startupAutoConnectDone = true;
    app.wifiFlowState = WIFI_FLOW_ERROR;
    markDirtyRegions(app, DIRTY_WIFI_CARD);
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
      return TFT_DARKGREY;
    case WIFI_FLOW_DISCONNECTED:
      return TFT_DARKGREY;
    case WIFI_FLOW_EDITING_SSID:
    case WIFI_FLOW_EDITING_PASSWORD:
      return TFT_NAVY;
    case WIFI_FLOW_READY_TO_CONNECT:
      return TFT_BROWN;
    case WIFI_FLOW_SCANNING:
      return TFT_DARKCYAN;
    case WIFI_FLOW_NETWORK_LIST:
      return TFT_NAVY;
    case WIFI_FLOW_NO_NETWORKS:
      return TFT_DARKGREY;
    case WIFI_FLOW_SCAN_ERROR:
      return TFT_MAROON;
    case WIFI_FLOW_RETRY_WAIT:
      return TFT_BROWN;
    case WIFI_FLOW_CONNECTING:
      return TFT_DARKCYAN;
    case WIFI_FLOW_CONNECTED:
      return TFT_DARKGREEN;
    case WIFI_FLOW_ERROR:
      return TFT_MAROON;
  }
  return TFT_DARKGREY;
}

static int networkPageCount() {
  int count = wifiService.networkCount();
  if (count <= 0) {
    return 1;
  }
  return (count + LIST_PAGE_SIZE - 1) / LIST_PAGE_SIZE;
}

static void drawNetworkListBody(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  tft.fillRect(0, LIST_Y, SCREEN_W, 144, TFT_BLACK);
  ui.drawHeaderBar("SELECT NETWORK", TFT_CYAN);

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
      tft.drawRect(LIST_X, y, LIST_W, LIST_ROW_H, TFT_DARKGREY);
      continue;
    }

    const WiFiNetworkInfo* n = wifiService.networkAt(idx);
    if (n == nullptr) {
      continue;
    }

    tft.fillRect(LIST_X, y, LIST_W, LIST_ROW_H, TFT_BLACK);
    tft.drawRect(LIST_X, y, LIST_W, LIST_ROW_H, TFT_WHITE);

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
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString("*", textX, rowMidY, 2);
      textX += secureMarkWidth;
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(ssidText, textX, rowMidY, 2);

    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(rssiText, LIST_X + LIST_W - rightPad, rowMidY, 2);
    tft.setTextDatum(TL_DATUM);
  }

  char pageTxt[24];
  snprintf(pageTxt, sizeof(pageTxt), "Page %d/%d", app.networkPage + 1, pages);
  ui.drawCenteredText(pageTxt, 160, 178, TFT_LIGHTGREY, TFT_BLACK, 1);
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
    app.fullRedrawRequested = true;
    return true;
  }
  if (edgeAwareButtonHit(ui, btnListNext, tx, ty, 8, 8) && app.networkPage + 1 < pages) {
    app.networkPage++;
    app.fullRedrawRequested = true;
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
    case ACTION_NONE:
      break;
  }

  app.pendingAction = ACTION_NONE;
}

static void drawAnimatedButton(SimpleUI& ui, ButtonAnimTarget target, bool pressed) {
  switch (target) {
    case BTN_ANIM_MENU_START:
      ui.drawButton(btnStart, pressed ? btnStart.pressedColor : btnStart.color);
      break;
    case BTN_ANIM_MENU_BALANCE:
      ui.drawButton(btnBalance, pressed ? btnBalance.pressedColor : btnBalance.color);
      break;
    case BTN_ANIM_MENU_SETTINGS:
      ui.drawButton(btnSettings, pressed ? btnSettings.pressedColor : btnSettings.color);
      break;
    case BTN_ANIM_BACK:
      ui.drawButton(btnBack, pressed ? btnBack.pressedColor : btnBack.color);
      break;
    case BTN_ANIM_NONE:
      break;
  }
}

static bool isAnimTargetVisible(ScreenState screen, ButtonAnimTarget target) {
  if (screen == SCREEN_MENU) {
    return (target == BTN_ANIM_MENU_START || target == BTN_ANIM_MENU_BALANCE || target == BTN_ANIM_MENU_SETTINGS);
  }
  return target == BTN_ANIM_BACK;
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

  app.currentScreen = SCREEN_MENU;
  app.lastDrawnScreen = SCREEN_MENU;

  app.balanceValue = 1000;
  app.winsCount = 0;
  app.soundEnabled = true;
  app.ssid[0] = '\0';
  app.password[0] = '\0';
  app.keyboardDraft[0] = '\0';
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

  app.gameCacheValid = false;

  startupAutoConnectDone = false;
  startupSavedAttempted = false;
  startupBuiltInScanStarted = false;
  startupBuiltInAttempted = false;
  wifiAutoReconnectEnabled = false;
  wifiReconnectPending = false;
  wifiReconnectDueMs = 0;

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
  tft.fillScreen(TFT_BLACK);
  yield();
}

static void drawCard(SimpleUI& ui, int x, int y, int w, int h) {
  ui.drawPanel(x, y, w, h, TFT_BLACK, TFT_WHITE);
}

static void clearValueArea(TFT_eSPI& tft, int x, int y, int w, int h) {
  tft.fillRect(x, y, w, h, TFT_BLACK);
}

// ---------------------------
// Dynamic updates
// ---------------------------
static void updateBalanceValueCard(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  char buffer[32];
  UIValueCard card = {
    CARD_X, 60, CARD_W, CARD_H,
    "BALANCE", "",
    TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, TFT_WHITE
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
    TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, TFT_WHITE
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

static void updateBalanceRegions(TFT_eSPI& tft, SimpleUI& ui, AppState& app, uint8_t regions, bool force) {
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

  tft.fillRect(SETTINGS_MAIN_WIFI_X, SETTINGS_MAIN_WIFI_Y, SETTINGS_MAIN_WIFI_W, SETTINGS_MAIN_WIFI_H, TFT_BLACK);

  UIValueCard wifiCard = {
    SETTINGS_MAIN_WIFI_X, SETTINGS_MAIN_WIFI_Y, SETTINGS_MAIN_WIFI_W, SETTINGS_MAIN_WIFI_H,
    "WiFi Settings", getSettingsWifiSummary(app),
    TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, TFT_WHITE
  };
  ui.drawValueCard(wifiCard);

  strncpy(app.lastDrawnSsid, app.ssid, sizeof(app.lastDrawnSsid) - 1);
  app.lastDrawnSsid[sizeof(app.lastDrawnSsid) - 1] = '\0';
  app.lastDrawnWifiFlowState = app.wifiFlowState;
}

static void updateMainSettingsTouchCard(TFT_eSPI& tft, SimpleUI& ui) {
  tft.fillRect(SETTINGS_MAIN_TOUCH_X, SETTINGS_MAIN_TOUCH_Y, SETTINGS_MAIN_TOUCH_W, SETTINGS_MAIN_TOUCH_H, TFT_BLACK);

  UIValueCard touchCard = {
    SETTINGS_MAIN_TOUCH_X, SETTINGS_MAIN_TOUCH_Y, SETTINGS_MAIN_TOUCH_W, SETTINGS_MAIN_TOUCH_H,
    "Touch Diagnostics", "Open service screen",
    TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, TFT_WHITE
  };
  ui.drawValueCard(touchCard);
}

static void updateWifiForm(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  char maskedPassword[65];
  maskPassword(app.password, maskedPassword, sizeof(maskedPassword));
  configureWifiPrimaryButton(app);

  if (!force && app.settingsCacheValid &&
      strcmp(app.ssid, app.lastDrawnSsid) == 0 &&
      strcmp(maskedPassword, app.lastDrawnPasswordMasked) == 0 &&
      app.wifiFlowState == app.lastDrawnWifiFlowState) {
    return;
  }

  tft.fillRect(WIFI_FIELD_X, WIFI_SSID_Y, WIFI_FIELD_W, 188, TFT_BLACK);

  UIValueCard ssidCard = {
    WIFI_FIELD_X, WIFI_SSID_Y, WIFI_FIELD_W, WIFI_FIELD_H,
    "SSID", (app.ssid[0] != '\0') ? app.ssid : "<tap to edit>",
    TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, TFT_WHITE
  };
  ui.drawValueCard(ssidCard);

  UIValueCard passCard = {
    WIFI_FIELD_X, WIFI_PASS_Y, WIFI_FIELD_W, WIFI_FIELD_H,
    "Password", (maskedPassword[0] != '\0') ? maskedPassword : "<tap to edit>",
    TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, TFT_WHITE
  };
  ui.drawValueCard(passCard);

  btnWifiScan.text = "SCAN NETWORKS";
  ui.drawButton(btnWifiScan);
  ui.drawButton(btnWifiConnect);
  ui.drawButton(btnWifiBack);

  UIStatusBadge wifiBadge = {
    WIFI_STATUS_X, WIFI_STATUS_Y, WIFI_STATUS_W, WIFI_STATUS_H,
    getWifiStatusText(app),
    getWifiStatusColor(app), TFT_WHITE, TFT_WHITE
  };
  ui.drawStatusBadge(wifiBadge);

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

  tft.fillRect(SETTINGS_MAIN_SOUND_X, SETTINGS_MAIN_SOUND_Y,
               SETTINGS_MAIN_SOUND_W, SETTINGS_MAIN_SOUND_H, TFT_BLACK);

  UIToggle soundToggle = {
    SETTINGS_MAIN_SOUND_X, SETTINGS_MAIN_SOUND_Y, SETTINGS_MAIN_SOUND_W, SETTINGS_MAIN_SOUND_H,
    "Sound", app.soundEnabled,
    TFT_BLACK, TFT_WHITE, TFT_WHITE, TFT_DARKGREEN, TFT_DARKGREY
  };
  ui.drawToggle(soundToggle);

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
  const uint16_t fill = active ? TFT_DARKGREEN : TFT_NAVY;
  const uint16_t border = active ? TFT_GREEN : TFT_WHITE;

  tft.fillRect(x, y, w, h, fill);
  tft.drawRect(x, y, w, h, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, fill);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
  tft.setTextDatum(TL_DATUM);
}

static void drawTouchDiagnosticsScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  tft.fillRect(0, 36, SCREEN_W, SCREEN_H - 36, TFT_BLACK);

  char coordText[32];
  if (app.touchPressed) {
    snprintf(coordText, sizeof(coordText), "X:%03d  Y:%03d", app.touchScreenX, app.touchScreenY);
  } else {
    snprintf(coordText, sizeof(coordText), "X:---  Y:---");
  }

  UIValueCard coordsCard = {
    TOUCH_DIAG_INFO_X, TOUCH_DIAG_INFO_Y, TOUCH_DIAG_INFO_W, TOUCH_DIAG_INFO_H,
    "Touch Coordinates", coordText,
    TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, TFT_WHITE
  };
  ui.drawValueCard(coordsCard);

  tft.drawRect(TOUCH_SAFE_MARGIN_X, 96, SCREEN_W - TOUCH_SAFE_MARGIN_X * 2, 88, TFT_DARKGREY);
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
    tft.drawFastVLine(app.touchScreenX, 96, 88, TFT_DARKCYAN);
    tft.drawFastHLine(TOUCH_SAFE_MARGIN_X, app.touchScreenY, SCREEN_W - TOUCH_SAFE_MARGIN_X * 2, TFT_DARKCYAN);
    tft.fillCircle(app.touchScreenX, app.touchScreenY, 3, TFT_YELLOW);
  }

  ui.drawButton(btnTouchDiagBack);
}

static void updateSettingsRegions(TFT_eSPI& tft, SimpleUI& ui, AppState& app, uint8_t regions, bool force) {
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

  if (app.settingsView == SETTINGS_VIEW_MAIN) {
    if (force || (regions & DIRTY_WIFI_CARD)) {
      updateMainSettingsWifiCard(tft, ui, app, force);
    }
    if (force) {
      updateMainSettingsTouchCard(tft, ui);
    }
    if (force || (regions & DIRTY_SOUND_CARD)) {
      updateSoundCard(tft, ui, app, force);
    }
  } else {
    if (force || (regions & DIRTY_WIFI_CARD)) {
      updateWifiForm(tft, ui, app, force);
    }
  }
  app.settingsCacheValid = true;
}

static void updateGameArea(TFT_eSPI& tft, SimpleUI& ui, AppState& app, bool force) {
  if (!force && app.gameCacheValid) {
    return;
  }

  clearValueArea(tft, 32, 70, 256, 66);
  ui.drawCenteredText("Here will be", SCREEN_W / 2, 92, TFT_WHITE, TFT_BLACK, 2);
  ui.drawCenteredText("game screen", SCREEN_W / 2, 116, TFT_WHITE, TFT_BLACK, 2);
  app.gameCacheValid = true;
}

// ---------------------------
// Full screen draw
// ---------------------------
static void drawMenuScreen(TFT_eSPI& tft, SimpleUI& ui) {
  drawScreenBase(tft);
  ui.drawHeaderBar("MAIN MENU", TFT_YELLOW);
  yield();

  ui.drawButton(btnStart);
  yield();
  ui.drawButton(btnBalance);
  yield();
  ui.drawButton(btnSettings);
  yield();
}

static void drawGameScreen(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  drawScreenBase(tft);
  ui.drawHeaderBar("GAME", TFT_GREEN);
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
  ui.drawHeaderBar("BALANCE", TFT_CYAN);
  yield();

  updateBalanceRegions(tft, ui, app, DIRTY_BALANCE_CARD | DIRTY_WINS_CARD, true);
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
    ui.drawHeaderBar("WIFI SETTINGS", TFT_CYAN);
    ui.drawCenteredText("Scanning networks...", 160, 108, TFT_WHITE, TFT_BLACK, 2);
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
      drawNetworkListBody(tft, ui, app);
      ui.drawButton(btnListPrev);
      ui.drawButton(btnListNext);
      ui.drawButton(btnListBack);
    } else if (app.wifiFlowState == WIFI_FLOW_NO_NETWORKS) {
      ui.drawHeaderBar("WIFI SETTINGS", TFT_CYAN);
      ui.drawCenteredText("No networks found", 160, 104, TFT_WHITE, TFT_BLACK, 2);
      ui.drawButton(btnWifiScan);
      ui.drawButton(btnListBack);
    } else {
      ui.drawHeaderBar("WIFI SETTINGS", TFT_CYAN);
      ui.drawCenteredText("Scan failed", 160, 104, TFT_WHITE, TFT_BLACK, 2);
      ui.drawButton(btnWifiScan);
      ui.drawButton(btnListBack);
    }
    yield();
    return;
  }

  drawScreenBase(tft);
  const char* title = "SETTINGS";
  uint16_t titleColor = TFT_MAGENTA;
  if (app.settingsView == SETTINGS_VIEW_WIFI) {
    title = "WIFI SETTINGS";
    titleColor = TFT_CYAN;
  } else if (app.settingsView == SETTINGS_VIEW_TOUCH_DIAGNOSTICS) {
    title = "TOUCH DIAGNOSTICS";
    titleColor = TFT_YELLOW;
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
  switch (app.currentScreen) {
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
  }
}

void updateCurrentScreenData(TFT_eSPI& tft, SimpleUI& ui, AppState& app) {
  const uint8_t regions = app.dirtyRegions;
  switch (app.currentScreen) {
    case SCREEN_MENU:
      break;

    case SCREEN_GAME:
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
  }
}

void updateTouchState(AppState& app, bool touched, int tx, int ty) {
  bool changed = (app.touchPressed != touched);
  if (touched) {
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

  if (!wifiServiceInit) {
    wifiService.begin();
    wifiServiceInit = true;
  }
  initWiFiProfilesIfNeeded();

  if (!startupAutoConnectDone && !app.keyboardActive) {
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

  if (wifiReconnectPending &&
      (long)(millis() - wifiReconnectDueMs) >= 0 &&
      (wifiService.state() == WIFI_SERVICE_IDLE || wifiService.state() == WIFI_SERVICE_ERROR)) {
    if (!startWifiConnect(app) && wifiAutoReconnectEnabled) {
      armWifiReconnect(app);
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
      if (wifiAutoReconnectEnabled && startupAutoConnectDone) {
        armWifiReconnect(app);
      } else {
        restoreWifiFormState(app);
      }
    }
    markDirtyRegions(app, DIRTY_WIFI_CARD);
  }

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

  if (processButtonAnimation(ui, app)) {
    drewSomething = true;
  }

  bool screenChanged = (app.currentScreen != app.lastDrawnScreen);
  if (screenChanged) {
    app.fullRedrawRequested = true;
  }

  static uint8_t lastDirtyMask = DIRTY_NONE;
  static uint16_t dirtyStreak = 0;
  const uint8_t pendingDirtyBeforeConsume = app.dirtyRegions;
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

  static uint16_t fullRedrawStreak = 0;
  if (app.fullRedrawRequested) {
    fullRedrawStreak++;
    if (fullRedrawStreak == 20) {
      Serial.println("WARN: full redraw requested for 20 frames");
    }

    invalidateDynamicCaches(app);
    drawCurrentScreen(tft, ui, app);
    app.lastDrawnScreen = app.currentScreen;
    drewSomething = true;

    app.fullRedrawRequested = false;
    app.dirtyRegions = DIRTY_NONE;
  } else {
    fullRedrawStreak = 0;

    const uint8_t regions = consumeDirtyRegions(app);
    if (regions != DIRTY_NONE) {
      switch (app.currentScreen) {
        case SCREEN_MENU:
          break;

        case SCREEN_GAME:
          if (regions & DIRTY_GAME_AREA) {
            updateGameArea(tft, ui, app, false);
            drewSomething = true;
          }
          break;

        case SCREEN_BALANCE:
          updateBalanceRegions(tft, ui, app, regions, false);
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
            drewSomething = true;
          }
          break;
      }
    }
  }

  if (drewSomething) {
    yield();
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

  const unsigned long nowMs = millis();
  if (nowMs - lastPrintMs >= PROFILE_PRINT_INTERVAL_MS) {
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
  }
}

// ---------------------------
// Touch handling
// ---------------------------
void handleAppTouch(SimpleUI& ui, AppState& app, int tx, int ty) {
  if (app.buttonAnimActive) {
    return;
  }

  if (millis() - app.lastTouchTime < TOUCH_DEBOUNCE) {
    return;
  }

  app.lastTouchTime = millis();

  switch (app.currentScreen) {
    case SCREEN_MENU:
      if (edgeAwareButtonHit(ui, btnStart, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_GAME, BTN_ANIM_MENU_START);
        return;
      }

      if (edgeAwareButtonHit(ui, btnBalance, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_BALANCE, BTN_ANIM_MENU_BALANCE);
        return;
      }

      if (edgeAwareButtonHit(ui, btnSettings, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_SETTINGS, BTN_ANIM_MENU_SETTINGS);
        return;
      }
      break;

    case SCREEN_GAME:
      if (edgeAwareButtonHit(ui, btnBack, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_MENU, BTN_ANIM_BACK);
        return;
      }
      break;

    case SCREEN_BALANCE:
      if (edgeAwareButtonHit(ui, btnBack, tx, ty, 10, 8)) {
        queueScreenAction(app, ACTION_GOTO_MENU, BTN_ANIM_BACK);
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

          Serial.print("WiFi fields updated. SSID=");
          Serial.print(app.ssid);
          Serial.print(" pass_len=");
          Serial.println(strlen(app.password));
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

        UIToggle soundToggle = {
          SETTINGS_MAIN_SOUND_X, SETTINGS_MAIN_SOUND_Y, SETTINGS_MAIN_SOUND_W, SETTINGS_MAIN_SOUND_H,
          "Sound", app.soundEnabled,
          TFT_BLACK, TFT_WHITE, TFT_WHITE, TFT_DARKGREEN, TFT_DARKGREY
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
