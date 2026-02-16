#ifndef SETTINGS_STORE_H
#define SETTINGS_STORE_H

#include "app_types.h"

namespace SettingsStore
{
Settings makeDefault();
bool isValid(const Settings &settings);
Settings load();
void save(const Settings &settings);
} // namespace SettingsStore

#endif
