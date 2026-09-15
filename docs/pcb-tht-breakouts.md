# The through-hole PCB — bare board, breakouts, hand-soldered

> ⚠️ **Superseded — the board went back to bare SMD.** The schematic that actually exists
> places `U2` (MiCS-5524), `U3` (SCD41-D-R2) and `U4` (DS3231SN#) as bare chips with their own
> support circuitry, not as breakouts on headers. `HDR1`/`HDR2`/`HDR3` were never drawn.
> [`pcb-assembly.md`](pcb-assembly.md) is the current plan; this document is kept for its
> reasoning about what the breakouts were providing, which is what the bare parts now have to
> provide themselves.
>
> Two things this document asserts are **false** for the board being built: the `EN` polarity
> does invert (`Q2` is a low-side FET, so gate high = heater on, and `gas_sensor.cpp` must be
> flipped), and `R12`/`R13` are mandatory rather than redundant.

**Superseded reasoning follows.** AISLER's assembly fee — a fixed
€7.50 per distinct component, per order, regardless of quantity — made the fabricated-and-
assembled route too expensive for one or a few boards. [`pcb-assembly.md`](pcb-assembly.md)
and [`cost-comparison.md`](cost-comparison.md) work through why; that reasoning stands, it
just points somewhere else now.

The new plan: order a **bare PCB only**, no assembly service, and hand-solder everything.
Prices and stock below were checked **2 September 2026** and will drift — confirm live
before ordering.

The three sensors go back to being **breakout boards**, because they already exist in the
parts box:

| Sensor | Breakout | Where it came from |
|---|---|---|
| Gas | Adafruit MiCS-5524 gas sensor breakout | Adafruit 3199 |
| RTC | Adafruit DS3231 Precision RTC breakout | Adafruit 3013 |
| CO₂ | A Chinese SCD4x breakout | AliExpress, unidentified — see below |

That reverses the one move `pcb-assembly.md` made: **the bare `SCD41-D-R2` / `MICS-5524` /
`DS3231SN#` parts, and the support circuitry drawn explicitly to replace what those
breakouts were quietly providing, are both out.** Everything else that document settled —
the gas divider maths, the Pico choice, the level-shifter fix, the arcade-lamp fix — carries
over unchanged, because none of it was about packaging.

## What the breakouts make unnecessary

Confirmed against `pcb-assembly.md`'s own "What the other two breakouts were doing" and
"gas front end" sections — each of these was drawn *because* the sensor was going in bare,
and every one of them is a thing the breakout already carries on its own board:

| Ref | Part | Why it was there | Why it's gone now |
|---|---|---|---|
| `Q2` | AO3400A heater FET | Bare MiCS-5524 has no `EN` pin; something had to switch the heater | The breakout has its own switching transistor behind `EN` |
| `R9` | 82 Ω heater resistor | Sets heater current for the bare die | On the breakout, off-board |
| `R10` | 10 kΩ R_LOAD | Sets the sense-resistor load for the bare die | On the breakout — this is the exact resistor measured at 9.94 kΩ in `pcb-assembly.md` |
| `R11` | 100 kΩ gate pulldown | Held `Q2` off before firmware configured GP27 | No `Q2`, nothing to hold off |
| `BT1` | Keystone 2894 coin cell holder | Bare DS3231 needs somewhere to put the backup cell | Adafruit 3013 has its own CR1220 holder on the board |
| `C5` | 100 nF DS3231 decoupling | Datasheet asks for it "by name" at the bare chip's `VCC` | On the breakout, next to the chip, where it belongs |
| `C4`, `C7` | 100 nF ∥ 10 µF SCD4x decoupling | Sensirion wants both close to the bare die | On the breakout — same argument, someone else already did it |
| `R12`, `R13` | 4.7 kΩ I²C pull-ups | All three breakouts' own pull-ups vanished with them | They're back — each breakout brings its own 10 kΩ, same as the LCD already does |

**The `EN` polarity does *not* invert.** `pcb-assembly.md` flagged that the bare part
would need `src/drivers/gas_sensor.cpp` flipped, because a low-side FET reads the opposite
way from the breakout's own transistor. Since the breakout is staying, that flip never
happens — GP27 keeps meaning exactly what it means today (`EN` high = heater off), and
`gas_sensor.cpp` does not change. This board needs **zero firmware changes**.

## What survives unchanged

None of this was ever about SMD-vs-THT or bare-vs-breakout — it stays exactly as
`pcb-assembly.md` settled it:

- **The gas divider — `R3` (10 kΩ 1%), `R4` (15.4 kΩ 1%), `C2` (100 nF)** — still needed.
  The breakout's `AOUT` still swings referenced to 5 V and still needs scaling into the
  3.3 V ADC on GP26; that has nothing to do with what's inside the breakout. Same maths,
  same measured R_LOAD, same divider.
- **`U5`, the SN74HCT125 level shifter** — still needed. The RGBDigit chain's `VIH` of
  3.5 V was always above what the RP2040 drives at 3.3 V; that is a digit-chain problem,
  not a sensor-packaging one.
- **`Q3`/`R15`, the arcade-lamp fix** — still needed. The lamp is a 5 V-rated LED with its
  own series resistor; driving it from a 3.3 V GPIO starves that resistor regardless of
  what the PCB does anywhere else.
- **`C1` (1000 µF bulk cap), `R1` (470 Ω ring data), `R8`/`R14` (buzzer)** — unchanged.
- **`D1` (1N5817) is dropped.** It OR'd the panel USB-C supply against the Pico's own micro-USB so
  both could be live at once. Once the Pico is mounted in the case that micro-USB is unreachable,
  and the panel supply now lands on **`VSYS` (pin 39)** rather than `VBUS` — where the Pico's *own*
  internal Schottky already provides the same isolation. Reflashing a boxed unit is SWD over a Debug
  Probe, which supplies no power. See [`../README.md`](../README.md) for the power path.
- **Five JST XH cables off-board** — `J1`–`J5`: USB-C breakout, LCD, NeoPixel ring, KY-040
  encoder, arcade button. Same five, same pitch caveat (XH is 2.50 mm, not 2.54).
- **RGBDigits stay on the board**, `C3`/`C8`/`C9`/`C10` decoupling per digit, unchanged.
- **The board-size open question** (one wide board vs. a separate digit board with a
  3-wire link, `J6`) is untouched by this revision — still open, for the same reasons.

## The sensors move to headers, not pads

Each breakout mounts on its own row of pin sockets instead of being placed as a chip. Two
of the three are known parts with documented pinouts; the third is not.

| Breakout | Pins | Pitch | Source |
|---|---|---|---|
| Adafruit 3199 (MiCS-5524) | 4: `VIN`, `GND`, `EN`, `AOUT` | 0.1″ single row | [Adafruit guide](https://learn.adafruit.com/adafruit-mics5524-gas-sensor-breakout) |
| Adafruit 3013 (DS3231) | 7: `VIN`, `GND`, `SDA`, `SCL`, `SQW`, `RST`, `32K` | 0.1″ single row | [Adafruit guide](https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout/pinouts) |
| Chinese SCD4x clone | **Unknown** | **Unknown** | — |

`SQW` gets a wire to GP11 as today; `RST` and `32K` stay unconnected, same as the working
Perma-Proto build. `BAT` is a separate small solder pad on the 3013 for tapping battery
voltage elsewhere — not part of the main header, not used here.

### The CO₂ breakout is the open item — measure before laying out

[`../cad/README.md`](../cad/README.md) already says it plainly: *"Generic SCD40 module
(AliExpress). No model exists... Measure yours."* Nothing else in this repo documents
pin order, pitch, or outline for it — [`wiring.html`](wiring.html) only warns that "some
AliExpress carriers put a 5 V→3.3 V regulator in front" of the sensor, which is a second
unknown (does this one need 5 V or 3.3 V at `VIN`?) on top of the pinout itself.

**This gates the layout.** Before the CO₂ header footprint can be drawn, measure the actual
board in hand for:

1. Pin count and labels (expect `VCC`/`VIN`, `GND`, `SDA`, `SCL` at minimum; some clones
   add a `PWR`/enable pin)
2. Pin pitch — 0.1″ (2.54 mm) pin header is common, but some SCD4x breakouts use a 2.0 mm
   JST-SH (Qwiic/StemmaQC-style) connector instead of a pin header entirely, which would
   change the connector choice on this board, not just the footprint
3. Row spacing and board outline, for the panel/enclosure clearance
4. Whether there's a small 3-pin regulator IC next to the input pins (settles the 5 V vs.
   3.3 V question directly, per the `wiring.html` warning)

Everything else in this document can be built without this; only the CO₂ header's exact
footprint waits on it.

## Female headers, or solder flat?

**Default assumption below: female headers, so all three breakouts (and the Pico) stay
removable.** This matches the standing decision on the Pico itself — `pcb-assembly.md`
already chose a socketed Pico over a soldered one because a five-year-old's box is worth
more with a replaceable brain, and the same logic extends to the breakouts.

If soldering flat instead:

- Drop the female header strips for that module from the BOM; the breakout's own header
  pins (or bare through-holes, if unpopulated) go straight through the board instead.
- The module is no longer removable without desoldering — harder rework, but one fewer
  mechanical connection and a lower-profile board.
- Fit order matters more: a flat-soldered breakout has to go in before anything that would
  block access to its pads, where a socketed one can be the very last thing placed.

This is a question for him, not a default to build around — everything below assumes
sockets until told otherwise.

## The BOM

See [`../eda/bom-tht.csv`](../eda/bom-tht.csv) for the full sheet. Digikey part numbers,
stock and unit price below — **checked 2 September 2026, confirm live before ordering.**

### Actives

| Ref | Part | Package | Digikey P/N | MPN | Stock | Price |
|---|---|---|---|---|---|---|
| `U1` | Raspberry Pi Pico 2 WH | 40+3 THT pads, socketed | *(existing — see `pcb-assembly.md`)* | `SC1634` | — | — |
| `U5` | SN74HCT125N level shifter | PDIP-14, socketed | [296-8386-5-ND](https://www.digikey.com/en/products/detail/texas-instruments/SN74HCT125N/376860) | `SN74HCT125N` | In stock | ~$0.76–0.92 |
| — | 14-pin DIP socket, 0.3″ | THT | `ED3045-5-ND` | `ED14DT` (On Shore Tech.) | In stock | ~$0.52 |
| `Q1` | 2N2222A, buzzer driver | TO-92 | `4878-2N2222ACT-ND` | `2N2222A` (Diotec) | In stock | ~$0.18 |
| `Q3` | 2N2222A, arcade lamp driver | TO-92 | `4878-2N2222ACT-ND` | `2N2222A` (Diotec) | In stock | ~$0.18 |

Both `Q1` and `Q3` are the same TO-92 2N2222A — order qty 2 of one line.

### Passives

| Ref | Value | Package | Digikey P/N | MPN | Notes |
|---|---|---|---|---|---|
| `R1` | 470 Ω | Axial THT | `CF14JT470RCT-ND` | Stackpole `CF14JT470R` | Ring data |
| `R3` | 10 kΩ 1% | Axial THT | `10.0KXBK-ND` | Yageo `MFR-25FBF52-10K` | Divider upper leg |
| `R4` | 15.4 kΩ 1% | Axial THT | `15.4KXBK-ND` | Yageo `MFR-25FBF52-15K4` | Divider lower leg — exact E96 value, in stock as a single leaded part |
| `R8` | 1 kΩ | Axial THT | `CF14JT1K00CT-ND` | Stackpole `CF14JT1K00` | `Q1` base |
| `R14` | 0 Ω | Axial THT | `CD14ZT0R00CT-ND` | Stackpole `CD14ZT0R00` | Buzzer volume trim link |
| `R15` | 1 kΩ | Axial THT | `CF14JT1K00CT-ND` | Stackpole `CF14JT1K00` | `Q3` base — same line as `R8`, qty 2 |
| `C1` | 1000 µF 16 V | Radial THT | `565-EKYB160ELL102MJ16S-ND` | United Chemi-Con `EKYB160ELL102MJ16S` | Bulk cap, 5 V rail |
| `C2`, `C3`, `C8`, `C9`, `C10` | 100 nF 50 V | Radial/leaded THT, 2.5 mm pitch | `BC3324-ND` | Vishay `K104K15X7RF53L2` | ADC settling + one per RGBDigit — qty 5 |

### Connectors

| Ref | Part | Digikey P/N | MPN | Notes |
|---|---|---|---|---|
| `J1` | JST XH 2-pin | `455-B2B-XH-A-ND` | `B2B-XH-A` | USB-C breakout |
| `J2`, `J5` | JST XH 4-pin | `455-B4B-XH-A-ND` | `B4B-XH-A` | LCD / arcade button, qty 2 |
| `J3` (`J6`) | JST XH 3-pin | `455-2248-ND` | `B3B-XH-A` | NeoPixel ring (+ digit-chain link, if the split-board option is ever taken) |
| `J4` | JST XH 5-pin | `455-B5B-XH-A-ND` | `B5B-XH-A` | KY-040 encoder |
| `J7` | 1×3 debug header, male | `S1012EC-40-ND` (cut to 3) | Sullins `PREC040SAAN-RC` | 40-pos breakaway strip, cut down |
| `HDR1` | 1×4 female socket, gas breakout | `S7069-ND` (cut to 4) | Sullins `PPPC361LFBN-RC` | 36-pos breakaway strip, cut down |
| `HDR2` | 1×7 female socket, RTC breakout | `S7069-ND` (cut to 7) | Sullins `PPPC361LFBN-RC` | Same strip as `HDR1` |
| `HDR3` | female socket, CO₂ breakout | **TBD** | **TBD** | Blocked on the pin-count/pitch measurement above |
| — | Pico socket, 2× 1×20 female | `S7069-ND` × 2 (cut to 20 each) | Sullins `PPPC361LFBN-RC` | Buy 2 strips of 36 to cover both Pico rows plus `HDR1`/`HDR2` with margin |

One female-header line (`S7069-ND`, 36-position, cut to length) covers the Pico and both
known breakouts — buy 2–3 strips depending on whether the CO₂ header turns out to need more
than a spare handful of positions.

### Not in the BOM, needed to build the box

Same as the bare-SMD plan: **JST XH housings** (`XHP-2/3/4/5`) and crimp terminals
(`SXH-001T-P0.6`) for the cable side, plus a **CR1220 cell** — except the cell now goes in
the DS3231 breakout's own holder, not a board-mounted `BT1`.

## Fab

**Bare PCB only — no assembly service.** This sidesteps every AISLER/PCBWay assembly
constraint `pcb-assembly.md` worked through (part-variance fees, reflow atmosphere for the
MiCS-5524, ODB++ vs. Gerber). What's still needed from that document: the **JST XH pitch
caveat** (2.50 mm, not 2.54), and the **board-size question**, both unaffected by dropping
assembly.

Get a genuine bare-board quote once there's a layout — AISLER's **€12 job fee +
€0.067/cm²** and PCBWay's Gerber-only, 5-piece-minimum bare-board pricing (from
`cost-comparison.md` and `pcb-assembly.md`) both apply without the per-component fee that
made assembly expensive.

## Open questions

1. **The CO₂ breakout's pinout, pitch and outline** — needs the physical board measured
   before `HDR3` can be drawn. See above.
2. **Female headers or solder-flat** — asked, not yet answered. Everything above assumes
   sockets.
3. **Board-size** (one wide board vs. a split digit board) — carried over from
   `pcb-assembly.md`, still open, unaffected by this revision.
4. **Fab-only quote** — not yet run; needs a layout first, same as before.
