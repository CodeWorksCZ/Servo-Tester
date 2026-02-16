#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config
{
// OLED SSD1306 (software SPI)
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_MOSI = 9; //-> LCD D1
constexpr uint8_t OLED_CLK = 10; //-> LCD D0
constexpr uint8_t OLED_DC = 11; //-> LCD DC
constexpr uint8_t OLED_CS = 12; //-> LCD DS
constexpr uint8_t OLED_RESET = 13; //-> LCD RSTs

// Hardware I/O
constexpr uint8_t SERVO_PIN = 6;
constexpr uint8_t POT_PIN = A0;
constexpr uint8_t BTN_UP_PIN = 2;
constexpr uint8_t BTN_DOWN_PIN = 3;
constexpr uint8_t BTN_SELECT_PIN = 5;

// UI timing and button debounce
constexpr uint16_t UI_REFRESH_MS = 80;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 35;
constexpr bool BUTTON_ACTIVE_LOW = true;
constexpr bool BUTTON_USE_INTERNAL_PULLUP = true;

// Servo and settings bounds
constexpr uint16_t PULSE_MIN_LIMIT = 500;
constexpr uint16_t PULSE_MAX_LIMIT = 2500;
constexpr uint16_t PULSE_DEFAULT_MIN = 1000;
constexpr uint16_t PULSE_DEFAULT_MAX = 2000;
constexpr uint16_t PULSE_STEP_US = 10;
constexpr uint16_t MIN_PULSE_SPAN_US = 100;

// Persisted settings
constexpr uint8_t SETTINGS_VERSION = 1;
constexpr int EEPROM_ADDR = 0;
} // namespace Config

#endif
