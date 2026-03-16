#ifndef BUZZER_SERVICE_H
#define BUZZER_SERVICE_H

#include <Arduino.h>

class BuzzerService {
public:
  explicit BuzzerService(int8_t pin);

  void begin();
  void tick();
  void setEnabled(bool enabled);
  bool isEnabled() const;
  bool isSupported() const;

  void playClick();
  void playError();
  void playWake();
  void playSleep();

private:
  void playTone(uint16_t frequency, uint16_t durationMs);

  int8_t pin;
  bool enabled;
  bool active;
  unsigned long activeUntilMs;
};

#endif
