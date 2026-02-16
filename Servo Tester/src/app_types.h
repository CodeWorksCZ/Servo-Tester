#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <Arduino.h>

struct Settings
{
  uint8_t version;
  uint16_t minPulseUs;
  uint16_t maxPulseUs;
  uint8_t reverse;
  uint16_t sweepCycleSec;
};

struct ButtonState
{
  uint8_t pin;
  bool stablePressed;
  bool lastReading;
  unsigned long lastDebounceMs;
  unsigned long pressedSinceMs;
  bool longPressSent;
};

enum UiMode : uint8_t
{
  UI_STATUS = 0,
  UI_MENU_NAVIGATION = 1,
  UI_MENU_EDIT = 2
};

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

enum StatusScreen : uint8_t
{
  SCREEN_DEFAULT = 0,
  SCREEN_CURRENT = 1,
  SCREEN_CURRENT_PEAK = 2,
  SCREEN_COUNT = 3
};

enum ControlMode : uint8_t
{
  CONTROL_POT = 0,
  CONTROL_CENTER = 1,
  CONTROL_SWEEP = 2,
  CONTROL_COUNT = 3
};

#endif
