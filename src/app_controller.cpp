#include "app_controller.h"

#include <Wire.h>

#include "config.h"
#include "display_ui.h"
#include "settings_store.h"

const char *AppController::modeLabel() const
{
  // Short labels are used in the OLED header.
  if (controlMode_ == CONTROL_POT)
  {
    return "POT";
  }

  if (controlMode_ == CONTROL_CENTER)
  {
    return "CEN";
  }

  return "SWP";
}

uint8_t AppController::pulseToAngle(const uint16_t pulseUs, const Settings &settings) const
{
  // Convert pulse range to 0..180 for user-friendly display.
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

uint8_t AppController::pulseToPercent(const uint16_t pulseUs, const Settings &settings) const
{
  // Convert pulse range to 0..100 for gauge screen.
  if (settings.maxPulseUs <= settings.minPulseUs)
  {
    return 0;
  }

  const long percent = map(pulseUs, settings.minPulseUs, settings.maxPulseUs, 0, 100);
  if (percent < 0)
  {
    return 0;
  }

  if (percent > 100)
  {
    return 100;
  }

  return static_cast<uint8_t>(percent);
}

uint16_t AppController::stabilizeDisplayPulseUs(const uint16_t rawPulseUs, const Settings &settings)
{
  // First sample initializes the filter to current value.
  if (!displayPulseInitialized_)
  {
    displayPulseUs_ = rawPulseUs;
    displayPulseInitialized_ = true;
    return displayPulseUs_;
  }

  if (settings.maxPulseUs <= settings.minPulseUs)
  {
    displayPulseUs_ = rawPulseUs;
    return displayPulseUs_;
  }

  const int16_t spanUs = static_cast<int16_t>(settings.maxPulseUs - settings.minPulseUs);
  int16_t deadbandUs = (spanUs * static_cast<int16_t>(Config::GAUGE_DEADBAND_PERCENT)) / 100;
  int16_t maxStepUs = (spanUs * static_cast<int16_t>(Config::GAUGE_MAX_STEP_PERCENT)) / 100;
  if (deadbandUs < 1)
  {
    deadbandUs = 1;
  }
  if (maxStepUs < 1)
  {
    maxStepUs = 1;
  }

  const int16_t diff = static_cast<int16_t>(rawPulseUs) - static_cast<int16_t>(displayPulseUs_);
  if (abs(diff) <= deadbandUs)
  {
    // Ignore tiny oscillations from ADC/pot noise.
    return displayPulseUs_;
  }

  // Smooth toward target while limiting per-frame movement.
  int16_t step = diff / 3;
  if (step == 0)
  {
    step = (diff > 0) ? 1 : -1;
  }

  if (step > maxStepUs)
  {
    step = maxStepUs;
  }
  else if (step < -maxStepUs)
  {
    step = -maxStepUs;
  }

  int16_t next = static_cast<int16_t>(displayPulseUs_) + step;
  if (next < static_cast<int16_t>(settings.minPulseUs))
  {
    next = settings.minPulseUs;
  }
  else if (next > static_cast<int16_t>(settings.maxPulseUs))
  {
    next = settings.maxPulseUs;
  }

  displayPulseUs_ = static_cast<uint16_t>(next);
  return displayPulseUs_;
}

void AppController::drawUi()
{
  if (!displayReady_)
  {
    // Skip all rendering if display init failed.
    return;
  }

  if (uiMode_ == UI_STATUS)
  {
    if (statusScreen_ == SCREEN_DEFAULT)
    {
      DisplayUi::drawStatusScreen(
          displayPulseUs_,
          pulseToAngle(displayPulseUs_, savedSettings_),
          savedSettings_.minPulseUs,
          savedSettings_.maxPulseUs,
          savedSettings_.reverse,
          hvMode_,
          modeLabel(),
          controlMode_ == CONTROL_SWEEP,
          sweepCycleCounter_);
    }
    else if (statusScreen_ == SCREEN_GAUGE)
    {
      const uint8_t gaugePercent = pulseToPercent(displayPulseUs_, savedSettings_);
      DisplayUi::drawGaugeScreen(
          gaugePercent,
          hvMode_,
          modeLabel());
    }
    else if (statusScreen_ == SCREEN_CURRENT)
    {
      DisplayUi::drawCurrentScreen(
          inaMonitor_.ready(),
          inaMonitor_.ch1mA(),
          inaMonitor_.ch2mA(),
          inaMonitor_.ch3mA(),
          inaMonitor_.warnCh1(),
          inaMonitor_.warnCh2(),
          inaMonitor_.warnCh3(),
          inaMonitor_.critCh1(),
          inaMonitor_.critCh2(),
          inaMonitor_.critCh3(),
          hvMode_,
          modeLabel());
    }
    else if (statusScreen_ == SCREEN_VBUS)
    {
      DisplayUi::drawVoltageScreen(
          inaMonitor_.ready(),
          inaMonitor_.busCh1V(),
          inaMonitor_.busCh2V(),
          inaMonitor_.busCh3V(),
          inaMonitor_.droopCh1V(),
          inaMonitor_.droopCh2V(),
          inaMonitor_.droopCh3V(),
          inaMonitor_.warnCh1(),
          inaMonitor_.warnCh2(),
          inaMonitor_.warnCh3(),
          inaMonitor_.critCh1(),
          inaMonitor_.critCh2(),
          inaMonitor_.critCh3(),
          hvMode_,
          modeLabel());
    }
    else
    {
      DisplayUi::drawCurrentPeakScreen(
          inaMonitor_.ready(),
          inaMonitor_.peakCh1mA(),
          inaMonitor_.peakCh2mA(),
          inaMonitor_.peakCh3mA(),
          hvMode_,
          modeLabel());
    }

    return;
  }

  DisplayUi::drawSettingsScreen(
      selectedMenuItem_,
      uiMode_ == UI_MENU_EDIT,
      editSettings_.minPulseUs,
      editSettings_.maxPulseUs,
      editSettings_.reverse,
      editSettings_.sweepCycleSec);
}

void AppController::cycleStatusScreen()
{
  // Rotate STATUS -> GAUGE -> CURRENT -> VBUS -> PEAK -> STATUS.
  statusScreen_ = static_cast<StatusScreen>((statusScreen_ + 1) % SCREEN_COUNT);
}

void AppController::resetSweepState()
{
  // Start SWP mode from min pulse and clear cycle counter.
  sweepPulseUs_ = savedSettings_.minPulseUs;
  sweepDirection_ = 1;
  sweepCycleCounter_ = 0;
  sweepReachedMax_ = false;
  lastSweepStepMs_ = millis();
}

void AppController::cycleControlMode(const int8_t direction)
{
  // Circular mode switch with UP/DOWN buttons.
  int8_t nextMode = static_cast<int8_t>(controlMode_) + direction;
  if (nextMode < 0)
  {
    nextMode = static_cast<int8_t>(CONTROL_COUNT - 1);
  }
  else if (nextMode >= static_cast<int8_t>(CONTROL_COUNT))
  {
    nextMode = 0;
  }

  const ControlMode previousMode = controlMode_;
  controlMode_ = static_cast<ControlMode>(nextMode);
  if (controlMode_ != previousMode)
  {
    // Counter is mode-local, reset when leaving/entering modes.
    sweepCycleCounter_ = 0;
    sweepReachedMax_ = false;
  }

  if (controlMode_ == CONTROL_SWEEP)
  {
    resetSweepState();
  }
}

void AppController::enterSettingsMenu()
{
  // Work on a copy, commit only when user selects Save.
  editSettings_ = savedSettings_;
  selectedMenuItem_ = MENU_MIN_PULSE;
  uiMode_ = UI_MENU_NAVIGATION;
}

void AppController::exitSettingsMenu(const bool save)
{
  if (save)
  {
    // Apply and persist edited settings.
    savedSettings_ = editSettings_;
    SettingsStore::save(savedSettings_);

    if (controlMode_ == CONTROL_SWEEP)
    {
      resetSweepState();
    }
  }

  uiMode_ = UI_STATUS;
}

void AppController::adjustCurrentSetting(const int16_t delta)
{
  // Apply bounded edits per selected menu item.
  if (selectedMenuItem_ == MENU_MIN_PULSE)
  {
    const uint16_t maxAllowed = editSettings_.maxPulseUs - Config::MIN_PULSE_SPAN_US;
    long candidate = static_cast<long>(editSettings_.minPulseUs) + delta;

    if (candidate < Config::PULSE_MIN_LIMIT)
    {
      candidate = Config::PULSE_MIN_LIMIT;
    }

    if (candidate > maxAllowed)
    {
      candidate = maxAllowed;
    }

    editSettings_.minPulseUs = static_cast<uint16_t>(candidate);
    return;
  }

  if (selectedMenuItem_ == MENU_MAX_PULSE)
  {
    const uint16_t minAllowed = editSettings_.minPulseUs + Config::MIN_PULSE_SPAN_US;
    long candidate = static_cast<long>(editSettings_.maxPulseUs) + delta;

    if (candidate > Config::PULSE_MAX_LIMIT)
    {
      candidate = Config::PULSE_MAX_LIMIT;
    }

    if (candidate < minAllowed)
    {
      candidate = minAllowed;
    }

    editSettings_.maxPulseUs = static_cast<uint16_t>(candidate);
    return;
  }

  if (selectedMenuItem_ == MENU_SWEEP_CYCLE)
  {
    long candidate = static_cast<long>(editSettings_.sweepCycleSec) + (delta > 0 ? 1 : -1);

    if (candidate < Config::SWEEP_CYCLE_MIN_SEC)
    {
      candidate = Config::SWEEP_CYCLE_MIN_SEC;
    }

    if (candidate > Config::SWEEP_CYCLE_MAX_SEC)
    {
      candidate = Config::SWEEP_CYCLE_MAX_SEC;
    }

    editSettings_.sweepCycleSec = static_cast<uint16_t>(candidate);
  }
}

uint16_t AppController::computeSweepStepIntervalMs(const Settings &settings) const
{
  // Compute step period from target full-cycle duration.
  const uint32_t spanUs = static_cast<uint32_t>(settings.maxPulseUs - settings.minPulseUs);
  const uint32_t fullCycleUs = spanUs * 2U;
  const uint32_t stepsPerCycle = (fullCycleUs / Config::SWEEP_STEP_US) > 0U ? (fullCycleUs / Config::SWEEP_STEP_US) : 1U;
  const uint32_t cycleMs = static_cast<uint32_t>(settings.sweepCycleSec) * 1000U;
  const uint32_t intervalMs = cycleMs / stepsPerCycle;

  return static_cast<uint16_t>(intervalMs > 0U ? intervalMs : 1U);
}

void AppController::updateServoOutput(const unsigned long nowMs)
{
  // Three independent output strategies: POT, CENTER, SWEEP.
  if (controlMode_ == CONTROL_POT)
  {
    const int potRaw = analogRead(Config::POT_PIN);
    const int control = savedSettings_.reverse ? (1023 - potRaw) : potRaw;
    currentPulseUs_ = static_cast<uint16_t>(map(control, 0, 1023, savedSettings_.minPulseUs, savedSettings_.maxPulseUs));
  }
  else if (controlMode_ == CONTROL_CENTER)
  {
    currentPulseUs_ = static_cast<uint16_t>((savedSettings_.minPulseUs + savedSettings_.maxPulseUs) / 2);
  }
  else
  {
    if (sweepPulseUs_ < savedSettings_.minPulseUs)
    {
      sweepPulseUs_ = savedSettings_.minPulseUs;
      sweepDirection_ = 1;
      sweepReachedMax_ = false;
    }
    else if (sweepPulseUs_ > savedSettings_.maxPulseUs)
    {
      sweepPulseUs_ = savedSettings_.maxPulseUs;
      sweepDirection_ = -1;
      sweepReachedMax_ = true;
    }

    const uint16_t sweepStepIntervalMs = computeSweepStepIntervalMs(savedSettings_);
    if ((nowMs - lastSweepStepMs_) >= sweepStepIntervalMs)
    {
      lastSweepStepMs_ = nowMs;

      // Move one discrete step, then clamp/flip direction at limits.
      const int32_t stepUs = (sweepDirection_ > 0) ? static_cast<int32_t>(Config::SWEEP_STEP_US)
                                                   : -static_cast<int32_t>(Config::SWEEP_STEP_US);
      const int32_t nextPulse = static_cast<int32_t>(sweepPulseUs_) + stepUs;

      if (nextPulse >= static_cast<int32_t>(savedSettings_.maxPulseUs))
      {
        sweepPulseUs_ = savedSettings_.maxPulseUs;
        sweepDirection_ = -1;
        sweepReachedMax_ = true;
      }
      else if (nextPulse <= static_cast<int32_t>(savedSettings_.minPulseUs))
      {
        sweepPulseUs_ = savedSettings_.minPulseUs;
        sweepDirection_ = 1;
        if (sweepReachedMax_ && sweepCycleCounter_ < UINT32_MAX)
        {
          // Count one completed round-trip after MAX was reached.
          ++sweepCycleCounter_;
        }
        sweepReachedMax_ = false;
      }
      else
      {
        sweepPulseUs_ = static_cast<uint16_t>(nextPulse);
      }
    }

    currentPulseUs_ = sweepPulseUs_;
  }

  servoOutput_.writeMicroseconds(currentPulseUs_);
  // Filtering is applied only to rendered values, not to servo output.
  stabilizeDisplayPulseUs(currentPulseUs_, savedSettings_);
}

float AppController::readServoRailVoltageV() const
{
  // ADC divider reconstruction to servo rail voltage.
  const int raw = analogRead(Config::SERVO_VSENSE_PIN);
  const float adcV = (static_cast<float>(raw) * Config::SERVO_VSENSE_ADC_REF_V) / 1023.0f;
  const float ratio = (Config::SERVO_VSENSE_R1_OHMS + Config::SERVO_VSENSE_R2_OHMS) / Config::SERVO_VSENSE_R2_OHMS;
  return adcV * ratio;
}

void AppController::handleUiInput(const bool upPressed, const bool downPressed, const bool selectShortPress, const bool selectLongPress)
{
  // Input behavior depends on current UI mode.
  switch (uiMode_)
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
      if (selectedMenuItem_ == 0)
      {
        selectedMenuItem_ = static_cast<uint8_t>(MENU_ITEM_COUNT - 1);
      }
      else
      {
        --selectedMenuItem_;
      }
    }

    if (downPressed)
    {
      selectedMenuItem_ = static_cast<uint8_t>((selectedMenuItem_ + 1) % MENU_ITEM_COUNT);
    }

    if (selectShortPress)
    {
      if (selectedMenuItem_ == MENU_SAVE_EXIT)
      {
        exitSettingsMenu(true);
      }
      else if (selectedMenuItem_ == MENU_CANCEL)
      {
        exitSettingsMenu(false);
      }
      else
      {
        uiMode_ = UI_MENU_EDIT;
      }
    }
    break;

  case UI_MENU_EDIT:
    if (selectedMenuItem_ == MENU_REVERSE)
    {
      if (upPressed || downPressed || selectShortPress)
      {
        editSettings_.reverse = editSettings_.reverse ? 0 : 1;
        uiMode_ = UI_MENU_NAVIGATION;
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
        uiMode_ = UI_MENU_NAVIGATION;
      }
    }
    break;
  }
}

void AppController::begin()
{
  // Load validated settings and initialize runtime defaults.
  savedSettings_ = SettingsStore::load();
  currentPulseUs_ = savedSettings_.minPulseUs;
  sweepPulseUs_ = savedSettings_.minPulseUs;
  displayPulseUs_ = savedSettings_.minPulseUs;
  displayPulseInitialized_ = true;

  Wire.begin();
#if defined(WIRE_HAS_TIMEOUT)
  // Recover from locked I2C lines without full MCU reset.
  Wire.setWireTimeout(25000, true);
#endif

  displayReady_ = DisplayUi::begin();
  if (displayReady_)
  {
    DisplayUi::drawBootScreen();
  }

  servoOutput_.attach(Config::SERVO_PIN, Config::PULSE_MIN_LIMIT, Config::PULSE_MAX_LIMIT);
  pinMode(Config::POT_PIN, INPUT);
  pinMode(Config::SERVO_VSENSE_PIN, INPUT);

  ButtonInput::init(buttonUp_, Config::BTN_UP_PIN);
  ButtonInput::init(buttonDown_, Config::BTN_DOWN_PIN);
  ButtonInput::init(buttonSelect_, Config::BTN_SELECT_PIN);

  if (Config::MODE_SWITCH_PIN != 255)
  {
    pinMode(Config::MODE_SWITCH_PIN, Config::MODE_SWITCH_USE_PULLUP ? INPUT_PULLUP : INPUT);
  }

  inaMonitor_.begin(Wire);
  // Short startup delay to let peripherals stabilize visually/electrically.
  delay(600);
}

void AppController::update()
{
  // Single main-loop tick for sensing, control, and rendering.
  const unsigned long nowMs = millis();

  if (Config::MODE_SWITCH_PIN != 255)
  {
    // Optional discrete STD/HV switch input.
    const bool modeReading = Config::MODE_SWITCH_ACTIVE_LOW ? (digitalRead(Config::MODE_SWITCH_PIN) == LOW)
                                                            : (digitalRead(Config::MODE_SWITCH_PIN) == HIGH);
    hvMode_ = modeReading;
  }
  else
  {
    // Automatic mode detection from measured servo rail voltage.
    servoRailVoltageV_ = readServoRailVoltageV();
    hvMode_ = (servoRailVoltageV_ >= Config::SERVO_VSENSE_HV_THRESHOLD_V);
  }

  const bool upPressed = ButtonInput::updatePressed(buttonUp_, nowMs);
  const bool downPressed = ButtonInput::updatePressed(buttonDown_, nowMs);

  bool selectShortPress = false;
  bool selectLongPress = false;
  ButtonInput::updateSelectEvents(buttonSelect_, nowMs, selectShortPress, selectLongPress);

  updateServoOutput(nowMs);
  inaMonitor_.update(nowMs);
  handleUiInput(upPressed, downPressed, selectShortPress, selectLongPress);

  if ((nowMs - lastUiDrawMs_) >= Config::UI_REFRESH_MS || upPressed || downPressed || selectShortPress || selectLongPress)
  {
    drawUi();
    lastUiDrawMs_ = nowMs;
  }
}
