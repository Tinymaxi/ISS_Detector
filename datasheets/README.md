# Datasheets

Every part in the ISS-Box, in one place. `./fetch.sh` re-downloads the ones with a
public URL; three came by hand and have no stable link.

Unlike `cad/`, **these PDFs are committed.** Datasheet URLs rot, and the point of
the folder is that the numbers survive.

| File | Part | Used for |
|---|---|---|
| `rp2040.pdf` | RP2040 | GPIO drive strength, ADC, PIO, the 12 mA per-pin limit |
| `pico-w.pdf` | Raspberry Pi Pico W | Pinout, VSYS/VBUS, power tree |
| `scd4x.pdf` | Sensirion SCD40/41 | I²C commands, 205 mA pulses, decoupling, Figure 3 package drawing |
| `sht3x.pdf` | Sensirion SHT3x | The sensor the SCD40 replaced; CRC-8 poly shared with SCD4x |
| `pca9633-backlight.pdf` | NXP PCA9633 | The LCD's RGB backlight — MODE1 wake-up, PWM registers |
| `ws2812b.pdf` | WorldSemi WS2812B | Ring and digit pixels: timing, GRB order |
| `rgbdigit.pdf` | RGBDigit 1" 7-segment | **Read the notes below — it changes two build decisions** |
| `ds3231.pdf` | Maxim DS3231 RTC | Register map, 1 Hz SQW, alarms, aging offset |
| `mics-5524.pdf` | SGX MiCS-5524 | Gas sensor: heater, `EN` polarity, output range |
| `2n2222a.pdf` | 2N2222A NPN | Buzzer driver |
| `1n5817-schottky.pdf` | 1N5817 | USB-C VBUS → VSYS, forward drop |
| `sn74hct125.pdf` | TI SN74HCT125N | 3.3→5 V level shifter, unused so far — see below |
| `buzzer-UDB-05B.pdf` | UDB-05B magnetic buzzer | 2300 Hz resonance, 30 mA |
| `arcade-button-AB30L5-B.pdf` | BerryBase arcade button | Ø 28 mm panel hole, lamp rating |

## Provenance notes

**`pca9633-backlight.pdf` is Rev. 05 (2008), not the current Rev. 6.0.** NXP answers
403/404 to anything that is not a browser, so this is a distributor mirror and it
carries a 100y.com.tw watermark on every page. The register map is unchanged;
check [NXP's page](https://www.nxp.com/products/power-drivers/lighting-driver-and-controller-ics/led-drivers/4-bit-fm-plus-ic-bus-led-driver:PCA9633)
if a number ever looks wrong.

**`ds3231.pdf` is Adafruit's mirror** of the Maxim datasheet, hosted under
product 3013. Analog Devices, who now own Maxim, serve a broken HTTP/2 stream to
anything that is not a browser.

**`2n2222a.pdf` and `1n5817-schottky.pdf` are mirrors too** — onsemi blocks
automated requests. They come from MIT's course reference collection and Vishay
respectively.

**No datasheet exists** for the KY-040 encoder or the DFR0464 LCD as a module.
The KY-040 is a commodity part; the LCD's two controllers are documented
separately (PCA9633 here, AiP31068 only through
[DFRobot's wiki](https://wiki.dfrobot.com/dfr0464/)).

## What the RGBDigit datasheet changes

Read after the digits were already working, and it contradicts two decisions:

**There is already a 330 Ω resistor in the Di line.** The datasheet is explicit:
*"There is a 330 ohm resistor included in the Di (pen2) line an external resistor
is not necessary."* Our R2 adds 470 Ω on top, for **800 Ω** in series with the data
line. That is a lot, and it is a plausible contributor to the digit data fault that
showed up after soldering — the one that metered 470 Ω correctly and still would
not pass data.

**The logic threshold is above what the Pico can drive.** VIH is specified as
0.7 × VDD, so at VDD = 5 V the digit wants **3.5 V** to read a high — and the RP2040
drives 3.3 V. On paper the margin is negative. One digit on a short breadboard lead
works anyway; four digits on soldered wiring inside a box is exactly where that
stops being true. The SN74HCT125N in the parts box is the fix, and its datasheet is
in this folder.

Also worth knowing:

- **Housing 34.0 × 24.0 mm**, digit height 1" = 25.40 mm, ±0.25 mm. Pin pitch 2.54 mm.
- **Pinout is 3-in / 3-out:** 1 = 5 V, 2 = Data in, 3 = GND, 4 = 5 V, 5 = Data out, 6 = GND.
- **VDD absolute maximum is 3.5–5.3 V** — a narrower window than most 5 V parts.
- **The vendor recommends brightness under 80**, and rates cascading to 10 digits
  only below that. We run 120. Four digits is well inside the cascade limit, but
  the number is theirs, not arbitrary.
- **Power:** one digit, all 8 pixels white at brightness 250, draws ~290 mA. At 120
  it is roughly 130 mA. Four digits at full white would be ~520 mA — which is why
  the frame is current-clamped in software.
- **GRB byte order confirmed**, matching `drivers/neopixels.cpp`.
