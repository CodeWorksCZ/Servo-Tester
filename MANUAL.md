# Servo Tester - User Manual

## 1. Device Purpose

This project is an RC servo tester based on Arduino Pro Mini with:

- servo control modes (`POT`, `CENTER`, `SWEEP`)
- OLED user interface
- current and voltage monitoring through INA3221 on up to 3 channels
- STD/HV servo rail indication
- optional warning/critical alert LED

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

Optional LED outputs:

- `D4` = INA3221 warning/critical alert LED
- `D7` = STD servo rail indicator LED
- `D8` = HV servo rail indicator LED

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

In `SWEEP`, the status screen shows a cycle counter:

- infinite mode: completed cycle count
- burn-in mode: remaining cycles

## 7. OLED Screens

- `STATUS`: pulse, angle, range, reverse, SWP counter
- `GAUGE`: 0-100% needle gauge
- `CURRENT`: channel currents and `WR/CR` flags
- `VBUS`: bus voltage and voltage droop (`dV`) on each channel
- `PEAK`: maximum current on each channel

Short press `SELECT` to cycle through the screens:

```text
STATUS -> GAUGE -> CURRENT -> VBUS -> PEAK -> STATUS
```

### 7.1 STATUS Screen

Shows the active servo output and the selected control mode.

```text
STATUS                    M:POT
PWR:STD
-------------------------------
Pulse:1500us
Angle:90deg
Range:1000-2000
Rev:OFF            U/D:Mode
```

Fields:

- `M`: active control mode (`POT`, `CEN`, `SWP`)
- `PWR`: detected servo rail mode (`STD` or `HV`)
- `Pulse`: current servo pulse width in microseconds
- `Angle`: approximate 0-180 degree value calculated from the configured pulse range
- `Range`: configured minimum and maximum pulse
- `Rev`: reverse mode state

In `SWP` mode, the bottom-right area changes:

```text
Rev:OFF            Cnt:12
```

If `Burn cycles` is greater than `0`, it shows remaining cycles instead:

```text
Rev:OFF            L:900
```

### 7.2 GAUGE Screen

Shows servo position as a 0-100% gauge.

```text
GAUGE                     M:POT
PWR:STD
-------------------------------

        .---------.
   0   /           \   100
            \|
             o      50%
```

The gauge uses the configured servo pulse range:

- `0%` = configured minimum pulse
- `100%` = configured maximum pulse
- the needle is display-filtered to reduce small visible jitter from potentiometer noise

### 7.3 CURRENT Screen

Shows live current from the INA3221 channels.

```text
CURRENT                   M:POT
PWR:STD
-------------------------------
CH1: 125.4 mA          --
CH2: 0.0 mA            --
CH3: 1.25 A            WR
S:Next H:Menu
```

Fields:

- `CH1-CH3`: current per INA3221 channel
- `--`: no alert
- `WR`: warning threshold exceeded
- `CR`: critical threshold exceeded

The unit switches automatically:

- below `1000 mA`: shown as `mA`
- from `1000 mA`: shown as `A`

### 7.4 VBUS Screen

Shows bus voltage and voltage droop for each INA3221 channel.

```text
VBUS                      M:POT
PWR:STD
-------------------------------
1:6.05V d0.03          --
2:6.04V d0.04          --
3:5.91V d0.17          WR
S:Next H:Menu
```

Fields:

- left value: measured bus voltage on the channel
- `d`: voltage droop from the highest observed bus voltage since startup/reset
- alert token: `--`, `WR`, or `CR`

This screen is useful for checking voltage sag when a servo is loaded.

### 7.5 PEAK Screen

Shows maximum measured current per channel.

```text
PEAK                      M:POT
PWR:STD
-------------------------------
CH1 max:420.0 mA
CH2 max:0.0 mA
CH3 max:1.80 A
S:Next H:Menu
```

Peak values are tracked while the device is running. They are useful for checking short servo current spikes during movement or stall testing.

## 8. Settings

- `Min pulse`
- `Max pulse`
- `Reverse`
- `Sweep cycle` (`0.5-10.0 s`, default `3.0 s`)
- `Burn cycles` (`0` = infinite SWEEP, otherwise stop after selected cycle count)
- `Save & exit`
- `Cancel`

Settings are stored in EEPROM.

## 9. Alerts (`WR` / `CR`)

- `WR` = warning threshold
- `CR` = critical threshold

Threshold values are configured in `include/config.h`.

When any enabled INA3221 channel enters `WR` or `CR`, the optional alert LED on `D4` turns on.

## 10. STD/HV Indication

The firmware reads the servo rail voltage on `A1` through a resistor divider.

- below the configured threshold: `STD`
- above the configured threshold: `HV`

The active mode is shown on the OLED and can also be indicated by LEDs:

- `D7`: STD mode LED
- `D8`: HV mode LED

## 11. Language Selection

The OLED language is selected at compile time in `include/config.h`:

```cpp
#define SERVO_TESTER_LANGUAGE SERVO_TESTER_LANG_EN
```

Available options:

- `SERVO_TESTER_LANG_EN`
- `SERVO_TESTER_LANG_CZ`

Text resources are stored in `include/lang.h`.

## 12. Serial INA3221 Check

If `SERIAL_DEBUG_ENABLED` is enabled in `include/config.h`, the device prints one INA3221 status line after boot at `115200 baud`:

- `INA3221 OK`
- `INA3221 NOT FOUND`
