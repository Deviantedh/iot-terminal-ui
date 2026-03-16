#include "Pcf8574Buttons.h"

Pcf8574Buttons::Pcf8574Buttons(uint8_t addressValue, uint8_t sdaPinValue, uint8_t sclPinValue, TwoWire& wireRef)
  : wire(&wireRef),
    address(addressValue),
    sdaPin(sdaPinValue),
    sclPin(sclPinValue),
    initialized(false),
    powerButton{false, false, 0, 0, false},
    menuButton{false, false, 0, 0, false} {
}

bool Pcf8574Buttons::begin() {
  wire->begin(sdaPin, sclPin);
  wire->beginTransmission(address);
  wire->write(0xFF);
  initialized = (wire->endTransmission() == 0);

  if (!initialized) {
    return false;
  }

  const uint8_t port = readPort();
  const bool powerPressed = ((port & (1 << 0)) == 0);
  const bool menuPressed = ((port & (1 << 1)) == 0);
  const unsigned long nowMs = millis();

  powerButton.rawPressed = powerPressed;
  powerButton.stablePressed = powerPressed;
  powerButton.rawChangedAtMs = nowMs;
  powerButton.pressedAtMs = powerPressed ? nowMs : 0;
  powerButton.longPressReported = false;

  menuButton.rawPressed = menuPressed;
  menuButton.stablePressed = menuPressed;
  menuButton.rawChangedAtMs = nowMs;
  menuButton.pressedAtMs = menuPressed ? nowMs : 0;
  menuButton.longPressReported = false;
  return true;
}

uint8_t Pcf8574Buttons::poll() {
  if (!initialized) {
    return PCF8574_BUTTON_EVENT_NONE;
  }

  const uint8_t port = readPort();
  const unsigned long nowMs = millis();
  uint8_t events = PCF8574_BUTTON_EVENT_NONE;

  events |= updatePowerButton(((port & (1 << 0)) == 0), nowMs);
  events |= updateMenuButton(((port & (1 << 1)) == 0), nowMs);
  return events;
}

uint8_t Pcf8574Buttons::readPort() {
  wire->requestFrom((int)address, 1);
  if (wire->available() <= 0) {
    return 0xFF;
  }
  return wire->read();
}

uint8_t Pcf8574Buttons::updateMenuButton(bool pressedSample, unsigned long nowMs) {
  if (pressedSample != menuButton.rawPressed) {
    menuButton.rawPressed = pressedSample;
    menuButton.rawChangedAtMs = nowMs;
    return PCF8574_BUTTON_EVENT_NONE;
  }

  if (menuButton.stablePressed == menuButton.rawPressed) {
    return PCF8574_BUTTON_EVENT_NONE;
  }

  if (nowMs - menuButton.rawChangedAtMs < DEBOUNCE_MS) {
    return PCF8574_BUTTON_EVENT_NONE;
  }

  menuButton.stablePressed = menuButton.rawPressed;
  return menuButton.stablePressed ? PCF8574_BUTTON_EVENT_MENU : PCF8574_BUTTON_EVENT_NONE;
}

uint8_t Pcf8574Buttons::updatePowerButton(bool pressedSample, unsigned long nowMs) {
  if (pressedSample != powerButton.rawPressed) {
    powerButton.rawPressed = pressedSample;
    powerButton.rawChangedAtMs = nowMs;
    return PCF8574_BUTTON_EVENT_NONE;
  }

  if (powerButton.stablePressed != powerButton.rawPressed) {
    if (nowMs - powerButton.rawChangedAtMs < DEBOUNCE_MS) {
      return PCF8574_BUTTON_EVENT_NONE;
    }

    powerButton.stablePressed = powerButton.rawPressed;
    if (powerButton.stablePressed) {
      powerButton.pressedAtMs = nowMs;
      powerButton.longPressReported = false;
      return PCF8574_BUTTON_EVENT_NONE;
    }

    const bool wasLongPress = powerButton.longPressReported;
    powerButton.pressedAtMs = 0;
    powerButton.longPressReported = false;
    return wasLongPress ? PCF8574_BUTTON_EVENT_NONE : PCF8574_BUTTON_EVENT_POWER_SHORT;
  }

  if (powerButton.stablePressed &&
      !powerButton.longPressReported &&
      powerButton.pressedAtMs != 0 &&
      nowMs - powerButton.pressedAtMs >= POWER_LONG_PRESS_MS) {
    powerButton.longPressReported = true;
    return PCF8574_BUTTON_EVENT_POWER_LONG;
  }

  return PCF8574_BUTTON_EVENT_NONE;
}
