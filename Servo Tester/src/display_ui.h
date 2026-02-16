#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>

namespace DisplayUi
{
bool begin();
void drawBootScreen();
void drawStatusScreen(
    uint16_t currentPulseUs,
    uint8_t angleDeg,
    uint16_t minPulseUs,
    uint16_t maxPulseUs,
    bool reverse,
    bool hvMode,
    const char *modeLabel,
    bool showSwpCounter,
    uint32_t swpCounter);
void drawCurrentScreen(bool sensorReady, float currentCh1mA, float currentCh2mA, float currentCh3mA, bool hvMode, const char *modeLabel);
void drawCurrentPeakScreen(bool sensorReady, float peakCh1mA, float peakCh2mA, float peakCh3mA, bool hvMode, const char *modeLabel);
void drawSettingsScreen(uint8_t selectedMenuItem, bool editMode, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleSec);
} // namespace DisplayUi

#endif
