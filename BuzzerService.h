#ifndef BUZZER_SERVICE_H
#define BUZZER_SERVICE_H

#include <Arduino.h>

class BuzzerService {
public:
  struct ToneStep {
    uint16_t frequency;
    uint16_t durationMs;
    uint16_t gapMs;
  };

  explicit BuzzerService(int8_t pin);

  void begin();
  void tick();
  void setEnabled(bool enabled);
  bool isEnabled() const;
  bool isSupported() const;

  void playClick();
  void playError();
  void playWake();
  void playBootMelody();
  void playTestSong();

private:
  void playTone(uint16_t frequency, uint16_t durationMs);
  void startSequence(const ToneStep* steps, uint16_t count);
  void stopPlayback();
  void startCurrentStep(unsigned long nowMs);

  int8_t pin;
  bool enabled;
  bool active;
  bool sequenceRunning;
  uint16_t sequenceCount;
  uint16_t sequenceIndex;
  unsigned long nextChangeMs;
  const ToneStep* sequence;
  ToneStep oneShotStep;
};

#endif
