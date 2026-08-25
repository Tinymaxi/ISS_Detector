# Moving to a fabricated PCB

**Scouting notes, not settled design.** Nothing here has been committed to in the
schematic, no footprint has been drawn, and no board has been ordered. This records
what was found while asking one question: *if the breakouts were replaced by bare SMD
parts and a house placed them, what would that actually involve?* Prices and stock are
as of **25 August 2026** and will drift — check live before ordering.

The build as it stands is hand-soldered onto Perma-Proto board, and
[`../README.md`](../README.md) plus [`wiring.html`](wiring.html) describe that build.
This document does not replace either of them.

## The idea

Three of the parts in the box are breakout boards carrying a bare SMD chip and a
handful of support components. All three chips are reflow-solderable in their own
right. If they were placed directly on a custom PCB, the breakouts disappear and so
does most of the hand-wiring — at the cost of taking over the support circuitry the
breakouts were quietly providing.

## The three modules as bare parts

| Part | Was | Bare MPN | Package | Approx. price |
|---|---|---|---|---|
| CO₂ sensor | Adafruit 5187 breakout | `SCD40-D-R2` / `SCD41-D-R2` | 16-pad, 10.1 × 10.1 × 6.5 mm | ~$18.66 / ~$20.54 |
| Gas sensor | Adafruit 3199 breakout | `MICS-5524` | 10-pad, 5 × 7 × 1.55 mm | — |
| RTC | Adafruit 3013 breakout | `DS3231S#` | SOIC-16, 300 mil | ~$13 |

All three are stocked at Digikey and/or Mouser, both of which are among the seven
distributors AISLER's BOM tool pulls live stock and price from (the others being
Würth, TI, Farnell, Arrow and LCSC). They should therefore resolve automatically in
an AISLER project rather than needing to be sourced by hand.

**None of this is a code change.** Same silicon, same I²C addresses, same registers —
`drivers/scd4x.cpp`, `drivers/gas_sensor.cpp` and the DS3231 handling are unaffected.

### The DS3231M alternative

`DS3231M` is the same SOIC-16 package, same I²C register map, same pinout, but uses a
MEMS resonator instead of the TCXO-plus-crystal. **±5 ppm against the DS3231's ±2 ppm**
— and one less discrete resonator to place. Both are far beyond what a child's clock
needs. Mentioned because it is cheaper and slightly simpler, not because the accuracy
matters here.

## What the breakouts were doing for free

This is the part that is easy to underestimate. Each breakout carries support
components that have to reappear on the new board.

### MiCS-5524 — two resistors, both critical

From the recommended circuit on p. 3 of [`../datasheets/mics-5524.pdf`](../datasheets/mics-5524.pdf):

```
                +5 V
                 │
          ┌──────┴──────┐
     (G)  │   heater    │  (F)      Rh1 = pin C, Rs1 = pin D
          │  ~74 Ω int. │           Rh2 = pin F, Rs2 = pin G
          └──────┬──────┘
                 │
              [ 82 Ω ]   ← external, sets heater temperature
                 │
                GND

  sense (D) ──┬──► existing 10k + 4.7k + 470 + 470 divider ──► GP26
              │
        [ R_LOAD ≥ 820 Ω ]
              │
             GND
```

**The 82 Ω heater resistor** divides against the sensor's own ~74 Ω internal heater to
land at V_H = 2.4 V typical off a 5 V rail, drawing I_H = 32 mA for 76 mW. Off-value
here shifts heater temperature, which shifts sensitivity — and the datasheet is blunt
that heater voltage above the maximum rating destroys the sensor.

**R_LOAD must be at least 820 Ω** "in order not to damage the sensitive layer". Its
actual value is what turns the sensor's resistance swing into the voltage the existing
divider maths in [`../README.md`](../README.md) was sized around.

> **Open: read the real R_LOAD off Adafruit's own schematic before choosing a value.**
> The 3199 breakout already has one fitted and its value was not established here.
> Guessing a different one silently moves the ADC range the gas scoring is calibrated
> against. Source:
> [Adafruit-MiCS-5524-Gas-Sensor-Breakout-PCB](https://github.com/adafruit/Adafruit-MiCS-5524-Gas-Sensor-Breakout-PCB).

Also from the datasheet, and relevant to a reflow house rather than to us:

- Must be **reflow soldered in a neutral atmosphere, without soldering flux vapours**.
- Must not be exposed to solvent, silicone or cigarette smoke — it poisons the
  sensitive layer.
- Wants a **filtered package** protecting it from water and dust projections.
- Supply is a narrow **4.9–5.1 V**.

### DS3231 — coin cell, decoupling, and eight pins that must be grounded

Confirmed against the Pin Description on p. 9 of
[`../datasheets/ds3231.pdf`](../datasheets/ds3231.pdf):

| Pin | Name | What it needs |
|---|---|---|
| 2 | `VCC` | **Decoupling, 0.1 µF to 1.0 µF.** The datasheet asks for it by name. |
| 3 | `INT/SQW` | Open-drain — **requires a pull-up**, to a supply of 5.5 V or less. |
| 4 | `RST` | **Internal 50 kΩ pull-up. "No external pullup resistors should be connected."** |
| 5–12 | `N.C.` | **"Must be connected to ground."** Eight pins, not optional. |
| 14 | `VBAT` | Coin cell. **No capacitor required** when used as the backup source. |
| 15 | `SDA` | Open-drain — **requires an external pull-up**. |
| 16 | `SCL` | Clock input. Tolerates up to 5.5 V regardless of `VCC`. |

Two things worth pulling out. **`RST` explicitly must not get an external pull-up** —
the opposite of the reflex. And **pins 5–12 being N.C. does not mean leave them
floating**; the datasheet requires them tied to ground, which is real copper on the
layout.

The existing `SQW` arrangement is already fine: [`../README.md`](../README.md) drives
it into GP11 using the RP2040's internal pull-up, which satisfies the datasheet's
requirement without a discrete part.

The coin cell holder is the one genuinely new mechanical part — see the footprint
table below.

### SCD4x — the keep-out is the whole story

No new support components: the existing `C4` (100 nF ∥ 10 µF) already covers
decoupling, and the datasheet confirms `VBAT`-style extras are not needed here.

What does change is layout. From §4.2 "Land Pattern Recommendation" on p. 24 of
[`../datasheets/scd4x.pdf`](../datasheets/scd4x.pdf):

- **16 pads at 1.25 mm pitch**, with full landing-pad, solder-paste and solder-mask
  drawings given in Figure 4 — nothing has to be derived.
- A **non-plated Ø 0.25 mm thermal relief hole in the centre** of the package, with a
  **Ø 0.6 mm area that must be kept free from solder and flux**. The datasheet states
  the keep-free area "must be respected" in any case.
- Pin 1 is marked by a circular mark **and** a notched corner.
- The **white protective membrane on top must not be removed or tampered with**.

That centre keep-out is the detail a generic footprint generator would not know about,
which makes it the thing to check on any downloaded footprint.

## The rest of the BOM as SMD

0805 chosen for passives throughout — no cost over 0402 and comfortably reworkable by
hand if a board ever needs fixing.

| Ref | Was | SMD part | Package | Critical? |
|---|---|---|---|---|
| `D1` | 1N5817 | `SS12` (Diodes `SS12-13-F`, Vishay `SS12-E3/61T`) | SMA / DO-214AC | No — SS14 works identically |
| `Q1` | 2N2222A | `MMBT2222A` (onsemi `MMBT2222ALT1G`, Diodes `MMBT2222A-7-F`) | SOT-23-3 | **Check pinout** — SOT-23 E-B-C order is not the TO-92 order |
| `R1` | 470 Ω | `RC0805FR-07470RL` | 0805 | No |
| `R8` | 1 kΩ | `RC0805FR-071KL` | 0805 | No — 470 Ω–4.7 kΩ all work |
| `R4`–`R7` | 10k / 4.7k / 470 / 470 | Yageo `RC0805FR-07` series, **1 %** | 0805 | **Yes** — these set the divider maths |
| `C1` | 1000 µF 6.3 V | *consider leaving THT* — see below | — | No |
| `C2`, `C3+` | 100 nF | Murata `GRM21BR71H104KA01L` | 0805 | No |
| `C4` | 100 nF ∥ 10 µF | as above ∥ Samsung `CL21A106KOQNNNG` | 0805 | No |

`R2` stays unfitted. The RGBDigit module already carries 330 Ω in its `Di` line and an
external resistor makes 800 Ω — the reasoning is in
[`../datasheets/README.md`](../datasheets/README.md) and has not changed.

**`C1` is the one part worth leaving through-hole.** SMD electrolytics that size exist
(Panasonic `EEEFK0J102P`, case D, ~10 × 10 mm) but they are bulky, pricier, and a mixed
THT/SMD board is entirely normal. A big obvious capacitor that can be eyeballed and
swapped is worth more here than package consistency.

If the 100 µF upgrade for `C4` mentioned in the wiring sheet is ever taken, note that
it jumps to a 1206/1210 package (e.g. Murata `GRM31CR60J107ME39`) — a layout change,
not a drop-in.

## Footprints and Fusion libraries

**Fusion Electronics is EAGLE underneath**, so it imports EAGLE `.lbr` files directly
via the Library Manager, or by uploading the library through the Data Panel. Anything
offered as "EAGLE" or "Fusion 360" format is the same file. This matters because it
means essentially every library source on the web is usable.

Two sources cover everything needed:

- **SnapMagic Search** (formerly SnapEDA) — has a **native Fusion 360 app** that
  searches and places symbols, footprints and 3D models from inside the Electronics
  workspace, no manual file handling. This is the path of least friction.
- **Ultra Librarian** — reached free from Digikey and Mouser product pages via the
  "EDA/CAD Models" row on any part. Exports 20-plus formats including EAGLE/Fusion,
  bundled with the 3D STEP.

Standard passives and generic packages (0805, SOT-23, SMA, SOIC-16) are very likely
already covered by Fusion's built-in library; the external sources are really only
needed for the manufacturer-specific parts.

### Availability — all six checked, all six found

| Part | Listing to use | Trap to avoid |
|---|---|---|
| `MICS-5524` | The **Amphenol** or **SGX Sensortech** listing | **Not the mikroElektronika one** — that is a Click board, i.e. another breakout |
| `SCD40/41-D-R2` | Listed directly by **Sensirion**, R1 and R2 both | Match the suffix to what is actually ordered |
| `DS3231S#` | `DS3231SN` / `DS3231SN#`, correctly shown as **16-SOIC 300 mil** | **Not `DS3231MZ+` or plain `DS3231`** — those list as 8-SOIC, wrong pinout |
| CR1220 holder | Keystone `2894` / `2894TR`, SMT locking | Prefer the locking type over an open cup — a five-year-old will handle this box |
| `MMBT2222A` | onsemi, Diodes, ST, Fairchild — all SOT-23-3 | Any is fine, second-sourced generic |
| `SS12` | onsemi, Vishay `SS12-E3/61T`, others — all SMA | Any is fine |

**Caveat on all of them:** existence was confirmed from listings, not by inspecting pad
geometry against the datasheets. SnapMagic content is vendor- and community-submitted
and occasionally stale. Thirty seconds of checking each against the datasheet drawing
is worth it — most of all for the SCD4x centre keep-out described above, which is
exactly the kind of thing a generic footprint gets wrong.

If a footprint ever does turn out to be unusable, hand-drawing is not severe: both
awkward parts have fully dimensioned land patterns in the datasheets already in
[`../datasheets/`](../datasheets/). Rough effort — MiCS-5524 15–20 minutes, SCD4x
30–45 minutes.

### The Pico itself

Not resolved. SnapMagic lists the Pico W directly from Raspberry Pi (as
`RASPBERRY PI PICO W` and `SC0918`); community EAGLE libraries exist at
[Lobo-T/RPI-Pico-Eagle-lib](https://github.com/Lobo-T/RPI-Pico-Eagle-lib) and
[codemicro/pico-eagle](https://github.com/codemicro/pico-eagle).

> **Open: check how any Pico symbol treats `AGND` on pin 33 before adopting it.**
> A generic symbol will almost certainly merge it into common GND. The gas divider and
> its 100 nF return there specifically, and [`references.md`](references.md) already
> makes the point that this only stays true on the PCB if the schematic says so. Easy
> to fix by editing the symbol after import — easy to miss if not looked for.

Also confirm any footprint is **Pico W** and not plain Pico: identical headers, but the
outline and keep-out differ around the antenna.

## Fab and assembly

Findings from a separate look at PCBWay's and AISLER's own interfaces. **Recorded as
observed, not negotiated** — confirm with the vendor before ordering.

### AISLER — the baseline quote

A sandbox project (`PCB_test_1`, 38.08 × 61.9 mm, 2-layer, 1 distinct component) quoted
**€193.40 for 3 assembled boards**, dispatch 17 September.

### PCBWay — what actually gates a quote

The assembly form's part-count fields — unique parts, SMD, BGA/QFP, through-hole — are
**optional**, and no minimum part count gates a quote. "Unique parts" means distinct
part numbers in the BOM, not total placements.

What is actually required:

- **Gerbers including the solder paste layer**
- **BOM with manufacturer part numbers and designators**
- **Centroid / pick-and-place (CPL) file**

Terms: MOQ is **5 bare PCBs**, but assembly can be **1 piece** with a setup fee.
Workmanship is **IPC-A-610 Class 2**.

### Three constraints that bite this board specifically

**PCBWay will not assemble boards fabricated elsewhere.** Assembly with PCBWay means
fabrication with PCBWay — so this is an either/or against AISLER, not a mix-and-match.

**Panelisation is required under 50 × 100 mm**, and break-away rails are required if
edge-to-copper clearance is under 3.5 mm. At roughly 38 × 62 mm this board is under
that threshold, so both apply.

**PCBWay accepts Gerber RS-274X + Excellon, `.pcb`, `.pcbdoc`, `.cam` — not ODB++**,
which is the current CAM output. That is an export-settings change, but it has to
happen before a quote can be run at all.

## What stays hand-soldered regardless

The user-facing modules are unlikely to be machine-placed either way — the NeoPixel
ring, the LCD1602, the KY-040 encoder and the arcade button all mount to the case, not
the board. They stay as headers, soldered by hand.

This is worth saying plainly because it caps the benefit: **assembly removes the
fiddly small-part soldering, not the module wiring.** The pin-spreading rationale in
[`../README.md`](../README.md) — a free pin between unrelated modules, same-module pins
kept adjacent — still applies to whatever header block survives.

## Open questions

1. **The real R_LOAD value** on the Adafruit MiCS-5524 breakout. Blocks choosing a
   value that keeps the existing gas divider maths valid.
2. **AISLER against PCBWay** — the €193.40 for 3 boards is a real number; PCBWay has
   no comparable quote yet, and getting one needs the CAM output switched off ODB++
   first.
3. **`AGND` handling in whichever Pico symbol is adopted.**
4. **Whether the SCD4x centre keep-out survives** in the SnapMagic footprint.
5. **Is the SCD40 or the SCD41 wanted?** The drivers already handle both. SCD41 has
   the wider range and single-shot mode for about $2 more.
6. **Does anything still supply the I²C bus pull-ups** once the breakouts are gone?
   The LCD may or may not carry its own. If not, a 4.7 k–10 k pair has to appear on
   the board.
