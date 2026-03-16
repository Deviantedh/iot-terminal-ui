#include "TimeService.h"

#include <time.h>

static const char* TIME_TZ_INFO = "MSK-3";
static const char* TIME_NTP_SERVER_1 = "pool.ntp.org";
static const char* TIME_NTP_SERVER_2 = "time.nist.gov";
static const unsigned long TIME_SYNC_RETRY_MS = 30000;
static const time_t TIME_VALID_EPOCH = 1700000000;

void TimeService::begin() {
  if (started) {
    return;
  }

  configTzTime(TIME_TZ_INFO, TIME_NTP_SERVER_1, TIME_NTP_SERVER_2);
  started = true;
  lastSyncAttemptMs = millis();
}

void TimeService::tick(bool wifiConnected) {
  if (!started) {
    begin();
  }

  const unsigned long nowMs = millis();
  if (wifiConnected && !timeValid && nowMs - lastSyncAttemptMs >= TIME_SYNC_RETRY_MS) {
    configTzTime(TIME_TZ_INFO, TIME_NTP_SERVER_1, TIME_NTP_SERVER_2);
    lastSyncAttemptMs = nowMs;
  }

  const time_t nowEpoch = time(nullptr);
  const bool validNow = (nowEpoch >= TIME_VALID_EPOCH);
  if (!validNow) {
    return;
  }

  timeValid = true;
  const long minuteKey = (long)(nowEpoch / 60);
  if (minuteKey != lastMinuteKey) {
    lastMinuteKey = minuteKey;
    minuteChanged = true;
  }
}

bool TimeService::isTimeValid() const {
  return timeValid;
}

bool TimeService::consumeMinuteChanged() {
  const bool changed = minuteChanged;
  minuteChanged = false;
  return changed;
}

void TimeService::formatTime(char* out, size_t outSize) const {
  if (outSize == 0) {
    return;
  }

  if (!timeValid) {
    strncpy(out, "--:--", outSize - 1);
    out[outSize - 1] = '\0';
    return;
  }

  time_t nowEpoch = time(nullptr);
  struct tm timeInfo;
  localtime_r(&nowEpoch, &timeInfo);
  snprintf(out, outSize, "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);
}
