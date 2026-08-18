# CAD models

Third-party STEP models for enclosure design. Run `./fetch.sh` to download them;
the binaries are gitignored, the script is not.

| File | Part | Source |
|---|---|---|
| `pico-w/` | Raspberry Pi Pico W | [Raspberry Pi datasheets](https://datasheets.raspberrypi.com/picow/PicoW-step.zip) — official |
| `neopixel-ring-24.step` | NeoPixel Ring 24 × 5050 | [Adafruit 1586](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/1586%20NeoPixel%20Ring%2024x) |
| `scd40.step` | SCD-40 CO₂ / temp / humidity | [Adafruit 5187](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/5187%20SCD-40%20C02%20Sensor) |
| `permaproto-half.step` | Perma-Proto Half-Size — 81.3 × 50.8 × 1.6 mm | [Adafruit 1609](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/1609%20Perma-Proto%20HalfSize) |
| `permaproto-quarter.step` | Perma-Proto Quarter-Size — 50.8 × 43.2 × 1.6 mm | [Adafruit 1608](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/1608%20perma-proto%20quarter) |
| `minty-permaproto.step` | Perma-Proto Mint Tin size — 91.4 × 56.9 × 1.6 mm | [Adafruit 723](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/723%20Minty%20Perma-Proto) |
| `arcade-button-30mm.step` | Arcade button, 30 mm | [Adafruit 471](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/471%20Arcade%20Button%2030mm) |

**The quarter-size model is an outline only.** It has 6 cylindrical surfaces against the
half-size model's 426, so the mounting holes are there but the perforated grid is not. Fine for
checking fit and clearance in an enclosure; useless for visualising where parts sit on the grid.
The half-size and mint-tin models are fully detailed.

**Pico WH** is mechanically a Pico W with headers fitted, so the Pico W model is the
right outline — but check whether the headers are modelled before relying on the
height.

## Not found as STEP

| Part | Where to look |
|---|---|
| MiCS-5524 gas sensor | Not in Adafruit's CAD repo. Request it on their [issues tab](https://github.com/adafruit/Adafruit_CAD_Parts/issues), or measure it. |
| DFR0464 LCD1602 RGB | [SnapMagic](https://www.snapeda.com/parts/DFR0464/DFRobot/view-part/) has a 3D model; DFRobot's wiki has dimensions. |
| KY-040 rotary encoder | Generic part — [GrabCAD](https://grabcad.com/library/tag/ky-040) has several. |
| Piezo buzzer | Generic — pick a GrabCAD model matching your disc diameter. |
| RGB digit module | Depends on vendor; no standard model. |
