#include "display_ui.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "app_types.h"
#include "config.h"
#include "lang.h"

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

void getMenuLabel(uint8_t item, char *buffer, size_t bufferLen, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleMs, uint16_t burnCycles)
{
  // Build one menu line based on current item index and live values.
  switch (item)
  {
  case 0:
    snprintf_P(buffer, bufferLen, TXT_FMT_MIN_PULSE, minPulseUs);
    break;
  case 1:
    snprintf_P(buffer, bufferLen, TXT_FMT_MAX_PULSE, maxPulseUs);
    break;
  case 2:
    snprintf_P(buffer, bufferLen, TXT_FMT_REVERSE, reverse ? TXT_ON : TXT_OFF);
    break;
  case 3:
    snprintf_P(buffer, bufferLen, TXT_FMT_SWEEP_CYCLE, sweepCycleMs / 1000U, (sweepCycleMs % 1000U) / 100U);
    break;
  case 4:
    snprintf_P(buffer, bufferLen, TXT_FMT_BURN_CYCLES, burnCycles);
    break;
  case 5:
    snprintf_P(buffer, bufferLen, TXT_FMT_SAVE_EXIT);
    break;
  case 6:
    snprintf_P(buffer, bufferLen, TXT_FMT_CANCEL);
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
    display.print(TXT_UNIT_A);
  }
  else
  {
    display.print(currentmA, 1);
    display.print(TXT_UNIT_MA);
  }
}

void printAlertToken(bool warn, bool crit)
{
  if (crit)
  {
    display.print(TXT_ALERT_CRIT);
  }
  else if (warn)
  {
    display.print(TXT_ALERT_WARN);
  }
  else
  {
    display.print(TXT_ALERT_NONE);
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
  display.print(TXT_HEADER_MODE);
  display.print(modeLabel != nullptr ? modeLabel : "---");

  display.setCursor(0, 9);
  display.print(TXT_HEADER_POWER);
  display.print(hvMode ? TXT_POWER_HV : TXT_POWER_STD);

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
  display.println(TXT_BOOT);
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
    uint32_t swpCounter,
    uint16_t burnCycles)
{
  display.clearDisplay();
  drawHeader(TXT_SCREEN_STATUS, hvMode, modeLabel);

  display.setCursor(0, 22);
  display.print(TXT_PULSE);
  display.print(currentPulseUs);
  display.println(TXT_US);

  display.setCursor(0, 34);
  display.print(TXT_ANGLE);
  display.print(angleDeg);
  display.println(TXT_DEG);

  display.setCursor(0, 46);
  display.print(TXT_RANGE);
  display.print(minPulseUs);
  display.print(TXT_DASH);
  display.print(maxPulseUs);

  display.setCursor(0, 56);
  display.print(TXT_REVERSE);
  display.print(reverse ? TXT_ON : TXT_OFF);
  if (showSwpCounter)
  {
    // SWP-only counter: number of completed MAX->MIN cycles.
    display.setCursor(56, 56);
    if (burnCycles > 0)
    {
      const uint16_t left = (swpCounter >= burnCycles) ? 0 : static_cast<uint16_t>(burnCycles - swpCounter);
      display.print(TXT_LEFT);
      display.print(left);
    }
    else
    {
      display.print(TXT_COUNT);
      display.print(swpCounter);
    }
  }
  else
  {
    display.setCursor(56, 56);
    display.print(TXT_HINT_MODE);
  }

  display.display();
}

void drawGaugeScreen(uint8_t valuePercent, bool hvMode, const char *modeLabel)
{
  display.clearDisplay();
  drawHeader(TXT_SCREEN_GAUGE, hvMode, modeLabel);

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
  display.print(TXT_PERCENT);

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
  drawHeader(TXT_SCREEN_CURRENT, hvMode, modeLabel);

  if (!sensorReady)
  {
    // Graceful error state if INA3221 is disconnected/miswired.
    display.setCursor(0, 26);
    display.print(TXT_INA_NOT_FOUND);
    display.setCursor(0, 38);
    display.print(TXT_CHECK_I2C);
    display.display();
    return;
  }

  display.setCursor(0, 22);
  display.print(TXT_CH1);
  printCurrentAutoUnit(currentCh1mA);
  display.setCursor(106, 22);
  printAlertToken(warnCh1, critCh1);

  display.setCursor(0, 34);
  display.print(TXT_CH2);
  printCurrentAutoUnit(currentCh2mA);
  display.setCursor(106, 34);
  printAlertToken(warnCh2, critCh2);

  display.setCursor(0, 46);
  display.print(TXT_CH3);
  printCurrentAutoUnit(currentCh3mA);
  display.setCursor(106, 46);
  printAlertToken(warnCh3, critCh3);

  display.setCursor(0, 56);
  display.print(TXT_HINT_NEXT_MENU);

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
  drawHeader(TXT_SCREEN_VBUS, hvMode, modeLabel);

  if (!sensorReady)
  {
    display.setCursor(0, 26);
    display.print(TXT_INA_NOT_FOUND);
    display.display();
    return;
  }

  display.setCursor(0, 22);
  display.print(F("1:"));
  display.print(busCh1V, 2);
  display.print(TXT_V_DROOP);
  display.print(droopCh1V, 2);
  display.setCursor(106, 22);
  printAlertToken(warnCh1, critCh1);

  display.setCursor(0, 34);
  display.print(F("2:"));
  display.print(busCh2V, 2);
  display.print(TXT_V_DROOP);
  display.print(droopCh2V, 2);
  display.setCursor(106, 34);
  printAlertToken(warnCh2, critCh2);

  display.setCursor(0, 46);
  display.print(F("3:"));
  display.print(busCh3V, 2);
  display.print(TXT_V_DROOP);
  display.print(droopCh3V, 2);
  display.setCursor(106, 46);
  printAlertToken(warnCh3, critCh3);

  display.setCursor(0, 56);
  display.print(TXT_HINT_NEXT_MENU);

  display.display();
}

void drawCurrentPeakScreen(bool sensorReady, float peakCh1mA, float peakCh2mA, float peakCh3mA, bool hvMode, const char *modeLabel)
{
  display.clearDisplay();
  drawHeader(TXT_SCREEN_PEAK, hvMode, modeLabel);

  if (!sensorReady)
  {
    display.setCursor(0, 26);
    display.print(TXT_INA_NOT_FOUND);
    display.display();
    return;
  }

  display.setCursor(0, 22);
  display.print(TXT_CH1_MAX);
  printCurrentAutoUnit(peakCh1mA);

  display.setCursor(0, 34);
  display.print(TXT_CH2_MAX);
  printCurrentAutoUnit(peakCh2mA);

  display.setCursor(0, 46);
  display.print(TXT_CH3_MAX);
  printCurrentAutoUnit(peakCh3mA);

  display.setCursor(0, 56);
  display.print(TXT_HINT_NEXT_MENU);

  display.display();
}

void drawSettingsScreen(uint8_t selectedMenuItem, bool editMode, uint16_t minPulseUs, uint16_t maxPulseUs, bool reverse, uint16_t sweepCycleMs, uint16_t burnCycles)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print(TXT_SETTINGS);
  if (editMode)
  {
    display.setCursor(86, 0);
    display.print(TXT_EDIT);
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
    getMenuLabel(item, line, sizeof(line), minPulseUs, maxPulseUs, reverse, sweepCycleMs, burnCycles);

    const int16_t y = rowStartY + (row * rowStepY);
    display.setCursor(0, y);
    display.print(item == selectedMenuItem ? '>' : ' ');
    display.print(line);
  }

  display.drawFastHLine(0, 54, Config::SCREEN_WIDTH, SSD1306_WHITE);
  display.setCursor(0, 56);
  if (editMode)
  {
    display.print(TXT_HINT_EDIT);
  }
  else
  {
    display.print(TXT_HINT_NAV);
  }

  display.display();
}
} // namespace DisplayUi
