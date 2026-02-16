#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>

namespace DisplayUi
{
// Initialize SSD1306 display object.
bool begin();
// Minimal boot splash while peripherals are initializing.
void drawBootScreen();
// Main status page with servo pulse/angle/range and SWP counter.
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
// Live current page for INA3221 channels.
void drawCurrentScreen(bool sensorReady, float currentCh1mA, float currentCh2mA, float currentCh3mA, bool hvMode, const char *modeLabel);
// Peak current page for INA3221 channels.
void drawCurrentPeakScreen(bool sensorReady, float peakCh1mA, float peakCh2mA, float peakCh3mA, bool hvMode, const char *modeLabel);
// Scrollable settings page with edit/navigation hints.
void drawSettingsScreen(uint8_t selectedMenuItem, bool editMode, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleSec);
} // namespace DisplayUi

#endif
