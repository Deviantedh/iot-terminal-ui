#ifndef ON_SCREEN_KEYBOARD_H
#define ON_SCREEN_KEYBOARD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "SimpleUI.h"

enum KeyboardAction {
  KEYBOARD_ACTION_NONE = 0,
  KEYBOARD_ACTION_INPUT_CHANGED,
  KEYBOARD_ACTION_LAYOUT_CHANGED,
  KEYBOARD_ACTION_EXIT,
  KEYBOARD_ACTION_DONE
};

enum KeyboardPage : uint8_t {
  KEYBOARD_PAGE_ALPHA = 0,
  KEYBOARD_PAGE_SYMBOLS
};

class OnScreenKeyboard {
public:
  OnScreenKeyboard(TFT_eSPI& display, SimpleUI& uiRef);

  void clearText();
  void setText(const char* text);
  const char* getText() const;
  void resetLayout();
  bool isUpperCase() const;
  KeyboardPage page() const;

  void draw();
  void drawInputOnly();
  void drawKeysOnly();
  KeyboardAction handleTap(int tx, int ty);

private:
  TFT_eSPI& tft;
  SimpleUI& ui;

  char inputText[65];
  uint8_t inputLen;
  bool upperCase;
  KeyboardPage currentPage;

  void drawInputBar();
  void drawControlButtons();
  void drawKeyGrid();
  void drawCharRow(int y, const char* chars);
  void drawSpecialRow(int y);

  bool inExitButton(int tx, int ty) const;
  bool inDoneButton(int tx, int ty) const;

  void getRowMetrics(const char* chars, int& startX, int& keyW, int& len) const;
  bool hitRect(int tx, int ty, int x, int y, int w, int h, int padX, int padY) const;
  bool handleCharTap(int tx, int ty);
  KeyboardAction handleSpecialTap(int tx, int ty);

  void appendChar(char c);
  void backspace();
};

#endif
