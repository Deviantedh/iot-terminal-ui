#include <Arduino.h>
#include "DisplayHAL.h"
#include "SimpleUI.h"
#include "AppScreens.h"

#ifndef USER_SETUP_LOADED
#error "Project-local TFT_eSPI setup was not injected. Check iot_terminal_ui.ino.globals.h and ESP8266 core globals injection."
#endif

#ifndef ST7789_DRIVER
#error "ST7789_DRIVER must be defined by the project-local TFT_eSPI setup."
#endif

#define TOUCH_CS  D2
#define TOUCH_IRQ D1

DisplayHAL displayHal(TOUCH_CS, TOUCH_IRQ);
AppState app;
SimpleUI* ui = nullptr;

void setup() {
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

  displayHal.begin();
  initAppState(app);

  static SimpleUI uiInstance(displayHal.getTft());
  ui = &uiInstance;

  processUiUpdates(displayHal.getTft(), *ui, app);
}

void loop() {
  int tx, ty;
  bool touching = displayHal.readTouch(tx, ty);
  updateTouchState(app, touching, tx, ty);

  if (displayHal.getTap(tx, ty)) {
    Serial.print("Tap: ");
    Serial.print(tx);
    Serial.print(", ");
    Serial.println(ty);
    handleAppTouch(*ui, app, tx, ty);
  }

  processUiUpdates(displayHal.getTft(), *ui, app);

  delay(1);
}
