# Servo Tester

> **Work In Progress (WIP):** This project is under active development.  
> Features, wiring, and behavior may change between commits.

RC servo tester for Arduino Pro Mini (ATmega328P, 16 MHz) with OLED UI, settings in EEPROM, and 3-channel current monitoring via INA3221.

User manual: `MANUAL.md`

![Servo Tester Render](hardware/Images/Render.png)

## Features

- Servo control modes: `POT`, `CENTER`, `SWEEP`
- OLED screens: `STATUS`, `GAUGE`, `CURRENT`, `VBUS`, `PEAK`
- SWEEP cycle counter shown only in SWEEP mode
- Settings menu with EEPROM persistence
- INA3221 current measurement on 3 channels
- INA3221 bus voltage and droop (`dV`) view per channel
- INA3221 per-channel warning/critical current alerts (`WR`/`CR`)
- Optional alert LED output when any channel is in `WR` or `CR`
- Optional STD/HV mode indicator LEDs
- LED startup self-test
- Automatic unit display (`mA` / `A`) on current screens
- Servo rail mode detection (`STD` / `HV`) from ADC voltage sensing on `A1`
- Compile-time UI language selection through `include/config.h` and `include/lang.h`
- Minimal serial debug line for INA3221 communication check at boot

## Controls

- `SELECT` short press: switch LCD screen (`STATUS -> GAUGE -> CURRENT -> VBUS -> PEAK`)
- `SELECT` long press: enter settings menu
- `UP` / `DOWN` in status mode: change control mode (`POT`, `CENTER`, `SWEEP`)
- `UP` / `DOWN` in menu: navigate items or change value
- `SELECT` in menu: confirm

## Settings Menu

- `Min pulse` (us)
- `Max pulse` (us)
- `Reverse`
- `Sweep cycle` (0.5-10.0 s, default 3.0 s)
- `Burn cycles` (`0` = infinite SWEEP, otherwise stop after selected cycle count)
- `Save & exit`
- `Cancel`

## Pin Mapping (Default)

| Arduino Pin | Config Name | Function |
| --- | --- | --- |
| `D2` | `BTN_UP_PIN` | UP button |
| `D3` | `BTN_DOWN_PIN` | DOWN button |
| `D4` | `ALERT_LED_PIN` | Optional INA3221 warning/critical alert LED |
| `D5` | `BTN_SELECT_PIN` | SELECT button |
| `D6` | `SERVO_PIN` | Servo signal output |
| `D7` | `STD_MODE_LED_PIN` | STD servo rail indicator LED |
| `D8` | `HV_MODE_LED_PIN` | HV servo rail indicator LED |
| `D9` | `OLED_CLK` | OLED clock (`CLK` / `D0`) |
| `D10` | `OLED_MOSI` | OLED data (`MOSI` / `DIN` / `D1`) |
| `D11` | `OLED_RESET` | OLED reset |
| `D12` | `OLED_DC` | OLED data/command |
| `D13` | `OLED_CS` | OLED chip select |
| `A0` | `POT_PIN` | Servo control potentiometer |
| `A1` | `SERVO_VSENSE_PIN` | Servo rail voltage sensing through divider |
| `A4` | I2C `SDA` | INA3221 data |
| `A5` | I2C `SCL` | INA3221 clock |

Full mapping is in `pins.txt`.

## Configuration

Main configuration is in `include/config.h`.

You can adjust:

- UI language (`SERVO_TESTER_LANGUAGE`)
- pin assignments
- button debounce and long-press timing
- servo pulse limits and step size
- sweep timing and limits
- burn-in cycle count limits
- INA3221 shunt values and calibration factors
- ADC divider and HV threshold
- LED polarity and optional LED outputs

### UI Language

The UI language is selected at compile time to keep flash usage low:

```cpp
#define SERVO_TESTER_LANGUAGE SERVO_TESTER_LANG_EN
```

Available options:

- `SERVO_TESTER_LANG_EN`
- `SERVO_TESTER_LANG_CZ`

Text resources are stored in `include/lang.h`. Only the selected language is compiled into the firmware.

### Serial Debug

When `SERIAL_DEBUG_ENABLED` is enabled, the firmware prints one INA3221 status line after boot at `115200 baud`:

- `INA3221 OK`
- `INA3221 NOT FOUND`

This is intended only as a simple wiring/communication check.

## Build

PlatformIO environment:

- file: `platformio.ini`
- env: `pro16MHzatmega328`

Libraries used:

- Adafruit SSD1306
- Adafruit GFX Library
- Adafruit INA3221 Library
- Servo

## Project Layout

- `src/main.cpp`: entry point (`setup` / `loop`)
- `src/app_controller.*`: app state machine and high-level control flow
- `src/display_ui.*`: OLED rendering
- `src/ina_monitor.*`: INA3221 handling and peak tracking
- `src/button_input.*`: button debounce and short/long press events
- `src/settings_store.*`: EEPROM load/save/validation
- `src/app_types.h`: shared app structs/enums
- `hardware/Kicad/Servo Tester/`: KiCad project files for PCB/schematic
- `hardware/Schematics/`: exported schematic PDFs
- `hardware/Datasheets/`: component datasheets
- `hardware/enclosure/`: 3D printed enclosure files and notes
