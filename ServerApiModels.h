#ifndef SERVER_API_MODELS_H
#define SERVER_API_MODELS_H

#include <Arduino.h>
#include "SlotGame.h"

enum ServerApiRequestKind : uint8_t {
  SERVER_API_REQUEST_NONE = 0,
  SERVER_API_REQUEST_ACCOUNTS,
  SERVER_API_REQUEST_AUTH,
  SERVER_API_REQUEST_BALANCE,
  SERVER_API_REQUEST_SPIN_3,
  SERVER_API_REQUEST_SPIN_5,
  SERVER_API_REQUEST_HEALTH,
  SERVER_API_REQUEST_EVENT
};

enum ServerApiLastStatus : uint8_t {
  SERVER_API_LAST_IDLE = 0,
  SERVER_API_LAST_QUEUED,
  SERVER_API_LAST_IN_FLIGHT,
  SERVER_API_LAST_SUCCESS,
  SERVER_API_LAST_TIMEOUT,
  SERVER_API_LAST_CONNECT_ERROR,
  SERVER_API_LAST_HTTP_ERROR,
  SERVER_API_LAST_PARSE_ERROR,
  SERVER_API_LAST_OFFLINE,
  SERVER_API_LAST_DISABLED
};

static const uint8_t SERVER_API_MAX_ACCOUNTS = 8;

struct ServerApiAccountInfo {
  char accountId[25];
  char displayName[33];
  int balance;
  bool balanceKnown;
};

struct ServerApiAuthResponse {
  bool ok;
  bool authorized;
  char token[65];
  char message[49];
};

struct ServerApiBalanceResponse {
  bool ok;
  int balance;
  char message[49];
};

struct ServerApiSpinResponse {
  bool ok;
  SlotSpinResult result;
  char message[49];
};

struct ServerApiEventRequest {
  char eventType[25];
  int32_t value;
  char message[65];
};

#endif
