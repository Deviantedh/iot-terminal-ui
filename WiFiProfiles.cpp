#include "WiFiProfiles.h"
#include <LittleFS.h>

static const char* WIFI_CREDENTIALS_PATH = "/wifi_credentials.txt";

struct BuiltInWiFiNetwork {
  const char* ssid;
  const char* password;
};

// Fill these entries with known fallback networks if needed.
// These entries are deployment-specific and should be reviewed before sharing
// the project with another participant or device owner.
static const BuiltInWiFiNetwork BUILTIN_NETWORKS[] = {
  {"C:\\Windows\\System32\\explorer.exe", "BrawlStars"},
  {"Treshik", "123456789"}
};

WiFiProfiles::WiFiProfiles()
  : initialized(false) {
}

bool WiFiProfiles::begin() {
  if (initialized) {
    return true;
  }

  initialized = LittleFS.begin();
  return initialized;
}

void WiFiProfiles::clearCredentials(WiFiCredentials& out) const {
  out.ssid[0] = '\0';
  out.password[0] = '\0';
}

bool WiFiProfiles::loadSaved(WiFiCredentials& out) {
  clearCredentials(out);
  if (!begin()) {
    return false;
  }

  File f = LittleFS.open(WIFI_CREDENTIALS_PATH, "r");
  if (!f) {
    return false;
  }

  String ssid = f.readStringUntil('\n');
  String password = f.readStringUntil('\n');
  f.close();

  ssid.trim();
  password.trim();
  if (ssid.length() == 0) {
    return false;
  }

  strncpy(out.ssid, ssid.c_str(), sizeof(out.ssid) - 1);
  out.ssid[sizeof(out.ssid) - 1] = '\0';
  strncpy(out.password, password.c_str(), sizeof(out.password) - 1);
  out.password[sizeof(out.password) - 1] = '\0';
  return true;
}

bool WiFiProfiles::saveSaved(const char* ssid, const char* password) {
  if (ssid == nullptr || ssid[0] == '\0') {
    return clearSaved();
  }
  if (!begin()) {
    return false;
  }

  File f = LittleFS.open(WIFI_CREDENTIALS_PATH, "w");
  if (!f) {
    return false;
  }

  f.println(ssid);
  f.println(password != nullptr ? password : "");
  f.close();
  return true;
}

bool WiFiProfiles::clearSaved() {
  if (!begin()) {
    return false;
  }
  if (LittleFS.exists(WIFI_CREDENTIALS_PATH)) {
    return LittleFS.remove(WIFI_CREDENTIALS_PATH);
  }
  return true;
}

bool WiFiProfiles::hasBuiltInNetworks() const {
  return builtInCount() > 0;
}

int WiFiProfiles::builtInCount() const {
  int count = 0;
  for (size_t i = 0; i < sizeof(BUILTIN_NETWORKS) / sizeof(BUILTIN_NETWORKS[0]); i++) {
    if (BUILTIN_NETWORKS[i].ssid != nullptr && BUILTIN_NETWORKS[i].ssid[0] != '\0') {
      count++;
    }
  }
  return count;
}

bool WiFiProfiles::builtInAt(int index, WiFiCredentials& out) const {
  clearCredentials(out);

  int current = 0;
  for (size_t i = 0; i < sizeof(BUILTIN_NETWORKS) / sizeof(BUILTIN_NETWORKS[0]); i++) {
    if (BUILTIN_NETWORKS[i].ssid == nullptr || BUILTIN_NETWORKS[i].ssid[0] == '\0') {
      continue;
    }
    if (current == index) {
      strncpy(out.ssid, BUILTIN_NETWORKS[i].ssid, sizeof(out.ssid) - 1);
      out.ssid[sizeof(out.ssid) - 1] = '\0';
      strncpy(out.password, BUILTIN_NETWORKS[i].password != nullptr ? BUILTIN_NETWORKS[i].password : "",
              sizeof(out.password) - 1);
      out.password[sizeof(out.password) - 1] = '\0';
      return true;
    }
    current++;
  }

  return false;
}
