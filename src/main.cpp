#include <Arduino.h>

#include "app_controller.h"

namespace
{
AppController app;
} // namespace

void setup()
{
  app.begin();
}

void loop()
{
  app.update();
}
