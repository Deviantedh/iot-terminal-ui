#include "OnScreenKeyboard.h"
#include <string.h>

static const int SCREEN_W = 320;
static const int SCREEN_H = 240;
static const int TOUCH_SAFE_MARGIN_X = 10;
static const int TOUCH_SAFE_MARGIN_Y = 8;
static const int TOUCH_EDGE_BOOST = 8;

static const int TOP_H = 48;
static const int KEYBOARD_Y = 48;
static const int KEYBOARD_H = 192;

static const int CTRL_BTN_W = 66;
static const int CTRL_BTN_H = 30;
static const int DONE_X = SCREEN_W - CTRL_BTN_W - 12;
static const int EXIT_X = DONE_X - CTRL_BTN_W - 8;
static const int CTRL_Y = 9;

static const int INPUT_X = 12;
static const int INPUT_Y = 8;
static const int INPUT_W = EXIT_X - INPUT_X - 8;
static const int INPUT_H = 32;

static const int GRID_MARGIN_X = 10;
static const int GRID_MARGIN_Y = 6;
static const int KEY_GAP_X = 3;
static const int KEY_GAP_Y = 6;
static const int KEY_H = 40;

static const int SPECIAL_Y = KEYBOARD_Y + GRID_MARGIN_Y + 3 * (KEY_H + KEY_GAP_Y);
static const int SHIFT_X = 10;
static const int SHIFT_W = 62;
static const int MODE_X = 78;
static const int MODE_W = 62;
static const int SPACE_X = 146;
static const int SPACE_W = 86;
static const int BKSP_X = 238;
static const int BKSP_W = 62;

static const char* ALPHA_UPPER_ROW1 = "QWERTYUIOP";
static const char* ALPHA_UPPER_ROW2 = "ASDFGHJKL";
static const char* ALPHA_UPPER_ROW3 = "ZXCVBNM";
static const char* ALPHA_LOWER_ROW1 = "qwertyuiop";
static const char* ALPHA_LOWER_ROW2 = "asdfghjkl";
static const char* ALPHA_LOWER_ROW3 = "zxcvbnm";

static const char* SYM_ROW1 = "12345678";
static const char* SYM_ROW2 = "90_-@!#$";
static const char* SYM_ROW3 = "()%+*./?";

OnScreenKeyboard::OnScreenKeyboard(TFT_eSPI& display, SimpleUI& uiRef)
  : tft(display), ui(uiRef), inputLen(0), upperCase(false), currentPage(KEYBOARD_PAGE_ALPHA) {
  inputText[0] = '\0';
}

void OnScreenKeyboard::clearText() {
  inputLen = 0;
  inputText[0] = '\0';
}

void OnScreenKeyboard::setText(const char* text) {
  if (text == nullptr) {
    clearText();
    return;
  }

  strncpy(inputText, text, sizeof(inputText) - 1);
  inputText[sizeof(inputText) - 1] = '\0';
  inputLen = strlen(inputText);
}

const char* OnScreenKeyboard::getText() const {
  return inputText;
}

void OnScreenKeyboard::resetLayout() {
  upperCase = false;
  currentPage = KEYBOARD_PAGE_ALPHA;
}

bool OnScreenKeyboard::isUpperCase() const {
  return upperCase;
}

KeyboardPage OnScreenKeyboard::page() const {
  return currentPage;
}

void OnScreenKeyboard::draw() {
  tft.fillRect(0, 0, SCREEN_W, TOP_H, TFT_BLACK);
  tft.fillRect(0, KEYBOARD_Y, SCREEN_W, KEYBOARD_H, TFT_BLACK);
  drawInputBar();
  drawControlButtons();
  drawKeyGrid();
}

void OnScreenKeyboard::drawInputOnly() {
  drawInputBar();
}

void OnScreenKeyboard::drawKeysOnly() {
  tft.fillRect(0, KEYBOARD_Y, SCREEN_W, KEYBOARD_H, TFT_BLACK);
  drawKeyGrid();
}

void OnScreenKeyboard::drawInputBar() {
  tft.fillRect(INPUT_X, INPUT_Y, INPUT_W, INPUT_H, TFT_BLACK);
  tft.drawRect(INPUT_X, INPUT_Y, INPUT_W, INPUT_H, TFT_WHITE);
  ui.drawCenteredText(inputLen > 0 ? inputText : "_",
                      INPUT_X + INPUT_W / 2, INPUT_Y + INPUT_H / 2,
                      TFT_WHITE, TFT_BLACK, 2);
}

void OnScreenKeyboard::drawControlButtons() {
  UIButton exitBtn = {EXIT_X, CTRL_Y, CTRL_BTN_W, CTRL_BTN_H, TFT_DARKGREY, TFT_LIGHTGREY, "EXIT"};
  UIButton doneBtn = {DONE_X, CTRL_Y, CTRL_BTN_W, CTRL_BTN_H, TFT_DARKGREEN, TFT_GREEN, "DONE"};
  ui.drawButton(exitBtn);
  ui.drawButton(doneBtn);
}

void OnScreenKeyboard::drawKeyGrid() {
  const char* row1 = nullptr;
  const char* row2 = nullptr;
  const char* row3 = nullptr;

  if (currentPage == KEYBOARD_PAGE_ALPHA) {
    row1 = upperCase ? ALPHA_UPPER_ROW1 : ALPHA_LOWER_ROW1;
    row2 = upperCase ? ALPHA_UPPER_ROW2 : ALPHA_LOWER_ROW2;
    row3 = upperCase ? ALPHA_UPPER_ROW3 : ALPHA_LOWER_ROW3;
  } else {
    row1 = SYM_ROW1;
    row2 = SYM_ROW2;
    row3 = SYM_ROW3;
  }

  int y = KEYBOARD_Y + GRID_MARGIN_Y;
  drawCharRow(y, row1);
  y += KEY_H + KEY_GAP_Y;
  drawCharRow(y, row2);
  y += KEY_H + KEY_GAP_Y;
  drawCharRow(y, row3);
  drawSpecialRow(SPECIAL_Y);
}

void OnScreenKeyboard::getRowMetrics(const char* chars, int& startX, int& keyW, int& len) const {
  len = strlen(chars);
  if (len <= 0) {
    startX = GRID_MARGIN_X;
    keyW = SCREEN_W - GRID_MARGIN_X * 2;
    len = 0;
    return;
  }

  const int totalGap = (len - 1) * KEY_GAP_X;
  keyW = (SCREEN_W - GRID_MARGIN_X * 2 - totalGap) / len;
  const int rowW = keyW * len + totalGap;
  startX = (SCREEN_W - rowW) / 2;
}

void OnScreenKeyboard::drawCharRow(int y, const char* chars) {
  int startX = 0;
  int keyW = 0;
  int len = 0;
  getRowMetrics(chars, startX, keyW, len);

  for (int col = 0; col < len; col++) {
    char label[2] = { chars[col], '\0' };
    UIButton key = {
      startX + col * (keyW + KEY_GAP_X),
      y,
      keyW,
      KEY_H,
      TFT_NAVY,
      TFT_BLUE,
      label
    };
    ui.drawButton(key);
  }
}

void OnScreenKeyboard::drawSpecialRow(int y) {
  UIButton shiftBtn = {
    SHIFT_X, y, SHIFT_W, KEY_H,
    (currentPage == KEYBOARD_PAGE_ALPHA && upperCase) ? TFT_DARKCYAN : TFT_DARKGREY,
    (currentPage == KEYBOARD_PAGE_ALPHA && upperCase) ? TFT_CYAN : TFT_LIGHTGREY,
    "SHIFT"
  };
  UIButton modeBtn = {
    MODE_X, y, MODE_W, KEY_H,
    TFT_DARKGREY, TFT_LIGHTGREY,
    (currentPage == KEYBOARD_PAGE_ALPHA) ? "123" : "ABC"
  };
  UIButton spaceBtn = {SPACE_X, y, SPACE_W, KEY_H, TFT_DARKGREEN, TFT_GREEN, "SPACE"};
  UIButton bkspBtn = {BKSP_X, y, BKSP_W, KEY_H, TFT_MAROON, TFT_RED, "BKSP"};

  ui.drawButton(shiftBtn);
  ui.drawButton(modeBtn);
  ui.drawButton(spaceBtn);
  ui.drawButton(bkspBtn);
}

bool OnScreenKeyboard::inExitButton(int tx, int ty) const {
  return hitRect(tx, ty, EXIT_X, CTRL_Y, CTRL_BTN_W, CTRL_BTN_H, 6, 5);
}

bool OnScreenKeyboard::inDoneButton(int tx, int ty) const {
  return hitRect(tx, ty, DONE_X, CTRL_Y, CTRL_BTN_W, CTRL_BTN_H, 6, 5);
}

bool OnScreenKeyboard::hitRect(int tx, int ty, int x, int y, int w, int h, int padX, int padY) const {
  int left = x - padX;
  int top = y - padY;
  int width = w + padX * 2;
  int height = h + padY * 2;

  if (x <= TOUCH_SAFE_MARGIN_X + 2) {
    left -= TOUCH_EDGE_BOOST;
    width += TOUCH_EDGE_BOOST;
  }
  if (x + w >= SCREEN_W - TOUCH_SAFE_MARGIN_X - 2) {
    width += TOUCH_EDGE_BOOST;
  }
  if (y <= TOUCH_SAFE_MARGIN_Y + 2) {
    top -= TOUCH_EDGE_BOOST / 2;
    height += TOUCH_EDGE_BOOST / 2;
  }
  if (y + h >= SCREEN_H - TOUCH_SAFE_MARGIN_Y - 2) {
    height += TOUCH_EDGE_BOOST / 2;
  }

  return ui.inRect(tx, ty, left, top, width, height);
}

KeyboardAction OnScreenKeyboard::handleTap(int tx, int ty) {
  if (inExitButton(tx, ty)) {
    return KEYBOARD_ACTION_EXIT;
  }
  if (inDoneButton(tx, ty)) {
    return KEYBOARD_ACTION_DONE;
  }
  if (handleCharTap(tx, ty)) {
    return KEYBOARD_ACTION_INPUT_CHANGED;
  }
  KeyboardAction specialAction = handleSpecialTap(tx, ty);
  if (specialAction != KEYBOARD_ACTION_NONE) {
    return specialAction;
  }
  return KEYBOARD_ACTION_NONE;
}

bool OnScreenKeyboard::handleCharTap(int tx, int ty) {
  const int firstRowY = KEYBOARD_Y + GRID_MARGIN_Y;
  for (int row = 0; row < 3; row++) {
    const int rowY = firstRowY + row * (KEY_H + KEY_GAP_Y);
    const char* chars = nullptr;
    if (currentPage == KEYBOARD_PAGE_ALPHA) {
      if (row == 0) {
        chars = upperCase ? ALPHA_UPPER_ROW1 : ALPHA_LOWER_ROW1;
      } else if (row == 1) {
        chars = upperCase ? ALPHA_UPPER_ROW2 : ALPHA_LOWER_ROW2;
      } else {
        chars = upperCase ? ALPHA_UPPER_ROW3 : ALPHA_LOWER_ROW3;
      }
    } else {
      chars = (row == 0) ? SYM_ROW1 : ((row == 1) ? SYM_ROW2 : SYM_ROW3);
    }

    int startX = 0;
    int keyW = 0;
    int len = 0;
    getRowMetrics(chars, startX, keyW, len);
    for (int col = 0; col < len; col++) {
      const int keyX = startX + col * (keyW + KEY_GAP_X);
      if (!hitRect(tx, ty, keyX, rowY, keyW, KEY_H, 2, 4)) {
        continue;
      }

      appendChar(chars[col]);
      return true;
    }
  }
  return false;
}

KeyboardAction OnScreenKeyboard::handleSpecialTap(int tx, int ty) {
  if (!ui.inRect(tx, ty, 0, SPECIAL_Y - 4, SCREEN_W, KEY_H + 8)) {
    return KEYBOARD_ACTION_NONE;
  }

  if (hitRect(tx, ty, SHIFT_X, SPECIAL_Y, SHIFT_W, KEY_H, 5, 5)) {
    if (currentPage != KEYBOARD_PAGE_ALPHA) {
      return KEYBOARD_ACTION_NONE;
    }
    upperCase = !upperCase;
    return KEYBOARD_ACTION_LAYOUT_CHANGED;
  }

  if (hitRect(tx, ty, MODE_X, SPECIAL_Y, MODE_W, KEY_H, 5, 5)) {
    currentPage = (currentPage == KEYBOARD_PAGE_ALPHA) ? KEYBOARD_PAGE_SYMBOLS : KEYBOARD_PAGE_ALPHA;
    return KEYBOARD_ACTION_LAYOUT_CHANGED;
  }

  if (hitRect(tx, ty, SPACE_X, SPECIAL_Y, SPACE_W, KEY_H, 4, 5)) {
    appendChar(' ');
    return KEYBOARD_ACTION_INPUT_CHANGED;
  }

  if (hitRect(tx, ty, BKSP_X, SPECIAL_Y, BKSP_W, KEY_H, 5, 5)) {
    backspace();
    return KEYBOARD_ACTION_INPUT_CHANGED;
  }
  return KEYBOARD_ACTION_NONE;
}

void OnScreenKeyboard::appendChar(char c) {
  if (inputLen >= sizeof(inputText) - 1) {
    return;
  }
  inputText[inputLen++] = c;
  inputText[inputLen] = '\0';
}

void OnScreenKeyboard::backspace() {
  if (inputLen == 0) {
    return;
  }
  inputLen--;
  inputText[inputLen] = '\0';
}
