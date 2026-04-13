#ifndef SERVER_API_SERVICE_H
#define SERVER_API_SERVICE_H

#include <Arduino.h>
#include <WiFiClient.h>
#include "ServerApiConfig.h"
#include "ServerApiModels.h"

class ServerApiService {
public:
  ServerApiService();

  void begin(const ServerApiConfig& cfg);
  void setDeviceId(const char* value);
  void setMode(ServerApiMode value);
  void tick(bool wifiConnected, unsigned long nowMs, bool allowRequestStart);
  void resetSession();

  bool requestAuth();
  bool requestBalance();
  bool requestHealth();
  void cancelBackgroundRequests();
  void prepareForUserSpinRequest();
  bool requestSpinResult(SlotMode mode, uint16_t betValue, int clientBalance = -1);
  bool postEvent(const char* eventType, int32_t value, const char* message = nullptr);

  bool consumeBalance(int& outBalance);
  bool consumeSpinResult(SlotMode mode, SlotSpinResult& outResult);

  bool serverReachable() const;
  bool authorized() const;
  bool busy() const;
  bool hasSessionToken() const;
  const char* sessionToken() const;
  const char* userName() const;
  ServerApiLastStatus lastStatus() const;
  ServerApiRequestKind activeRequest() const;
  int lastKnownBalance() const;
  bool healthKnown() const;
  bool healthOk() const;
  bool spinRequestPending(SlotMode mode) const;

private:
  enum InternalState : uint8_t {
    STATE_IDLE = 0,
    STATE_READING
  };

  ServerApiConfig config;
  WiFiClient client;
  InternalState state;
  ServerApiRequestKind currentRequest;
  ServerApiLastStatus currentLastStatus;
  uint8_t pendingMask;
  uint16_t pendingSpinBet3;
  uint16_t pendingSpinBet5;
  int pendingSpinClientBalance3;
  int pendingSpinClientBalance5;
  ServerApiEventRequest pendingEvent;
  char deviceId[25];
  char authToken[65];
  char authUserName[33];
  bool isAuthorized;
  bool isServerReachable;
  int knownBalance;
  bool balanceReady;
  bool serverHealthKnown;
  bool serverHealthOk;
  SlotSpinResult cachedSpin3;
  SlotSpinResult cachedSpin5;
  bool cachedSpin3Ready;
  bool cachedSpin5Ready;

  bool queueRequest(ServerApiRequestKind kind);
  bool startNextRequest(unsigned long nowMs);
  bool startRequest(ServerApiRequestKind kind, unsigned long nowMs);
  void failRequest(ServerApiLastStatus status);
  void clearActiveRequest();

  String requestUrl(ServerApiRequestKind kind) const;
  String requestPath(ServerApiRequestKind kind) const;
  String requestBody(ServerApiRequestKind kind) const;
  bool requestIsPost(ServerApiRequestKind kind) const;
  String authBody() const;
  String spinBody(SlotMode mode, uint16_t betValue, int clientBalance) const;
  String eventBody() const;
  String jsonString(const char* value) const;
  bool handleBody(ServerApiRequestKind kind, int httpStatus, const String& body);

  static bool extractBool(const String& body, const char* key, bool& out);
  static bool extractInt(const String& body, const char* key, int& out);
  static bool extractString(const String& body, const char* key, char* out, size_t outSize);
  static bool extractSpinSymbols(const String& body, SlotMode mode, SlotSpinResult& out);
  static bool responseLooksOk(const String& body);
};

#endif
