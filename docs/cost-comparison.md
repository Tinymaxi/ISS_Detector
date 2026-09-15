# Breakouts against a fabricated board — what it actually costs

Prices checked **29 August 2026** and will drift. Currency is EUR at **1 EUR = 1.16 USD**;
AISLER quotes net of VAT, Adafruit and Digikey quote before shipping and import. Where a
figure is estimated rather than looked up it says so.

This answers one question: *is keeping the Adafruit breakouts cheaper than having bare SMD
parts placed by an assembly house?* The design side of that move is in
[`pcb-assembly.md`](pcb-assembly.md); this is only the money.

## The short answer

**Yes, but not for the reason it looks like.** The bare parts are *cheaper* than the
breakouts — €35.20 against €66.72 for the three sensors, a saving of €31.53 on every board
built. What makes the fabricated route expensive at low volume is not the parts and not the
board. It is **€120 of one-time part-variance fees**, charged once per order at €7.50 per
distinct component whether you build one board or a hundred.

That reframes the decision. It is not "breakouts are cheap, assembly is expensive." It is
**"assembly has a €132 entry ticket and then undercuts breakouts by €35 a board."**

Break-even is at **four boards**.

## Only the difference is counted

Both routes need the Pico, the LCD, the NeoPixel ring, the encoder, the arcade button, the
RGBDigit chain, the USB-C breakout, the buzzer, `C1` and a CR1220 cell. Those cost the same
either way, so they are left out entirely — including them would inflate both columns by the
same ~€120 and hide the thing being measured.

What differs is the three sensor modules, the board they sit on, and the labour of attaching
them.

## Side A — the breakouts

| Item | Source | Price |
|---|---|---|
| Adafruit 5187 — SCD-40 CO₂ breakout | Adafruit | $44.95 |
| Adafruit 3199 — MiCS5524 gas breakout | Adafruit | $14.95 |
| Adafruit 3013 — DS3231 RTC breakout | Adafruit | $17.50 |
| **Three breakouts** | | **$77.40 — €66.72** |
| Perma-Proto half + quarter | Adafruit 1609 / 1608 | $7.90 — €6.81 |
| `SN74HCT125` level shifter | Digikey | $0.55 — €0.47 |
| Hook-up wire, headers, drawer discretes | *estimated* | €4.00 |
| **Per board** | | **€78.01** |

**No fixed cost, no minimum order, and one is already built.** That last point does most of
the work in this comparison and is easy to skip past: the marginal cost of the box that
currently exists is **zero**. Everything below is priced against building a *second* one.

The three breakouts carry their own LDOs, level shifters, pull-ups and — on the MiCS — the
heater-switching transistor. That circuitry is what you are paying the €31.53 premium for,
and on this design most of it is redundant. Nothing here needs the SCD-40 breakout's 5 V
tolerance; the die takes 3.3 V directly.

## Side B — bare parts, placed by AISLER

### The fee structure

AISLER publishes assembly as three fees on top of the bare board and the components:

| Fee | Rate | Scales with |
|---|---|---|
| Part variance | €7.50 per **distinct** component | order, **not** quantity |
| Placement | €0.04 per placement | quantity |
| Handling | €0.01 per cm² | quantity |

Bare boards are separate: **€12 job fee + €0.067/cm² per board**, 2-layer HASL Budget,
minimum 3.

This model reproduces AISLER's own worked example exactly — a 10 × 8 cm board, 50 placements,
10 distinct parts, quoted at €77.80 for one unit and €355.00 for a hundred. The formula gives
€75.00 + €2.00 + €0.80 = €77.80 and €75.00 + €200.00 + €80.00 = €355.00. Both to the cent, so
the model is trustworthy as far as it goes. **See the caveat at the end, though — an observed
quote does not fit it.**

### This board's numbers

From [`../eda/bom.csv`](../eda/bom.csv): **23 machine placements across 17 MPNs**, of which
`R3` and `R10` are the same `RC0805FR-0710KL`. AISLER charges per distinct part, so that is
**16 distinct parts**, not 17.

Board area is taken as **80 × 60 mm = 48 cm²**. There is no layout yet, so this is an
estimate — but at €0.077/cm² combined it is worth €3.70 a board and cannot move the
conclusion.

| | |
|---|---|
| Part variance — 16 × €7.50 | €120.00 |
| Board job fee | €12.00 |
| **Fixed, per order** | **€132.00** |
| Placement — 23 × €0.04 | €0.92 |
| Handling — 48 × €0.01 | €0.48 |
| Bare board — 48 × €0.067 | €3.22 |
| Components (below) | €38.72 |
| **Per board** | **€43.33** |

The striking line is **€1.40**. That is what it costs AISLER to actually place all 23 parts on
one board. The machine time is nearly free; the setup is the entire cost.

### Components, per board

| Part | MPN | Qty | Unit | Line |
|---|---|---|---|---|
| CO₂ sensor | `SCD40-D-R2` | 1 | $18.66 | $18.66 |
| RTC | `DS3231SN#` | 1 | $14.43 | $14.43 |
| Gas sensor | `MICS-5524` | 1 | $7.74 | $7.74 |
| Coin cell holder | Keystone `2894` | 1 | ~$0.85 | $0.85 |
| Schottky | `SS12` | 1 | ~$0.55 | $0.55 |
| Level shifter | `SN74HCT125DR` | 1 | ~$0.55 | $0.55 |
| Heater FET | `AO3400A` | 1 | ~$0.30 | $0.30 |
| Buzzer BJT | `MMBT2222A` | 1 | ~$0.25 | $0.25 |
| 100 nF 0805 | `GRM21BR71H104KA01L` | 5 | ~$0.10 | $0.50 |
| 10 µF 0805 | `CL21A106KOQNNNG` | 1 | ~$0.18 | $0.18 |
| 0805 resistors | Yageo `RC0805` | 9 | ~$0.10 | $0.90 |
| **Total** | | **23** | | **$44.91 — €38.72** |

Everything below the three sensors totals $3.63 — **8% of the component cost across 20 of the
23 placements.** Chasing better prices on the passives is not worth the afternoon. Prices marked
`~` are estimates; the three sensors are looked up.

## The two curves

| Boards | Fabricated | per board | Breakouts | per board | Difference |
|---:|---:|---:|---:|---:|---:|
| 1 | €175.33 | €175.33 | €78.01 | €78.01 | **+€97.32** |
| 3 | €261.99 | €87.33 | €234.03 | €78.01 | **+€27.97** |
| 5 | €348.66 | €69.73 | €390.04 | €78.01 | **−€41.39** |
| 10 | €565.32 | €56.53 | €780.09 | €78.01 | **−€214.77** |
| 20 | €998.63 | €49.93 | €1560.17 | €78.01 | **−€561.54** |

**Crossover at 3.8 boards.** Below four, breakouts. At or above four, the fabricated board —
and it keeps pulling away, because the breakout line has no economy of scale at all. Adafruit
does not discount three units.

Against the box that already exists, though, the comparison is €175.33 against **€0**. No
arithmetic rescues that.

## What this means

**The naive read was wrong in an interesting direction.** Assembly is not expensive per
board — it is €1.40 per board. It is expensive per *order*. Once that is clear, the levers
change: there is no point optimising placements or shaving board area, and every point in
cutting distinct part count.

**€7.50 per value is a real design pressure.** Merging `R3` and `R10` in the BOM is free money
— they are already the same MPN and only appear as two lines because they were written as two
lines. Beyond that: `R8` is documented as working anywhere from 470 Ω to 4.7 kΩ, so it can
become a 4.7 kΩ and share with `R12`/`R13` — another €7.50. That is €15 saved for two edits
that change nothing electrically, or 11% of the fixed cost.

**Never mark a part customer-supplied.** AISLER charges €15 per self-supplied part against
€7.50 to source it — exactly double, before the hassle of shipping them your buzzer.
[`../eda/README.md`](../eda/README.md) already flags this for `BZ1` and the conclusion holds
for everything else.

**Shipping and tax are not in these numbers and do not fall evenly.** AISLER ships within the
EU; Adafruit and Digikey ship from the US with duty and handling on top, which can add real
money to a $77 order and pushes the crossover *down*, in the fabricated board's favour. Price
your own postcode before treating 3.8 as exact.

## PCBWay

Not quotable yet. [`pcb-assembly.md`](pcb-assembly.md) records why — the CAM output is ODB++
and PCBWay wants Gerber RS-274X plus Excellon, and a quote needs Gerbers, a BOM and a
centroid file that do not exist because there is no layout.

Structurally they price differently: assembly is a per-order engineering and stencil setup
plus a per-joint rate, with no equivalent of the €7.50-per-value fee. That shape should favour
this board, which is part-diverse and placement-light — the exact profile AISLER's model
punishes. It is worth an actual quote once there are Gerbers, and it may well move the
crossover.

Remember the constraint from the design doc: **PCBWay will not assemble boards fabricated
elsewhere.** It is AISLER end to end or PCBWay end to end.

## RESOLVED — the real AISLER quote, 13 September 2026

The layout exists, the BOM was assigned against AISLER's own catalogue, and the quote below is
theirs, valid 24 hours from generation. **This supersedes the modelling above.**

| Boards | Unit | Total |
|---:|---:|---:|
| **1** | €306.36 | **€306.36** |
| **5** | €106.02 | **€530.11** |
| 10 | €78.84 | €788.43 |
| 50 | €53.77 | €2,688.72 |
| 100 | €50.63 | €5,063.10 |
| 500 | €46.37 | €23,183.77 |
| 1000 | *out of stock* | — |

### What it decomposes to

Boards 2–5 cost **€55.94 each** ((530.11 − 306.36) ÷ 4). Working back, the fixed cost of an
order is therefore about **€250**, with roughly **€56** of board and components per unit.

So the published fee model was in the right shape but too optimistic: it predicted €132 fixed
and €43.33 per board. Reality is **€250 and €56**. Against the caveat table above, that lands
between the €200 and €300 rows — the pessimistic half of the range, not the optimistic one.

### What the quote does and does not cover

Not the whole board. Assigned and priced: `U2`–`U5`, `U7`, `Q1`/`Q2`/`Q3`, all sixteen 0805
resistors, all twelve 0805 capacitors, `BZ1`, and `J4`–`J7`. **Excluded:** `BT1`, `FID1`–`FID6`,
`J1`, `J2`, `U1`. **Left unassigned:** `C1` and `DIGIT1`–`DIGIT4`.

That is a different scope from Side A above, which counts only the three sensor breakouts plus
protoboard and level shifter. **The crossover table is therefore not directly comparable any
more** and wants rebuilding like-for-like before it is quoted at anyone.

### Two things AISLER cannot do

- **`U6` (BS612 PIR) is out of stock.** No alternative offered. It would have to be
  self-supplied — which their own pricing punishes at €15 against €7.50 to source.
- **`BT1` has no sourceable SMD CR1220 holder.** Every SMD listing came back unknown
  availability; the only one with stock and a price is **through-hole**. Building here would
  mean changing that footprint.

Neither is a problem at JLCPCB, who stock the BS612 (`C389589`) and the SMD holder
(`C70381`) — the latter being the very drawing this board's land pattern was corrected against.

### What did work, against expectations

Sourcing was **not** the obstacle. AISLER resolved the SCD41, the DS3231, the SN74HCT125, the
VEML7700 and the MiCS-5524 without trouble — the gas sensor at **€4.14** from Amphenol, well
under the $7.74 assumed above. Four MPNs that had never been checked against a distributor
(`R2`, `R14`/`R17`, `R16`/`R18`, `R19`) all resolved on the first try.

Two traps in their catalogue, both of which would have quietly wrecked the number: an NTE
`MMBT2222A` at **€101.50** and a "Partner Stock" `RC0805FR-071KL` at **€96.75**, both sitting
next to €0.02 alternatives with the same part number. Read the price column, not the stock tick.

## The caveat that could overturn all of this

The scouting pass recorded a real AISLER quote: a sandbox project, 38.08 × 61.9 mm, 2-layer,
**one distinct component**, at **€193.40 for 3 assembled boards**.

The published model above predicts about **€30** for that project — €7.50 variance, €12 job
fee, €4.74 of board area, pennies of placement and handling. **The observed number is roughly
six times the modelled one and I cannot reconcile it.** Candidates: the fee schedule has moved
since the 2022 announcement it comes from and no newer one is published; the quote included a
stencil, express dispatch, shipping or VAT; or the sandbox part was an expensive one and its
component cost dominates.

It matters, because the fixed cost is the whole argument:

| If fixed cost is | Crossover at |
|---:|---:|
| €132 (published model) | 3.8 boards |
| €200 | 5.8 boards |
| €300 | 8.7 boards |
| €400 | 11.5 boards |

The direction never changes — the fabricated board always wins eventually, because its per-board
cost is €43 against €78. But "eventually" moves from four boards to twelve.

**Resolving this needs a real quote, and a real quote needs a layout.** Until then treat 3.8 as
the optimistic bound.

## Recommendation

**Build the second box on breakouts if there is going to be a second box soon.** At one or two
units the fabricated board cannot win, the parts are already in the drawer, and the
Perma-Proto build is known to work.

**Draw the board anyway.** The reason to fabricate is not cost at these volumes — it is that a
hand-wired Perma-Proto box is not something you can hand to someone else, and the design work
in [`pcb-assembly.md`](pcb-assembly.md) is most of the way done. Do the layout, run a genuine
AISLER quote against it, and replace the caveat above with a number.

**Then decide.** If the real quote lands near the published model, four boards pays for itself
and that is a plausible number for a thing people ask for copies of. If it lands near €400 of
fixed cost, the breakouts stay.
