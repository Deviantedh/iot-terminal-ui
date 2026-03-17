#include "BuzzerService.h"

BuzzerService::BuzzerService(int8_t pinValue)
  : pin(pinValue),
    enabled(true),
    active(false),
    sequenceRunning(false),
    sequenceCount(0),
    sequenceIndex(0),
    nextChangeMs(0) {
}

void BuzzerService::begin() {
  if (!isSupported()) {
    return;
  }

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void BuzzerService::tick() {
  if (!sequenceRunning) {
    return;
  }

  const unsigned long nowMs = millis();
  if (active) {
    if ((long)(nowMs - nextChangeMs) < 0) {
      return;
    }

    noTone(pin);
    digitalWrite(pin, LOW);
    active = false;

    const uint16_t gapMs = sequence[sequenceIndex].gapMs;
    sequenceIndex++;
    if (sequenceIndex >= sequenceCount) {
      sequenceRunning = false;
      nextChangeMs = 0;
      return;
    }

    nextChangeMs = nowMs + gapMs;
    return;
  }

  if ((long)(nowMs - nextChangeMs) >= 0) {
    startCurrentStep(nowMs);
  }
}

void BuzzerService::setEnabled(bool enabledValue) {
  enabled = enabledValue;
  if (enabled || !isSupported()) {
    return;
  }

  stopPlayback();
}

bool BuzzerService::isEnabled() const {
  return enabled;
}

bool BuzzerService::isSupported() const {
  return pin >= 0;
}

void BuzzerService::playClick() {
  playTone(1320, 10);
}

void BuzzerService::playError() {
  static const ToneStep kErrorSteps[] = {
    {720, 26, 24},
    {620, 44, 0}
  };
  startSequence(kErrorSteps, 2);
}

void BuzzerService::playWake() {
  playTone(1480, 20);
}

void BuzzerService::playBootMelody() {
  static const ToneStep kBootSteps[] = {
    {784, 160, 5}, 
    {659, 160, 5},  
    {523, 320, 8},   
    {600, 320, 5},   
    {698, 320, 20}  
  };

  startSequence(kBootSteps, sizeof(kBootSteps) / sizeof(kBootSteps[0]));
}

void BuzzerService::playTone(uint16_t frequency, uint16_t durationMs) {
  static const ToneStep step = {0, 0, 0};
  if (!enabled || !isSupported()) {
    return;
  }

  ToneStep oneShot = step;
  oneShot.frequency = frequency;
  oneShot.durationMs = durationMs;
  oneShot.gapMs = 0;
  startSequence(&oneShot, 1);
}

void BuzzerService::startSequence(const ToneStep* steps, uint8_t count) {
  if (!enabled || !isSupported() || steps == nullptr || count == 0) {
    return;
  }

  if (count > (sizeof(sequence) / sizeof(sequence[0]))) {
    count = sizeof(sequence) / sizeof(sequence[0]);
  }

  stopPlayback();
  for (uint8_t i = 0; i < count; i++) {
    sequence[i] = steps[i];
  }

  sequenceCount = count;
  sequenceIndex = 0;
  sequenceRunning = true;
  startCurrentStep(millis());
}

void BuzzerService::stopPlayback() {
  if (isSupported()) {
    noTone(pin);
    digitalWrite(pin, LOW);
  }
  active = false;
  sequenceRunning = false;
  sequenceCount = 0;
  sequenceIndex = 0;
  nextChangeMs = 0;
}

void BuzzerService::startCurrentStep(unsigned long nowMs) {
  if (!sequenceRunning || sequenceIndex >= sequenceCount) {
    sequenceRunning = false;
    active = false;
    return;
  }

  const ToneStep& step = sequence[sequenceIndex];
  if (step.frequency == 0 || step.durationMs == 0) {
    sequenceRunning = false;
    active = false;
    return;
  }

  tone(pin, step.frequency, step.durationMs);
  active = true;
  nextChangeMs = nowMs + step.durationMs;
}
