#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

class DisplayHAL {
public:
  DisplayHAL(uint8_t touchCs, uint8_t touchIrq);

  void begin();
  bool readTouch(int &tx, int &ty);
  bool getTap(int &tx, int &ty);
  void waitTouchRelease();

  TFT_eSPI& getTft();

private:
  TFT_eSPI tft;
  XPT2046_Touchscreen ts;
  bool touchLatched;

  int touchToScreenX(int rawY);
  int touchToScreenY(int rawX);
};

#endif
