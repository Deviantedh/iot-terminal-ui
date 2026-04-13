#ifndef SIMPLE_UI_H
#define SIMPLE_UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>

struct UIButton {
  int x;
  int y;
  int w;
  int h;
  uint16_t color;
  uint16_t pressedColor;
  const char* text;
};

struct UIValueCard {
  int x;
  int y;
  int w;
  int h;
  const char* label;
  const char* value;
  uint16_t fillColor;
  uint16_t borderColor;
  uint16_t labelColor;
  uint16_t valueColor;
};

struct UIToggle {
  int x;
  int y;
  int w;
  int h;
  const char* label;
  bool value;
  uint16_t fillColor;
  uint16_t borderColor;
  uint16_t textColor;
  uint16_t onColor;
  uint16_t offColor;
};

struct UIStatusBadge {
  int x;
  int y;
  int w;
  int h;
  const char* text;
  uint16_t fillColor;
  uint16_t borderColor;
  uint16_t textColor;
};

class SimpleUI {
public:
  SimpleUI(TFT_eSPI& display);

  void drawCenteredText(const char* text, int centerX, int centerY,
                        uint16_t textColor, uint16_t bgColor, int textSize);
  void drawCenteredTextInRect(const char* text, int x, int y, int w, int h,
                              uint16_t textColor, uint16_t bgColor, int textSize);

  void drawTitle(const char* text, uint16_t color);
  void drawHeaderBar(const char* title, uint16_t titleColor);
  void drawPanel(int x, int y, int w, int h, uint16_t fillColor, uint16_t borderColor);

  void drawButton(const UIButton& btn);
  void drawButton(const UIButton& btn, uint16_t fillColor);
  void pressButton(const UIButton& btn);

  void drawValueCard(const UIValueCard& card);
  void drawValueCardValue(const UIValueCard& card, const char* value);

  void drawToggle(const UIToggle& toggle);
  void drawToggleValue(const UIToggle& toggle, bool value);
  bool toggleHit(const UIToggle& toggle, int tx, int ty, int padding = 8);

  void drawStatusBadge(const UIStatusBadge& badge);

  bool inRect(int tx, int ty, int x, int y, int w, int h);
  bool buttonHit(const UIButton& btn, int tx, int ty, int padding = 10);

private:
  TFT_eSPI& tft;
};

#endif
