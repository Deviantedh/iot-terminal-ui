#include "ServerApiService.h"

#include <ESP8266HTTPClient.h>
#include <ctype.h>
#include <string.h>

static const uint8_t REQUEST_BIT_AUTH = 1 << 0;
static const uint8_t REQUEST_BIT_BALANCE = 1 << 1;
static const uint8_t REQUEST_BIT_SPIN_3 = 1 << 2;
static const uint8_t REQUEST_BIT_SPIN_5 = 1 << 3;
static const uint8_t REQUEST_BIT_HEALTH = 1 << 4;
static const uint8_t REQUEST_BIT_EVENT = 1 << 5;
static const uint8_t REQUEST_BIT_ACCOUNTS = 1 << 6;

ServerApiService::ServerApiService()
  : config(SERVER_API_DEFAULT_CONFIG),
    state(STATE_IDLE),
    currentRequest(SERVER_API_REQUEST_NONE),
    currentLastStatus(SERVER_API_LAST_IDLE),
    pendingMask(0),
    pendingSpinBet3(0),
    pendingSpinBet5(0),
    pendingSpinClientBalance3(-1),
    pendingSpinClientBalance5(-1),
    isAuthorized(false),
    isServerReachable(false),
    knownBalance(0),
    balanceReady(false),
    serverHealthKnown(false),
    serverHealthOk(false),
    accountListReady(false),
    accountsCount(0),
    cachedSpin3Ready(false),
    cachedSpin5Ready(false) {
  deviceId[0] = '\0';
  authToken[0] = '\0';
  strncpy(authUserName, "PLAYER", sizeof(authUserName) - 1);
  authUserName[sizeof(authUserName) - 1] = '\0';
  selectedAccountIdValue[0] = '\0';
  selectedAccountNameValue[0] = '\0';
  pendingEvent.eventType[0] = '\0';
  pendingEvent.message[0] = '\0';
  pendingEvent.value = 0;
  for (uint8_t i = 0; i < SERVER_API_MAX_ACCOUNTS; i++) {
    accounts[i].accountId[0] = '\0';
    accounts[i].displayName[0] = '\0';
    accounts[i].balance = 0;
    accounts[i].balanceKnown = false;
  }
}

void ServerApiService::begin(const ServerApiConfig& cfg) {
  config = cfg;
  client.setTimeout(config.connectTimeoutMs);
}

void ServerApiService::setDeviceId(const char* value) {
  if (value == nullptr) {
    value = "";
  }
  strncpy(deviceId, value, sizeof(deviceId) - 1);
  deviceId[sizeof(deviceId) - 1] = '\0';
}

void ServerApiService::setMode(ServerApiMode value) {
  config.mode = value;
  if (value == SERVER_API_MODE_STUB_ONLY) {
    pendingMask = 0;
    failRequest(SERVER_API_LAST_DISABLED);
  }
}

void ServerApiService::tick(bool wifiConnected, unsigned long nowMs, bool allowRequestStart) {
  if (config.mode == SERVER_API_MODE_STUB_ONLY) {
    return;
  }

  if (!wifiConnected) {
    if (state != STATE_IDLE) {
      failRequest(SERVER_API_LAST_OFFLINE);
    }
    isServerReachable = false;
    serverHealthOk = false;
    return;
  }

  if (allowRequestStart && pendingMask != 0) {
    startNextRequest(nowMs);
  }
}

void ServerApiService::resetSession() {
  isAuthorized = false;
  authToken[0] = '\0';
  strncpy(authUserName, "PLAYER", sizeof(authUserName) - 1);
  authUserName[sizeof(authUserName) - 1] = '\0';
  isServerReachable = false;
  serverHealthKnown = false;
  serverHealthOk = false;
  pendingMask = 0;
  balanceReady = false;
  cachedSpin3Ready = false;
  cachedSpin5Ready = false;
  client.stop();
  clearActiveRequest();
  currentLastStatus = SERVER_API_LAST_IDLE;
}

bool ServerApiService::requestAuth() {
  if (isAuthorized || selectedAccountIdValue[0] == '\0') {
    return true;
  }
  return queueRequest(SERVER_API_REQUEST_AUTH);
}

bool ServerApiService::requestAccounts() {
  return queueRequest(SERVER_API_REQUEST_ACCOUNTS);
}

bool ServerApiService::requestBalance() {
  return queueRequest(SERVER_API_REQUEST_BALANCE);
}

bool ServerApiService::requestHealth() {
  return queueRequest(SERVER_API_REQUEST_HEALTH);
}

void ServerApiService::cancelBackgroundRequests() {
  pendingMask &= (uint8_t)~(REQUEST_BIT_AUTH | REQUEST_BIT_BALANCE | REQUEST_BIT_HEALTH |
                            REQUEST_BIT_EVENT | REQUEST_BIT_ACCOUNTS);
  if (currentRequest == SERVER_API_REQUEST_ACCOUNTS ||
      currentRequest == SERVER_API_REQUEST_AUTH ||
      currentRequest == SERVER_API_REQUEST_BALANCE ||
      currentRequest == SERVER_API_REQUEST_HEALTH ||
      currentRequest == SERVER_API_REQUEST_EVENT) {
    client.stop();
    clearActiveRequest();
  }
}

void ServerApiService::prepareForUserSpinRequest() {
  cancelBackgroundRequests();
  currentLastStatus = SERVER_API_LAST_IDLE;
}

bool ServerApiService::requestSpinResult(SlotMode mode, uint16_t betValue, int clientBalance) {
  cancelBackgroundRequests();
  if (state != STATE_IDLE || currentRequest != SERVER_API_REQUEST_NONE || pendingMask != 0) {
    currentLastStatus = SERVER_API_LAST_IN_FLIGHT;
    return false;
  }

  if (mode == SLOT_MODE_5) {
    pendingSpinBet5 = betValue;
    pendingSpinClientBalance5 = clientBalance;
    return queueRequest(SERVER_API_REQUEST_SPIN_5);
  }
  pendingSpinBet3 = betValue;
  pendingSpinClientBalance3 = clientBalance;
  return queueRequest(SERVER_API_REQUEST_SPIN_3);
}

bool ServerApiService::postEvent(const char* eventType, int32_t value, const char* message) {
  if (eventType == nullptr || eventType[0] == '\0') {
    return false;
  }
  strncpy(pendingEvent.eventType, eventType, sizeof(pendingEvent.eventType) - 1);
  pendingEvent.eventType[sizeof(pendingEvent.eventType) - 1] = '\0';
  pendingEvent.value = value;
  pendingEvent.message[0] = '\0';
  if (message != nullptr) {
    strncpy(pendingEvent.message, message, sizeof(pendingEvent.message) - 1);
    pendingEvent.message[sizeof(pendingEvent.message) - 1] = '\0';
  }
  return queueRequest(SERVER_API_REQUEST_EVENT);
}

bool ServerApiService::consumeBalance(int& outBalance) {
  if (!balanceReady) {
    return false;
  }
  outBalance = knownBalance;
  balanceReady = false;
  return true;
}

bool ServerApiService::consumeSpinResult(SlotMode mode, SlotSpinResult& outResult) {
  if (mode == SLOT_MODE_5) {
    if (!cachedSpin5Ready) {
      return false;
    }
    outResult = cachedSpin5;
    cachedSpin5Ready = false;
    return true;
  }

  if (!cachedSpin3Ready) {
    return false;
  }
  outResult = cachedSpin3;
  cachedSpin3Ready = false;
  return true;
}

bool ServerApiService::serverReachable() const {
  return isServerReachable;
}

bool ServerApiService::authorized() const {
  return isAuthorized;
}

bool ServerApiService::busy() const {
  return state != STATE_IDLE ||
         currentRequest != SERVER_API_REQUEST_NONE ||
         pendingMask != 0;
}

bool ServerApiService::accountsReady() const {
  return accountListReady;
}

uint8_t ServerApiService::accountCount() const {
  return accountsCount;
}

const ServerApiAccountInfo* ServerApiService::accountAt(uint8_t index) const {
  if (index >= accountsCount) {
    return nullptr;
  }
  return &accounts[index];
}

bool ServerApiService::hasSelectedAccount() const {
  return selectedAccountIdValue[0] != '\0';
}

const char* ServerApiService::selectedAccountId() const {
  return selectedAccountIdValue;
}

const char* ServerApiService::selectedAccountName() const {
  if (isAuthorized && authUserName[0] != '\0') {
    return authUserName;
  }
  return selectedAccountNameValue;
}

void ServerApiService::selectAccount(const char* accountId, const char* displayName) {
  if (accountId == nullptr) {
    accountId = "";
  }
  strncpy(selectedAccountIdValue, accountId, sizeof(selectedAccountIdValue) - 1);
  selectedAccountIdValue[sizeof(selectedAccountIdValue) - 1] = '\0';
  selectedAccountNameValue[0] = '\0';
  if (displayName != nullptr) {
    strncpy(selectedAccountNameValue, displayName, sizeof(selectedAccountNameValue) - 1);
    selectedAccountNameValue[sizeof(selectedAccountNameValue) - 1] = '\0';
  }
  isAuthorized = false;
  authToken[0] = '\0';
  strncpy(authUserName, selectedAccountNameValue, sizeof(authUserName) - 1);
  authUserName[sizeof(authUserName) - 1] = '\0';
  balanceReady = false;
}

bool ServerApiService::hasSessionToken() const {
  return authToken[0] != '\0';
}

const char* ServerApiService::sessionToken() const {
  return authToken;
}

const char* ServerApiService::userName() const {
  return authUserName;
}

ServerApiLastStatus ServerApiService::lastStatus() const {
  return currentLastStatus;
}

ServerApiRequestKind ServerApiService::activeRequest() const {
  return currentRequest;
}

int ServerApiService::lastKnownBalance() const {
  return knownBalance;
}

bool ServerApiService::healthKnown() const {
  return serverHealthKnown;
}

bool ServerApiService::healthOk() const {
  return serverHealthKnown && serverHealthOk;
}

bool ServerApiService::spinRequestPending(SlotMode mode) const {
  if (mode == SLOT_MODE_5) {
    return (pendingMask & REQUEST_BIT_SPIN_5) != 0 ||
           currentRequest == SERVER_API_REQUEST_SPIN_5;
  }
  return (pendingMask & REQUEST_BIT_SPIN_3) != 0 ||
         currentRequest == SERVER_API_REQUEST_SPIN_3;
}

bool ServerApiService::queueRequest(ServerApiRequestKind kind) {
  if (config.mode == SERVER_API_MODE_STUB_ONLY) {
    currentLastStatus = SERVER_API_LAST_DISABLED;
    return false;
  }

  if (state != STATE_IDLE || currentRequest != SERVER_API_REQUEST_NONE || pendingMask != 0) {
    currentLastStatus = SERVER_API_LAST_IN_FLIGHT;
    return false;
  }

  uint8_t bit = 0;
  switch (kind) {
    case SERVER_API_REQUEST_ACCOUNTS:
      bit = REQUEST_BIT_ACCOUNTS;
      break;
    case SERVER_API_REQUEST_AUTH:
      bit = REQUEST_BIT_AUTH;
      break;
    case SERVER_API_REQUEST_BALANCE:
      bit = REQUEST_BIT_BALANCE;
      break;
    case SERVER_API_REQUEST_SPIN_3:
      bit = REQUEST_BIT_SPIN_3;
      break;
    case SERVER_API_REQUEST_SPIN_5:
      bit = REQUEST_BIT_SPIN_5;
      break;
    case SERVER_API_REQUEST_HEALTH:
      bit = REQUEST_BIT_HEALTH;
      break;
    case SERVER_API_REQUEST_EVENT:
      bit = REQUEST_BIT_EVENT;
      break;
    default:
      return false;
  }

  pendingMask = bit;
  currentLastStatus = SERVER_API_LAST_QUEUED;
  return true;
}

bool ServerApiService::startNextRequest(unsigned long nowMs) {
  if (pendingMask & REQUEST_BIT_SPIN_3) {
    return startRequest(SERVER_API_REQUEST_SPIN_3, nowMs);
  }
  if (pendingMask & REQUEST_BIT_SPIN_5) {
    return startRequest(SERVER_API_REQUEST_SPIN_5, nowMs);
  }
  if (pendingMask & REQUEST_BIT_ACCOUNTS) {
    return startRequest(SERVER_API_REQUEST_ACCOUNTS, nowMs);
  }
  if ((pendingMask & REQUEST_BIT_AUTH) && !isAuthorized) {
    return startRequest(SERVER_API_REQUEST_AUTH, nowMs);
  }
  if (pendingMask & REQUEST_BIT_BALANCE) {
    return startRequest(SERVER_API_REQUEST_BALANCE, nowMs);
  }
  if (pendingMask & REQUEST_BIT_HEALTH) {
    return startRequest(SERVER_API_REQUEST_HEALTH, nowMs);
  }
  if (pendingMask & REQUEST_BIT_EVENT) {
    return startRequest(SERVER_API_REQUEST_EVENT, nowMs);
  }
  pendingMask &= (uint8_t)~REQUEST_BIT_AUTH;
  return false;
}

bool ServerApiService::startRequest(ServerApiRequestKind kind, unsigned long nowMs) {
  (void)nowMs;
  client.stop();
  currentRequest = kind;
  currentLastStatus = SERVER_API_LAST_IN_FLIGHT;
  state = STATE_READING;

  client.setTimeout(config.connectTimeoutMs);
  client.setNoDelay(true);

  HTTPClient http;
  http.setReuse(false);
  http.setTimeout(config.responseTimeoutMs);
  http.useHTTP10(true);

  const String url = requestUrl(kind);
  if (!http.begin(client, url)) {
    http.end();
    failRequest(SERVER_API_LAST_CONNECT_ERROR);
    return false;
  }

  http.addHeader("Accept", "application/json");
  if (hasSessionToken()) {
    String bearer = "Bearer ";
    bearer += authToken;
    http.addHeader("Authorization", bearer);
  }

  int httpStatus = -1;
  if (requestIsPost(kind)) {
    http.addHeader("Content-Type", "application/json");
    httpStatus = http.POST(requestBody(kind));
  } else {
    httpStatus = http.GET();
  }

  const String body = (httpStatus > 0) ? http.getString() : String();
  http.end();

  const bool handled = (httpStatus > 0) && handleBody(kind, httpStatus, body);
  if (handled) {
    currentLastStatus = SERVER_API_LAST_SUCCESS;
    isServerReachable = true;
  } else if (httpStatus <= 0) {
    currentLastStatus = SERVER_API_LAST_CONNECT_ERROR;
    isServerReachable = false;
  } else if (httpStatus < 200 || httpStatus >= 300) {
    currentLastStatus = SERVER_API_LAST_HTTP_ERROR;
    isServerReachable = false;
  } else {
    currentLastStatus = SERVER_API_LAST_PARSE_ERROR;
    isServerReachable = false;
  }

  clearActiveRequest();
  return handled;
}

void ServerApiService::failRequest(ServerApiLastStatus status) {
  client.stop();
  if (currentRequest == SERVER_API_REQUEST_HEALTH) {
    serverHealthKnown = true;
    serverHealthOk = false;
  }
  currentLastStatus = status;
  if (status != SERVER_API_LAST_DISABLED) {
    isServerReachable = false;
  }
  clearActiveRequest();
}

void ServerApiService::clearActiveRequest() {
  switch (currentRequest) {
    case SERVER_API_REQUEST_ACCOUNTS:
      pendingMask &= (uint8_t)~REQUEST_BIT_ACCOUNTS;
      break;
    case SERVER_API_REQUEST_AUTH:
      pendingMask &= (uint8_t)~REQUEST_BIT_AUTH;
      break;
    case SERVER_API_REQUEST_BALANCE:
      pendingMask &= (uint8_t)~REQUEST_BIT_BALANCE;
      break;
    case SERVER_API_REQUEST_SPIN_3:
      pendingMask &= (uint8_t)~REQUEST_BIT_SPIN_3;
      break;
    case SERVER_API_REQUEST_SPIN_5:
      pendingMask &= (uint8_t)~REQUEST_BIT_SPIN_5;
      break;
    case SERVER_API_REQUEST_HEALTH:
      pendingMask &= (uint8_t)~REQUEST_BIT_HEALTH;
      break;
    case SERVER_API_REQUEST_EVENT:
      pendingMask &= (uint8_t)~REQUEST_BIT_EVENT;
      break;
    default:
      break;
  }

  currentRequest = SERVER_API_REQUEST_NONE;
  state = STATE_IDLE;
  client.stop();
}

String ServerApiService::requestUrl(ServerApiRequestKind kind) const {
  String url = "http://";
  url += config.host;
  url += ":";
  url += config.port;
  url += requestPath(kind);
  return url;
}

String ServerApiService::requestPath(ServerApiRequestKind kind) const {
  switch (kind) {
    case SERVER_API_REQUEST_ACCOUNTS: {
      String path = "/api/v1/devices/";
      path += deviceId;
      path += "/accounts";
      return path;
    }
    case SERVER_API_REQUEST_AUTH:
      return "/api/v1/devices/auth";
    case SERVER_API_REQUEST_BALANCE: {
      String path = "/api/v1/devices/";
      path += deviceId;
      path += "/balance";
      return path;
    }
    case SERVER_API_REQUEST_SPIN_3:
      return "/api/v1/games/slot/3-reels/spin";
    case SERVER_API_REQUEST_SPIN_5:
      return "/api/v1/games/slot/5-reels/spin";
    case SERVER_API_REQUEST_HEALTH:
      return "/api/v1/health";
    case SERVER_API_REQUEST_EVENT:
      return "/api/v1/devices/events";
    default:
      return "/api/v1/health";
  }
}

String ServerApiService::requestBody(ServerApiRequestKind kind) const {
  switch (kind) {
    case SERVER_API_REQUEST_AUTH:
      return authBody();
    case SERVER_API_REQUEST_SPIN_3:
      return spinBody(SLOT_MODE_3, pendingSpinBet3, pendingSpinClientBalance3);
    case SERVER_API_REQUEST_SPIN_5:
      return spinBody(SLOT_MODE_5, pendingSpinBet5, pendingSpinClientBalance5);
    case SERVER_API_REQUEST_EVENT:
      return eventBody();
    default:
      return "";
  }
}

bool ServerApiService::requestIsPost(ServerApiRequestKind kind) const {
  return kind == SERVER_API_REQUEST_AUTH ||
         kind == SERVER_API_REQUEST_SPIN_3 ||
         kind == SERVER_API_REQUEST_SPIN_5 ||
         kind == SERVER_API_REQUEST_EVENT;
}

String ServerApiService::authBody() const {
  String body = "{\"deviceId\":";
  body += jsonString(deviceId);
  body += ",\"accountId\":";
  body += jsonString(selectedAccountIdValue);
  body += ",\"firmwareVersion\":\"iot_terminal_ui\",\"capabilities\":[\"slot3\",\"slot5\"]}";
  return body;
}

String ServerApiService::spinBody(SlotMode mode, uint16_t betValue, int clientBalance) const {
  String body = "{\"deviceId\":";
  body += jsonString(deviceId);
  body += ",\"sessionToken\":";
  body += jsonString(authToken);
  body += ",\"mode\":\"";
  body += (mode == SLOT_MODE_5) ? "5-reels" : "3-reels";
  body += "\",\"bet\":";
  body += betValue;
  if (clientBalance >= 0) {
    body += ",\"clientBalance\":";
    body += clientBalance;
  }
  body += "}";
  return body;
}

String ServerApiService::eventBody() const {
  String body = "{\"deviceId\":";
  body += jsonString(deviceId);
  body += ",\"sessionToken\":";
  body += jsonString(authToken);
  body += ",\"eventType\":";
  body += jsonString(pendingEvent.eventType);
  body += ",\"value\":";
  body += pendingEvent.value;
  body += ",\"message\":";
  body += jsonString(pendingEvent.message);
  body += "}";
  return body;
}

String ServerApiService::jsonString(const char* value) const {
  String out = "\"";
  if (value != nullptr) {
    for (const char* p = value; *p != '\0'; p++) {
      if (*p == '"' || *p == '\\') {
        out += '\\';
      }
      out += *p;
    }
  }
  out += "\"";
  return out;
}

bool ServerApiService::handleBody(ServerApiRequestKind kind, int httpStatus, const String& body) {
  if (httpStatus < 200 || httpStatus >= 300 || !responseLooksOk(body)) {
    return false;
  }

  switch (kind) {
    case SERVER_API_REQUEST_ACCOUNTS: {
      uint8_t parsedCount = 0;
      if (!extractAccounts(body, accounts, SERVER_API_MAX_ACCOUNTS, parsedCount)) {
        return false;
      }
      accountsCount = parsedCount;
      accountListReady = true;
      bool selectedStillPresent = false;
      const ServerApiAccountInfo* firstAccount = (accountsCount > 0) ? &accounts[0] : nullptr;
      for (uint8_t i = 0; i < accountsCount; i++) {
        if (strcmp(accounts[i].accountId, selectedAccountIdValue) == 0) {
          selectedStillPresent = true;
          strncpy(selectedAccountNameValue, accounts[i].displayName, sizeof(selectedAccountNameValue) - 1);
          selectedAccountNameValue[sizeof(selectedAccountNameValue) - 1] = '\0';
          break;
        }
      }
      if (!selectedStillPresent) {
        selectedAccountIdValue[0] = '\0';
        selectedAccountNameValue[0] = '\0';
        isAuthorized = false;
        authToken[0] = '\0';
        authUserName[0] = '\0';
      }
      if (selectedAccountIdValue[0] == '\0' && firstAccount != nullptr && accountsCount == 1) {
        selectAccount(firstAccount->accountId, firstAccount->displayName);
      }
      return true;
    }
    case SERVER_API_REQUEST_AUTH: {
      bool authorizedValue = false;
      if (!extractBool(body, "authorized", authorizedValue) || !authorizedValue) {
        return false;
      }
      isAuthorized = true;
      extractString(body, "token", authToken, sizeof(authToken));
      if (!extractString(body, "userName", authUserName, sizeof(authUserName))) {
        extractString(body, "username", authUserName, sizeof(authUserName));
      }
      if (authUserName[0] != '\0') {
        strncpy(selectedAccountNameValue, authUserName, sizeof(selectedAccountNameValue) - 1);
        selectedAccountNameValue[sizeof(selectedAccountNameValue) - 1] = '\0';
      }
      return true;
    }
    case SERVER_API_REQUEST_BALANCE: {
      int balance = 0;
      if (!extractInt(body, "balance", balance)) {
        return false;
      }
      knownBalance = balance;
      balanceReady = true;
      return true;
    }
    case SERVER_API_REQUEST_SPIN_3: {
      SlotSpinResult parsed;
      if (!extractSpinSymbols(body, SLOT_MODE_3, parsed)) {
        return false;
      }
      int balance = 0;
      if (extractInt(body, "balance", balance)) {
        knownBalance = balance;
        balanceReady = true;
      }
      cachedSpin3 = parsed;
      cachedSpin3Ready = true;
      return true;
    }
    case SERVER_API_REQUEST_SPIN_5: {
      SlotSpinResult parsed;
      if (!extractSpinSymbols(body, SLOT_MODE_5, parsed)) {
        return false;
      }
      int balance = 0;
      if (extractInt(body, "balance", balance)) {
        knownBalance = balance;
        balanceReady = true;
      }
      cachedSpin5 = parsed;
      cachedSpin5Ready = true;
      return true;
    }
    case SERVER_API_REQUEST_HEALTH:
      serverHealthKnown = true;
      serverHealthOk = true;
      return true;
    case SERVER_API_REQUEST_EVENT:
      return true;
    default:
      return false;
  }
}

bool ServerApiService::extractBool(const String& body, const char* key, bool& out) {
  String needle = "\"";
  needle += key;
  needle += "\"";
  int pos = body.indexOf(needle);
  if (pos < 0) {
    return false;
  }
  int colon = body.indexOf(':', pos + needle.length());
  if (colon < 0) {
    return false;
  }
  int valueStart = colon + 1;
  while (valueStart < (int)body.length() && isspace(body[valueStart])) {
    valueStart++;
  }
  if (body.substring(valueStart, valueStart + 4) == "true") {
    out = true;
    return true;
  }
  if (body.substring(valueStart, valueStart + 5) == "false") {
    out = false;
    return true;
  }
  return false;
}

bool ServerApiService::extractInt(const String& body, const char* key, int& out) {
  String needle = "\"";
  needle += key;
  needle += "\"";
  int pos = body.indexOf(needle);
  if (pos < 0) {
    return false;
  }
  int colon = body.indexOf(':', pos + needle.length());
  if (colon < 0) {
    return false;
  }
  int valueStart = colon + 1;
  while (valueStart < (int)body.length() && isspace(body[valueStart])) {
    valueStart++;
  }
  int valueEnd = valueStart;
  if (valueEnd < (int)body.length() && body[valueEnd] == '-') {
    valueEnd++;
  }
  while (valueEnd < (int)body.length() && isdigit(body[valueEnd])) {
    valueEnd++;
  }
  if (valueEnd == valueStart) {
    return false;
  }
  out = body.substring(valueStart, valueEnd).toInt();
  return true;
}

bool ServerApiService::extractAccounts(const String& body, ServerApiAccountInfo* out, uint8_t capacity, uint8_t& outCount) {
  outCount = 0;
  int accountsPos = body.indexOf("\"accounts\"");
  if (accountsPos < 0) {
    return false;
  }
  int arrayStart = body.indexOf('[', accountsPos);
  if (arrayStart < 0) {
    return false;
  }
  int cursor = arrayStart + 1;
  while (cursor < (int)body.length() && outCount < capacity) {
    int objectStart = body.indexOf('{', cursor);
    if (objectStart < 0) {
      break;
    }
    int objectEnd = body.indexOf('}', objectStart);
    if (objectEnd < 0) {
      return false;
    }
    String item = body.substring(objectStart, objectEnd + 1);
    if (extractString(item, "accountId", out[outCount].accountId, sizeof(out[outCount].accountId))) {
      if (!extractString(item, "displayName", out[outCount].displayName, sizeof(out[outCount].displayName))) {
        extractString(item, "userName", out[outCount].displayName, sizeof(out[outCount].displayName));
      }
      out[outCount].balanceKnown = extractInt(item, "balance", out[outCount].balance);
      outCount++;
    }
    cursor = objectEnd + 1;
    int arrayEnd = body.indexOf(']', cursor);
    if (arrayEnd >= 0 && cursor > arrayEnd) {
      break;
    }
  }
  return true;
}

bool ServerApiService::extractString(const String& body, const char* key, char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return false;
  }
  out[0] = '\0';
  String needle = "\"";
  needle += key;
  needle += "\"";
  int pos = body.indexOf(needle);
  if (pos < 0) {
    return false;
  }
  int colon = body.indexOf(':', pos + needle.length());
  int quoteStart = body.indexOf('"', colon + 1);
  if (colon < 0 || quoteStart < 0) {
    return false;
  }
  int quoteEnd = quoteStart + 1;
  size_t outLen = 0;
  while (quoteEnd < (int)body.length()) {
    char c = body[quoteEnd++];
    if (c == '\\' && quoteEnd < (int)body.length()) {
      c = body[quoteEnd++];
    } else if (c == '"') {
      out[outLen] = '\0';
      return true;
    }
    if (outLen + 1 < outSize) {
      out[outLen++] = c;
    }
  }
  out[outLen] = '\0';
  return false;
}

bool ServerApiService::extractSpinSymbols(const String& body, SlotMode mode, SlotSpinResult& out) {
  memset(&out, 0, sizeof(out));
  out.mode = mode;
  out.reelCount = slotModeReelCount(mode);
  out.rowCount = slotModeRowCount(mode);

  int pos = body.indexOf("\"symbols\"");
  if (pos < 0) {
    return false;
  }
  int arrayStart = body.indexOf('[', pos);
  if (arrayStart < 0) {
    return false;
  }

  const uint8_t expectedCount = out.reelCount * out.rowCount;
  uint8_t count = 0;
  int i = arrayStart + 1;
  while (i < (int)body.length() && count < expectedCount) {
    while (i < (int)body.length() && !isdigit(body[i]) && body[i] != '-') {
      i++;
    }
    if (i >= (int)body.length()) {
      break;
    }
    int end = i;
    if (body[end] == '-') {
      end++;
    }
    while (end < (int)body.length() && isdigit(body[end])) {
      end++;
    }
    const int value = body.substring(i, end).toInt();
    if (value < 0 || value >= SLOT_SYMBOL_COUNT) {
      return false;
    }
    const uint8_t row = count / out.reelCount;
    const uint8_t reel = count % out.reelCount;
    out.symbols[row][reel] = (SlotSymbol)value;
    count++;
    i = end;
  }

  return count == expectedCount;
}

bool ServerApiService::responseLooksOk(const String& body) {
  String compact = body;
  compact.replace(" ", "");
  compact.replace("\n", "");
  compact.replace("\r", "");
  compact.replace("\t", "");
  return compact.indexOf("\"status\":\"ok\"") >= 0 ||
         compact.indexOf("\"status\":\"success\"") >= 0;
}
