# Bench references

Datasheets and pinouts for the parts in this build.

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
