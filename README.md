# Servo-Tester

RC servo tester for Arduino Pro Mini (ATmega328, 16 MHz) with an OLED settings menu.

## Features

- Servo control via potentiometer (`A0`)
- OLED UI (SSD1306, software SPI)
- Settings menu with options: `Min pulse`, `Max pulse`, `Reverse`, `Save & exit`, `Cancel`
- Settings stored in EEPROM

## Controls

- `SELECT` opens the Settings menu from the main screen
- `UP/DOWN` navigates menu items
- `SELECT` enters/confirms edit mode

## Default Pinout

- Servo: `D6`
- Potentiometer: `A0`
- Buttons: `UP` = `D2`, `DOWN` = `D3`, `SELECT` = `D5`
- OLED SSD1306 (software SPI): `MOSI` = `D9`, `CLK` = `D10`, `DC` = `D11`, `CS` = `D12`, `RESET` = `D13`

## Configuration

All main configuration is in `Servo Tester/include/config.h`:

- Pin mapping
- Debounce and UI timing
- Servo pulse limits/defaults
- EEPROM address/version
- Button polarity (`BUTTON_ACTIVE_LOW`) and internal pull-up usage (`BUTTON_USE_INTERNAL_PULLUP`)

## PlatformIO

The project is configured for PlatformIO in `Servo Tester/platformio.ini` (`env:pro16MHzatmega328`).
