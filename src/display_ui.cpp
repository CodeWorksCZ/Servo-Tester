#include "display_ui.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "app_types.h"
#include "config.h"

namespace
{
// Software SPI SSD1306 instance.
Adafruit_SSD1306 display(
    Config::SCREEN_WIDTH,
    Config::SCREEN_HEIGHT,
    Config::OLED_MOSI,
    Config::OLED_CLK,
    Config::OLED_DC,
    Config::OLED_RESET,
    Config::OLED_CS);

constexpr uint8_t GAUGE_POINT_COUNT = 9;
constexpr int16_t GAUGE_PIVOT_X = 64;
constexpr int16_t GAUGE_PIVOT_Y = 58;
const int16_t GAUGE_ARC_X[GAUGE_POINT_COUNT] = {22, 30, 40, 52, 64, 76, 88, 98, 106};
const int16_t GAUGE_ARC_Y[GAUGE_POINT_COUNT] = {48, 38, 31, 26, 24, 26, 31, 38, 48};

void gaugeNeedleEndpoint(uint8_t valuePercent, int16_t &xOut, int16_t &yOut)
{
  const uint8_t clamped = (valuePercent > 100) ? 100 : valuePercent;
  const uint16_t scaled = static_cast<uint16_t>(clamped) * static_cast<uint16_t>((GAUGE_POINT_COUNT - 1) * 16U) / 100U;
  const uint8_t segment = scaled / 16U;
  const uint8_t frac = scaled % 16U;

  if (segment >= (GAUGE_POINT_COUNT - 1))
  {
    xOut = GAUGE_ARC_X[GAUGE_POINT_COUNT - 1];
    yOut = GAUGE_ARC_Y[GAUGE_POINT_COUNT - 1];
    return;
  }

  const int16_t x0 = GAUGE_ARC_X[segment];
  const int16_t y0 = GAUGE_ARC_Y[segment];
  const int16_t x1 = GAUGE_ARC_X[segment + 1];
  const int16_t y1 = GAUGE_ARC_Y[segment + 1];

  xOut = x0 + ((x1 - x0) * frac) / 16;
  yOut = y0 + ((y1 - y0) * frac) / 16;
}

void getMenuLabel(uint8_t item, char *buffer, size_t bufferLen, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleMs)
{
  // Build one menu line based on current item index and live values.
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
    snprintf(buffer, bufferLen, "Sweep cycle: %u.%us", sweepCycleMs / 1000U, (sweepCycleMs % 1000U) / 100U);
    break;
  case 4:
    snprintf(buffer, bufferLen, "Save & exit");
    break;
  case 5:
    snprintf(buffer, bufferLen, "Cancel");
    break;
  default:
    if (bufferLen > 0)
    {
      buffer[0] = '\0';
    }
    break;
  }
}

void printCurrentAutoUnit(float currentmA)
{
  // Use amps above 1000 mA for better readability on 128x64 display.
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

void printAlertToken(bool warn, bool crit)
{
  if (crit)
  {
    display.print(F("CR"));
  }
  else if (warn)
  {
    display.print(F("WR"));
  }
  else
  {
    display.print(F("--"));
  }
}

void drawHeader(const __FlashStringHelper *title, bool hvMode, const char *modeLabel)
{
  // Common two-line header shared by all runtime screens.
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
  // Display object already has pin mapping from config constants.
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
    // SWP-only counter: number of completed MAX->MIN cycles.
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

void drawGaugeScreen(uint8_t valuePercent, bool hvMode, const char *modeLabel)
{
  display.clearDisplay();
  drawHeader(F("GAUGE"), hvMode, modeLabel);

  for (uint8_t i = 0; i < (GAUGE_POINT_COUNT - 1); ++i)
  {
    display.drawLine(GAUGE_ARC_X[i], GAUGE_ARC_Y[i], GAUGE_ARC_X[i + 1], GAUGE_ARC_Y[i + 1], SSD1306_WHITE);
  }

  display.setCursor(6, 49);
  display.print(F("0"));
  display.setCursor(108, 49);
  display.print(F("100"));

  int16_t needleX = GAUGE_ARC_X[0];
  int16_t needleY = GAUGE_ARC_Y[0];
  gaugeNeedleEndpoint(valuePercent, needleX, needleY);
  display.drawLine(GAUGE_PIVOT_X, GAUGE_PIVOT_Y, needleX, needleY, SSD1306_WHITE);
  display.fillCircle(GAUGE_PIVOT_X, GAUGE_PIVOT_Y, 2, SSD1306_WHITE);

  display.setCursor(54, 49);
  display.print(valuePercent);
  display.print(F("%"));

  display.display();
}

void drawCurrentScreen(
    bool sensorReady,
    float currentCh1mA,
    float currentCh2mA,
    float currentCh3mA,
    bool warnCh1,
    bool warnCh2,
    bool warnCh3,
    bool critCh1,
    bool critCh2,
    bool critCh3,
    bool hvMode,
    const char *modeLabel)
{
  display.clearDisplay();
  drawHeader(F("CURRENT"), hvMode, modeLabel);

  if (!sensorReady)
  {
    // Graceful error state if INA3221 is disconnected/miswired.
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
  display.setCursor(106, 22);
  printAlertToken(warnCh1, critCh1);

  display.setCursor(0, 34);
  display.print(F("CH2: "));
  printCurrentAutoUnit(currentCh2mA);
  display.setCursor(106, 34);
  printAlertToken(warnCh2, critCh2);

  display.setCursor(0, 46);
  display.print(F("CH3: "));
  printCurrentAutoUnit(currentCh3mA);
  display.setCursor(106, 46);
  printAlertToken(warnCh3, critCh3);

  display.setCursor(0, 56);
  display.print(F("S:Next H:Menu"));

  display.display();
}

void drawVoltageScreen(
    bool sensorReady,
    float busCh1V,
    float busCh2V,
    float busCh3V,
    float droopCh1V,
    float droopCh2V,
    float droopCh3V,
    bool warnCh1,
    bool warnCh2,
    bool warnCh3,
    bool critCh1,
    bool critCh2,
    bool critCh3,
    bool hvMode,
    const char *modeLabel)
{
  display.clearDisplay();
  drawHeader(F("VBUS"), hvMode, modeLabel);

  if (!sensorReady)
  {
    display.setCursor(0, 26);
    display.print(F("INA3221 not found"));
    display.display();
    return;
  }

  display.setCursor(0, 22);
  display.print(F("1:"));
  display.print(busCh1V, 2);
  display.print(F("V d"));
  display.print(droopCh1V, 2);
  display.setCursor(106, 22);
  printAlertToken(warnCh1, critCh1);

  display.setCursor(0, 34);
  display.print(F("2:"));
  display.print(busCh2V, 2);
  display.print(F("V d"));
  display.print(droopCh2V, 2);
  display.setCursor(106, 34);
  printAlertToken(warnCh2, critCh2);

  display.setCursor(0, 46);
  display.print(F("3:"));
  display.print(busCh3V, 2);
  display.print(F("V d"));
  display.print(droopCh3V, 2);
  display.setCursor(106, 46);
  printAlertToken(warnCh3, critCh3);

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

void drawSettingsScreen(uint8_t selectedMenuItem, bool editMode, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleMs)
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
  constexpr uint8_t menuItemCount = static_cast<uint8_t>(MENU_ITEM_COUNT);
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
    getMenuLabel(item, line, sizeof(line), minPulseUs, maxPulseUs, reverse, sweepCycleMs);

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
