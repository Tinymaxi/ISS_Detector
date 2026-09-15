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

## Datasheets

[`datasheets/`](datasheets/) holds the manufacturer PDF for every part, committed rather
than linked, with an index in [`datasheets/README.md`](datasheets/README.md).
`./datasheets/fetch.sh` re-downloads the ones that have a public URL.

## Wiring sheet

[`docs/wiring.pdf`](docs/wiring.pdf) is the printable build reference — pin map, net list, module
tables, schematics for the gas divider, buzzer driver and power inlet, plus the solder order.
[`docs/wiring.html`](docs/wiring.html) is its source; open it in any browser and print, or run
`./docs/build-wiring.sh` to re-render the PDF after an edit.

## Fabricated PCB

[`docs/pcb-assembly.md`](docs/pcb-assembly.md) is the current plan: a fabricated board with the
three sensors as **bare SMD parts** — MiCS-5524, SCD41-D-R2 and DS3231SN# soldered straight down,
with the support circuitry the breakouts used to provide drawn explicitly. The schematic in Fusion
matches it. The build described above is still the build until this one is finished.

A detour in between looked at the opposite approach — a bare PCB, no assembly service,
hand-soldered, with the three sensors staying as breakout boards on headers — and is recorded in
[`docs/pcb-tht-breakouts.md`](docs/pcb-tht-breakouts.md). It is superseded, and kept for its
account of what those breakouts were quietly providing.
[`docs/cost-comparison.md`](docs/cost-comparison.md) has the money behind both calls; the
assembly vendor is still open.

## Hardware

Raspberry Pi Pico WH (RP2040), C++17 on the Pico SDK. The "H" means headers are pre-soldered,
including the three debug pins — so a spare plain Pico can act as an SWD probe when needed.

| Function | GPIO | Pin |
|---|---|---|
| NeoPixel ring data | GP2 | 4 |
| I2C SDA / SCL (LCD + SCD40 + DS3231) | GP4 / GP5 | 6 / 7 |
| NeoPixel digits data | GP7 | 10 |
| Buzzer (via 2N2222A) | GP9 | 12 |
| DS3231 RTC `SQW` — 1 Hz tick | GP11 | 15 |
| Rotary encoder SW / CLK / DT | GP13 / GP14 / GP15 | 17 / 19 / 20 |
| Arcade button / lamp | GP19 / GP20 | 25 / 26 |
| PIR `REL` / `OEN` — motion out / enable | GP21 / GP22 | 27 / 29 |
| MiCS-5524 `A0` / `EN` | GP26 / GP27 | 31 / 32 |

The arcade lamp **is switched from the 5 V rail by `Q3`**, not driven from a GPIO. It is an
LED 5 V DC part with its own series resistor sized for 5 V, and the LED is blue — forward voltage
around 3.0–3.2 V. From a 3.3 V pin there is almost nothing left across that resistor and the lamp
is dim. GP20 drives `Q3`'s base through `R15`, in the same arrangement as the buzzer.

The **buzzer** works the same way. It is a magnetic transducer, not a piezo: 5 V rated, 30 mA,
resonant at 2300 ± 400 Hz. 30 mA is over the RP2040's 12 mA per-pin rating, so `Q1` switches it —
buzzer between the 5 V rail and the collector,
1 kΩ from GP9 to the base, emitter to ground. No code change; PWM drives a base perfectly well,
and it runs the buzzer at 5 V, where the datasheet's 83 dBA is measured.

Melodies live in A6–F7 (1760–2794 Hz), straddling resonance. Off resonance a magnetic buzzer is
quiet *and* current-hungry — its impedance is mostly inductive reactance, which falls with
frequency, so low notes draw more current to make less noise.

The **DS3231 RTC** (Adafruit 3013, address 0x68) is what the box actually keeps time by. NTP
cannot answer until WiFi associates — seconds of blank display on every power-up, and nothing at
all if the router is down — which is the wrong clock for a toy specified as instant-on and
yank-proof. The DS3231 answers immediately and holds the time on a CR1220 with the power off.
NTP still runs, but to discipline the RTC rather than to be the clock. Being a TCXO it holds
±2 ppm, roughly two minutes a year, so it stays right even if WiFi never works again.

Its `SQW` pin gives a 1 Hz square wave on GP11 for a hardware-exact seconds tick. It is open-drain,
so it needs a pull-up — the RP2040's internal one, no resistor. Strictly optional, wired anyway
because adding a wire to a closed box is much harder than fitting one now.

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

Peripherals: SCD40 CO2+temp+humidity (0x62) and/or SHT3x temp+humidity (0x44), DS3231 precision RTC (0x68),
DFRobot Gravity LCD1602 RGB (DFR0464), 24-pixel NeoPixel ring,
32-pixel NeoPixel seven-segment (4 digits x 8 pixels, own data line), MiCS-5524 gas sensor, rotary encoder, arcade button, buzzer.

**On the breadboard**, power comes through the Pico's own USB connector — a USB-C Raspberry Pi
supply with a USB-C-to-micro-USB cable. `VBUS` (pin 40) then distributes 5 V to the pixels, the gas
sensor and the buzzer.

**In the finished box** an Adafruit 4090 USB-C breakout gives the case a proper panel port, and it
feeds **`VSYS` (pin 39)** — not pin 40. The 5 V rail hangs off that same net:

```
USB-C breakout VBUS ──┬──► VSYS (pin 39)
                      └──► 5 V rail (pixels, gas sensor, buzzer)
```

**`VSYS` is the entry point precisely because it is not `VBUS`.** Pin 40 is the same copper as the
Pico's own micro-USB 5 V, so feeding it from the panel would wire two supplies straight together
whenever a USB cable is also plugged in. `VSYS` accepts 1.8–5.5 V and the Pico's own `VBUS` reaches
it through an internal Schottky, so whichever source is higher wins and neither back-feeds the
other. **Pin 40 stays unconnected.**

**The 1N5817 that used to sit in this path is gone.** It was an external copy of the Schottky the
Pico already has between `VBUS` and `VSYS`; feeding `VSYS` directly gets the same isolation from
that internal diode for free. Reflashing a boxed unit happens over SWD with a Debug Probe, which
supplies no power of its own.

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
