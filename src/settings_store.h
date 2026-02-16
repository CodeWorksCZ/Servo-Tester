#ifndef SETTINGS_STORE_H
#define SETTINGS_STORE_H

#include "app_types.h"

namespace SettingsStore
{
// Build default settings from compile-time config constants.
Settings makeDefault();
// Validate loaded settings before using them.
bool isValid(const Settings &settings);
// Load settings from EEPROM or restore defaults if invalid.
Settings load();
// Save settings to EEPROM.
void save(const Settings &settings);
} // namespace SettingsStore

#endif
