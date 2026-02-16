#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <Arduino.h>
#include <Servo.h>

#include "app_types.h"
#include "button_input.h"
#include "ina_monitor.h"

class AppController
{
public:
  void begin();
  void update();

private:
  const char *modeLabel() const;
  uint8_t pulseToAngle(uint16_t pulseUs, const Settings &settings) const;
  void drawUi();
  void cycleStatusScreen();
  void resetSweepState();
  void cycleControlMode(int8_t direction);
  void enterSettingsMenu();
  void exitSettingsMenu(bool save);
  void adjustCurrentSetting(int16_t delta);
  uint16_t computeSweepStepIntervalMs(const Settings &settings) const;
  void updateServoOutput(unsigned long nowMs);
  float readServoRailVoltageV() const;
  void handleUiInput(bool upPressed, bool downPressed, bool selectShortPress, bool selectLongPress);

  Servo servoOutput_{};
  InaMonitor inaMonitor_{};

  Settings savedSettings_{};
  Settings editSettings_{};

  ButtonState buttonUp_{};
  ButtonState buttonDown_{};
  ButtonState buttonSelect_{};

  UiMode uiMode_ = UI_STATUS;
  uint8_t selectedMenuItem_ = MENU_MIN_PULSE;
  StatusScreen statusScreen_ = SCREEN_DEFAULT;
  ControlMode controlMode_ = CONTROL_POT;

  uint16_t currentPulseUs_ = 0;
  uint16_t sweepPulseUs_ = 0;
  int8_t sweepDirection_ = 1;
  uint32_t sweepCycleCounter_ = 0;
  bool sweepReachedMax_ = false;

  unsigned long lastSweepStepMs_ = 0;
  unsigned long lastUiDrawMs_ = 0;

  bool displayReady_ = false;
  bool hvMode_ = false;
  float servoRailVoltageV_ = 0.0f;
};

#endif
