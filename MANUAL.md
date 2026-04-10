# Servo Tester - Manual Pouzivani

## 1. Co zarizeni umi

RC servo tester postaveny na Arduino Pro Mini:
- rizeni serva (`POT`, `CENTER`, `SWEEP`)
- OLED uzivatelske rozhrani
- mereni proudu/napeti pres INA3221 az na 3 kanalech

## 2. Zakladni zapojeni

Aktualni mapovani pinu je v `pins.txt`.

Minimum pro beh:
- Arduino Pro Mini + nahrany firmware
- OLED displej (software SPI)
- tlacitko `SELECT` (idealne i `UP` + `DOWN`)
- servo signal na `D6`
- spolecna `GND` mezi Arduinem a napajenim serv

INA3221 (volitelne):
- `A4` = `SDA`
- `A5` = `SCL`

## 3. Napajeni

- logika (Arduino + OLED + INA3221): stabilnich `5V`
- serva: podle typu (`STD`/`HV`)
- vzdy spolecna zem logiky a servo vetve

Doporuceni pro servo rail:
- bulk kondenzator `470-1000 uF`
- lokalni `100 nF` u kazdeho servo konektoru

## 4. Build a Upload

Z root slozky projektu:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -t upload
```

## 5. Ovladani

Ve status modu:
- kratky stisk `SELECT`: dalsi obrazovka
- dlouhy stisk `SELECT`: vstup do settings
- `UP` / `DOWN`: zmena control modu (`POT` / `CENTER` / `SWEEP`)

V settings:
- `UP` / `DOWN`: pohyb v menu nebo zmena hodnoty
- `SELECT`: potvrzeni / vstup do editace / ukonceni editace

## 6. Rezimy rizeni

- `POT`: poloha serva podle potenciometru
- `CENTER`: pevny stred
- `SWEEP`: automaticky cyklus min <-> max

V `SWEEP` se na status obrazovce zobrazuje citac cyklu.

## 7. OLED obrazovky

- `STATUS`: pulse, angle, range, reverse, SWP counter
- `GAUGE`: rucicka 0-100 %
- `CURRENT`: proudy na kanalech + `WR/CR`
- `VBUS`: napeti bus a pokles (`dV`) na kazdem kanalu
- `PEAK`: maximalni proud na kanalech

## 8. Settings

- `Min pulse`
- `Max pulse`
- `Reverse`
- `Sweep cycle` (`0.5-10.0 s`, default `3.0 s`)
- `Save & exit`
- `Cancel`

Nastaveni se uklada do EEPROM.

## 9. Alerty (`WR` / `CR`)

- `WR` = warning threshold
- `CR` = critical threshold

Thresholdy jsou v `include/config.h`.

## 10. Reseni problemu

### Servo cukne nebo se nehrybe spravne
- over napajeci napeti a proudovou rezervu pro servo
- over spolecnou `GND`
- otestuj nejdriv `CENTER` mod
- zkontroluj rozsah pulzu v settings

### OLED nic nezobrazuje
- over piny podle `include/config.h`
- over napajeni OLED a `GND`
- kdyz je treba, zkontroluj `CLK` a `MOSI`

### INA hlasi alerty na nepouzitem kanalu
- nepouzity kanal muze plavat
- sniz/vypni thresholdy pro nepouzity kanal
- HW reseni: vysokohodnotovy odpor mezi `IN+` a `IN-` na nepouzitem kanalu
