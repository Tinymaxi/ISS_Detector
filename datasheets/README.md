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
| `scd4x-design-in-guide.pdf` | Sensirion SCD4x Design-In Guide v1.1 | **Housing and placement rules the datasheet does not cover** — dead volume, air turbulence, vibration, sunlight, 30 mV supply-ripple limit, and the four-part design-in sanity check |
| `sht3x.pdf` | Sensirion SHT3x | The sensor the SCD40 replaced; CRC-8 poly shared with SCD4x |
| `pca9633-backlight.pdf` | NXP PCA9633 | The LCD's RGB backlight — MODE1 wake-up, PWM registers |
| `ws2812b.pdf` | WorldSemi WS2812B | Ring and digit pixels: timing, GRB order |
| `rgbdigit.pdf` | RGBDigit 1" 7-segment | **Read the notes below — it changes two build decisions** |
| `ds3231.pdf` | Maxim DS3231 RTC | Register map, 1 Hz SQW, alarms, aging offset |
| `mics-5524.pdf` | SGX MiCS-5524 | Gas sensor: heater, `EN` polarity, output range |
| `2n2222a.pdf` | 2N2222A NPN | Buzzer driver |
| `1n5817-schottky.pdf` | 1N5817 | USB-C VBUS → VSYS, forward drop |
| `sn74hct125.pdf` | TI SN74HCT125N | 3.3→5 V level shifter, unused so far — see below |
| `buzzer-UDB-05B.pdf` | UDB-05B magnetic buzzer | 2300 Hz resonance, 30 mA — **the Perma-Proto build's part** |
| `buzzer-CST-951AP.pdf` | Same Sky CST-951AP magnetic transducer | **The fabricated board's part.** Ø9.0 × 4.5 mm, 2730 Hz, 80 mA, 85–92 dBA at 10 cm. Same OEM part as Soberton's GT-0905A, but Same Sky publish a footprint and a 3D model. Ø9.0 matches the UDB-05B's 9.2, so the panel hole is unchanged. |
| `arcade-button-AB30L5-B.pdf` | BerryBase arcade button | Ø 28 mm panel hole, lamp rating |
| `pir-BS612.pdf` | Senba BS612 PIR | **The motion sensor.** 2.2–3.7 V, 9.5 µA, 6-pin TO-5. `SENS`/`ONTIME` divider tables, the mandatory 100 nF, the layout and no-wash rules |
| `lens-SB-F-02.pdf` | Senba SB-F-02 Fresnel lens | The BS612's optics — **focal length 11.5 mm**, 120°, 10 m, HDPE. Without it the PIR reaches about a metre |
| `C429947.pdf` | JILN `22850120ANG1SYA01` 1×20 socket | **The Pico's socket.** 8.5 mm insulator, 3 A, 0.64 mm square tail 3.2 mm long, recommended hole **Ø1.02**. Note the **180 °C / 30–60 s max processing temperature** — these cannot be reflowed |
| `C70381.pdf` | Q&J `BS-12-B3AA004` CR1220 holder | **The coin-cell holder's land pattern.** Its *PCB Layout Diagram (Top View)* is the authority for `BT1` — and it proved the EasyEDA-derived footprint had the polarity reversed. See the note below |

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

> ⚠️ **`rgbdigit.pdf` is the 1" module's datasheet and ours is the 0.8".** Confirmed by
> callipers: **19.9 × 27.6 mm** against the sheet's 34.0 × 24.0. **Every mechanical figure
> in it is wrong for our part** — housing size, and the 25.40 mm digit height.
>
> **The electrical figures do carry over.** RGBDigit.com's own product page for the 0.8"
> says the two are *"technically the same as the 1" version"* — same NeoPixel library,
> same 3-in/3-out wiring, same 5 V maximum, same 10-digit cascade limit. So `VIH = 0.7 ×
> VDD`, the built-in 330 Ω, the GRB byte order and the pin functions below all stand, and
> `U5` (the SN74HCT125) is justified on the same grounds as before.
>
> One number the product page adds that the datasheet does not: **the vendor's own typical
> brightness is 30 out of 255** — lower than the "under 80" in the datasheet, and well
> under the 120 we run. Not a fault; see the note below.


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

- **Housing 19.9 × 27.6 mm, pin rows 15.24 mm apart, pin pitch 2.54 mm — all measured.**
  ⚠️ The datasheet says 34.0 × 24.0 mm with a 1" (25.40 mm) digit; **it describes a module
  we do not own.** The Fusion footprint `RGBDIGIT_08"_6PIN` is **verified against callipers
  on all three dimensions** — 20.32 × 27.94 nominal against 19.9 × 27.6 measured (moulding
  tolerance), and 15.24 mm row spacing confirmed exactly. The footprint is correct; it was
  the documentation that was wrong.
- **Pinout is 3-in / 3-out:** 1 = 5 V, 2 = Data in, 3 = GND, 4 = 5 V, 5 = Data out, 6 = GND.
- **VDD absolute maximum is 3.5–5.3 V** — a narrower window than most 5 V parts.
- **The vendor recommends brightness under 80**, and rates cascading to 10 digits
  only below that. Their product page goes further and calls **30 out of 255** typical
  use. We run 120. Four digits is well inside the cascade limit, so this is about glare
  and power rather than correctness — and [`../docs/pcb-assembly.md`](../docs/pcb-assembly.md)
  now adds a VEML7700 ambient light sensor precisely so the number does not have to be one
  number. Daylight can stay at 120; a dark bedroom is where the vendor's 30 belongs.
- **Power:** one digit, all 8 pixels white at brightness 250, draws ~290 mA. At 120
  it is roughly 130 mA. Four digits at full white would be ~520 mA — which is why
  the frame is current-clamped in software.
- **GRB byte order confirmed**, matching `drivers/neopixels.cpp`.

## What the CR1220 drawing changed

`C70381.pdf` is the manufacturer's specification for the coin-cell holder JLCPCB stock.
It was fetched to settle one question — which tab is positive — and answered a second one
nobody had asked.

**Polarity was reversed in our footprint.** The drawing's *PCB Layout Diagram (Top View)*
marks the land pattern in red: **`−` on the left pad, `+` on the right.** The footprint in
[`../eda/libraries/CR1220-2.lbr`](../eda/libraries/CR1220-2.lbr) had been transcribed from
EasyEDA's symbol, which put `+` on the left. A reversed cell puts the DS3231's `VBAT`
outside its −0.3 to +6.0 V rating.

**The two locating holes are the same size.** Both are **Ø1.20 at ±3.75**, not Ø1.2 and
Ø1.0. So they do *not* polarise the holder, and the earlier note claiming they did was
wrong. The orientation key on the board is the **tab widths**: 4.04 mm on the negative
side, 4.35 mm on the positive.

**The pads are asymmetric** — centres at −8.15 and +7.815, against EasyEDA's ±8.60. The
two sources disagree by under a millimetre, so the library now uses pads that span the
union of both rather than betting on one reading of a scanned drawing.

All three are fixed in the library. `BT1` has to be deleted and re-placed in the schematic
for the change to reach the board, which is cheap — it carries two nets.
