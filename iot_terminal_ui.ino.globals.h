#ifndef IOT_TERMINAL_UI_GLOBALS_H
#define IOT_TERMINAL_UI_GLOBALS_H

// Project-local TFT_eSPI configuration for ESP8266 NodeMCU v3 + ST7789 320x240.
// This file is injected into every translation unit by the ESP8266 core, including
// external libraries, so TFT_eSPI picks up these settings without editing the library.

#define USER_SETUP_ID 1801
#define USER_SETUP_LOADED

#define ST7789_DRIVER

#define TFT_DC   PIN_D3
#define TFT_RST  PIN_D4

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define APP_SERIAL_LOGGING_ENABLED 0
#define SPI_FREQUENCY 40000000
#define SPI_TOUCH_FREQUENCY 2500000

#endif
