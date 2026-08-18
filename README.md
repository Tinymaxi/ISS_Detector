# Die ISS-Box

A birthday present for a five-year-old: one box, four modes, driven by a big arcade button
and a knob.

| Mode | NeoPixel ring (24 px) | NeoPixel 7-segment | LCD |
|---|---|---|---|
| **UHR** | 24-hour dial | `HH:MM`, NTP-synced | date + weekday |
| **ISS** | compass pointing at the space station | distance in km | ground position |
| **WETTER** | thermometer arc, blue → red | temperature | temp + humidity |
| **PUPS** | countdown, then a 0–24 bar graph | score 0–99 | verdict |

The ring is the centrepiece: 24 pixels divide 360° into exactly 15° each, so with a north arrow
marked on the case, the lit pixel genuinely points at the ISS. Colour carries elevation — deep blue
when it's on the far side of the planet, blazing white when it's overhead.

Whatever mode the box is in, it interrupts itself when the ISS actually rises above the horizon:
buzzer, ring flash, and an automatic switch to ISS mode.

## Hardware

Raspberry Pi Pico WH (RP2040), C++17 on the Pico SDK. The "H" means headers are pre-soldered,
including the three debug pins — so a spare plain Pico can act as an SWD probe when needed.

| Function | GPIO | Pin |
|---|---|---|
| NeoPixel ring data | GP2 | 4 |
| I2C SDA / SCL (SCD40 + LCD) | GP4 / GP5 | 6 / 7 |
| NeoPixel digits data | GP7 | 10 |
| Buzzer (via 2N2222A) | GP9 | 12 |
| Rotary encoder SW / CLK / DT | GP13 / GP14 / GP15 | 17 / 19 / 20 |
| Arcade button / lamp | GP16 / GP17 | 21 / 22 |
| MiCS-5524 `A0` / `EN` | GP26 / GP27 | 31 / 32 |

The arcade lamp is a low-current LED with its own series resistor, driven straight from GP17 on
PWM at 12 mA drive strength — no transistor needed.

The **buzzer** does need one. The UDB-05B is a magnetic transducer, not a piezo: 5 V rated, 30 mA,
resonant at 2300 ± 400 Hz. 30 mA is over the RP2040's 12 mA per-pin rating, so the 2N2222A that the
lamp turned out not to need sits here instead — buzzer between the 5 V rail and the collector,
1 kΩ from GP9 to the base, emitter to ground. No code change; PWM drives a base perfectly well,
and it runs the buzzer at 5 V, where the datasheet's 83 dBA is measured.

Melodies live in A6–F7 (1760–2794 Hz), straddling resonance. Off resonance a magnetic buzzer is
quiet *and* current-hungry — its impedance is mostly inductive reactance, which falls with
frequency, so low notes draw more current to make less noise.

The gas sensor runs at 5 V and its `A0` reaches the 3.3 V ADC through a divider: 10 k from `A0`,
15.64 k (10 k + 4.7 k + 470 + 470 in series) to ground. Sized for the Raspberry Pi adapter's
5.1 V rather than a round 5.0 V — 3.111 V at nominal, 3.270 V at the +5% worst case, 94% of the
ADC range. A 100 nF cap from GP26 to AGND settles the sampling.

The 470 Ω resistors on each NeoPixel data line belong **close to the pixels**, not close to the
Pico: their job is limiting current into the first pixel's input protection diodes, so they have to
sit at the input they protect.

Pins are spread deliberately: the final build is hand-soldered onto Adafruit Perma-Proto boards
(half-size plus quarter-size), so unrelated signals get a free pin between them. Pins from the same
module stay adjacent, since they arrive as one bundle.

Peripherals: SCD40 CO2+temp+humidity (0x62) and/or SHT3x temp+humidity (0x44), DFRobot Gravity LCD1602 RGB (DFR0464), 24-pixel NeoPixel ring,
32-pixel NeoPixel seven-segment (4 digits x 8 pixels, own data line), MiCS-5524 gas sensor, rotary encoder, arcade button, buzzer.

**On the breadboard**, power comes through the Pico's own USB connector — a USB-C Raspberry Pi
supply with a USB-C-to-micro-USB cable. `VBUS` (pin 40) then distributes 5 V to the pixels, the gas
sensor and the buzzer. One source means no back-feeding.

**In the finished box** an Adafruit 4090 USB-C breakout gives the case a proper panel port. That is
a second supply, so the 1N5817 comes back:

```
USB-C breakout VBUS ──┬───────────────► 5 V rail (pixels, gas sensor, buzzer)
                      └──[1N5817]────► VSYS (pin 39)
```

`VSYS` accepts 1.8–5.5 V and the Pico's own `VBUS` reaches it through an identical internal
Schottky, so whichever source is higher wins and neither back-feeds the other — the box can stay
powered from the panel while being reflashed over micro-USB. **Pin 40 stays unconnected in that
arrangement**: tying it to the 5 V rail would join the two supplies directly and bypass the diode.

Pixel brightness is clamped in software against a current budget — 56 pixels at full white would
pull ~3.4 A.

## Building

```bash
./go.sh                # build
./go.sh flash          # build and load onto the board
./go.sh flash monitor  # ...and open the serial console
```

First flash needs `BOOTSEL` held while plugging in USB. After that the firmware exposes a reset
interface and `picotool` reboots the board itself.

## Configuration

```bash
cp src/config.h.example src/config.h
```

`src/config.h` holds WiFi credentials and the home coordinates the ISS compass points relative to.
It is gitignored and must stay that way.
