#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <Arduino.h>

#include "app_types.h"

namespace ButtonInput
{
// Initialize button pin mode and reset debounce state.
void init(ButtonState &button, uint8_t pin);
// Return true once when a stable press edge is detected.
bool updatePressed(ButtonState &button, unsigned long nowMs);
// Generate short/long press events for a single button.
void updateSelectEvents(ButtonState &button, unsigned long nowMs, bool &shortPress, bool &longPress);
} // namespace ButtonInput

#endif
