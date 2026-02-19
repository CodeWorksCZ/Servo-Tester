# Hardware Notes (WIP)

This folder contains schematics/PCB files for the Servo Tester hardware.

## Pre-Layout Checklist

- Keep logic rail (`VCC` for Pro Mini + OLED + INA3221) separate from servo power rail.
- Do not feed logic directly from HV servo rail.
- Use a common GND, but route high-current servo return separately and join at one star point.

## Servo Power Protection

- Add bulk capacitor on servo rail input: `470-1000 uF` (low-ESR, voltage rating for your rail).
- Add local decoupling near each servo connector: `100 nF` from `+BATT` to `GND`.
- Add TVS diode across servo rail (`+BATT` to `GND`) near power entry.
- Add PTC fuse in series with servo rail input.

## INA3221 Current Measurement

- Place shunts in series with servo `+` path only.
- Route INA sense lines as Kelvin connections directly to shunt pads.
- For channels that may stay unconnected, add `100k` between `INx+` and `INx-` to reduce floating behavior.
- For `R100` shunts (`0.1 ohm`), keep alert thresholds below ~`1.64 A`.

## Servo Signal Robustness

- Add series resistor on each servo signal line: `220-470 ohm` (`330 ohm` recommended).
- Keep signal and GND return close together in routing/connectors.

## Connector Safety

- Prefer keyed/polarized connectors where possible.
- If using standard 3-pin servo headers, clearly mark `SIG`, `+`, `-` on silkscreen.

## Firmware-Coupled Pins (Current Config)

- `D6`: Servo PWM output
- `D4`: Alert LED output
- `A4/A5`: INA3221 I2C
- `A0`: Potentiometer input
- `A1`: Servo rail voltage sense
