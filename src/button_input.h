#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <Arduino.h>

#include "app_types.h"

namespace ButtonInput
{
void init(ButtonState &button, uint8_t pin);
bool updatePressed(ButtonState &button, unsigned long nowMs);
void updateSelectEvents(ButtonState &button, unsigned long nowMs, bool &shortPress, bool &longPress);
} // namespace ButtonInput

#endif
