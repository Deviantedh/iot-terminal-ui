#include <Arduino.h>
#include "BuzzerService.h"
#include "DisplayHAL.h"
#include "Pcf8574Buttons.h"
#include "SimpleUI.h"
#include "AppScreens.h"

#ifndef USER_SETUP_LOADED
#error "Project-local TFT_eSPI setup was not injected. Check iot_terminal_ui.ino.globals.h and ESP8266 core globals injection."
#endif

#ifndef ST7789_DRIVER
#error "ST7789_DRIVER must be defined by the project-local TFT_eSPI setup."
#endif

#ifndef APP_DEBUG_MODE_DEFAULT
#define APP_DEBUG_MODE_DEFAULT 0
#endif

#ifndef APP_SERIAL_LOGGING_ENABLED
#define APP_SERIAL_LOGGING_ENABLED 0
#endif

#define TOUCH_CS  D2
#define TOUCH_IRQ D1
// Set a PWM-capable GPIO here if the TFT backlight transistor is wired to a controllable pin.
#define BACKLIGHT_PIN -1
#define BUZZER_PIN D0
#define PCF8574_ADDR 0x20
#define PCF8574_SDA_PIN 3
#define PCF8574_SCL_PIN 1

DisplayHAL displayHal(TOUCH_CS, TOUCH_IRQ, BACKLIGHT_PIN);
BuzzerService buzzer(BUZZER_PIN);
Pcf8574Buttons buttons(PCF8574_ADDR, PCF8574_SDA_PIN, PCF8574_SCL_PIN);
AppState app;
SimpleUI* ui = nullptr;
bool buttonsReady = false;
unsigned long lastButtonsPollMs = 0;

void setup() {
#if APP_SERIAL_LOGGING_ENABLED
  Serial.begin(115200);
  Serial.println();
  Serial.println("Boot iot_terminal_ui");
#ifdef USER_SETUP_ID
  Serial.print("TFT USER_SETUP_ID: ");
  Serial.println(USER_SETUP_ID);
#endif
#ifdef SPI_FREQUENCY
  Serial.print("TFT SPI_FREQUENCY: ");
  Serial.println((uint32_t)SPI_FREQUENCY);
#endif
#ifdef SPI_TOUCH_FREQUENCY
  Serial.print("TFT SPI_TOUCH_FREQUENCY: ");
  Serial.println((uint32_t)SPI_TOUCH_FREQUENCY);
#endif
  Serial.print("Backlight control: ");
  Serial.println(displayHal.backlightControlSupported() ? "GPIO PWM" : "not wired (stub only)");
#endif

  displayHal.begin();
  buzzer.begin();
  buttonsReady = buttons.begin();
  initAppState(app);
  appSetDebugMode(app, APP_DEBUG_MODE_DEFAULT != 0);
#if APP_SERIAL_LOGGING_ENABLED
  Serial.print("PCF8574 buttons: ");
  Serial.println(buttonsReady ? "ready" : "not detected");
#endif

  static SimpleUI uiInstance(displayHal.getTft());
  ui = &uiInstance;

  processUiUpdates(displayHal.getTft(), *ui, app);
}

void loop() {
  const unsigned long nowMs = millis();
  const unsigned long buttonsPollIntervalMs = appIsSleeping(app) ? 60 : (appIsIdleMode(app) ? 40 : 20);
  const PowerMode previousPowerMode = app.powerMode;
  const WifiFlowState previousWifiFlowState = app.wifiFlowState;
  buzzer.setEnabled(app.soundEnabled);

  if (buttonsReady && (long)(nowMs - lastButtonsPollMs) >= (long)buttonsPollIntervalMs) {
    lastButtonsPollMs = nowMs;
    const uint8_t buttonEvents = buttons.poll();
    const bool wakeRequested = appIsSleeping(app) &&
      (buttonEvents & (PCF8574_BUTTON_EVENT_POWER_SHORT | PCF8574_BUTTON_EVENT_POWER_LONG));
    if (wakeRequested && displayHal.displayIsSleeping()) {
      displayHal.displaySleepOff();
    }
    if ((buttonEvents & PCF8574_BUTTON_EVENT_MENU) &&
        app.powerMode == POWER_MODE_NORMAL) {
      buzzer.playClick();
      appPostEvent(app, APP_EVENT_BUTTON_MENU, 0, "MENU");
    }
    if (buttonEvents & PCF8574_BUTTON_EVENT_POWER_SHORT) {
      appPostEvent(app, APP_EVENT_BUTTON_POWER_SHORT, 0, "POWER SHORT");
    }
    if (buttonEvents & PCF8574_BUTTON_EVENT_POWER_LONG) {
      appPostEvent(app, APP_EVENT_BUTTON_POWER_LONG, 0, "POWER LONG");
    }
  }

  int tx = 0;
  int ty = 0;
  if (appIsSleeping(app)) {
    // Hard gate touch in sleep/standby: no IRQ probing, no SPI touch reads,
    // no tap events and no wake-from-touch path.
    updateTouchState(app, false, 0, 0);
  } else {
    const bool touching = displayHal.readTouch(tx, ty);
    updateTouchState(app, touching, tx, ty);
  }

  if (!appIsSleeping(app) && displayHal.getTap(tx, ty)) {
#if APP_SERIAL_LOGGING_ENABLED
    Serial.print("Tap: ");
    Serial.print(tx);
    Serial.print(", ");
    Serial.println(ty);
#endif
    handleAppTouch(*ui, app, tx, ty);
  }

  processUiUpdates(displayHal.getTft(), *ui, app);
  buzzer.setEnabled(app.soundEnabled);

  if (app.testSongRequested) {
    app.testSongRequested = false;
    buzzer.playTestSong();
  }

  if (appIsSleeping(app)) {
    if (!displayHal.displayIsSleeping()) {
      displayHal.displaySleepOn();
    }
  } else if (displayHal.displayIsSleeping()) {
    displayHal.displaySleepOff();
    app.fullRedrawRequested = true;
  }

  if (previousPowerMode != app.powerMode) {
    if (app.powerMode == POWER_MODE_NORMAL && previousPowerMode == POWER_MODE_STANDBY) {
      buzzer.playBootMelody();
    } else if (app.powerMode == POWER_MODE_NORMAL) {
      buzzer.playWake();
    }
  }

  if (previousWifiFlowState != app.wifiFlowState &&
      app.wifiFlowState == WIFI_FLOW_ERROR) {
    buzzer.playError();
  }

  if (appIsSleeping(app)) {
    displayHal.setBacklightLevel(BACKLIGHT_OFF);
  } else if (appIsIdleMode(app)) {
    displayHal.setBacklightLevel(BACKLIGHT_DIM);
  } else {
    displayHal.setBacklightLevel(BACKLIGHT_NORMAL);
  }

  buzzer.tick();
  delay(appLoopDelayMs(app));
}
