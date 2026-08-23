# Bench references

Pinouts, guides and gotchas for the parts in this build.

**The datasheets themselves live in [`../datasheets/`](../datasheets/)** — every part,
committed, with an index explaining where each came from. This file is for the things a
datasheet does not tell you.

## Board — Raspberry Pi Pico WH (RP2040)

- [Pico series documentation](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html)
  — Figure 5 is the wireless pinout. **One diagram covers Pico W, Pico WH and Pico 2 W**; the docs
  state the only visual differences are pre-soldered headers and the SWD interface.
- `PICO_BOARD=pico_w` — the SDK has no separate `pico_wh`; the H only means headers.
- BOOTSEL volume mounts as `RPI-RP2`. (An RP2350 board would mount as `RP2350` — useful for telling
  boards apart in a hurry.)
- [SWCLK/SWDIO reported swapped in the docs](https://github.com/raspberrypi/documentation/issues/3888)
  — filed against a "Pico WH 2022", the same board as ours. If SWD ever refuses to connect, swap
  those two wires before debugging anything else.

### Naming, because it is genuinely confusing

| Term | What it is |
|---|---|
| **RP2040** | The *chip*. 56-pin QFN. Not a board. |
| **Pico** | The *board* built around an RP2040. 40 pins. |
| **Pico W** | Pico + Infineon CYW43439 wireless. |
| **Pico WH** | Pico W with the headers already soldered. **This is ours.** |
| RP2350 | The newer chip. Not ours. |
| Pico 2 / Pico 2 W | Boards around the RP2350. Not ours. |

Confirmed rather than assumed: BOOTSEL mounts as `RPI-RP2` (an RP2350 board mounts as
`RP2350`), and the build uses `PICO_BOARD=pico_w`.

### The 40-pin header, for drawing schematic symbols

From Figure 2 of [`../datasheets/pico-w.pdf`](../datasheets/pico-w.pdf). **Pico and Pico W
have identical headers** — the wireless chip took over GP23, GP24, GP25 and GP29, and none
of those reach the header on either board.

| Pin | Name | | Pin | Name |
|---|---|---|---|---|
| 1 | `GP0` | | 40 | `VBUS` |
| 2 | `GP1` | | 39 | `VSYS` |
| 3 | `GND` | | 38 | `GND` |
| 4 | `GP2` | | 37 | `3V3_EN` |
| 5 | `GP3` | | 36 | `3V3(OUT)` |
| 6 | `GP4` | | 35 | `ADC_VREF` |
| 7 | `GP5` | | 34 | `GP28` / ADC2 |
| 8 | `GND` | | 33 | **`AGND`** |
| 9 | `GP6` | | 32 | `GP27` / ADC1 |
| 10 | `GP7` | | 31 | `GP26` / ADC0 |
| 11 | `GP8` | | 30 | `RUN` |
| 12 | `GP9` | | 29 | `GP22` |
| 13 | `GND` | | 28 | `GND` |
| 14 | `GP10` | | 27 | `GP21` |
| 15 | `GP11` | | 26 | `GP20` |
| 16 | `GP12` | | 25 | `GP19` |
| 17 | `GP13` | | 24 | `GP18` |
| 18 | `GND` | | 23 | `GND` |
| 19 | `GP14` | | 22 | `GP17` |
| 20 | `GP15` | | 21 | `GP16` |

Plus a separate 3-pin debug header on the bottom edge: `SWCLK`, `GND`, `SWDIO`.

**`AGND` on pin 33 deserves its own symbol pin**, not merging into the common GND. The gas
divider and its 100 nF return there specifically, and that only stays true on the PCB if the
schematic says so.

The **official schematic is Appendix B** of the Pico W datasheet in `datasheets/`, so the
authoritative symbol can be copied from Raspberry Pi's own drawing.

## I2C peripherals (GP4 = SDA pin 6, GP5 = SCL pin 7, power from 3V3 pin 36)

| Part | Reference | Notes |
|---|---|---|
| Gravity LCD1602 RGB (DFR0464) | [DFRobot wiki](https://wiki.dfrobot.com/dfr0464/) | 4-pin Gravity: `VCC` `GND` `SCL` `SDA`. 3.3–5.0 V — **use 3.3 V**, RP2040 pins are not 5 V tolerant. AiP31068 controller + PCA9633 backlight = two I2C addresses. |
| SHT31-D | [Adafruit pinouts](https://learn.adafruit.com/adafruit-sht31-d-temperature-and-humidity-sensor-breakout/pinouts) | `VIN` 2.5–5 V. Address 0x44, or 0x45 by pulling `ADDR` high. |
| SHT40 / 41 / 45 | [Adafruit pinouts](https://learn.adafruit.com/adafruit-sht40-temperature-humidity-sensor/pinouts) | `VIN` at logic level. Address 0x44. |

Both Sensirion parts share the 0x44 address, so `tools::i2c_scan()` identifies them actively:
SHT4x answers command `0x89` with a CRC'd serial number, SHT3x answers `0xF32D` with a status word.

## Everything else

| Part | Reference | Notes |
|---|---|---|
| MiCS-5524 gas sensor | [Adafruit guide](https://learn.adafruit.com/adafruit-mics5524-gas-sensor-breakout) | Power at **5 V** — the heater is sized for it. `EN` **high disables** the heater. Cannot distinguish gases, only change. |
| NeoPixels | [Adafruit Überguide](https://learn.adafruit.com/adafruit-neopixel-uberguide) | Source of the 1000 µF / 470 Ω / level-shifting guidance. |
| SN74HCT125N | [TI datasheet](https://www.ti.com/product/SN74HCT125) | Level shifter for pixel data if 3.3 V proves marginal. TTL thresholds, must be powered at 5 V. |
| USB-C breakout | [Adafruit 4090](https://www.adafruit.com/product/4090) | `VBUS` + `GND` to the rails; CC resistors advertise 1.5 A. |

## Toolchain

Installed by the VS Code Pico extension under `~/.pico-sdk/`: SDK 2.3.0, GCC 15_2_Rel1,
picotool 2.3.0, ninja, cmake. `go.sh`, `CMakeLists.txt` and `.vscode/settings.json` must all name
the same versions.
