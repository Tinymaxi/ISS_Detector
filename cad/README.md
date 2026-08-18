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
| `usb-c-breakout-4090.step` | USB-C breakout — 24.8 × 14.4 × 11.7 mm | [Adafruit 4090](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/4090%20USB%20C%20Breakout) |

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
| MiCS-5524 gas sensor | No 3D model, but the board outline is exact: **12.70 × 19.05 mm**, read out of the dimension layer of Adafruit's own [EAGLE .brd](https://github.com/adafruit/Adafruit-MiCS-5524-Gas-Sensor-Breakout-PCB). More authoritative than any community model. |
| Generic SCD40 module (AliExpress) | No model exists. `scd40.step` here is **Adafruit's 5187 board, 30.5 × 27.9 mm — not the same board**. Measure yours. The SCD4x sensor package itself is drawn in Figure 3 (p. 24) of the [Sensirion datasheet](https://sensirion.com/media/documents/48C4B7FB/67FE0194/CD_DS_SCD4x_Datasheet_D1.pdf). |
| DFR0464 LCD1602 RGB | [SnapMagic](https://www.snapeda.com/parts/DFR0464/DFRobot/view-part/) has a 3D model; DFRobot's wiki has dimensions. |
| KY-040 rotary encoder | Generic part — [GrabCAD](https://grabcad.com/library/tag/ky-040) has several. |
| Piezo buzzer | Generic — pick a GrabCAD model matching your disc diameter. |
| RGB digit module | Depends on vendor; no standard model. |


## Panel cutouts

For the parts where no STEP file exists, these are the numbers that actually
matter — a hole diameter beats a mesh for enclosure work.

| Part | Panel hole | Body | Source |
|---|---|---|---|
| Arcade button AB30L5-B | **Ø 28.0 mm (M28)** | ~33 mm ⌀, ~29 mm deep | BerryBase datasheet in this folder |
| KY-040 encoder | **Ø 7 mm** threaded bush | PCB ~26 × 19 mm; knob 15 mm ⌀ × 16.5 mm | Supplier listings — **varies, measure yours** |
| Passive magnetic buzzer | Ø ~9.5 mm if front-mounted | **Ø 9.2 × 9 mm**, 4 mm pin pitch | bastelgarage listing |

**KY-040 dimensions vary between suppliers** — 15 × 16.5, 26 × 19, 30 × 18 and
32 × 19 mm are all listed by different sellers for the "same" module. Measure the
one in your hand rather than trusting any of them.

The Adafruit 471 model is a usable stand-in for the BerryBase arcade button
(34.4 mm body against the datasheet's ~33 mm), but note its Z axis extends to
228 mm because the wire harness is modelled at full length. Use the datasheet's
Ø 28 mm for the panel hole, not a measurement off the model.

## No STEP available

The KY-040 and the buzzer are commodity parts with no manufacturer CAD.
[GrabCAD](https://grabcad.com/library) has community models of both if a solid is
really wanted, though for a cylinder 9.2 mm across it is quicker to draw one.
