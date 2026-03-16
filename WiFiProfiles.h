#ifndef WIFI_PROFILES_H
#define WIFI_PROFILES_H

#include <Arduino.h>

struct WiFiCredentials {
  char ssid[33];
  char password[65];
};

class WiFiProfiles {
public:
  WiFiProfiles();

  bool begin();
  bool loadSaved(WiFiCredentials& out);
  bool saveSaved(const char* ssid, const char* password);
  bool clearSaved();

  bool hasBuiltInNetworks() const;
  int builtInCount() const;
  bool builtInAt(int index, WiFiCredentials& out) const;

private:
  bool initialized;

  void clearCredentials(WiFiCredentials& out) const;
};

#endif
