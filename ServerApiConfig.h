#ifndef SERVER_API_CONFIG_H
#define SERVER_API_CONFIG_H

#include <Arduino.h>

enum ServerApiMode : uint8_t {
  SERVER_API_MODE_STUB_ONLY = 0,
  SERVER_API_MODE_ONLINE_WITH_STUB_FALLBACK,
  SERVER_API_MODE_SERVER_ONLY
};

struct ServerApiConfig {
  const char* host;
  uint16_t port;
  ServerApiMode mode;
  uint16_t connectTimeoutMs;
  uint16_t responseTimeoutMs;
  uint16_t minRequestIntervalMs;
  uint16_t failureBackoffMs;
};

static const bool SERVER_API_NETWORK_ENABLED = true;
static const bool SERVER_API_REQUIRE_SERVER_FOR_SPIN = true;

static const ServerApiConfig SERVER_API_DEFAULT_CONFIG = {
  "62.113.118.76",
  8080,
  SERVER_API_NETWORK_ENABLED
    ? (SERVER_API_REQUIRE_SERVER_FOR_SPIN
        ? SERVER_API_MODE_SERVER_ONLY
        : SERVER_API_MODE_ONLINE_WITH_STUB_FALLBACK)
    : SERVER_API_MODE_STUB_ONLY,
  2500,
  3000,
  20,
  1200
};

#endif
