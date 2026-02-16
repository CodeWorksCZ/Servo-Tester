# Servo-Tester
RC servo tester pro Arduino Pro Mini (ATmega328, 16 MHz) s OLED menu pro nastaveni.

## Funkce
- Ovládání serva potenciometrem (`A0`)
- OLED UI (SSD1306, software SPI)
- Settings menu:
  - `Min pulse`
  - `Max pulse`
  - `Reverse`
  - `Save & exit`
  - `Cancel`
- Ukládání nastavení do EEPROM

## Ovládání
- `SELECT` otevře Settings menu z hlavní obrazovky
- `UP/DOWN` pohyb v menu
- `SELECT` vstup/potvrzení editace

## Výchozí piny
- Servo: `D6`
- Potenciometr: `A0`
- Tlačítka:
  - `UP` = `D2`
  - `DOWN` = `D3`
  - `SELECT` = `D5`
- OLED SSD1306 (SW SPI):
  - `MOSI` = `D9`
  - `CLK` = `D10`
  - `DC` = `D11`
  - `CS` = `D12`
  - `RESET` = `D13`

## Konfigurace
Všechna hlavní nastavení jsou v `Servo Tester/include/config.h`:
- pinout
- debounce a UI timing
- limity servo pulzů
- EEPROM adresa/verze
- polarita tlačítek (`BUTTON_ACTIVE_LOW`) a použití interního pull-up (`BUTTON_USE_INTERNAL_PULLUP`)

## PlatformIO
Projekt je připraven pro PlatformIO v `Servo Tester/platformio.ini` (`env:pro16MHzatmega328`).
