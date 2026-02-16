#include "display_ui.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

namespace
{
Adafruit_SSD1306 display(
    Config::SCREEN_WIDTH,
    Config::SCREEN_HEIGHT,
    Config::OLED_MOSI,
    Config::OLED_CLK,
    Config::OLED_DC,
    Config::OLED_RESET,
    Config::OLED_CS);

void getMenuLabel(uint8_t item, char *buffer, size_t bufferLen, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleSec)
{
  switch (item)
  {
  case 0:
    snprintf(buffer, bufferLen, "Min pulse: %u", minPulseUs);
    break;
  case 1:
    snprintf(buffer, bufferLen, "Max pulse: %u", maxPulseUs);
    break;
  case 2:
    snprintf(buffer, bufferLen, "Reverse: %s", reverse ? "ON" : "OFF");
    break;
  case 3:
    snprintf(buffer, bufferLen, "Sweep cycle: %us", sweepCycleSec);
    break;
  case 4:
    snprintf(buffer, bufferLen, "Save & exit");
    break;
  case 5:
    snprintf(buffer, bufferLen, "Cancel");
    break;
  default:
    snprintf(buffer, bufferLen, "");
    break;
  }
}

void printCurrentAutoUnit(float currentmA)
{
  if (currentmA >= 1000.0f || currentmA <= -1000.0f)
  {
    display.print(currentmA / 1000.0f, 2);
    display.print(F(" A"));
  }
  else
  {
    display.print(currentmA, 1);
    display.print(F(" mA"));
  }
}

void drawHeader(const __FlashStringHelper *title, bool hvMode, const char *modeLabel)
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print(title);

  display.setCursor(74, 0);
  display.print(F("M:"));
  display.print(modeLabel != nullptr ? modeLabel : "---");

  display.setCursor(0, 9);
  display.print(F("PWR:"));
  display.print(hvMode ? F("HV") : F("STD"));

  display.drawFastHLine(0, 18, Config::SCREEN_WIDTH, SSD1306_WHITE);
}
} // namespace

namespace DisplayUi
{
bool begin()
{
  return display.begin(SSD1306_SWITCHCAPVCC);
}

void drawBootScreen()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.println(F("Starting..."));
  display.display();
}

void drawStatusScreen(
    uint16_t currentPulseUs,
    uint8_t angleDeg,
    uint16_t minPulseUs,
    uint16_t maxPulseUs,
    bool reverse,
    bool hvMode,
    const char *modeLabel,
    bool showSwpCounter,
    uint32_t swpCounter)
{
  display.clearDisplay();
  drawHeader(F("STATUS"), hvMode, modeLabel);

  display.setCursor(0, 22);
  display.print(F("Pulse:"));
  display.print(currentPulseUs);
  display.println(F("us"));

  display.setCursor(0, 34);
  display.print(F("Angle:"));
  display.print(angleDeg);
  display.println(F("deg"));

  display.setCursor(0, 46);
  display.print(F("Range:"));
  display.print(minPulseUs);
  display.print(F("-"));
  display.print(maxPulseUs);

  display.setCursor(0, 56);
  display.print(F("Rev:"));
  display.print(reverse ? F("ON") : F("OFF"));
  if (showSwpCounter)
  {
    display.setCursor(56, 56);
    display.print(F("Cnt:"));
    display.print(swpCounter);
  }
  else
  {
    display.setCursor(56, 56);
    display.print(F("U/D:Mode"));
  }

  display.display();
}

void drawCurrentScreen(bool sensorReady, float currentCh1mA, float currentCh2mA, float currentCh3mA, bool hvMode, const char *modeLabel)
{
  display.clearDisplay();
  drawHeader(F("CURRENT"), hvMode, modeLabel);

  if (!sensorReady)
  {
    display.setCursor(0, 26);
    display.print(F("INA3221 not found"));
    display.setCursor(0, 38);
    display.print(F("Check I2C wiring"));
    display.display();
    return;
  }

  display.setCursor(0, 22);
  display.print(F("CH1: "));
  printCurrentAutoUnit(currentCh1mA);

  display.setCursor(0, 34);
  display.print(F("CH2: "));
  printCurrentAutoUnit(currentCh2mA);

  display.setCursor(0, 46);
  display.print(F("CH3: "));
  printCurrentAutoUnit(currentCh3mA);

  display.setCursor(0, 56);
  display.print(F("S:Next H:Menu"));

  display.display();
}

void drawCurrentPeakScreen(bool sensorReady, float peakCh1mA, float peakCh2mA, float peakCh3mA, bool hvMode, const char *modeLabel)
{
  display.clearDisplay();
  drawHeader(F("PEAK"), hvMode, modeLabel);

  if (!sensorReady)
  {
    display.setCursor(0, 26);
    display.print(F("INA3221 not found"));
    display.display();
    return;
  }

  display.setCursor(0, 22);
  display.print(F("CH1 max:"));
  printCurrentAutoUnit(peakCh1mA);

  display.setCursor(0, 34);
  display.print(F("CH2 max:"));
  printCurrentAutoUnit(peakCh2mA);

  display.setCursor(0, 46);
  display.print(F("CH3 max:"));
  printCurrentAutoUnit(peakCh3mA);

  display.setCursor(0, 56);
  display.print(F("S:Next H:Menu"));

  display.display();
}

void drawSettingsScreen(uint8_t selectedMenuItem, bool editMode, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleSec)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print(F("Settings"));
  if (editMode)
  {
    display.setCursor(86, 0);
    display.print(F("EDIT"));
  }

  // Keep one line free for control hints at the bottom.
  constexpr uint8_t visibleRows = 3;
  constexpr uint8_t menuItemCount = 5;
  constexpr int16_t rowStartY = 14;
  constexpr int16_t rowStepY = 14;
  uint8_t firstVisible = 0;

  if (selectedMenuItem >= visibleRows)
  {
    firstVisible = selectedMenuItem - visibleRows + 1;
  }

  for (uint8_t row = 0; row < visibleRows; ++row)
  {
    const uint8_t item = firstVisible + row;
    if (item >= menuItemCount)
    {
      break;
    }

    char line[24];
    getMenuLabel(item, line, sizeof(line), minPulseUs, maxPulseUs, reverse, sweepCycleSec);

    const int16_t y = rowStartY + (row * rowStepY);
    display.setCursor(0, y);
    display.print(item == selectedMenuItem ? '>' : ' ');
    display.print(line);
  }

  display.drawFastHLine(0, 54, Config::SCREEN_WIDTH, SSD1306_WHITE);
  display.setCursor(0, 56);
  if (editMode)
  {
    display.print(F("U/D:Val SEL:Done"));
  }
  else
  {
    display.print(F("U/D:Item SEL:OK"));
  }

  display.display();
}
} // namespace DisplayUi
