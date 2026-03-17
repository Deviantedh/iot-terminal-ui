#include "DisplayHAL.h"

static const uint8_t ST7789_CMD_SLEEP_IN = 0x10;
static const uint8_t ST7789_CMD_SLEEP_OUT = 0x11;
static const uint8_t ST7789_CMD_DISPLAY_OFF = 0x28;
static const uint8_t ST7789_CMD_DISPLAY_ON = 0x29;
static const bool ST7789_COLOR_INVERSION_ENABLED = false;
static const unsigned long ST7789_BLACK_FRAME_SETTLE_MS = 18;
static const unsigned long ST7789_DISPLAY_OFF_DELAY_MS = 20;
static const unsigned long ST7789_SLEEP_DELAY_MS = 120;

static int medianOf5(int values[5]) {
  for (int i = 1; i < 5; i++) {
    int key = values[i];
    int j = i - 1;
    while (j >= 0 && values[j] > key) {
      values[j + 1] = values[j];
      j--;
    }
    values[j + 1] = key;
  }
  return values[2];
}

DisplayHAL::DisplayHAL(uint8_t touchCs, uint8_t touchIrq, int8_t backlightPinValue)
  : tft(),
    ts(touchCs, touchIrq),
    touchLatched(false),
    backlightPin(backlightPinValue),
    currentBacklightLevel(BACKLIGHT_NORMAL),
    displaySleeping(false),
    controllerSleepActive(false) {
}

void DisplayHAL::begin() {
  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(ST7789_COLOR_INVERSION_ENABLED);
  displaySleeping = false;
  controllerSleepActive = false;

  ts.begin();
  ts.setRotation(1);
  touchLatched = false;

  if (backlightPin >= 0) {
    pinMode(backlightPin, OUTPUT);
    analogWriteRange(255);
    analogWrite(backlightPin, 255);
  }
}

void DisplayHAL::displaySleepOn() {
  if (displaySleeping) {
    return;
  }

  // Keep the last visible frame black before any controller power command.
  // On boards with fixed backlight power, this is the only stable way to
  // guarantee a black-looking sleep screen.
  tft.fillScreen(TFT_BLACK);
  delay(ST7789_BLACK_FRAME_SETTLE_MS);

  controllerSleepActive = false;
  if (backlightControlSupported()) {
  tft.writecommand(ST7789_CMD_DISPLAY_OFF);
    delay(ST7789_DISPLAY_OFF_DELAY_MS);
  tft.writecommand(ST7789_CMD_SLEEP_IN);
    delay(ST7789_SLEEP_DELAY_MS);
    controllerSleepActive = true;
  }
  displaySleeping = true;
}

void DisplayHAL::displaySleepOff() {
  if (!displaySleeping) {
    return;
  }

  if (controllerSleepActive) {
    tft.writecommand(ST7789_CMD_SLEEP_OUT);
    delay(ST7789_SLEEP_DELAY_MS);
    tft.writecommand(ST7789_CMD_DISPLAY_ON);
    delay(ST7789_DISPLAY_OFF_DELAY_MS);
    tft.invertDisplay(ST7789_COLOR_INVERSION_ENABLED);
    controllerSleepActive = false;
  }

  displaySleeping = false;
}

bool DisplayHAL::displayIsSleeping() const {
  return displaySleeping;
}

int DisplayHAL::touchToScreenX(int rawY) {
  int x = map(rawY, 585, 3725, 0, 320);
  return constrain(x, 0, 319);
}

int DisplayHAL::touchToScreenY(int rawX) {
  int y = map(rawX, 3740, 430, 0, 240);
  return constrain(y, 0, 239);
}

bool DisplayHAL::readTouch(int &tx, int &ty) {
  if (!ts.tirqTouched()) {
    return false;
  }

  if (!ts.touched()) {
    return false;
  }

  int rawX[5];
  int rawY[5];
  for (int i = 0; i < 5; i++) {
    TS_Point p = ts.getPoint();
    rawX[i] = p.x;
    rawY[i] = p.y;
  }

  int filteredRawX = medianOf5(rawX);
  int filteredRawY = medianOf5(rawY);

  tx = touchToScreenX(filteredRawY);
  ty = touchToScreenY(filteredRawX);

  return true;
}

bool DisplayHAL::getTap(int &tx, int &ty) {
  if (!ts.tirqTouched()) {
    touchLatched = false;
    return false;
  }

  if (!ts.touched()) {
    touchLatched = false;
    return false;
  }

  if (touchLatched) {
    return false;
  }

  if (!readTouch(tx, ty)) {
    return false;
  }

  touchLatched = true;
  return true;
}

void DisplayHAL::waitTouchRelease() {
  while (ts.tirqTouched() || ts.touched()) {
    delay(10);
  }
}

void DisplayHAL::setBacklightLevel(BacklightLevel level) {
  if (currentBacklightLevel == level) {
    return;
  }
  currentBacklightLevel = level;
  if (backlightPin < 0) {
    return;
  }

  switch (level) {
    case BACKLIGHT_NORMAL:
      analogWrite(backlightPin, 255);
      break;
    case BACKLIGHT_DIM:
      analogWrite(backlightPin, 96);
      break;
    case BACKLIGHT_OFF:
      analogWrite(backlightPin, 0);
      break;
  }
}

BacklightLevel DisplayHAL::backlightLevel() const {
  return currentBacklightLevel;
}

bool DisplayHAL::backlightControlSupported() const {
  return backlightPin >= 0;
}

TFT_eSPI& DisplayHAL::getTft() {
  return tft;
}
