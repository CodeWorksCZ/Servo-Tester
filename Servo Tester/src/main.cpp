#include <Arduino.h>
#include <SPI.h>
#include <Servo.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// Global display instance configured for software SPI pins from config.
Adafruit_SSD1306 display(
    Config::SCREEN_WIDTH,
    Config::SCREEN_HEIGHT,
    Config::OLED_MOSI,
    Config::OLED_CLK,
    Config::OLED_DC,
    Config::OLED_RESET,
    Config::OLED_CS);

// Persisted user settings stored in EEPROM.
struct Settings
{
  uint8_t version;
  uint16_t minPulseUs;
  uint16_t maxPulseUs;
  uint8_t reverse;
};

// Debounced button state container.
struct Button
{
  uint8_t pin;
  bool stablePressed;
  bool lastReading;
  unsigned long lastDebounceMs;
};

// Top-level UI states.
enum UiMode : uint8_t
{
  UI_STATUS = 0,
  UI_MENU_NAVIGATION = 1,
  UI_MENU_EDIT = 2
};

// Menu entries shown on the Settings screen.
enum MenuItem : uint8_t
{
  MENU_MIN_PULSE = 0,
  MENU_MAX_PULSE = 1,
  MENU_REVERSE = 2,
  MENU_SAVE_EXIT = 3,
  MENU_CANCEL = 4,
  MENU_ITEM_COUNT = 5
};

// Runtime state.
Servo servoOutput;
Settings savedSettings;
Settings editSettings;

Button buttonUp;
Button buttonDown;
Button buttonSelect;

UiMode uiMode = UI_STATUS;
uint8_t selectedMenuItem = MENU_MIN_PULSE;
uint16_t currentPulseUs = Config::PULSE_DEFAULT_MIN;
unsigned long lastUiDrawMs = 0;
bool displayReady = false;

// Factory defaults used when EEPROM data is invalid or missing.
Settings defaultSettings()
{
  Settings s{};
  s.version = Config::SETTINGS_VERSION;
  s.minPulseUs = Config::PULSE_DEFAULT_MIN;
  s.maxPulseUs = Config::PULSE_DEFAULT_MAX;
  s.reverse = 0;
  return s;
}

// Sanity-check loaded settings before they are used.
bool isValidSettings(const Settings &s)
{
  if (s.version != Config::SETTINGS_VERSION)
  {
    return false;
  }

  if (s.minPulseUs < Config::PULSE_MIN_LIMIT || s.maxPulseUs > Config::PULSE_MAX_LIMIT)
  {
    return false;
  }

  if (s.maxPulseUs <= s.minPulseUs)
  {
    return false;
  }

  if ((s.maxPulseUs - s.minPulseUs) < Config::MIN_PULSE_SPAN_US)
  {
    return false;
  }

  if (s.reverse > 1)
  {
    return false;
  }

  return true;
}

// Save current settings struct into EEPROM.
void saveSettings()
{
  EEPROM.put(Config::EEPROM_ADDR, savedSettings);
}

// Load settings from EEPROM, fallback to defaults if needed.
void loadSettings()
{
  EEPROM.get(Config::EEPROM_ADDR, savedSettings);
  if (!isValidSettings(savedSettings))
  {
    savedSettings = defaultSettings();
    saveSettings();
  }
}

// Initialize one button and prime its debounce state.
void initButton(Button &button, uint8_t pin)
{
  button.pin = pin;
  // Current wiring uses internal pull-up and active-low button logic.
  pinMode(pin, INPUT_PULLUP);

  const bool pressed = (digitalRead(pin) == LOW);
  button.stablePressed = pressed;
  button.lastReading = pressed;
  button.lastDebounceMs = 0;
}

// Returns true once on a clean press edge (debounced).
bool updateButtonPressed(Button &button, const unsigned long nowMs)
{
  const bool reading = (digitalRead(button.pin) == LOW);

  if (reading != button.lastReading)
  {
    button.lastDebounceMs = nowMs;
  }

  button.lastReading = reading;

  if ((nowMs - button.lastDebounceMs) > Config::BUTTON_DEBOUNCE_MS && reading != button.stablePressed)
  {
    button.stablePressed = reading;
    if (button.stablePressed)
    {
      return true;
    }
  }

  return false;
}

// Compose a visible label for one settings menu item.
void getMenuLabel(const uint8_t item, char *buffer, const size_t bufferLen)
{
  switch (item)
  {
  case MENU_MIN_PULSE:
    snprintf(buffer, bufferLen, "Min pulse: %u", editSettings.minPulseUs);
    break;
  case MENU_MAX_PULSE:
    snprintf(buffer, bufferLen, "Max pulse: %u", editSettings.maxPulseUs);
    break;
  case MENU_REVERSE:
    snprintf(buffer, bufferLen, "Reverse: %s", editSettings.reverse ? "ON" : "OFF");
    break;
  case MENU_SAVE_EXIT:
    snprintf(buffer, bufferLen, "Save & exit");
    break;
  case MENU_CANCEL:
    snprintf(buffer, bufferLen, "Cancel");
    break;
  default:
    snprintf(buffer, bufferLen, "");
    break;
  }
}

// Convert current pulse width to a 0-180 preview angle.
uint8_t pulseToAngle(const uint16_t pulseUs, const Settings &settings)
{
  if (settings.maxPulseUs <= settings.minPulseUs)
  {
    return 0;
  }

  const long angle = map(pulseUs, settings.minPulseUs, settings.maxPulseUs, 0, 180);
  if (angle < 0)
  {
    return 0;
  }

  if (angle > 180)
  {
    return 180;
  }

  return static_cast<uint8_t>(angle);
}

// Draw main status screen with current output values.
void drawStatusScreen()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(F("Servo Tester"));

  display.setCursor(0, 14);
  display.print(F("Pulse: "));
  display.print(currentPulseUs);
  display.println(F(" us"));

  display.setCursor(0, 26);
  display.print(F("Angle: "));
  display.print(pulseToAngle(currentPulseUs, savedSettings));
  display.println(F(" deg"));

  display.setCursor(0, 38);
  display.print(F("Range: "));
  display.print(savedSettings.minPulseUs);
  display.print(F("-"));
  display.print(savedSettings.maxPulseUs);

  display.setCursor(0, 50);
  display.print(F("Rev: "));
  display.print(savedSettings.reverse ? F("ON") : F("OFF"));
  display.setCursor(68, 50);
  display.print(F("SEL=Menu"));

  display.display();
}

// Draw settings menu, including simple scrolling window.
void drawSettingsScreen()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print(F("Settings"));
  if (uiMode == UI_MENU_EDIT)
  {
    display.setCursor(86, 0);
    display.print(F("EDIT"));
  }

  constexpr uint8_t visibleRows = 4;
  uint8_t firstVisible = 0;

  if (selectedMenuItem >= visibleRows)
  {
    firstVisible = selectedMenuItem - visibleRows + 1;
  }

  for (uint8_t row = 0; row < visibleRows; ++row)
  {
    const uint8_t item = firstVisible + row;
    if (item >= MENU_ITEM_COUNT)
    {
      break;
    }

    char line[24];
    getMenuLabel(item, line, sizeof(line));

    const int16_t y = 14 + (row * 12);
    display.setCursor(0, y);
    display.print(item == selectedMenuItem ? '>' : ' ');
    display.print(line);
  }

  display.display();
}

// Route drawing to the active screen.
void drawUi()
{
  if (!displayReady)
  {
    return;
  }

  if (uiMode == UI_STATUS)
  {
    drawStatusScreen();
  }
  else
  {
    drawSettingsScreen();
  }
}

// Enter menu mode and start editing a working copy.
void enterSettingsMenu()
{
  editSettings = savedSettings;
  selectedMenuItem = MENU_MIN_PULSE;
  uiMode = UI_MENU_NAVIGATION;
}

// Exit menu mode and optionally persist edited settings.
void exitSettingsMenu(const bool save)
{
  if (save)
  {
    savedSettings = editSettings;
    saveSettings();
  }

  uiMode = UI_STATUS;
}

// Adjust currently selected numeric setting while preserving bounds.
void adjustCurrentSetting(const int16_t delta)
{
  if (selectedMenuItem == MENU_MIN_PULSE)
  {
    const uint16_t maxAllowed = editSettings.maxPulseUs - Config::MIN_PULSE_SPAN_US;
    long candidate = static_cast<long>(editSettings.minPulseUs) + delta;
    if (candidate < Config::PULSE_MIN_LIMIT)
    {
      candidate = Config::PULSE_MIN_LIMIT;
    }
    if (candidate > maxAllowed)
    {
      candidate = maxAllowed;
    }
    editSettings.minPulseUs = static_cast<uint16_t>(candidate);
  }
  else if (selectedMenuItem == MENU_MAX_PULSE)
  {
    const uint16_t minAllowed = editSettings.minPulseUs + Config::MIN_PULSE_SPAN_US;
    long candidate = static_cast<long>(editSettings.maxPulseUs) + delta;
    if (candidate > Config::PULSE_MAX_LIMIT)
    {
      candidate = Config::PULSE_MAX_LIMIT;
    }
    if (candidate < minAllowed)
    {
      candidate = minAllowed;
    }
    editSettings.maxPulseUs = static_cast<uint16_t>(candidate);
  }
}

// Map potentiometer input to pulse output and drive servo.
void updateServoOutput()
{
  const int potRaw = analogRead(Config::POT_PIN);
  const int control = savedSettings.reverse ? (1023 - potRaw) : potRaw;
  currentPulseUs = static_cast<uint16_t>(map(control, 0, 1023, savedSettings.minPulseUs, savedSettings.maxPulseUs));
  servoOutput.writeMicroseconds(currentPulseUs);
}

// Handle button-driven UI state transitions and value edits.
void handleUiInput(const bool upPressed, const bool downPressed, const bool selectPressed)
{
  switch (uiMode)
  {
  case UI_STATUS:
    if (selectPressed)
    {
      enterSettingsMenu();
    }
    break;

  case UI_MENU_NAVIGATION:
    if (upPressed)
    {
      if (selectedMenuItem == 0)
      {
        selectedMenuItem = static_cast<uint8_t>(MENU_ITEM_COUNT - 1);
      }
      else
      {
        --selectedMenuItem;
      }
    }

    if (downPressed)
    {
      selectedMenuItem = static_cast<uint8_t>((selectedMenuItem + 1) % MENU_ITEM_COUNT);
    }

    if (selectPressed)
    {
      if (selectedMenuItem == MENU_SAVE_EXIT)
      {
        exitSettingsMenu(true);
      }
      else if (selectedMenuItem == MENU_CANCEL)
      {
        exitSettingsMenu(false);
      }
      else
      {
        uiMode = UI_MENU_EDIT;
      }
    }
    break;

  case UI_MENU_EDIT:
    if (selectedMenuItem == MENU_REVERSE)
    {
      if (upPressed || downPressed || selectPressed)
      {
        editSettings.reverse = editSettings.reverse ? 0 : 1;
        uiMode = UI_MENU_NAVIGATION;
      }
    }
    else
    {
      if (upPressed)
      {
        adjustCurrentSetting(Config::PULSE_STEP_US);
      }
      if (downPressed)
      {
        adjustCurrentSetting(-Config::PULSE_STEP_US);
      }
      if (selectPressed)
      {
        uiMode = UI_MENU_NAVIGATION;
      }
    }
    break;
  }
}

void setup()
{
  // Initialize persistent configuration and peripherals.
  loadSettings();

  servoOutput.attach(Config::SERVO_PIN, Config::PULSE_MIN_LIMIT, Config::PULSE_MAX_LIMIT);
  pinMode(Config::POT_PIN, INPUT);

  initButton(buttonUp, Config::BTN_UP_PIN);
  initButton(buttonDown, Config::BTN_DOWN_PIN);
  initButton(buttonSelect, Config::BTN_SELECT_PIN);

  displayReady = display.begin(SSD1306_SWITCHCAPVCC);
  if (displayReady)
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Servo Tester"));
    display.setCursor(0, 14);
    display.println(F("Starting..."));
    display.display();
    delay(600);
  }
}

void loop()
{
  // Read button events, update output, then refresh UI if needed.
  const unsigned long nowMs = millis();
  const bool upPressed = updateButtonPressed(buttonUp, nowMs);
  const bool downPressed = updateButtonPressed(buttonDown, nowMs);
  const bool selectPressed = updateButtonPressed(buttonSelect, nowMs);

  updateServoOutput();
  handleUiInput(upPressed, downPressed, selectPressed);

  if ((nowMs - lastUiDrawMs) >= Config::UI_REFRESH_MS || upPressed || downPressed || selectPressed)
  {
    drawUi();
    lastUiDrawMs = nowMs;
  }
}
