#include "WiFiService.h"

WiFiService::WiFiService()
  : currentState(WIFI_SERVICE_IDLE),
    currentStatus(WL_IDLE_STATUS),
    connectStartedAtMs(0),
    currentScanState(WIFI_SCAN_STATE_IDLE),
    scanStateChanged(false),
    scanResultCount(0) {
}

void WiFiService::begin() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  currentStatus = WiFi.status();
}

bool WiFiService::beginConnect(const char* ssid, const char* password) {
  if (ssid == nullptr || ssid[0] == '\0') {
    currentState = WIFI_SERVICE_ERROR;
    currentStatus = WL_NO_SSID_AVAIL;
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(0);
  WiFi.begin(ssid, password);

  connectStartedAtMs = millis();
  currentState = WIFI_SERVICE_CONNECTING;
  currentStatus = WiFi.status();
  return true;
}

bool WiFiService::tick() {
  WiFiServiceState prevState = currentState;
  currentStatus = WiFi.status();

  if (currentState == WIFI_SERVICE_CONNECTING) {
    if (currentStatus == WL_CONNECTED) {
      currentState = WIFI_SERVICE_CONNECTED;
    } else if (millis() - connectStartedAtMs > CONNECT_TIMEOUT_MS) {
      WiFi.disconnect();
      currentState = WIFI_SERVICE_ERROR;
    }
  } else if (currentState == WIFI_SERVICE_CONNECTED) {
    if (currentStatus != WL_CONNECTED) {
      currentState = WIFI_SERVICE_IDLE;
    }
  }

  if (currentScanState == WIFI_SCAN_STATE_RUNNING) {
    int scanComplete = WiFi.scanComplete();
    if (scanComplete >= 0) {
      scanResultCount = (scanComplete > MAX_SCAN_RESULTS) ? MAX_SCAN_RESULTS : scanComplete;
      for (int i = 0; i < scanResultCount; i++) {
        String s = WiFi.SSID(i);
        strncpy(scanResults[i].ssid, s.c_str(), sizeof(scanResults[i].ssid) - 1);
        scanResults[i].ssid[sizeof(scanResults[i].ssid) - 1] = '\0';
        scanResults[i].rssi = WiFi.RSSI(i);
        scanResults[i].secured = (WiFi.encryptionType(i) != ENC_TYPE_NONE);
      }
      WiFi.scanDelete();

      currentScanState = (scanResultCount > 0) ? WIFI_SCAN_STATE_DONE : WIFI_SCAN_STATE_NO_RESULTS;
      scanStateChanged = true;
    } else if (scanComplete == WIFI_SCAN_FAILED) {
      currentScanState = WIFI_SCAN_STATE_ERROR;
      scanStateChanged = true;
    }
  }

  return prevState != currentState;
}

void WiFiService::disconnect() {
  WiFi.disconnect();
  currentState = WIFI_SERVICE_IDLE;
  currentStatus = WiFi.status();
}

WiFiServiceState WiFiService::state() const {
  return currentState;
}

wl_status_t WiFiService::lastStatus() const {
  return currentStatus;
}

bool WiFiService::startScan() {
  if (currentScanState == WIFI_SCAN_STATE_RUNNING) {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  int r = WiFi.scanNetworks(true, true);
  if (r == WIFI_SCAN_RUNNING) {
    currentScanState = WIFI_SCAN_STATE_RUNNING;
    scanStateChanged = true;
    scanResultCount = 0;
    return true;
  }

  currentScanState = WIFI_SCAN_STATE_ERROR;
  scanStateChanged = true;
  return false;
}

WiFiScanState WiFiService::scanState() const {
  return currentScanState;
}

int WiFiService::networkCount() const {
  return scanResultCount;
}

const WiFiNetworkInfo* WiFiService::networkAt(int index) const {
  if (index < 0 || index >= scanResultCount) {
    return nullptr;
  }
  return &scanResults[index];
}

bool WiFiService::consumeScanStateChanged() {
  bool changed = scanStateChanged;
  scanStateChanged = false;
  return changed;
}
