#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config
{
// SSD1306 OLED parameters (software SPI wiring)
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_MOSI = 9;   // Display DIN / D1
constexpr uint8_t OLED_CLK = 10;   // Display CLK / D0
constexpr uint8_t OLED_DC = 11;    // Display D/C
constexpr uint8_t OLED_CS = 12;    // Display CS
constexpr uint8_t OLED_RESET = 13; // Display RST

// Device I/O pins
constexpr uint8_t SERVO_PIN = 6;
constexpr uint8_t POT_PIN = A0;
constexpr uint8_t BTN_UP_PIN = 2;
constexpr uint8_t BTN_DOWN_PIN = 3;
constexpr uint8_t BTN_SELECT_PIN = 5;

// UI refresh rate and button debounce
constexpr uint16_t UI_REFRESH_MS = 80;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 35;
// true: pressed = LOW (button to GND), false: pressed = HIGH
constexpr bool BUTTON_ACTIVE_LOW = true;
// true: enable internal pull-up resistors on button pins
constexpr bool BUTTON_USE_INTERNAL_PULLUP = true;

// Servo pulse limits and edit behavior
constexpr uint16_t PULSE_MIN_LIMIT = 500;
constexpr uint16_t PULSE_MAX_LIMIT = 2500;
constexpr uint16_t PULSE_DEFAULT_MIN = 1000;
constexpr uint16_t PULSE_DEFAULT_MAX = 2000;
constexpr uint16_t PULSE_STEP_US = 10;
constexpr uint16_t MIN_PULSE_SPAN_US = 100;

// EEPROM storage layout/version
constexpr uint8_t SETTINGS_VERSION = 1;
constexpr int EEPROM_ADDR = 0;
} // namespace Config

#endif
