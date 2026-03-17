#include "BuzzerService.h"

BuzzerService::BuzzerService(int8_t pinValue)
  : pin(pinValue),
    enabled(true),
    active(false),
    sequenceRunning(false),
    sequenceCount(0),
    sequenceIndex(0),
    nextChangeMs(0),
    sequence(nullptr),
    oneShotStep{0, 0, 0} {
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

void BuzzerService::playTestSong() {
  static const ToneStep kMelody[] = {
    {44, 125, 5}, {98, 125, 5}, {131, 125, 5}, {44, 125, 5}, {87, 125, 5}, {131, 125, 5}, {44, 125, 5}, {87, 125, 5},
    {55, 125, 5}, {87, 125, 5}, {131, 125, 5}, {55, 125, 5}, {82, 125, 5}, {131, 125, 5}, {55, 125, 5}, {82, 125, 5},
    {33, 125, 5}, {98, 125, 5}, {147, 125, 5}, {33, 125, 5}, {98, 125, 5}, {147, 125, 5}, {33, 125, 5}, {110, 125, 5},
    {37, 125, 5}, {87, 125, 5}, {131, 125, 5}, {37, 125, 5}, {87, 125, 5}, {147, 125, 5}, {37, 125, 5}, {87, 125, 5},
    {44, 125, 5}, {98, 125, 5}, {131, 125, 5}, {44, 125, 5}, {87, 125, 5}, {131, 125, 5}, {44, 125, 5}, {87, 125, 5},
    {55, 125, 5}, {87, 125, 5}, {131, 125, 5}, {55, 125, 5}, {82, 125, 5}, {131, 125, 5}, {55, 125, 5}, {82, 125, 5},
    {33, 125, 5}, {98, 125, 5}, {147, 125, 5}, {33, 125, 5}, {98, 125, 5}, {147, 125, 5}, {33, 125, 5}, {110, 125, 5},
    {37, 125, 5}, {87, 125, 5}, {131, 125, 5}, {37, 125, 5}, {87, 125, 5}, {147, 125, 5}, {37, 125, 5}, {87, 125, 5},
    {131, 125, 5}, {98, 125, 5}, {131, 125, 5}, {41, 125, 5}, {98, 125, 5}, {123, 125, 5}, {41, 125, 5}, {98, 125, 5},
    {131, 125, 5}, {110, 125, 5}, {165, 125, 5}, {44, 125, 5}, {110, 125, 5}, {175, 125, 5}, {44, 125, 5}, {110, 125, 5},
    {110, 125, 5}, {82, 125, 5}, {131, 125, 5}, {55, 125, 5}, {131, 125, 5}, {147, 125, 5}, {55, 125, 5}, {82, 125, 5},
    {49, 125, 5}, {73, 125, 5}, {131, 125, 5}, {49, 125, 5}, {123, 125, 5}, {147, 125, 5}, {49, 125, 5}, {98, 125, 5},
    {131, 125, 5}, {98, 125, 5}, {131, 125, 5}, {41, 125, 5}, {98, 125, 5}, {123, 125, 5}, {41, 125, 5}, {98, 125, 5},
    {123, 125, 5}, {110, 125, 5}, {165, 125, 5}, {44, 125, 5}, {110, 125, 5}, {175, 125, 5}, {44, 125, 5}, {110, 125, 5},
    {110, 125, 5}, {82, 125, 5}, {131, 125, 5}, {55, 125, 5}, {131, 125, 5}, {147, 125, 5}, {55, 125, 5}, {82, 125, 5},
    {49, 125, 5}, {73, 125, 5}, {131, 125, 5}, {49, 125, 5}, {123, 125, 5}, {147, 125, 5}, {49, 125, 5}, {98, 125, 5}
  };

  startSequence(kMelody, sizeof(kMelody) / sizeof(kMelody[0]));
}

void BuzzerService::playTone(uint16_t frequency, uint16_t durationMs) {
  if (!enabled || !isSupported()) {
    return;
  }

  oneShotStep.frequency = frequency;
  oneShotStep.durationMs = durationMs;
  oneShotStep.gapMs = 0;
  startSequence(&oneShotStep, 1);
}

void BuzzerService::startSequence(const ToneStep* steps, uint16_t count) {
  if (!enabled || !isSupported() || steps == nullptr || count == 0) {
    return;
  }

  stopPlayback();
  sequence = steps;
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
  sequence = nullptr;
}

void BuzzerService::startCurrentStep(unsigned long nowMs) {
  if (!sequenceRunning || sequence == nullptr || sequenceIndex >= sequenceCount) {
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
