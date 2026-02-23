#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config
{
// Centralized compile-time configuration.
// Keep hardware pinout and behavior constants here.

// SSD1306 OLED parameters (software SPI wiring)
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_MOSI = 10;   // Display DIN / D1
constexpr uint8_t OLED_CLK = 9;   // Display CLK / D0
constexpr uint8_t OLED_DC = 12;    // Display D/C
constexpr uint8_t OLED_CS = 13;    // Display CS
constexpr uint8_t OLED_RESET = 11; // Display RST

// Device I/O pins
constexpr uint8_t SERVO_PIN = 6;
constexpr uint8_t POT_PIN = A0;
// Servo rail voltage sensing (ADC)
constexpr uint8_t SERVO_VSENSE_PIN = A1;
constexpr float SERVO_VSENSE_R1_OHMS = 180000.0f; // top resistor to V+
constexpr float SERVO_VSENSE_R2_OHMS = 100000.0f; // bottom resistor to GND
constexpr float SERVO_VSENSE_ADC_REF_V = 5.0f;      // AVR ADC reference voltage.
constexpr float SERVO_VSENSE_HV_THRESHOLD_V = 7.2f; // Above this, UI reports HV mode.
constexpr uint8_t BTN_UP_PIN = 2;
constexpr uint8_t BTN_DOWN_PIN = 3;
constexpr uint8_t BTN_SELECT_PIN = 5;
// Optional alert LED output. Set to 255 to disable.
constexpr uint8_t ALERT_LED_PIN = 4;
constexpr bool ALERT_LED_ACTIVE_HIGH = true;
// External STD/HV mode switch input (to GND or VCC).
// Set MODE_SWITCH_PIN to 255 to disable and use ADC voltage detection.
constexpr uint8_t MODE_SWITCH_PIN = 255;
constexpr bool MODE_SWITCH_ACTIVE_LOW = true;
constexpr bool MODE_SWITCH_USE_PULLUP = true;

// UI refresh rate and button debounce
constexpr uint16_t UI_REFRESH_MS = 80;       // OLED redraw period.
constexpr uint16_t BUTTON_DEBOUNCE_MS = 35;  // Debounce filter time.
constexpr uint16_t BUTTON_LONG_PRESS_MS = 700;
constexpr uint8_t GAUGE_DEADBAND_PERCENT = 1; // Ignore tiny +/- display jitter (% of span).
constexpr uint8_t GAUGE_MAX_STEP_PERCENT = 4; // Max display step per redraw (% of span).
// true: pressed = LOW (button to GND), false: pressed = HIGH
constexpr bool BUTTON_ACTIVE_LOW = true;
// true: enable internal pull-up resistors on button pins
constexpr bool BUTTON_USE_INTERNAL_PULLUP = true;

// INA3221 current monitor configuration
constexpr uint8_t INA3221_I2C_ADDRESS = 0x40;
constexpr float INA3221_SHUNT_OHMS_CH1 = 0.1f;
constexpr float INA3221_SHUNT_OHMS_CH2 = 0.1f;
constexpr float INA3221_SHUNT_OHMS_CH3 = 0.1f;
// Software calibration multipliers (1.0 = no correction).
constexpr float INA3221_CAL_FACTOR_CH1 = 1.0f;
constexpr float INA3221_CAL_FACTOR_CH2 = 1.0f;
constexpr float INA3221_CAL_FACTOR_CH3 = 1.0f;
constexpr uint16_t INA3221_REFRESH_MS = 200; // Sensor polling interval.
// Alert thresholds in milliamps (shown as WR/CR on LCD).
// For 0.1 ohm shunts (R100), keep thresholds below ~1.64 A measurement limit.
constexpr float INA3221_WARN_MA_CH1 = 900.0f;
constexpr float INA3221_WARN_MA_CH2 = 900.0f;
constexpr float INA3221_WARN_MA_CH3 = 900.0f;
constexpr float INA3221_CRIT_MA_CH1 = 1300.0f;
constexpr float INA3221_CRIT_MA_CH2 = 1300.0f;
constexpr float INA3221_CRIT_MA_CH3 = 1300.0f;
// Ignore alert flags for channels with effectively no bus voltage (floating/unwired).
constexpr float INA3221_ALERT_MIN_BUS_V = 0.5f;

// Servo pulse limits and edit behavior
constexpr uint16_t PULSE_MIN_LIMIT = 500;
constexpr uint16_t PULSE_MAX_LIMIT = 2500;
constexpr uint16_t PULSE_DEFAULT_MIN = 1000;
constexpr uint16_t PULSE_DEFAULT_MAX = 2000;
constexpr uint16_t PULSE_STEP_US = 10;
constexpr uint16_t MIN_PULSE_SPAN_US = 100;
// Sweep mode behavior
constexpr uint16_t SWEEP_STEP_US = 5; // Delta applied every sweep step.
constexpr uint16_t SWEEP_CYCLE_MIN_MS = 500;
constexpr uint16_t SWEEP_CYCLE_MAX_MS = 10000;
constexpr uint16_t SWEEP_CYCLE_DEFAULT_MS = 3000;
constexpr uint16_t SWEEP_CYCLE_STEP_MS = 500;

// EEPROM storage layout/version
constexpr uint8_t SETTINGS_VERSION = 3;
constexpr int EEPROM_ADDR = 0; // Base EEPROM address for Settings struct.
} // namespace Config

#endif
