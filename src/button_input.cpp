#include "button_input.h"

#include "config.h"

namespace ButtonInput
{
void init(ButtonState &button, uint8_t pin)
{
  button.pin = pin;
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
      button.pressedSinceMs = nowMs;
      button.longPressSent = false;
    }
    else
    {
      const unsigned long pressDuration = nowMs - button.pressedSinceMs;
      if (!button.longPressSent && pressDuration < Config::BUTTON_LONG_PRESS_MS)
      {
        shortPress = true;
      }
    }
  }

  if (button.stablePressed && !button.longPressSent)
  {
    const unsigned long pressDuration = nowMs - button.pressedSinceMs;
    if (pressDuration >= Config::BUTTON_LONG_PRESS_MS)
    {
      longPress = true;
      button.longPressSent = true;
    }
  }
}
} // namespace ButtonInput
