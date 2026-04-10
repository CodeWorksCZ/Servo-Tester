# Servo Tester - User Manual

## 1. Device Purpose

This project is an RC servo tester based on Arduino Pro Mini with:
- servo control modes (`POT`, `CENTER`, `SWEEP`)
- OLED user interface
- current and voltage monitoring through INA3221 on up to 3 channels

## 2. Basic Wiring

Use `pins.txt` as the current source of truth for pin mapping.

Minimum required for operation:
- Arduino Pro Mini with uploaded firmware
- OLED display (software SPI)
- `SELECT` button, ideally also `UP` and `DOWN`
- servo signal on `D6`
- common `GND` between Arduino and servo power

Optional INA3221 connection:
- `A4` = `SDA`
- `A5` = `SCL`

## 3. Power

- logic side (Arduino + OLED + INA3221): stable `5V`
- servo side: according to servo type (`STD` or `HV`)
- always use a common ground between logic and servo power

Recommended on the servo rail:
- bulk capacitor `470-1000 uF`
- local `100 nF` capacitor near each servo connector

## 4. Build and Upload

From the project root:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -t upload
```

## 5. Controls

In status mode:
- short press `SELECT`: next screen
- long press `SELECT`: enter settings
- `UP` / `DOWN`: change control mode (`POT` / `CENTER` / `SWEEP`)

In settings:
- `UP` / `DOWN`: move in the menu or change the current value
- `SELECT`: confirm / enter edit mode / leave edit mode

## 6. Control Modes

- `POT`: servo position follows the potentiometer
- `CENTER`: fixed center position
- `SWEEP`: automatic cycle between minimum and maximum pulse

In `SWEEP`, the status screen shows a completed cycle counter.

## 7. OLED Screens

- `STATUS`: pulse, angle, range, reverse, SWP counter
- `GAUGE`: 0-100% needle gauge
- `CURRENT`: channel currents and `WR/CR` flags
- `VBUS`: bus voltage and voltage droop (`dV`) on each channel
- `PEAK`: maximum current on each channel

## 8. Settings

- `Min pulse`
- `Max pulse`
- `Reverse`
- `Sweep cycle` (`0.5-10.0 s`, default `3.0 s`)
- `Save & exit`
- `Cancel`

Settings are stored in EEPROM.

## 9. Alerts (`WR` / `CR`)

- `WR` = warning threshold
- `CR` = critical threshold

Threshold values are configured in `include/config.h`.

## 10. Troubleshooting

### Servo jitters or does not move correctly
- verify servo supply voltage and current capability
- verify common `GND`
- test `CENTER` mode first
- check pulse range in settings

### OLED shows nothing
- verify pins against `include/config.h`
- check OLED power and `GND`
- if needed, recheck `CLK` and `MOSI`

### INA reports alerts on an unused channel
- an unused channel may float
- reduce or disable thresholds for that unused channel
- hardware mitigation: add a high-value resistor between unused `IN+` and `IN-`
