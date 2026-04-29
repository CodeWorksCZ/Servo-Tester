#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <Arduino.h>
#include <Servo.h>

#include "app_types.h"
#include "button_input.h"
#include "ina_monitor.h"

// Owns runtime application state and orchestrates all modules.
class AppController
{
public:
  // Initialize hardware and module state.
  void begin();
  // Execute one non-blocking firmware cycle.
  void update();

private:
  // UI helpers and state-machine transitions.
  const char *modeLabel() const;
  uint8_t pulseToAngle(uint16_t pulseUs, const Settings &settings) const;
  uint8_t pulseToPercent(uint16_t pulseUs, const Settings &settings) const;
  uint16_t stabilizeDisplayPulseUs(uint16_t rawPulseUs, const Settings &settings);
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
  void updateAlertLed() const;
  void updatePowerModeLeds() const;
  void handleUiInput(bool upPressed, bool downPressed, bool selectShortPress, bool selectLongPress);

  // Hardware/service modules.
  Servo servoOutput_{};
  InaMonitor inaMonitor_{};

  // Settings currently applied and temporary editable copy.
  Settings savedSettings_{};
  Settings editSettings_{};

  // Debounced button states.
  ButtonState buttonUp_{};
  ButtonState buttonDown_{};
  ButtonState buttonSelect_{};

  // UI and control state machines.
  UiMode uiMode_ = UI_STATUS;
  uint8_t selectedMenuItem_ = MENU_MIN_PULSE;
  StatusScreen statusScreen_ = SCREEN_DEFAULT;
  ControlMode controlMode_ = CONTROL_POT;

  // Servo output state (including SWP mode internals).
  uint16_t currentPulseUs_ = 0;
  uint16_t sweepPulseUs_ = 0;
  int8_t sweepDirection_ = 1;
  uint32_t sweepCycleCounter_ = 0;
  bool sweepReachedMax_ = false;
  bool sweepBurnDone_ = false;

  // Periodic task timing.
  unsigned long lastSweepStepMs_ = 0;
  unsigned long lastUiDrawMs_ = 0;

  // Display and detected power mode.
  bool displayReady_ = false;
  bool hvMode_ = false;
  float servoRailVoltageV_ = 0.0f;
  uint16_t displayPulseUs_ = 0;
  bool displayPulseInitialized_ = false;
};

#endif
