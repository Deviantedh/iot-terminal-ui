#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <Arduino.h>

class TimeService {
public:
  void begin();
  void tick(bool wifiConnected);
  bool isTimeValid() const;
  bool consumeMinuteChanged();
  void formatTime(char* out, size_t outSize) const;

private:
  bool started = false;
  bool timeValid = false;
  bool minuteChanged = false;
  unsigned long lastSyncAttemptMs = 0;
  long lastMinuteKey = -1;
};

#endif
