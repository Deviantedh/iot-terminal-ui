#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

enum WiFiServiceState : uint8_t {
  WIFI_SERVICE_IDLE = 0,
  WIFI_SERVICE_CONNECTING,
  WIFI_SERVICE_CONNECTED,
  WIFI_SERVICE_ERROR
};

enum WiFiScanState : uint8_t {
  WIFI_SCAN_STATE_IDLE = 0,
  WIFI_SCAN_STATE_RUNNING,
  WIFI_SCAN_STATE_DONE,
  WIFI_SCAN_STATE_NO_RESULTS,
  WIFI_SCAN_STATE_ERROR
};

struct WiFiNetworkInfo {
  char ssid[33];
  int32_t rssi;
  bool secured;
};

class WiFiService {
public:
  WiFiService();

  // Initialize STA mode and snapshot the initial link status.
  void begin();

  // Start a non-blocking connection attempt.
  bool beginConnect(const char* ssid, const char* password);

  // Poll Wi-Fi state machine. Returns true when service state changes.
  bool tick();

  // Explicit user-driven disconnect.
  void disconnect();

  WiFiServiceState state() const;
  wl_status_t lastStatus() const;

  // Start an async scan and later consume results through networkAt().
  bool startScan();
  WiFiScanState scanState() const;
  int networkCount() const;
  const WiFiNetworkInfo* networkAt(int index) const;
  bool consumeScanStateChanged();

private:
  WiFiServiceState currentState;
  wl_status_t currentStatus;
  unsigned long connectStartedAtMs;
  WiFiScanState currentScanState;
  bool scanStateChanged;

  static const int MAX_SCAN_RESULTS = 25;
  WiFiNetworkInfo scanResults[MAX_SCAN_RESULTS];
  int scanResultCount;

  static const unsigned long CONNECT_TIMEOUT_MS = 15000;
};

#endif
