#include "BuzzerService.h"

BuzzerService::BuzzerService(int8_t pinValue)
  : pin(pinValue),
    enabled(true),
    active(false),
    activeUntilMs(0) {
}

void BuzzerService::begin() {
  if (!isSupported()) {
    return;
  }

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void BuzzerService::tick() {
  if (!active) {
    return;
  }

  if ((long)(millis() - activeUntilMs) < 0) {
    return;
  }

  noTone(pin);
  digitalWrite(pin, LOW);
  active = false;
  activeUntilMs = 0;
}

void BuzzerService::setEnabled(bool enabledValue) {
  enabled = enabledValue;
  if (enabled || !isSupported()) {
    return;
  }

  noTone(pin);
  digitalWrite(pin, LOW);
  active = false;
  activeUntilMs = 0;
}

bool BuzzerService::isEnabled() const {
  return enabled;
}

bool BuzzerService::isSupported() const {
  return pin >= 0;
}

void BuzzerService::playClick() {
  playTone(2400, 24);
}

void BuzzerService::playError() {
  playTone(880, 120);
}

void BuzzerService::playWake() {
  playTone(1760, 70);
}

void BuzzerService::playSleep() {
  playTone(1100, 55);
}

void BuzzerService::playTone(uint16_t frequency, uint16_t durationMs) {
  if (!enabled || !isSupported()) {
    return;
  }

  tone(pin, frequency, durationMs);
  active = true;
  activeUntilMs = millis() + durationMs + 8;
}
