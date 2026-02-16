#include "settings_store.h"

#include <EEPROM.h>

#include "config.h"

namespace SettingsStore
{
Settings makeDefault()
{
  // Keep defaults centralized in config.h.
  Settings settings{};
  settings.version = Config::SETTINGS_VERSION;
  settings.minPulseUs = Config::PULSE_DEFAULT_MIN;
  settings.maxPulseUs = Config::PULSE_DEFAULT_MAX;
  settings.reverse = 0;
  settings.sweepCycleSec = Config::SWEEP_CYCLE_DEFAULT_SEC;
  return settings;
}

bool isValid(const Settings &settings)
{
  // Reject old EEPROM layout versions.
  if (settings.version != Config::SETTINGS_VERSION)
  {
    return false;
  }

  if (settings.minPulseUs < Config::PULSE_MIN_LIMIT || settings.maxPulseUs > Config::PULSE_MAX_LIMIT)
  {
    return false;
  }

  if (settings.maxPulseUs <= settings.minPulseUs)
  {
    return false;
  }

  if ((settings.maxPulseUs - settings.minPulseUs) < Config::MIN_PULSE_SPAN_US)
  {
    return false;
  }

  if (settings.reverse > 1)
  {
    return false;
  }

  if (settings.sweepCycleSec < Config::SWEEP_CYCLE_MIN_SEC || settings.sweepCycleSec > Config::SWEEP_CYCLE_MAX_SEC)
  {
    return false;
  }

  return true;
}

Settings load()
{
  Settings settings{};
  // Raw binary read of Settings struct from EEPROM.
  EEPROM.get(Config::EEPROM_ADDR, settings);

  if (!isValid(settings))
  {
    // Auto-recover from corrupted/old EEPROM content.
    settings = makeDefault();
    save(settings);
  }

  return settings;
}

void save(const Settings &settings)
{
  // EEPROM.put writes only changed bytes on AVR.
  EEPROM.put(Config::EEPROM_ADDR, settings);
}
} // namespace SettingsStore
