#include <Arduino.h>

#include "app_controller.h"

namespace
{
// Single app instance for the whole firmware runtime.
AppController app;
} // namespace

void setup()
{
  // Initialize all peripherals and application state.
  app.begin();
}

void loop()
{
  // Periodic non-blocking update of IO, state machine, and UI.
  app.update();
}
