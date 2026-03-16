#include "SimpleUI.h"

static void drawFastFrame(TFT_eSPI& tft, int x, int y, int w, int h,
                          uint16_t fillColor, uint16_t borderColor) {
  tft.fillRect(x, y, w, h, fillColor);
  tft.drawRect(x, y, w, h, borderColor);
}

SimpleUI::SimpleUI(TFT_eSPI& display)
  : tft(display) {
}

void SimpleUI::drawCenteredText(const char* text, int centerX, int centerY,
                                uint16_t textColor, uint16_t bgColor, int textSize) {
  tft.setTextSize(textSize);
  tft.setTextColor(textColor, bgColor);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, centerX, centerY);
  tft.setTextDatum(TL_DATUM);
}

void SimpleUI::drawTitle(const char* text, uint16_t color) {
  drawCenteredText(text, 160, 20, color, TFT_BLACK, 2);
}

void SimpleUI::drawHeaderBar(const char* title, uint16_t titleColor) {
  tft.fillRect(0, 0, 320, 36, TFT_BLACK);
  tft.drawFastHLine(0, 35, 320, TFT_WHITE);
  drawCenteredText(title, 160, 18, titleColor, TFT_BLACK, 2);
}

void SimpleUI::drawPanel(int x, int y, int w, int h, uint16_t fillColor, uint16_t borderColor) {
  drawFastFrame(tft, x, y, w, h, fillColor, borderColor);
}

void SimpleUI::drawButton(const UIButton& btn, uint16_t fillColor) {
  drawFastFrame(tft, btn.x, btn.y, btn.w, btn.h, fillColor, TFT_WHITE);
  if (btn.h >= 18) {
    tft.drawFastHLine(btn.x + 2, btn.y + 2, btn.w - 4, TFT_DARKGREY);
  }

  drawCenteredText(
    btn.text,
    btn.x + btn.w / 2,
    btn.y + btn.h / 2,
    TFT_WHITE,
    fillColor,
    2
  );
}

void SimpleUI::drawButton(const UIButton& btn) {
  drawButton(btn, btn.color);
}

void SimpleUI::pressButton(const UIButton& btn) {
  drawButton(btn, btn.pressedColor);
}

void SimpleUI::drawValueCard(const UIValueCard& card) {
  drawFastFrame(tft, card.x, card.y, card.w, card.h, card.fillColor, card.borderColor);
  tft.drawFastHLine(card.x + 1, card.y + 16, card.w - 2, TFT_DARKGREY);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(card.labelColor, card.fillColor);
  tft.drawString(card.label, card.x + 8, card.y + 6);

  drawCenteredText(card.value, card.x + card.w / 2, card.y + card.h - 14,
                   card.valueColor, card.fillColor, 2);
}

void SimpleUI::drawValueCardValue(const UIValueCard& card, const char* value) {
  const int valueY = card.y + card.h / 2;
  const int valueH = card.h / 2 - 4;
  tft.fillRect(card.x + 4, valueY, card.w - 8, valueH, card.fillColor);
  drawCenteredText(value, card.x + card.w / 2, card.y + card.h - 14,
                   card.valueColor, card.fillColor, 2);
}

void SimpleUI::drawToggle(const UIToggle& toggle) {
  drawFastFrame(tft, toggle.x, toggle.y, toggle.w, toggle.h, toggle.fillColor, toggle.borderColor);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(toggle.textColor, toggle.fillColor);
  tft.drawString(toggle.label, toggle.x + 8, toggle.y + 8);

  drawToggleValue(toggle, toggle.value);
}

void SimpleUI::drawToggleValue(const UIToggle& toggle, bool value) {
  const int sw = 66;
  const int sh = 26;
  const int sx = toggle.x + toggle.w - sw - 8;
  const int sy = toggle.y + (toggle.h - sh) / 2;
  const int knob = sh - 8;

  drawFastFrame(tft, sx, sy, sw, sh, value ? toggle.onColor : toggle.offColor, toggle.borderColor);

  const int knobX = value ? (sx + sw - knob - 4) : (sx + 4);
  tft.fillRect(knobX, sy + 4, knob, knob, TFT_WHITE);
  tft.drawRect(knobX, sy + 4, knob, knob, TFT_DARKGREY);
}

bool SimpleUI::toggleHit(const UIToggle& toggle, int tx, int ty, int padding) {
  return inRect(tx, ty, toggle.x - padding, toggle.y - padding,
                toggle.w + padding * 2, toggle.h + padding * 2);
}

void SimpleUI::drawStatusBadge(const UIStatusBadge& badge) {
  drawFastFrame(tft, badge.x, badge.y, badge.w, badge.h, badge.fillColor, badge.borderColor);
  drawCenteredText(badge.text, badge.x + badge.w / 2, badge.y + badge.h / 2,
                   badge.textColor, badge.fillColor, 1);
}

bool SimpleUI::inRect(int tx, int ty, int x, int y, int w, int h) {
  return (tx >= x && tx <= (x + w) && ty >= y && ty <= (y + h));
}

bool SimpleUI::buttonHit(const UIButton& btn, int tx, int ty, int padding) {
  return inRect(
    tx, ty,
    btn.x - padding,
    btn.y - padding,
    btn.w + padding * 2,
    btn.h + padding * 2
  );
}
