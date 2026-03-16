#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

enum BacklightLevel : uint8_t {
  BACKLIGHT_NORMAL = 0,
  BACKLIGHT_DIM,
  BACKLIGHT_OFF
};

class DisplayHAL {
public:
  DisplayHAL(uint8_t touchCs, uint8_t touchIrq, int8_t backlightPin = -1);

  void begin();
  void displaySleepOn();
  void displaySleepOff();
  bool displayIsSleeping() const;
  bool readTouch(int &tx, int &ty);
  bool getTap(int &tx, int &ty);
  void waitTouchRelease();
  void setBacklightLevel(BacklightLevel level);
  BacklightLevel backlightLevel() const;
  bool backlightControlSupported() const;

  TFT_eSPI& getTft();

private:
  TFT_eSPI tft;
  XPT2046_Touchscreen ts;
  bool touchLatched;
  int8_t backlightPin;
  BacklightLevel currentBacklightLevel;
  bool displaySleeping;

  int touchToScreenX(int rawY);
  int touchToScreenY(int rawX);
};

#endif
