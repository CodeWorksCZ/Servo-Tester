#include "button_input.h"

#include "config.h"

namespace ButtonInput
{
void init(ButtonState &button, uint8_t pin)
{
  button.pin = pin;
  // Use shared project polarity/pull-up configuration for all buttons.
  pinMode(pin, Config::BUTTON_USE_INTERNAL_PULLUP ? INPUT_PULLUP : INPUT);

  const bool pressed = Config::BUTTON_ACTIVE_LOW ? (digitalRead(pin) == LOW) : (digitalRead(pin) == HIGH);
  button.stablePressed = pressed;
  button.lastReading = pressed;
  button.lastDebounceMs = 0;
  button.pressedSinceMs = pressed ? millis() : 0;
  button.longPressSent = false;
}

bool updatePressed(ButtonState &button, const unsigned long nowMs)
{
  // Raw read with configurable active level (LOW or HIGH pressed).
  const bool reading = Config::BUTTON_ACTIVE_LOW ? (digitalRead(button.pin) == LOW) : (digitalRead(button.pin) == HIGH);

  if (reading != button.lastReading)
  {
    button.lastDebounceMs = nowMs;
  }

  button.lastReading = reading;

  if ((nowMs - button.lastDebounceMs) > Config::BUTTON_DEBOUNCE_MS && reading != button.stablePressed)
  {
    // Stable transition reached after debounce interval.
    button.stablePressed = reading;
    if (button.stablePressed)
    {
      return true;
    }
  }

  return false;
}

void updateSelectEvents(ButtonState &button, const unsigned long nowMs, bool &shortPress, bool &longPress)
{
  shortPress = false;
  longPress = false;

  const bool reading = Config::BUTTON_ACTIVE_LOW ? (digitalRead(button.pin) == LOW) : (digitalRead(button.pin) == HIGH);

  if (reading != button.lastReading)
  {
    button.lastDebounceMs = nowMs;
  }

  button.lastReading = reading;

  if ((nowMs - button.lastDebounceMs) > Config::BUTTON_DEBOUNCE_MS && reading != button.stablePressed)
  {
    button.stablePressed = reading;
    if (button.stablePressed)
    {
      // Press edge: start measuring hold time.
      button.pressedSinceMs = nowMs;
      button.longPressSent = false;
    }
    else
    {
      // Release edge: short press is emitted only if long press was not sent.
      const unsigned long pressDuration = nowMs - button.pressedSinceMs;
      if (!button.longPressSent && pressDuration < Config::BUTTON_LONG_PRESS_MS)
      {
        shortPress = true;
      }
    }
  }

  if (button.stablePressed && !button.longPressSent)
  {
    // While held, emit one long-press event after the configured threshold.
    const unsigned long pressDuration = nowMs - button.pressedSinceMs;
    if (pressDuration >= Config::BUTTON_LONG_PRESS_MS)
    {
      longPress = true;
      button.longPressSent = true;
    }
  }
}
} // namespace ButtonInput
