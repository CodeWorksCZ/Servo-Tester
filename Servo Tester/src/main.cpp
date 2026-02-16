#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

#include "app_types.h"
#include "button_input.h"
#include "config.h"
#include "display_ui.h"
#include "ina_monitor.h"
#include "settings_store.h"

Servo servoOutput;
InaMonitor inaMonitor;

Settings savedSettings{};
Settings editSettings{};

ButtonState buttonUp{};
ButtonState buttonDown{};
ButtonState buttonSelect{};

UiMode uiMode = UI_STATUS;
uint8_t selectedMenuItem = MENU_MIN_PULSE;
StatusScreen statusScreen = SCREEN_DEFAULT;
ControlMode controlMode = CONTROL_POT;

uint16_t currentPulseUs = Config::PULSE_DEFAULT_MIN;
uint16_t sweepPulseUs = Config::PULSE_DEFAULT_MIN;
int8_t sweepDirection = 1;
uint32_t sweepCycleCounter = 0;
bool sweepReachedMax = false;

unsigned long lastSweepStepMs = 0;
unsigned long lastUiDrawMs = 0;

bool displayReady = false;
bool hvMode = false;
float servoRailVoltageV = 0.0f;

const char *modeLabel()
{
  if (controlMode == CONTROL_POT)
  {
    return "POT";
  }

  if (controlMode == CONTROL_CENTER)
  {
    return "CEN";
  }

  return "SWP";
}

uint8_t pulseToAngle(const uint16_t pulseUs, const Settings &settings)
{
  if (settings.maxPulseUs <= settings.minPulseUs)
  {
    return 0;
  }

  const long angle = map(pulseUs, settings.minPulseUs, settings.maxPulseUs, 0, 180);
  if (angle < 0)
  {
    return 0;
  }

  if (angle > 180)
  {
    return 180;
  }

  return static_cast<uint8_t>(angle);
}

void drawUi()
{
  if (!displayReady)
  {
    return;
  }

  if (uiMode == UI_STATUS)
  {
    if (statusScreen == SCREEN_DEFAULT)
    {
      DisplayUi::drawStatusScreen(
          currentPulseUs,
          pulseToAngle(currentPulseUs, savedSettings),
          savedSettings.minPulseUs,
          savedSettings.maxPulseUs,
          savedSettings.reverse,
          hvMode,
          modeLabel(),
          controlMode == CONTROL_SWEEP,
          sweepCycleCounter);
    }
    else if (statusScreen == SCREEN_CURRENT)
    {
      DisplayUi::drawCurrentScreen(
          inaMonitor.ready(),
          inaMonitor.ch1mA(),
          inaMonitor.ch2mA(),
          inaMonitor.ch3mA(),
          hvMode,
          modeLabel());
    }
    else
    {
      DisplayUi::drawCurrentPeakScreen(
          inaMonitor.ready(),
          inaMonitor.peakCh1mA(),
          inaMonitor.peakCh2mA(),
          inaMonitor.peakCh3mA(),
          hvMode,
          modeLabel());
    }

    return;
  }

  DisplayUi::drawSettingsScreen(
      selectedMenuItem,
      uiMode == UI_MENU_EDIT,
      editSettings.minPulseUs,
      editSettings.maxPulseUs,
      editSettings.reverse,
      editSettings.sweepCycleSec);
}

void cycleStatusScreen()
{
  statusScreen = static_cast<StatusScreen>((statusScreen + 1) % SCREEN_COUNT);
}

void resetSweepState()
{
  sweepPulseUs = savedSettings.minPulseUs;
  sweepDirection = 1;
  sweepCycleCounter = 0;
  sweepReachedMax = false;
  lastSweepStepMs = millis();
}

void cycleControlMode(const int8_t direction)
{
  int8_t nextMode = static_cast<int8_t>(controlMode) + direction;
  if (nextMode < 0)
  {
    nextMode = static_cast<int8_t>(CONTROL_COUNT - 1);
  }
  else if (nextMode >= static_cast<int8_t>(CONTROL_COUNT))
  {
    nextMode = 0;
  }

  const ControlMode previousMode = controlMode;
  controlMode = static_cast<ControlMode>(nextMode);
  if (controlMode != previousMode)
  {
    sweepCycleCounter = 0;
    sweepReachedMax = false;
  }

  if (controlMode == CONTROL_SWEEP)
  {
    resetSweepState();
  }
}

void enterSettingsMenu()
{
  editSettings = savedSettings;
  selectedMenuItem = MENU_MIN_PULSE;
  uiMode = UI_MENU_NAVIGATION;
}

void exitSettingsMenu(const bool save)
{
  if (save)
  {
    savedSettings = editSettings;
    SettingsStore::save(savedSettings);

    if (controlMode == CONTROL_SWEEP)
    {
      resetSweepState();
    }
  }

  uiMode = UI_STATUS;
}

void adjustCurrentSetting(const int16_t delta)
{
  if (selectedMenuItem == MENU_MIN_PULSE)
  {
    const uint16_t maxAllowed = editSettings.maxPulseUs - Config::MIN_PULSE_SPAN_US;
    long candidate = static_cast<long>(editSettings.minPulseUs) + delta;

    if (candidate < Config::PULSE_MIN_LIMIT)
    {
      candidate = Config::PULSE_MIN_LIMIT;
    }

    if (candidate > maxAllowed)
    {
      candidate = maxAllowed;
    }

    editSettings.minPulseUs = static_cast<uint16_t>(candidate);
    return;
  }

  if (selectedMenuItem == MENU_MAX_PULSE)
  {
    const uint16_t minAllowed = editSettings.minPulseUs + Config::MIN_PULSE_SPAN_US;
    long candidate = static_cast<long>(editSettings.maxPulseUs) + delta;

    if (candidate > Config::PULSE_MAX_LIMIT)
    {
      candidate = Config::PULSE_MAX_LIMIT;
    }

    if (candidate < minAllowed)
    {
      candidate = minAllowed;
    }

    editSettings.maxPulseUs = static_cast<uint16_t>(candidate);
    return;
  }

  if (selectedMenuItem == MENU_SWEEP_CYCLE)
  {
    long candidate = static_cast<long>(editSettings.sweepCycleSec) + (delta > 0 ? 1 : -1);

    if (candidate < Config::SWEEP_CYCLE_MIN_SEC)
    {
      candidate = Config::SWEEP_CYCLE_MIN_SEC;
    }

    if (candidate > Config::SWEEP_CYCLE_MAX_SEC)
    {
      candidate = Config::SWEEP_CYCLE_MAX_SEC;
    }

    editSettings.sweepCycleSec = static_cast<uint16_t>(candidate);
  }
}

uint16_t computeSweepStepIntervalMs(const Settings &settings)
{
  const uint32_t spanUs = static_cast<uint32_t>(settings.maxPulseUs - settings.minPulseUs);
  const uint32_t fullCycleUs = spanUs * 2U;
  const uint32_t stepsPerCycle = (fullCycleUs / Config::SWEEP_STEP_US) > 0U ? (fullCycleUs / Config::SWEEP_STEP_US) : 1U;
  const uint32_t cycleMs = static_cast<uint32_t>(settings.sweepCycleSec) * 1000U;
  const uint32_t intervalMs = cycleMs / stepsPerCycle;

  return static_cast<uint16_t>(intervalMs > 0U ? intervalMs : 1U);
}

void updateServoOutput(const unsigned long nowMs)
{
  if (controlMode == CONTROL_POT)
  {
    const int potRaw = analogRead(Config::POT_PIN);
    const int control = savedSettings.reverse ? (1023 - potRaw) : potRaw;
    currentPulseUs = static_cast<uint16_t>(map(control, 0, 1023, savedSettings.minPulseUs, savedSettings.maxPulseUs));
  }
  else if (controlMode == CONTROL_CENTER)
  {
    currentPulseUs = static_cast<uint16_t>((savedSettings.minPulseUs + savedSettings.maxPulseUs) / 2);
  }
  else
  {
    if (sweepPulseUs < savedSettings.minPulseUs)
    {
      sweepPulseUs = savedSettings.minPulseUs;
      sweepDirection = 1;
      sweepReachedMax = false;
    }
    else if (sweepPulseUs > savedSettings.maxPulseUs)
    {
      sweepPulseUs = savedSettings.maxPulseUs;
      sweepDirection = -1;
      sweepReachedMax = true;
    }

    const uint16_t sweepStepIntervalMs = computeSweepStepIntervalMs(savedSettings);
    if ((nowMs - lastSweepStepMs) >= sweepStepIntervalMs)
    {
      lastSweepStepMs = nowMs;

      const int32_t stepUs = (sweepDirection > 0) ? static_cast<int32_t>(Config::SWEEP_STEP_US)
                                                  : -static_cast<int32_t>(Config::SWEEP_STEP_US);
      const int32_t nextPulse = static_cast<int32_t>(sweepPulseUs) + stepUs;

      if (nextPulse >= static_cast<int32_t>(savedSettings.maxPulseUs))
      {
        sweepPulseUs = savedSettings.maxPulseUs;
        sweepDirection = -1;
        sweepReachedMax = true;
      }
      else if (nextPulse <= static_cast<int32_t>(savedSettings.minPulseUs))
      {
        sweepPulseUs = savedSettings.minPulseUs;
        sweepDirection = 1;
        if (sweepReachedMax)
        {
          if (sweepCycleCounter < UINT32_MAX)
          {
            ++sweepCycleCounter;
          }
        }
        sweepReachedMax = false;
      }
      else
      {
        sweepPulseUs = static_cast<uint16_t>(nextPulse);
      }
    }

    currentPulseUs = sweepPulseUs;
  }

  servoOutput.writeMicroseconds(currentPulseUs);
}

float readServoRailVoltageV()
{
  const int raw = analogRead(Config::SERVO_VSENSE_PIN);
  const float adcV = (static_cast<float>(raw) * Config::SERVO_VSENSE_ADC_REF_V) / 1023.0f;
  const float ratio = (Config::SERVO_VSENSE_R1_OHMS + Config::SERVO_VSENSE_R2_OHMS) / Config::SERVO_VSENSE_R2_OHMS;
  return adcV * ratio;
}

void handleUiInput(const bool upPressed, const bool downPressed, const bool selectShortPress, const bool selectLongPress)
{
  switch (uiMode)
  {
  case UI_STATUS:
    if (selectLongPress)
    {
      enterSettingsMenu();
    }
    else if (upPressed)
    {
      cycleControlMode(1);
    }
    else if (downPressed)
    {
      cycleControlMode(-1);
    }
    else if (selectShortPress)
    {
      cycleStatusScreen();
    }
    break;

  case UI_MENU_NAVIGATION:
    if (upPressed)
    {
      if (selectedMenuItem == 0)
      {
        selectedMenuItem = static_cast<uint8_t>(MENU_ITEM_COUNT - 1);
      }
      else
      {
        --selectedMenuItem;
      }
    }

    if (downPressed)
    {
      selectedMenuItem = static_cast<uint8_t>((selectedMenuItem + 1) % MENU_ITEM_COUNT);
    }

    if (selectShortPress)
    {
      if (selectedMenuItem == MENU_SAVE_EXIT)
      {
        exitSettingsMenu(true);
      }
      else if (selectedMenuItem == MENU_CANCEL)
      {
        exitSettingsMenu(false);
      }
      else
      {
        uiMode = UI_MENU_EDIT;
      }
    }
    break;

  case UI_MENU_EDIT:
    if (selectedMenuItem == MENU_REVERSE)
    {
      if (upPressed || downPressed || selectShortPress)
      {
        editSettings.reverse = editSettings.reverse ? 0 : 1;
        uiMode = UI_MENU_NAVIGATION;
      }
    }
    else
    {
      if (upPressed)
      {
        adjustCurrentSetting(Config::PULSE_STEP_US);
      }

      if (downPressed)
      {
        adjustCurrentSetting(-Config::PULSE_STEP_US);
      }

      if (selectShortPress)
      {
        uiMode = UI_MENU_NAVIGATION;
      }
    }
    break;
  }
}

void setup()
{
  savedSettings = SettingsStore::load();

  Wire.begin();
#if defined(WIRE_HAS_TIMEOUT)
  Wire.setWireTimeout(25000, true);
#endif

  displayReady = DisplayUi::begin();
  if (displayReady)
  {
    DisplayUi::drawBootScreen();
  }

  servoOutput.attach(Config::SERVO_PIN, Config::PULSE_MIN_LIMIT, Config::PULSE_MAX_LIMIT);
  pinMode(Config::POT_PIN, INPUT);
  pinMode(Config::SERVO_VSENSE_PIN, INPUT);

  ButtonInput::init(buttonUp, Config::BTN_UP_PIN);
  ButtonInput::init(buttonDown, Config::BTN_DOWN_PIN);
  ButtonInput::init(buttonSelect, Config::BTN_SELECT_PIN);

  if (Config::MODE_SWITCH_PIN != 255)
  {
    pinMode(Config::MODE_SWITCH_PIN, Config::MODE_SWITCH_USE_PULLUP ? INPUT_PULLUP : INPUT);
  }

  inaMonitor.begin(Wire);
  delay(600);
}

void loop()
{
  const unsigned long nowMs = millis();

  if (Config::MODE_SWITCH_PIN != 255)
  {
    const bool modeReading = Config::MODE_SWITCH_ACTIVE_LOW ? (digitalRead(Config::MODE_SWITCH_PIN) == LOW)
                                                            : (digitalRead(Config::MODE_SWITCH_PIN) == HIGH);
    hvMode = modeReading;
  }
  else
  {
    servoRailVoltageV = readServoRailVoltageV();
    hvMode = (servoRailVoltageV >= Config::SERVO_VSENSE_HV_THRESHOLD_V);
  }

  const bool upPressed = ButtonInput::updatePressed(buttonUp, nowMs);
  const bool downPressed = ButtonInput::updatePressed(buttonDown, nowMs);

  bool selectShortPress = false;
  bool selectLongPress = false;
  ButtonInput::updateSelectEvents(buttonSelect, nowMs, selectShortPress, selectLongPress);

  updateServoOutput(nowMs);
  inaMonitor.update(nowMs);
  handleUiInput(upPressed, downPressed, selectShortPress, selectLongPress);

  if ((nowMs - lastUiDrawMs) >= Config::UI_REFRESH_MS || upPressed || downPressed || selectShortPress || selectLongPress)
  {
    drawUi();
    lastUiDrawMs = nowMs;
  }
}
