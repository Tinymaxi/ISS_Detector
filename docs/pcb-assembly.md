# The fabricated PCB

**Current plan, and the schematic matches it.** The ISS-Box is drawn as a fabricated board in
Fusion Electronics with the sensor breakouts replaced by bare SMD parts. A detour through a
bare-board/breakout/hand-soldered version is recorded in
[`pcb-tht-breakouts.md`](pcb-tht-breakouts.md) and was abandoned; this is the design being
built. Prices and stock were checked on **25 August 2026** and will drift — confirm live
before ordering.

> **The assembly route is the one open commercial question.** The header of this document used
> to say AISLER; the *MiCS-5524 and reflow* section below lands on JLCPCB instead, and
> [`cost-comparison.md`](cost-comparison.md) is what pushed it. Machine placement is not really
> optional — `U2` and `U3` both have pads under the body — so what is open is *who*, not
> *whether*. Confirm before ordering.

> **Changes since this document was first written**, all reflected in the schematic and not
> always in the prose below:
>
> | Change | Where |
> |---|---|
> | `D1` (Schottky) **dropped**; `J1` feeds `VSYS` (pin 39) directly, `VBUS` unconnected | [`../README.md`](../README.md) |
> | Arcade switch/lamp moved to **GP19/GP20** (pins 25/26) | pin table below |
> | PIR `REL`/`OEN` on **GP21/GP22** (pins 27/29), PIR ground on pin 28 | pin table below |
> | Module is a **Pico 2 WH** (headers, JST-SH debug connector); the footprint's three debug pads must be deleted | *The Pico* / *First power-up* below |
> | `J7` carries **TX, RX, RUN, GND, 3V3** — SWD goes to the Pico's own top-side connector instead | *First power-up* below |
> | Board settled at **83.82 × 90 mm** with the antenna notch cut into the outline | *Board size* below |
> | Digit rotations fixed to exactly `R0`; pitch 20.32 mm | *ones already on the list* below |
> | Front/back split: digits, PIR, light sensor and gas front end on the front; Pico, connectors, SCD41, RTC, level shifter on the back | — |

The hand-soldered Perma-Proto build still exists and still works;
[`../README.md`](../README.md) and [`wiring.html`](wiring.html) describe it. Treat that
as the running prototype and this as the board that replaces it.

## The shape of the board

Three of the sensors were breakout boards carrying a bare SMD chip and a handful of
support components. All three are now placed directly, and the support circuitry the
breakouts were quietly providing has been taken over explicitly.

Everything that mounts to the **case** rather than the board stays off-board and
connects by cable. That is not a compromise — a panel-mount arcade button and a ring
that has to point at the sky were never going to be machine-placed.

### On the board

| Ref | Part | Package |
|---|---|---|
| `U1` | Raspberry Pi Pico 2 W | 40 THT pads + 3 debug — see below |
| `U2` | `MICS-5524` gas sensor | 10-pad, 5 × 7 × 1.55 mm |
| `U3` | `SCD41-D-R2` | 16-pad, 10.1 × 10.1 × **7.8 mm** (8.5 over the cap) |
| `U4` | `DS3231SN#` RTC | SOIC-16, 300 mil |
| `U5` | `SN74HCT125` level shifter | SO-14 |
| `Q1` | `MMBT2222A` buzzer driver | SOT-23-3 |
| `Q2` | `AO3400A` heater switch | SOT-23-3 |
| `Q3` | `MMBT2222A` arcade lamp driver | SOT-23-3 |
| `U6` | Senba `BS612` PIR | THT, Ø9.2 mm can |
| `U7` | Vishay `VEML7700-TR` ambient light | 6.8 × 2.4 mm |
| `BT1` | CR1220 holder, Keystone `2894` | SMT, locking |
| `BZ1` | `CST-951AP` buzzer | THT, Ø9.0 × 4.5 mm |
| `C1` | 1000 µF 16 V, `E5-10,5` | THT — deliberately |
| `DIGIT1`–`4` | RGBDigit modules | THT, **19.9 × 27.6 mm** each, measured — see below |

**Digit power is starred, not daisy-chained.** Each digit's `5V_IN`/`GND_IN` *and*
`V5_OUT`/`GND_OUT` go straight to `+5V` and `GND`; only the data line is chained
`DIGIT1 → 2 → 3 → 4`. The module supports pass-through power and the vendor rates ten in
cascade, but chaining would have pushed the whole string's current — around 520 mA at
brightness 120 — through DIGIT1's internal copper to feed the three behind it. On a
single board there is no reason to accept that: wiring both pairs costs nothing, halves
the path resistance into each module, and removes the voltage sag at DIGIT4.
| passives | 0805 throughout | |

Both SOT-23 libraries in use map **control → pad 1, common → pad 2, output → pad 3**
(`B/E/C` and `G/S/D`), so the two part types are consistent with each other. Check
against the footprints anyway before routing.

### The arcade lamp needs 5 V, not a GPIO

The button is an **LED 5 V DC** part with its own series resistor sized for 5 V, and the
LED is blue — forward voltage around 3.0–3.2 V. Driven from a 3.3 V GPIO there is almost
no voltage left across that resistor and the lamp is dim. `Q3` and `R15` switch it from
the 5 V rail instead, in the same arrangement as the buzzer.

[`../README.md`](../README.md) still says the transistor "turned out not to be needed"
for the lamp. That line is out of date and should be corrected once 5 V drive is
confirmed on the bench.

### Off the board, on JST

Five cable assemblies, all **JST XH**:

| Ref | Goes to | Pins | Nets |
|---|---|---|---|
| `J1` | USB-C breakout 4090 | 2 | VBUS, GND |
| `J2` | LCD1602 RGB DFR0464 | 4 | 3V3, GND, SDA, SCL |
| `J4` | KY-040 encoder | 5 | 3V3, GND, CLK, DT, SW |
| `J5` | Arcade button | 4 | SW, GND, LAMP+, LAMP− |
| `J6` | NeoPixel ring 24 | 5 | 5V ×2, DIN, GND ×2 |

**The RGBDigits are on the board**, not on a cable — `J6` is the ring, not the digits.

> **`J3` is gone; the ring moved to `J6`.** It was a 3-pin `B3B-XH-A` carrying one 5 V,
> one GND and the data line. The ring brings out **two** 5 V and **two** GND pads, so it
> is now a 5-pin `B5B-XH-A` (`C157991`) with both pairs wired. Three consequences that do
> not live in the schematic: the BOM loses a `B3B-XH-A` (`C51940188`) and gains a second
> `B5B-XH-A`; the board needs `J3`'s footprint removed and `J6`'s larger one placed; and
> the ring's cable tail now needs a **5-way** XH housing and five crimps, not three.

**JST XH is 2.50 mm pitch, not 2.54.** Sellers advertise it as 2.54 because the
difference does not show in a listing. Over a 5-pin `J4` it is 0.16 mm cumulative —
survivable in through-hole, but use the real XH footprint rather than a 0.1" header.
To check a connector in hand, measure across the outer pins of a 5-way: 10.0 mm is XH,
10.16 mm is genuine 2.54.

**`R1` sits at the board end, and that is fine.** The Überguide's "mount it at the
pixels" advice targets runs of a metre or more. The ring is 6 cm away on `J6`: 470 Ω
into roughly 15 pF of cable and pixel input gives a ~7 ns edge against WS2812 bit times
of 400–800 ns. The same argument covers `C1` — six centimetres of wire is about 60 nH,
which does nothing on the microsecond timescale a pixel surge occupies.

### Board size — the open question

**As drawn today: 81.26 × 173.05 mm.** Digits at **20.32 mm pitch**, `R0`, `DIGIT1` at
x 10.16 — outlines nominally touching, which against the measured 19.9 mm housing leaves
about 0.4 mm of real air. The swap in *SCD4x design-in* above pulls the bottom edge up to
roughly −148, taking the board to about **81 × 148 mm**, and there is still spare space
below the connectors after that.

> **Superseded — salvaged from `PCB-for-quote` before it was deleted.** That layout existed
> only to get a number out of JLCPCB. Its geometry is recorded here because the quote was
> priced off it:
>
> | | |
> |---|---|
> | Board outline | **139.68 × 87.30 mm** |
> | Digit column | x = 125.73, rotated **R270** |
> | Digit positions | y = 24.13 / 45.72 / 67.31 / 88.90 |
> | Digit pitch | **21.59 mm** |
>
> None of it survived. The row runs horizontally at `R0` rather than as an `R270` column,
> and the pitch went to 20.32 rather than 21.59 — tighter, and deliberate, since the digits
> should read as one display.
>
> The $345.17 quote, the 1.14 kg shipping weight and the Standard PCBA size bracket all
> came off that outline. A materially smaller board changes all three.


~~Four RGBDigits at 34 mm each is **136 mm of board width, minimum**.~~ **Wrong — the
34 mm came from the wrong datasheet.** The digits measure **19.9 × 27.6 mm**, so four in a
row is about **80 mm**, not 136. Against the 38 × 62 mm the AISLER sandbox quote assumed
that is still a large increase, but it no longer forces the board past the 100 mm price
break on that axis alone.

It also forces the PCB to *be* the front panel, which fights three other things: the
SCD41 and MiCS want vents, `C1` stands ~20 mm tall, and the LCD needs its own window.

Two ways out, neither yet chosen:

1. **One wide board**, ~140 × 60 mm. Simple, one fabrication, but large and awkward.
2. **A separate digit board** carrying the four modules, with a 3-wire link back. Keeps
   the main board small and makes the front panel one rigid assembly, at the cost of a
   second fabrication and one more connector.

### Cost, roughly

For **3 assembled boards at 100 × 100 mm**, as an order-of-magnitude check:

| | Estimate |
|---|---|
| Parts, AISLER-sourced SMD | ~€45/board → €135 |
| PCB, 2-layer, ×3 | €40–70 |
| Assembly: ~18 unique parts, ~29 placements | €150–300 |
| **Total** | **≈ €330–500** |

Parts are the firm number and dominate: SCD41 ~€20, DS3231 ~€12, MiCS ~€10, everything
else under €4 combined. **The assembly figure is soft** — extrapolated from the €193.40
sandbox quote, which was for *one* distinct component, so it mostly measures fixed
setup. Going to 18 unique parts is what drives the cost, not the area.

Twelve parts stay off the assembly quote entirely: `U1`, `C1`, `BZ1`, `J1`–`J5`, `J7`
and the four digits. Mark them **do not place / customer supplied** at quote time, or
AISLER's tool will try to source parts its distributors do not carry.

## The gas front end — complete

The most involved block on the board, and the only one where going bare is not a
drop-in. Pin names are from the connection table on p. 4 of
[`../datasheets/mics-5524.pdf`](../datasheets/mics-5524.pdf).

```
       +5V ──┬── F/RH2 ─[ heater ~74Ω ]─ C/RH1 ──[ R9 82Ω ]──┬── Q2 drain
             │                                                │
             └── G/RS2 ─[   sense Rs   ]─ D/RS1 ──┬──[R10 10k]┴── GND
                                                   │
                                      R3 10k ──────┴──► GAS_ADC ──► GP26
                                                              │
                                            R4 15.4k ─┬───────┤
                                              C2 100n ─┘       │
                                                             AGND
```

`A`, `B`, `E`, `H`, `J` and `K` are genuinely unconnected on this part — there are no
pads to ground, unlike the DS3231. They carry `NC` pin direction in the symbol.

### R_LOAD is 10 kΩ, measured

The open question that blocked this block is answered. Metering A0 to GND on the
Adafruit 3199 breakout, disconnected from everything, reads **9.94 kΩ** — a 1% 10 k
resistor neat. (Had it still been wired to the divider it would have read ~7.2 kΩ, so
the reading also confirms the measurement was clean.)

**The existing divider maths therefore stands unchanged.** Cross-check: the divider
presents 10 k + 15.4 k = 25.4 kΩ to the sense node, loading R_LOAD down to an effective
7.19 kΩ. With Rs ≈ 100 kΩ in air that puts GP26 at ~0.21 V, sitting inside the
0.1–0.4 V baseline [`wiring.html`](wiring.html) records as actually observed. Two
independent numbers agreeing.

### R4–R7 collapse to one resistor

The four-in-series chain making 15.64 kΩ was a what-is-in-the-drawer solution. On a
machine-placed board it becomes a single **15.4 kΩ 1% 0805** — the nearest E96 value.
Full scale moves from 3.111 V to 3.092 V, a 0.6% shift, invisible against the sensor's
own spread. 15.8 kΩ is arithmetically closer but leaves only 18 mV of headroom at the
5.36 V worst case; 15.4 k preserves the original margin.

### `EN` inverts — and this is a code change

**The bare MiCS-5524 has no enable pin.** `EN` was a feature of the breakout, which
switched the heater with its own transistor. Here that is `Q2`, a low-side AO3400A under
`R9`, gate straight to GP27.

That flips the polarity. The breakout was `EN` high = heater **off**; a low-side FET is
gate high = heater **on**. `src/drivers/gas_sensor.cpp` currently drives GP27 low to
enable and must be flipped when the board changes — **not before**, since the
Perma-Proto build still uses the breakout.

This is the *only* firmware consequence of the whole move. Same silicon everywhere else,
same I²C addresses, same registers.

`R11`, a 100 kΩ gate-to-source pulldown, holds the heater off between power-on and
firmware configuring GP27. AO3400A's ~30 mΩ Rds(on) is 0.04% of the 82 Ω and does not
move the heater voltage; a 2N7002 would, at a few ohms.

### Constraints from the datasheet

- Reflow **in a neutral atmosphere, without soldering flux vapours**.
- No exposure to solvents, silicone or cigarette smoke — it poisons the sensitive layer.
- Wants a **filtered package** against water and dust projections.
- Supply is a narrow **4.9–5.1 V**.

The first of these is an open question against AISLER — see below.

## What the other two breakouts were doing

### DS3231 — coin cell, decoupling, and eight pins that must be grounded

From the Pin Description on p. 9 of [`../datasheets/ds3231.pdf`](../datasheets/ds3231.pdf):

| Pin | Name | What it needs |
|---|---|---|
| 2 | `VCC` | **Decoupling, 0.1 µF to 1.0 µF** (`C5`). Asked for by name. |
| 3 | `INT/SQW` | Open-drain — needs a pull-up. GP11's internal one suffices. |
| 4 | `RST` | Internal 50 kΩ pull-up. **"No external pullup resistors should be connected."** |
| 5–12 | `N.C.` | **"Must be connected to ground."** Eight pins, not optional. |
| 14 | `VBAT` | `BT1`. No capacitor required as backup source. |
| 15 | `SDA` | Open-drain — needs an external pull-up. |
| 16 | `SCL` | Tolerates up to 5.5 V regardless of `VCC`. |

Two worth pulling out. **`RST` must not get an external pull-up** — the opposite of the
reflex. And **N.C. does not mean floating**: pins 5–12 are real copper on the layout.
Note this is the exact inverse of the MiCS's unconnected pins, on the same sheet.

### SCD4x — the keep-out is the whole story

No new support components: `C4` (100 nF) and `C7` (10 µF) cover decoupling. What
changes is layout. From §4.2 on p. 24 of [`../datasheets/scd4x.pdf`](../datasheets/scd4x.pdf):

- **21 pads**: 16 around the perimeter at 1.25 mm pitch, four small inner pads, and one
  4.8 × 4.8 mm centre pad. Landing-pad, paste and mask drawings are all given separately
  in Figure 4 — nothing has to be derived.
- The centre needs a **Ø 0.6 mm area kept free of solder and flux**. The datasheet says
  this "must be respected" in any case. In practice the paste layer is split into four
  quadrants with a gap through the middle. (The Ø 0.25 mm NPTH thermal relief hole in
  Figure 3 is a feature of the *sensor*, not of the land pattern — the board needs no
  hole.)
- **`VDDH` must be tied to `VDD`**, at the same voltage, and joined *close to the
  sensor*. It supplies the IR source.
- **`DNC` pads must be soldered, to floating copper.** Not grounded — the third distinct
  treatment of unused pins on this board, after the MiCS's genuinely-open pins and the
  DS3231's must-be-grounded ones. During layout they must be kept out of any ground pour.
- Pin 1 marked by both a circular mark and a notched corner.
- The **white protective membrane must not be removed or tampered with**.
- Sensirion recommend a **separate LDO**, and that supply ripple stay under **30 mV**.
  This board shares the Pico's 3V3 rail with the LCD, RTC and encoder instead. See
  *Supply ripple* below — that 30 mV turns out to mean something narrower than it looks,
  and it gets fixed for free.

That centre keep-out is what a generic footprint generator does not know about, which
makes it the thing to check on any downloaded footprint.

If you go bare here, the Adafruit 5187's LDO and level shifters go with it — neither is
needed, since the die takes 2.4–5.5 V and 3.3 V suits it.

### SCD4x design-in — the housing is most of it

[`../datasheets/scd4x-design-in-guide.pdf`](../datasheets/scd4x-design-in-guide.pdf) is a
separate six-page application note, and it carries the rules the datasheet does not. Its own
warning sets the stakes: a poor design-in causes *"significant deviations in CO2 and
temperature readings, increased noise levels as well as significantly longer response
times."* These are accuracy requirements, not preferences.

**Hot air rises, so the sensor goes at the bottom.** §1.2 asks for the sensor in the
device's *coldest* part, and says that is normally the lowest part of the device, at maximum
distance from self-heating components. The digits and the Pico are above `U3` already, which
is right. The gas sensor's heater was not — it sat *below*, so its plume rose past the CO₂
sensor. **`U2` and `U3` swap ends:**

| Block | Was | Now |
|---|---|---|
| MiCS-5524 + driver — front | y −123…−150 | **y −85…−110** |
| SCD41 + `C4`/`C7` — back | y −82.5…−97.5 | **y −128…−143** |
| Board bottom edge | −173 | **~−148** |

That puts every heat source above `U3`, takes it 54 mm from the buzzer instead of 22, still
leaves 58 mm between the MiCS and the PIR, and trims 25 mm off the board on the way past.

**The PCB slit is real, but not needed here.** Figure 3(a) shows *"superb decoupling from
external heat sources enabled by a slit in customer PCB"* — but what it contrasts against is
self-heating components in *immediate proximity*. At tens of millimetres a slit buys nothing
and costs a gap through the ground pour.

**Vibration** (§1.4): *"no stiff material (such as a PCB or a metal bar) connects a
vibration source to the sensor."* `BZ1` is a magnetic transducer on the same board. Judged
acceptable — it sounds rarely and briefly — and the swap above takes it to 54 mm anyway.

**What the enclosure has to do**, which is the larger half of this document:

- **A large opening right at the sensor** (§1.1). The bigger and closer, the better the air
  exchange.
- **Small dead volume.** Seal the sensor's pocket off from the air trapped in the rest of
  the case. Large dead volumes increase response time significantly, and it is the mistake
  the guide spends most of its figures illustrating.
- **Out of any draught** (§1.3). Air flow across the sensor generates pressure fluctuations
  that raise noise and cut accuracy. A separate compartment solves this and the dead volume
  together.
- **No direct sunlight** (§1.5) — it also accelerates ageing. Inside a box this is mostly
  true already, but the digit window is an opening; worth a thought about where the sun
  falls.

#### Supply ripple — narrower than it reads, and free to fix

The limit is **30 mV peak to peak**, and footnote 10 of the datasheet is the part that
matters: *"Determined on the supply voltage without the load of the sensor."* It specifies
the **regulator's own ripple**, not the droop the sensor's own 205 mA pulses cause. That
rules out the obvious remedy — more bulk at `C7` helps the sensor's self-inflicted droop,
which is not what is being measured.

What *is* being measured is the Pico's **RT6154 buck-boost**, and it has a mode pin. From
§3.4 of [`../datasheets/pico-w.pdf`](../datasheets/pico-w.pdf):

> WL_GPIO1 controls the RT6154 PS (power save) pin. When PS is low (**the default on Pico
> W**) the regulator is in pulse frequency modulation (PFM) mode … Setting PS high forces
> the regulator into pulse width modulation (PWM) mode. PWM mode … reduces the output
> ripple considerably at light loads … at the expense of much worse efficiency.

The board ships in the noisy mode. One line after `cyw43_arch_init()` changes that:

```c
cyw43_arch_gpio_put(1, true);   // WL_GPIO1 → SMPS PS high, forces PWM
```

The efficiency cost is irrelevant on a mains-fed board. The same section notes that *"under
heavy load the SMPS will be in PWM mode irrespective"*, so with the RP2040, the radio and
the LCD on that rail it is probably in PWM much of the time already — forcing it covers the
quiet moments.

**A dedicated LDO was considered and rejected.** It is Sensirion's own recommendation, but
5 V → 3.3 V at 205 mA peaks dissipates about 350 mW, and §1.2 of the same guide names
voltage regulators as a heat source to keep away from the sensor. It trades the ripple
problem for the heat problem. A series resistor is worse still: 1 Ω costs 205 mV of droop.

#### The acceptance test

§2 gives a sanity check, which is how you find out whether any of the above was got wrong.
Run the finished device under normal conditions for **15 minutes** to reach thermal
equilibrium, then:

| Measurement | Pass | If it fails |
|---|---|---|
| Temperature against a reference | within **0.5 °C** | set the offset over I²C |
| Difference between consecutive CO₂ readings | under **20 ppm** | the design-in needs work |
| Response to a >1000 ppm step, high-performance mode | under **60 s** | dead volume, or too small an opening |

The CO₂ step is easy to produce — breathe near the device a few times. Sensirion suggest an
SHTxx evaluation kit as the temperature reference, isolated from heat sources.

### I²C pull-ups are now mandatory

All three breakouts brought their own 10 kΩ. With them gone, the LCD is the only part
still carrying pull-ups — and it is on the far end of `J2`. A bus whose pull-ups vanish
when a connector is unplugged is not a bus.

**Fit `R12`/`R13`, 4.7 kΩ, on the board.** In parallel with the LCD's own that lands
near 3.2 kΩ, which [`wiring.html`](wiring.html) already establishes as fine — about
1 mA of sink against the 3 mA these parts tolerate, and a stiffer pull-up sharpens edges
on a bit-banged bus.

## The Pico

**Pico 2 W (RP2350), kept.** Same 40-pin pinout, same 51 × 21 mm outline, same
1.8–5.5 V VSYS as the Pico W, so nothing on the schematic depends on the choice. What it
costs is `PICO_BOARD=pico2_w`, a BOOTSEL volume that mounts as `RP2350` rather than
`RPI-RP2`, and three documentation files to keep in step —
[`references.md`](references.md) currently files Pico 2 W under "not ours".

Erratum `E9` affects GPIO used as inputs **with pull-downs**. Every input on this board
uses a pull-**up** — encoder CLK/DT through the KY-040's own resistors, arcade switch and
RTC `SQW` through internal ones — so it does not bite. Read the current errata anyway
before ordering.

### `AGND` is broken out — resolved

The stock symbol tied pad 33 into the common GND bundle, exactly as feared. It now has
its own `AGND` pin, and the net contains **exactly three things**: pin 33, `R4`, `C2`.

The join back to digital ground happens inside the Pico, where it is already done
properly. ERC will warn that two supply nets never meet; **approve that warning rather
than "fixing" it** — adding a tie would parallel the Pico's internal one and create the
loop the split exists to avoid.

### The footprint is through-hole

`PICO2W_SMT_THT` is, despite the name, **outline plus 43 through-hole pads** (40 header
+ 3 debug) with **no castellated SMD lands** — its own description says "No copper pads
– add your own."

**Delete the three debug pads.** The module is a WH, which has a JST-SH connector where a bare
Pico 2 W has holes, so `SWCLK`/`GND`/`SWDIO` at y 19.80 have nothing to mate with. Nothing replaces
them — the connector faces up, away from this board. The 40 header pads stay exactly as they are.

The library is **`Pico2W_SMT_footprint_v9`** (symbol `PICO-SYM-DEBUG`, deviceset `PICO_2_W`). Three
edits: drop those pads from the package, drop the `SWCLK`/`SWDIO` pins from the symbol, and trim
the device's `GND` connect, which currently reads `pad 3 8 13 18 23 28 38 GND` — the trailing named
`GND` pad goes with the other two. **Rewire `J7` in the schematic first**, or Fusion will be
dropping connections that are still live.

**Machine-placing the Pico is not an option, and that is settled rather than open.** SWD reaches
the module only through that debug connector — it is on **none** of the 40 header pins (see the
table in [`references.md`](references.md)). Reflowing the module by castellated edge pads would
therefore connect the header and lose SWD entirely, which on a board whose USB is sealed in means
no way to flash it at all.

So the Pico is hand-soldered, not machine-placed. That is not necessarily the wrong
answer: a pin-soldered or socketed Pico stays replaceable, which for a board a
five-year-old will own is arguably worth more than one fewer joint. But it puts the Pico
on the hand-fitted list alongside the JST connectors, `C1` and the buzzer.

## The rest of the BOM

0805 throughout — no cost over 0402 and comfortably reworkable by hand.

| Ref | Value | Note |
|---|---|---|
| `R1` | 470 Ω | Ring data, between `RING_5V` and `J6.3`. At the board end, not the ring end. |
| `R3` | 10 kΩ 1% | Divider upper leg |
| `R4` | 15.4 kΩ 1% | Divider lower leg — replaces the old `R4`–`R7` chain |
| `R8` | 1 kΩ | `Q1` base. 470 Ω–4.7 kΩ all work. |
| `R9` | 82 Ω 1% | Heater. Off-value shifts heater temperature. |
| `R10` | 10 kΩ 1% | R_LOAD. Sets the ADC scaling. |
| `R11` | 100 kΩ | `Q2` gate pulldown |
| `R12`, `R13` | 4.7 kΩ | I²C pull-ups |
| `R14` | 0 Ω | **Buzzer volume trim.** A link, fitted so the pads exist — swap for ~33 Ω to lose about 6 dB once the box is built and you have heard it. |
| `R15` | 1 kΩ | `Q3` base, arcade lamp |
| `C1` | 1000 µF 16 V | **THT deliberately** — see below |
| `C2` | 100 nF | ADC settling, to AGND |
| `C3`, `C8`, `C9`, `C10` | 100 nF | One per RGBDigit, across its `5V_IN`/`GND_IN`. Digits 2–4 sit on the chain's pass-through nets, **not** on `+5V`/`GND`. |
| `C4` | 100 nF | SCD41 VDD |
| `C5` | 100 nF | DS3231 VCC |
| `C6` | 100 nF | SN74HCT125 VCC |
| `C7` | 10 µF | SCD41 VDD — Sensirion ask for both |

`R2` is gone entirely rather than fitted-and-marked-DNF. The RGBDigit carries 330 Ω in
its `Di` line and an external resistor makes 800 Ω; the reasoning is in
[`../datasheets/README.md`](../datasheets/README.md).

**`C1` stays through-hole on purpose.** SMD electrolytics that size exist (Panasonic
`EEEFK0J102P`, case D, ~10 × 10 mm) but they are bulky, pricier, and a mixed THT/SMD
board is entirely normal. A big obvious capacitor that can be eyeballed and swapped is
worth more here than package consistency.

If the 100 µF upgrade for `C4` mentioned in the wiring sheet is ever taken, it jumps to
1206/1210 (e.g. Murata `GRM31CR60J107ME39`) — a layout change, not a drop-in.

Suggested MPNs, all confirmed as listings rather than by inspecting pad geometry:

| Value | MPN |
|---|---|
| 82 Ω 1% | `RC0805FR-0782RL` |
| 10 kΩ 1% | `RC0805FR-0710KL` |
| 15.4 kΩ 1% | `RC0805FR-0715K4L` |
| 100 kΩ | `RC0805FR-07100KL` |
| 4.7 kΩ | `RC0805FR-074K7L` |
| 100 nF 50 V X7R | `GRM21BR71H104KA01L` |

Yageo suffixes vary by reel (`-07` vs `-13`); paste each into Digikey and take whichever
is in stock. **Set the MPN attribute on every part as you place it** — AISLER's BOM tool
resolves live against seven distributors, and parts without an MPN become manual work at
quote time.

## Libraries

**Fusion Electronics is EAGLE underneath**, so `.lbr` files import directly via Library
Manager. "EAGLE format" and "Fusion 360 format" are the same file.

Libraries not covered by the built-ins live in
[`../eda/libraries/`](../eda/libraries/), committed for the same reason as the
datasheets: they are small, hand-picked, and the links rot.

Generic parts come from Fusion's own libraries. Note the package naming: the modern
managed libraries use **IPC-7351 metric** names, so 0805 is `RESC2012X65` /
`CAPC2012X110`, not `R0805` / `C0805`. The classic `rcl` library is also available as a
cloud library, where the old `R-EU_`/`C-EU` + `R0805`/`C0805` naming still applies.
Either is fine — pick one family and stay in it. **EU symbols throughout.**

For manufacturer-specific parts: **SnapMagic Search** has a native Fusion app that
places symbol, footprint and 3D model from inside the Electronics workspace;
**Ultra Librarian** is reachable free from any Digikey or Mouser product page.

| Part | Listing to use | Trap to avoid |
|---|---|---|
| `MICS-5524` | **Amphenol** or **SGX Sensortech** | **Not mikroElektronika** — that is a Click board, i.e. another breakout |
| `SCD40/41-D-R2` | **Sensirion**, R1 and R2 both | Match the suffix to what is ordered |
| `DS3231S#` | `DS3231SN` / `DS3231SN#`, **16-SOIC 300 mil** | **Not `DS3231MZ+` or plain `DS3231`** — those list as 8-SOIC, wrong pinout |
| CR1220 holder | Keystone `2894` / `2894TR`, SMT locking | Prefer locking over an open cup — a five-year-old will handle this box |
| `MMBT2222A` | onsemi, Diodes, ST, Fairchild | Any is fine, second-sourced generic |
| `SS12` | onsemi, Vishay `SS12-E3/61T` | Any is fine |

**SnapMagic content is vendor- and community-submitted and occasionally stale.** Thirty
seconds against the datasheet drawing is worth it every time. The MiCS-5524 library
already in `eda/libraries/` is a case in point: correct pin names, but its end-row pad
pitch is drawn at 1.25 mm where the datasheet says 1.27 — harmless at that scale, but it
shows the footprint was redrawn rather than lifted.

If a footprint is unusable, hand-drawing is not severe: both awkward parts have fully
dimensioned land patterns in [`../datasheets/`](../datasheets/) already. Rough effort —
MiCS-5524 15–20 minutes, SCD4x 30–45.

### The DS3231M alternative

`DS3231M` is the same SOIC-16, same register map, same pinout, but a MEMS resonator
instead of TCXO-plus-crystal: **±5 ppm against ±2 ppm**, and one less discrete resonator.
Both are far beyond what a child's clock needs. Noted because it is cheaper and simpler,
not because the accuracy matters.

## Fab and assembly

Observed from the vendors' own interfaces, **not negotiated** — confirm before ordering.

### AISLER

A sandbox project (`PCB_test_1`, 38.08 × 61.9 mm, 2-layer, 1 distinct component) quoted
**€193.40 for 3 assembled boards**, dispatch 17 September. That is the baseline number;
this board has considerably more on it.

### PCBWay

The assembly form's part-count fields are optional and no minimum part count gates a
quote. What is actually required: **Gerbers including solder paste**, a **BOM with MPNs
and designators**, and a **centroid / pick-and-place file**. MOQ is 5 bare PCBs but
assembly can be 1 piece with a setup fee. Workmanship IPC-A-610 Class 2.

### Three constraints that bite this board

**PCBWay will not assemble boards fabricated elsewhere** — so it is AISLER *or* PCBWay
end to end, not a mix.

**Panelisation is required under 50 × 100 mm**, with break-away rails if edge-to-copper
clearance is under 3.5 mm. This board is under that threshold, so both apply.

**PCBWay accepts Gerber RS-274X + Excellon, `.pcb`, `.pcbdoc`, `.cam` — not ODB++**,
which is the current CAM output. An export-settings change, but it has to happen before a
quote can be run at all.

### What stays hand-soldered

Decided, not incidental — everything through-hole on this board is fitted by hand:

| Ref | Part | Why |
|---|---|---|
| `U1` | Pico 2 WH | Plugs into two 1×20 sockets; the sockets are what gets soldered |
| `C1` | 1000 µF 16 V, `E5-10,5` | Radial electrolytic, 10 mm can on 5 mm pitch — **check enclosure height, it stands ~20 mm** |
| `BZ1` | UDB-05B buzzer | Through-hole part |
| `J1`, `J2`, `J4`–`J6` | JST XH housings | Through-hole, and they are the case wiring |
| `J7` | 1×3 debug header | Through-hole |

Assembly removes the fiddly small-part soldering, not the module wiring. Everything SMD
— the three sensors, the level shifter, both SOT-23s, the Schottky, the coin cell holder
and every 0805 — is machine-placed.

## Open questions

1. ~~**Will AISLER reflow the MiCS-5524 at all?**~~ **Answered — see below.** No normal
   assembly line meets the datasheet's requirement, JLCPCB's included, but the design
   does not depend on it. Kept as a decision rather than a question.
2. **Ventilation.** Both the gas sensor and the SCD40 are now inside the box on the main
   board. A CO₂ sensor and a fart detector sealed in a case measure the case. The
   enclosure needs vents at those two positions, which is an enclosure change, not a
   board one.
3. **AISLER against PCBWay.** €193.40 for 3 is a real number; PCBWay has no comparable
   quote, and getting one needs the CAM output switched off ODB++ first.
   [`cost-comparison.md`](cost-comparison.md) works the money through against the
   breakouts and flags that this €193.40 does not fit AISLER's published fee model.
4. **Does the SCD4x centre keep-out survive** in whichever footprint is adopted?
5. **Castellated pads for `U1`**, if machine placement of the Pico is ever wanted.
6. **Does AISLER's assembly service cover through-hole**, or SMD only? Determines
   whether the hand-fitted list above is right. Their public pages do not say — it is
   behind a login, so ask directly.

## The MiCS-5524 and reflow

Page 2 of [`../datasheets/mics-5524.pdf`](../datasheets/mics-5524.pdf) is unambiguous:
the sensor must be reflow soldered in a **neutral atmosphere, without soldering flux
vapours**, and must not meet organic solvents, silicone vapours or smoke, which poison
the sensitive layer.

**No standard SMT line meets that.** Flux vapour is not a side effect of reflow, it is
the process. Nitrogen reflow exists but "without flux vapours" means a specialist line,
and JLCPCB will not guarantee it. Taken literally, the part cannot be machine-assembled
anywhere we can afford.

**Taken practically, it does not matter here**, for two reasons that are specific to this
design rather than general reassurance:

- **The datasheet's own part spread is larger than the damage.** `R0` is specified as
  **100 kΩ minimum, 1500 kΩ maximum** — a 15:1 range between two good sensors out of the
  same reel. Any design that survives that cannot be reading absolute resistance, and
  ours does not.
- **The firmware never looks at an absolute value.** `drivers/gas_sensor.cpp` reports
  `excess_volts()`, which is the current reading minus a self-chasing baseline with a
  90-second time constant. A sensor whose `R0` shifted during reflow simply settles at a
  different baseline, and the tracker absorbs it within the first couple of minutes of
  being powered.

What that argument does **not** cover is loss of *sensitivity* — poisoning degrades the
response to gas, not just the resting value, and no amount of baseline tracking recovers
that. It cannot be quantified in advance. The mitigating evidence is that Adafruit's
MiCS-5524 breakout is machine-reflowed on an ordinary line and works.

**Decision: let JLCPCB place it, with one instruction attached to the order —**
**do not wash or ultrasonically clean the assembled boards.** Board cleaning is the
"organic solvents" clause hit head-on, and unlike the reflow itself it is avoidable for
free by asking.

If `U2` comes out dead or deaf, the fallback is unchanged and cheap: drop it from the
board, fit a breakout on a JST tail, and mount it at the vent — where it wanted to be
anyway. The enclosure still owes it a filtered opening; the datasheet asks for a package
that keeps water and dust off the sensor.

## Presence and ambient light

Added after the JLCPCB route was settled, for one reason: the clock stands in a bedroom.
Nobody wants four amber digits and a NeoPixel ring lighting the room at 3 a.m., and there
is no point running the display at all when the room is empty.

Two sensors, both machine-placed, both from JLCPCB stock.

| Ref | Part | LCSC | Package | Price | Stock |
|---|---|---|---|---|---|
| `U6` | Senba **BS612** PIR | `C389589` | `SENSORM-TH_BS612`, THT | $1.27 | 690 |
| `U7` | Vishay **VEML7700-TR** ambient light | `C504893` | `LED-SMD_4P`, 6.8 × 2.4 mm | $1.17 | 5,943 |
| — | Senba **SB-F-02** Fresnel lens | `C89924` | *enclosure part, not fitted* | $0.12 | 669 |

### The light sensor — VEML7700

I²C, address **0x10**. The bus already carries 0x3E (LCD), 0x60 (PCA9633 backlight),
0x62 (SCD41) and 0x68 (DS3231), so 0x10 is clear and it costs **no new GPIO** — it goes
straight on GP4/GP5.

Chosen over the BH1750 (`C78960`, cheaper, five times the stock) for one reason:
**low-light range is the whole job.** The BH1750 bottoms out around 0.5–1 lx. A dark
bedroom *is* 0.1–1 lx, so the decision would be made on noise, at exactly the point that
matters. The VEML7700 resolves to ~0.0036 lx. It also has no pad hidden under the body.

**Placement — the front face is fine.** An earlier draft of this document said to put it
on the top or a side, away from the digits. That was wrong, and it is worth correcting
because it would have forced an awkward layout. Every television, thermostat and bedside
clock with auto-dim carries its light sensor on the same face as the display. What matters
is not the face, it is the **light path**: the digits emit forwards, away from the panel
interior, and the sensor looks forwards through its own aperture. There is no direct path
between them.

The real risk is **internal leakage** — ring and digit light bouncing around inside the
case and reaching the sensor from the side or behind. So:

- a **light-tight collar** from the board surface to the front panel around `U7`, so it
  only ever sees through its own hole — this is the part that actually matters;
- a **3–4 mm aperture**; the VEML7700 sees ±55°, and a short tunnel usefully narrows that;
- put it in a **corner**, as far from the digits and the ring as the layout allows.

Mounting it on the back of the board would remove the risk entirely, but a clock pushed
against a wall would then read the wall and dim itself in daylight. Front with a collar is
the better answer.

### The motion sensor — BS612

The first pass picked the **AS312**, on the assumption it was the familiar white-domed
module. **It is not** — the whole 312/412/612 family is a bare TO-5 can with an IR window
and no optics at all. The BS612 replaces it and is the better part regardless:

| | AS312 | BS612 |
|---|---|---|
| Supply | 2.7–3.3 V, **abs. max 3.3 V** | **2.2–3.7 V**, typ 3.0 |
| Window | 3 × 4 mm | **4.2 × 5.2 mm** |
| Sensitivity | fixed | **32 steps, resistor-set** |
| On-time | fixed ~2 s | **2 s – 3600 s, resistor-set** |
| Enable | — | **`OEN` pin** |

The supply row decides it. The AS312's *absolute maximum* is 3.3 V and the Pico's 3V3 rail
sits at 3.3 V nominal — no margin at all, at the rating that kills parts rather than the
one that degrades them. The BS612 gives 0.4 V of headroom. It is also the part Adafruit
use, which is not an argument on its own but is a reasonable sanity check.

**Pinout:** 1 `SENS`, 2 `OEN`, 3 `VSS`, 4 `VDD`, 5 `REL`, 6 `ONTIME`. Six 0.91 mm holes on
a ~5.4 mm circle, Ø9.2 mm can. Idle current 9.5 µA; `REL` drives ±10 mA, so it goes
straight to a GPIO.

**Pins, as drawn:** `REL` → **GP21** (pin 27), `OEN` → **GP22** (pin 29), ground on **pin 28**
between them, `VDD` from the filtered `+3V3_PIR` rail. `OEN` on a GPIO buys a clean way to
suppress the sensor during warm-up and to offer a "motion wake off" setting. GP16–GP18 and
GP28/ADC2 stay free.

### The supporting parts

**The 100 nF is not optional.** The datasheet states it: 100 nF from `VDD` to ground, as
close to the sensor's `VDD` pin as it will go.

Given what else shares this rail, put an RC filter in front of it:
**100 Ω in series from +3V3, then 10 µF, then the 100 nF at the pin.** At 9.5 µA the
resistor drops about a microvolt, so the isolation from the ring slamming the rail is
free.

**`ONTIME`** takes a divider — `RH` = 1 MΩ fixed, `RL` selects one of sixteen times, 1%
parts:

| Time | `RL` | | Time | `RL` |
|---:|---|---|---:|---|
| 2 s | 0 Ω (omit `RH`) | | 120 s | 422 kΩ |
| 30 s | 210 kΩ | | 180 s | 487 kΩ |
| 60 s | 309 kΩ | | 300 s | 560 kΩ |

**Fit the footprints, populate `RL` = 0 Ω** for the 2 s minimum, and hold the display up in
firmware instead. Timing then lives somewhere it can be changed without an iron, and the
divider is still there if that turns out to be wrong.

**`SENS`** is a second divider, 32 steps, grounded being most sensitive. Maximum
sensitivity is not wanted here — with the lens it reaches 10 m and would see straight
through an open bedroom door. `RH` = 1 MΩ with `RL` = 150 kΩ lands about mid-scale. Make
them fitted parts and tune on the bench.

**`OEN`** to `VDD` for always-on, or to GP19 as above.

### The lens is mandatory, and JLCPCB will not fit it

The BS612 has no optics. Bare, it reaches about a metre; the range and the coverage pattern
come entirely from the Fresnel lens. **`SB-F-02` (`C89924`)** is the matching Senba part:

| | |
|---|---|
| Focal length | **11.5 mm** |
| Coverage | **120°** |
| Range | **10 m** |
| Material | **HDPE** |
| Size | Ø25.75 mm flange, 16.9 mm tall |

HDPE is why the lens is *also* the window — it passes 8–14 µm, and ordinary plastic, glass
and acrylic do not. Nothing else goes over it, and no open hole either.

**JLCPCB stock it but cannot place it.** It is filed under Optoelectronics Accessories with
`assemblyComponentFlag: false` — no footprint, so it cannot appear in the CPL, so it cannot
be a line in the PCBA BOM. What they *will* do is sell it: minimum purchase 1, $0.1157 each
at 1–49, so **five lenses cost $0.58**, added as a parts order alongside the PCBA order and
arriving loose in the same shipment. Fitting it is enclosure work.

Note the mechanical consequence, which is larger than a sensor hole: a **25.75 mm flange
and a 16.9 mm dome** on the front panel, with the sensor **11.5 mm behind it on the optical
axis**. That distance is the focal length and the datasheet is blunt that getting it wrong
collapses sensitivity. `SB-F-01` (`C123326`) is 13.6 mm square if that is too much on the
clock face — but its focal length is not published, and focal length is the whole game.

### Layout rules the datasheet insists on

These conflict with this board in particular, so they are not boilerplate:

- **Keep the PIR's connections short, and route nothing underneath them — high-current
  traces least of all.** The NeoPixel ring and both digit chains are exactly that. Keep
  them away from `U6`.
- **Give the PIR its own ground island**, tied to the rest of the board by only three
  connections: +, −, and output.
- Keep it away from `U2` as well. The MiCS runs a 76 mW heater and pyroelectric sensors
  respond to thermal drift.
- The sensor must be behind its lens **and inside the finished housing** before it is
  tested. The datasheet is explicit that an exposed can and exposed pins give poor
  sensitivity and wind-triggered false alarms.

### Two more lines for the order remark

Both belong with the no-wash instruction already recorded for the MiCS:

- **Do not wash or ultrasonically clean the assembled boards.** Two parts now require it —
  the MiCS-5524, and the BS612, where cleaning fluid wicking into the lens degrades it
  permanently.
- **Hand-solder the BS612 at ≤ 350 °C for ≤ 3 s.** The datasheet warns that wave soldering
  may degrade it, and JLCPCB solder through-hole parts by hand or selectively.

### What it costs

About **$2.56 a board** in parts — $1.27 + $1.17 + $0.12 — plus roughly four passives and
two more Extended-part feeder loads. Call it **+$25 on the order of five**.

### Still open

- **`SENS` and `ONTIME` values are starting points, not results.** Both want a bench pass
  with the lens fitted and the case closed, because the datasheet is clear that sensitivity
  is a joint function of the lens, the housing, the target's temperature and the ambient.
- **Whether the display should go dark or merely dim at night.** Off stops it being a clock
  at 3 a.m.; dim-plus-motion means a glance and a shift in bed brings it up. The second is
  probably what is actually wanted, and costs nothing extra.
- **The 11.5 mm focal distance is an enclosure feature**, and the enclosure already owes
  vents to the MiCS and the SCD41.

## What the board owes, beyond the parts

Four things that are not components and are easy to reach the order without.

### M3 mounting holes

**Use `MOUNT-HOLE`, variant `3.2`, from Fusion's managed `holes` library** — already
installed, no import needed. That is a 3.2 mm unplated hole, the right clearance for an M3
screw. The same library also offers `MOUNT-PAD-ROUND` and `MOUNT-PAD-SQUARE`, which are the
*plated* versions; take those only if the board is to be grounded to a metal chassis. Ours
sits in a plastic box, so unplated is right.

- **Three, and that is deliberate** — `H1` (78.74, −22.86), `H2` (78.74, −85.09), `H3`
  (5.08, −60.96), all at least 3 mm in from the board edge. The unsupported corner is the
  top-left, which is the **antenna end**, and a steel fastener there detunes the radio more
  than anything else on the board. Three screws hold a board this size perfectly well.
- **Keep-out about 7 mm across** each — no copper, no components. An M3 head is 5.5–6 mm
  and needs somewhere to land.
- Keep them clear of the digit column, `U6` and `U7`.

> **Take the positions from the enclosure, not from the board.** This PCB *is* the front
> panel — the digits show through it — so the holes have to meet the case bosses. That
> makes the enclosure the thing to settle first, along with the two sensor vents, the PIR
> lens aperture and the light sensor's collar. Drawing holes before the box exists means
> drawing them twice.

### The ground plane

**Both layers are poured `GND`**, filled around the routing rather than left as a signal-only
board. Settings, which matter and are easy to get wrong:

| | | |
|---|---|---|
| Fill width | **0.2 mm** | must be ≥ the `Copper Width` design rule, or DRC floods with width errors |
| Isolate | 0.3 mm | comfortably above the fab minimum |
| Thermals | **on** | the through-hole parts are hand- or selective-soldered, and a pad tied straight into a plane sinks the iron's heat into a cold joint |
| Orphans | off | drops islands that cannot reach the net |
| Rank | 1 | |

**Fusion's layer names do not match EAGLE's numbers**, which wasted an evening. Bottom copper is
**304**, not 16 — layer 16 exists as an inactive `Route16`. There is one **`e3 RestrictCopper`**
layer rather than separate top and bottom ones, and it binds to whichever **copper layer is
active** when you draw: the same rectangle landed on 41 from the top and 42 from the bottom.
`LAYER 41;` silently fails because that name does not exist in Fusion's scheme. Set layers from
the dropdown, not the command. And the command line takes no `/` prefix — it is the field along
the bottom of the window.

**One restrict rectangle, over the antenna**, at (10.3, −6.2) → (20.5, −20.2): 10.2 × 14.0 mm,
sitting in the notch band between the Pico's two pad rows. Do not make it taller — the Pico's own
GND pads sit at x 7.72 on y −4.32 and −22.10, just above and below, and a taller rectangle starves
them of plane. Nothing else needs a restrict: the pour keeps its isolate from `AGND` and
`+3V3_PIR` automatically, because they are different nets.

#### The failure this plane already had

`U4`'s SOIC pad ring **fenced the pour off from itself**. The gap between adjacent 1.27 mm-pitch
pads is narrower than isolate plus fill width, so copper cannot squeeze through, and the pocket
inside the ring became an island of 246 mm² carrying all **nine of the DS3231's GND pads** and
nothing else. Every pad had a thermal, the board looked finished, and the RTC had **no ground at
all**. It was fixed by moving one trace so the pour could flow in and merge the pocket.

**This is the failure mode to check for after any reroute.** It is invisible in the ratsnest —
the pads are connected to *something* named GND, so no airwire shows. The test is whether every
poured region reaches the main plane, not whether every pad has copper on it.

**Eleven stitching vias** now tie the planes together, placed for reasons rather than on a grid:

| Where | Why |
|---|---|
| (8.43, −26.45), (47.55, −20.18) | the two regions that depended on a **single** component pad each |
| (32.18, −25.75), (56.23, −27.72), (72.89, −26.00) | digit power-tap return current |
| (22.90, −55.83), (25.66, −37.26), (26.92, −20.27) | along the `GAS_ADC` run — see *Supply ripple* and the divider notes |
| (6.50, −84.00), (78.91, −80.62), (77.60, −7.70) | corners, general hygiene |

Before them, **every** layer-to-layer ground connection went through a component's through-hole
pad — there were no dedicated vias at all. A dense grid was considered and rejected: the fastest
edge on this board is the WS2812 line at 800 kHz, so via spacing buys nothing at RF, and the real
requirement is redundancy against fenced-off pockets.

**`AGND` stays out of all of it.** Three pads (`U1.33`, `R4.2`, `C2.2`), three segments, no vias,
and verified that none of its copper falls inside the GND pour. It meets digital ground only
inside the Pico. Note what that does and does not buy: the divider's lower leg and the settling
cap return cleanly to the Pico's analog pin, but the sensor's own `R_LOAD` (`R10`) still returns
to main `GND` down at `U2`, so the measurement is referenced across two grounds. The split
protects half the path; the solid plane and the stitching along `GAS_ADC` are what protect the
other half.

### Edge rails

Strips of spare board, usually 5 mm, along two opposite edges. The assembly line's conveyor
grips **those** instead of the board, so parts can sit right up to the true edge rather than
being pushed 5 mm inward. They snap off along a V-score afterwards.

JLCPCB list them as **Necessary** for Standard PCBA, which is the service this board is on.

Two ways to satisfy it, and the second is easier: draw them yourself — which grows the board
to roughly 139.68 × 97.30, adds area you pay for, and puts the V-scoring on you — or choose
**"Panel by JLCPCB"** at order time and let them add rails and scoring. For five boards the
second is almost certainly the right call. Worth asking them which they would rather receive.

### Fiducials

The machine's eyes: bare copper dots, connected to nothing, that the pick-and-place camera
uses to work out exactly where the board is sitting and how far it is skewed, then corrects
every placement against them.

- **1 mm copper pad, 2 mm solder-mask opening**, no net.
- **Three of them**, near the corners, **placed asymmetrically** so the orientation cannot
  be read two ways.
- At least 5 mm from the board edge.

**There is no fiducial part in Fusion's managed libraries** — `holes.lbr` covers mounting
holes only. So they are either drawn directly on the board as a pad with an explicit
`tStop` circle, or made once as a tiny library device and placed three times. If JLCPCB
panelise, they normally put fiducials on the rails themselves; adding three to our own board
anyway costs nothing and guarantees alignment of *this* board rather than the panel.

### The Pico's antenna, and the hole in the board

> ⚠️ **This section previously had the two ends of the Pico swapped.** It claimed the
> antenna keep-out was a single 5.6 × 15.4 mm block on `tRestrict`/`bRestrict` at the end
> away from USB, and that the milled cutout was a USB window. Both wrong, and three rounds
> of parts were moved on the strength of it. What follows is read out of the footprint and
> cross-checked against pad numbering.

`PICO2W_SMT_THT` carries three separate things that are easy to confuse:

| Feature | Layer | Where | What it is |
|---|---|---|---|
| **Antenna relief** | `Milling` (46) | **antenna end** | **11.54 × 14.00 mm hole straight through the board** |
| Antenna label | `tDocu` (51) | antenna end | the text `ANTENNA KEEP-OUT 14 x 9` — documentation only |
| Six 2.1 × 2.1 mm squares | `tRestrict` + `bRestrict` (41/42) | **USB end** | keep-outs round the Pico's **underside test points** — nothing to do with the radio |

**Which end is which** is settled by the pad numbering, not by intuition. Pins 1 and 40
flank the USB connector; pins 20 and 21 are at the debug-header end, which on a Pico W is
the antenna end. With `U1` at (1.27, −23.71) `MR90` on this board:

| | Board coordinates |
|---|---|
| Pads 20 / 21 — **antenna end** | x **2.64** |
| Pads 1 / 40 — **USB end** | x **50.90** |
| Antenna relief cutout | x **−1.27 … 10.27**, y **−6.21 … −20.21** |
| Test-point squares | scattered within x 36.72 … 52.12, y −11.16 … −16.76 |

> **Do not bounding-box the restrict rectangles.** There are six of them, scattered, each
> 2.1 mm square — 1.5 mm test point plus 0.3 mm margin all round. Taking min/max across all
> six produces a phantom 15.4 × 5.6 mm block at the wrong end of the module, which is
> exactly the error above. Check parts against each rectangle individually.

**The antenna needs no copper rule, because there is no board under it.** The relief is a
hole, so nothing can encroach on it by construction. That also means **the side the Pico
sits on makes no difference to the radio** — mounting it on the back costs nothing.

On this board the cutout runs **1.27 mm past the left board edge**, so it comes out as an
edge notch rather than an enclosed slot — marginally cheaper to machine, and it means the
antenna end already faces open air, which is what you want.

**It will not appear in the 3D view.** Fusion builds the 3D body from the layer-20 outline
and ignores internal `Milling` geometry. Check layer 46 in 2D; its absence in 3D means
nothing.

> ⚠️ **It reached AISLER but not JLCPCB.** AISLER accept the **native board file** and read
> the EAGLE data directly, so footprint-level layer 46 is visible to them — the relief was
> cut correctly there. JLCPCB take **Gerbers**, and JLCPCB's viewer showed a plain rectangle.
>
> **That is a CAM-job setting, not a defect in the board.** The outline output must list
> **both Dimension (20) and Milling (46)**; if it lists only Dimension, footprint milling is
> silently dropped. Check that before changing any geometry. Worth also confirming whether
> Fusion's output path respects layer *visibility* — classic EAGLE's CAM processor does not,
> but the newer one is unverified, and hiding layer 46 to reduce clutter is an easy thing to
> have done.
>
> **The export could not be fixed.** Fusion's Profile output has `Board Shape` and `Cutouts`
> both enabled and still emits a plain rectangle — it does not reach into footprints for
> milling. **Done: the relief is now drawn at board level** on `m1 BoardOutline` (EAGLE layer
> 20; `m6 Milling` is layer 46). The outline is a closed 8-segment contour of 7400.0 mm²,
> which is the 83.82 × 90 rectangle less exactly 10.27 × 14.00. For the record, with `U1` at
> (1.27, −23.71) `MR90`, the left edge was deleted and replaced by:
>
> | From | To |
> |---|---|
> | (0, −90) | (0, −20.21) |
> | (0, −20.21) | (10.27, −20.21) |
> | (10.27, −20.21) | (10.27, −6.21) |
> | (10.27, −6.21) | (0, −6.21) |
> | (0, −6.21) | (0, 0) |
>
> Drawn with the layer dropdown set to `m1 BoardOutline` and the PCB editor's own command line
> (press `/`) — *not* the Text Commands palette, which is Fusion's API console and does not
> speak EAGLE:
>
> ```
> WIRE 0 (0 -90) (0 -20.21) (10.27 -20.21) (10.27 -6.21) (0 -6.21) (0 0);
> ```
>
> That is the footprint's relief clipped to the board — the original overhangs the left edge by
> 1.27 mm, so it becomes a **notch in the profile** rather than an enclosed slot. Being part of
> the Board Shape, it now also appears in the 3D view, which the layer-46 version never did. Profile
> routing is included in the board price; an internal slot is not. It sits strictly between the
> Pico's two pad rows (y −4.32 and −22.10), so the module stays supported and only the antenna
> end overhangs air. **The enclosure has to match the notch.**

What the footprint cannot control:

- **Keep metal away from the antenna end.** This board has exactly three metal objects:
  `C1` (10 mm aluminium can), `BZ1` (Ø9 mm buzzer can) and `BT1` (the cell). All three are
  at the far end — `C1` is 57 mm from the Pico.
- **Keep the M3 screws away too.** A steel fastener near the antenna detunes it more than
  any amount of plastic. Nylon in the nearest, if the layout forces it. **Brass is no
  better** — it is a *better* conductor than steel, so it reflects just as effectively; all
  it saves is a little absorption loss. Distance beats alloy.
- **Keep the pour back** from the antenna end beyond the module's own footprint.

> **Vent holes do nothing for WiFi.** Plastic is transparent at 2.4 GHz — ABS, PLA, PETG,
> resin, all of it, with a loss you could not measure. Vents belong where the *sensors* need
> air and nowhere else. The exception is a metallic paint finish or a metal-filled filament,
> which would matter a great deal.

**There is no USB window.** The only milled feature is the antenna relief, at the opposite
end. Once the Pico is soldered down its micro-USB socket is unreachable, full stop — which
is why `J7` and the debug probe exist, and why the `stdio` changes below are not optional.

### First power-up: four things that are not on the board

**`J7` is a 5-way JST XH, and the debug cable does not exist yet.**

| `J7` | net | Pico | goes to |
|---|---|---|---|
| 1 | `TX` | `GP0` — UART0 TX | probe `U` port **RX** |
| 2 | `RX` | `GP1` — UART0 RX | probe `U` port **TX** |
| 3 | `RUN` | `RUN` — pin 30 | pull to GND to reset |
| 4 | `GND` | | GND |
| 5 | `3V3` | `3V3(OUT)` — pin 36 | reference / meter point |

**SWD does not come out to `J7`. It goes to the Pico's own connector.**

**The module is a Pico 2 W *with headers* (WH) — decided.** On the `H` variants the debug port
is a **keyed 3-pin JST-SH connector** rather than three through-holes, and on the wireless
boards it sits *"underneath the microcontroller chip nearer the middle of the board"*. Read
that carefully: it means **below the chip in the layout, on the same component side** — not on
the reverse of the PCB. The connector faces **up**, away from this board, so the Debug Probe's
supplied JST-SH-to-JST-SH cable plugs straight into it with nothing in the way. Standoff height
is irrelevant.

> **An earlier draft of this section had that wrong**, and concluded the connector was buried
> under the module and that `J7` had to carry SWD. It does not.

**One consequence for the footprint.** `PICO2W_SMT_THT` carries three through-hole debug pads —
`SWCLK` (11.08, 19.80), `GND` (13.62, 19.80), `SWDIO` (16.16, 19.80). A WH has a connector there,
not holes, so **those three pads should be deleted from the library**. No keep-out is needed in
their place: the connector is on the module's *top* face, so the underside over your board is bare
and nothing has to be cleared. The 40 header pads are unaffected — a WH's pre-soldered pins drop
straight into them.

**Two 1×20 female sockets, and the Pico plugs in.** The assembler fits the sockets — ordinary
2.54 mm THT parts they stock — and the off-the-shelf WH goes in afterwards, by hand, with nothing
soldered to the module at all. Its headers and its debug connector both arrive fitted.

The socket is **JILN `22850120ANG1SYA01`, LCSC `C429947`** — 1×20, 2.54 mm, 8.5 mm insulator,
3 A, brass with a gold flash, datasheet at
[`../datasheets/C429947.pdf`](../datasheets/C429947.pdf). Two per board.

The 40 pads need no change for this: drill 1.0 mm, 2.2 mm offset ovals, rows 17.78 mm apart at
2.54 mm pitch. The socket's tail is a 0.64 mm square post 3.2 mm long, so it passes a 1.6 mm board
with enough left to solder. **The datasheet recommends Ø1.02 mm rather than our 1.00** — 0.02 mm
under, against a 0.905 mm pin diagonal, so there is still roughly 0.1 mm of clearance and it is not
worth changing. Note it only if the sockets ever fight going in.

**These cannot be reflowed.** The insulator is polyester rated **180 °C for 30–60 s** (200 °C for
10 s), well under a reflow peak, so they are hand- or selective-soldered — which is what a
through-hole part on this board was going to get anyway.

Two consequences. The socket lifts the module **~8.5 mm** off the board, which the enclosure has to
allow for — nothing on the back sits under it. And the **BOM and CPL must name the sockets**, not
the Pico.

#### `U1` means the sockets, not the module

Rather than invent a second designator with no footprint behind it, `U1` stays one designator and
carries **quantity 2 with the socket's MPN**. Two 1×20 strips go into its 40 holes; the Pico is not
on the assembly at all. That needs no agreement with the fab — the BOM says two sockets, so two
sockets get fitted.

**The board has to say so as well, because almost nothing in this footprint prints.** Its useful
annotation lives on layers **48 (Document)** and **51 (tDocu)** — dimensions, test-point geometry,
`ANTENNA KEEP-OUT 14 x 9` — and neither is manufactured. What actually reaches the board is layer
**21 (tPlace)**: an outline rectangle, a corner tick and a pin-1 dot at local (−3.3, 49.63), plus
`>NAME` and `>VALUE`. So:

| Add | Where | Why |
|---|---|---|
| `U1` value → `PICO 2 WH — FIT 2x 1x20 SOCKET` | schematic, prints via `>VALUE` | the instruction ends up on the board itself |
| `USB` and `ANT` end labels | board silkscreen, layer 21 | orientation cannot be misread |
| `1` and `40` numerals | board silkscreen, layer 21 | backs up the single pin-1 dot |

Local pad-1 is at (1.61, 49.63) — the **USB** end; pads 20/21 at y 1.37 are the **antenna** end.

**Ways 3 and 5 carry `RUN` and `3V3` instead.** With USB sealed inside the case, `RUN` is the
one recovery line worth having — pull it to GND and the Pico resets without the plug being
pulled — and it costs no GPIO. `3V3` gives the service tail a voltage reference and a meter
point at the connector. `J7`'s single ground pin is fine now that it only ever serves the
probe's `U` port.

Crimp all five. The cable ends in three Dupont females for the probe's `U` port (ways 1, 2, 4);
`RUN` and `3V3` are bench conveniences, not part of the probe connection.

**Flashing over SWD needs OpenOCD, not `picotool`.** `picotool` talks USB only (BOOTSEL or a
running USB stdio stack), so `./go.sh flash` stops working the moment the micro-USB is
unreachable. The VS Code Pico extension bundles OpenOCD, GDB and the Arm toolchain; confirm
OpenOCD is actually present under `~/.pico-sdk/`, since
[`references.md`](references.md) does not list it. The Debug Probe supplies **no power** on
either port, so it never acts as a second supply.

> **Label the cable.** Pin 1 is TX and pin 2 is RX, and TX must reach the probe's RX. A
> swapped pair gives a silent console and no error message to explain it.

**And the firmware still sends `printf` to USB, which will be unreachable.**
[`../CMakeLists.txt`](../CMakeLists.txt) has `pico_enable_stdio_uart(iss_box 0)` and
`pico_enable_stdio_usb(iss_box 1)`; `src/main.cpp` then waits on `stdio_usb_connected()`.
Once the Pico is soldered down, that console is gone. Three changes bring it back on `J7`:

1. `pico_enable_stdio_uart(iss_box 1)`
2. `pico_enable_stdio_usb(iss_box 0)`
3. drop the `stdio_usb_connected()` wait, which would otherwise sit through its timeout on
   every boot waiting for something that never arrives

**Do not make these yet.** Bring-up is still happening on a breadboard Pico where USB stdio
is the convenient thing; flipping them now makes today's work harder for a board that does
not exist. This is a first-power-up step, not a today step. Flashing over SWD works either
way — only the console depends on it.

**And two more, both Sensirion's.**

**Force the SMPS into PWM** — `cyw43_arch_gpio_put(1, true)` immediately after
`cyw43_arch_init()`. See *Supply ripple* above. It costs nothing, and it is the only thing
standing between the 3V3 rail and the SCD41's 30 mV limit. It can be measured honestly even
with the sensor soldered down: send `stop_periodic_measurement` first, and the rail is then
unloaded by the sensor, which is the exact condition the 30 mV is specified under.

**`set_temperature_offset`** — it defaults to 4 °C and is only meaningful once the sensor is
in the finished enclosure, at thermal equilibrium, in the operating mode the product will
actually run. Determine it against a reference thermometer as part of the acceptance test
above, then `persist_settings` to write it to EEPROM. Skipping this does not just cost you
the temperature reading — RH and T feed the on-chip compensation of the CO₂ output, so it
costs accuracy even if you never display them.

### And the ones already on the list

- `DIGIT2` and `DIGIT3` still carry rotations of **269.6°** and **270.5°**. Harmless to
  assembly, since the digits are hand-fitted, but it makes their drilled holes sit at an
  angle. Set both to exactly 270°.
- `C11` as close to `U6` pin 4 as it will physically go.
- `U6` away from `U2`, with its own ground island and nothing routed underneath.
- `U7` in a corner, away from the digits and the ring.

## Resolved since the scouting pass

- **R_LOAD** — 10 kΩ, measured. Divider maths unchanged.
- **`AGND` in the Pico symbol** — broken out as its own pin; net holds three things.
- **I²C pull-ups** — 4.7 kΩ on the board, decided.
- **Which modules stay off-board** — six, all JST XH.
- **SCD40 or SCD41** — **SCD41**. Table 1 of [`../datasheets/scd4x.pdf`](../datasheets/scd4x.pdf)
  gives ±(50 ppm + 2.5%) over 400–1000 ppm against the SCD40's ±(50 ppm + 5%), and it
  keeps reading to 5000 ppm rather than stopping at 2000. Twice the accuracy in the band
  a room actually sits in, for about $2. Same package, same pinout, no driver change.
