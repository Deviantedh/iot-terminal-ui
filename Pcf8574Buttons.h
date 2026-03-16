#ifndef PCF8574_BUTTONS_H
#define PCF8574_BUTTONS_H

#include <Arduino.h>
#include <Wire.h>

enum Pcf8574ButtonEvent : uint8_t {
  PCF8574_BUTTON_EVENT_NONE = 0,
  PCF8574_BUTTON_EVENT_MENU = 1 << 0,
  PCF8574_BUTTON_EVENT_POWER_SHORT = 1 << 1,
  PCF8574_BUTTON_EVENT_POWER_LONG = 1 << 2
};

class Pcf8574Buttons {
public:
  Pcf8574Buttons(uint8_t address, uint8_t sdaPin, uint8_t sclPin, TwoWire& wire = Wire);

  bool begin();
  uint8_t poll();

private:
  struct DebouncedButton {
    bool rawPressed;
    bool stablePressed;
    unsigned long rawChangedAtMs;
    unsigned long pressedAtMs;
    bool longPressReported;
  };

  uint8_t readPort();
  uint8_t updateMenuButton(bool pressedSample, unsigned long nowMs);
  uint8_t updatePowerButton(bool pressedSample, unsigned long nowMs);

  TwoWire* wire;
  uint8_t address;
  uint8_t sdaPin;
  uint8_t sclPin;
  bool initialized;
  DebouncedButton powerButton;
  DebouncedButton menuButton;

  static const unsigned long DEBOUNCE_MS = 35;
  static const unsigned long POWER_LONG_PRESS_MS = 1200;
};

#endif
