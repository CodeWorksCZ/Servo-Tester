#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <Arduino.h>

// Persisted user settings stored in EEPROM.
struct Settings
{
  uint8_t version;        // Settings structure version for migration/validation.
  uint16_t minPulseUs;    // Servo minimum pulse width in microseconds.
  uint16_t maxPulseUs;    // Servo maximum pulse width in microseconds.
  uint8_t reverse;        // 0 = normal, 1 = reversed potentiometer direction.
  uint16_t sweepCycleSec; // Target duration for one full sweep cycle.
};

// Debounced button runtime state.
struct ButtonState
{
  uint8_t pin;                 // Physical input pin.
  bool stablePressed;          // Debounced pressed/released state.
  bool lastReading;            // Raw sampled state from previous loop.
  unsigned long lastDebounceMs; // Last timestamp when raw state changed.
  unsigned long pressedSinceMs; // Timestamp when button became pressed.
  bool longPressSent;           // Prevents repeated long-press events.
};

// Top-level UI state.
enum UiMode : uint8_t
{
  UI_STATUS = 0,
  UI_MENU_NAVIGATION = 1,
  UI_MENU_EDIT = 2
};

// Editable settings menu items.
enum MenuItem : uint8_t
{
  MENU_MIN_PULSE = 0,
  MENU_MAX_PULSE = 1,
  MENU_REVERSE = 2,
  MENU_SWEEP_CYCLE = 3,
  MENU_SAVE_EXIT = 4,
  MENU_CANCEL = 5,
  MENU_ITEM_COUNT = 6
};

// Pages visible from the status screen cycle.
enum StatusScreen : uint8_t
{
  SCREEN_DEFAULT = 0,
  SCREEN_GAUGE = 1,
  SCREEN_CURRENT = 2,
  SCREEN_VBUS = 3,
  SCREEN_CURRENT_PEAK = 4,
  SCREEN_COUNT = 5
};

// Servo output control strategies.
enum ControlMode : uint8_t
{
  CONTROL_POT = 0,
  CONTROL_CENTER = 1,
  CONTROL_SWEEP = 2,
  CONTROL_COUNT = 3
};

#endif
