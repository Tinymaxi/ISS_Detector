# EDA

Fusion Electronics work for the board. The design rationale lives in
[`../docs/pcb-assembly.md`](../docs/pcb-assembly.md) (current plan — bare SMD sensor parts) and
[`../docs/pcb-tht-breakouts.md`](../docs/pcb-tht-breakouts.md) (superseded — bare board,
hand-soldered, sensors as breakouts; kept for the reasoning). This folder holds the files.

| Path | What |
|---|---|
| `libraries/` | Hand-checked `.lbr` files for parts not in Fusion's built-ins. The `MICS-5524`, `DS3231SN` and `SCD41-D-R2` device libraries **are** used — those three go in as placed chips |
| `bom.csv` | **Current BOM** — bare SMD, generated against the schematic |
| `bom-tht.csv` | Superseded BOM for the breakout/hand-soldered plan — kept for reference |

**The schematic and board live in Fusion, not in this folder.** They are held in Fusion's own
cloud/local store rather than committed here, so this directory does not contain anything a fab
can be sent. As of the last check the schematic is complete and the board is partly placed and
partly routed — `GND` is entirely unrouted and there are no copper pours yet.

## `bom.csv`

**Regenerated against the schematic**, so the designators and quantities match what is actually
drawn rather than what was planned. `U6`/`U7` (PIR and ambient light) and their support parts are
included; `D1` is gone; `J3` does not exist and `J6` is a 5-pin part.

Several MPNs are **blank and must be filled in** before ordering: `C1`, `R2` (100 Ω), `R14`/`R17`
(0 Ω), `R16`/`R18` (1 MΩ), `R19` (150 kΩ), and the RGBDigit modules. The buzzer is worse than
blank — the schematic calls it `QMB-09B-05`, the prose has said `UDB-05B`, and `datasheets/` holds
`CST-951AP`. Settle which part is in the box before buying anything.

Columns are `Designator, Quantity, Value, MPN, Manufacturer, Package, Placement, Notes`.
`MPN` is the column name AISLER's importer looks for — it also accepts `Part Number`,
`PartNumber`, `Comment` and `VALUE`, and can Smart Match generic `R`/`C`/`D`/`LED`
designators from value alone. `Placement` and `Notes` are ours; importers ignore
columns they do not recognise.

Quantities are **per board**. Multiply before ordering.

### Designator gaps are intentional

`R5`–`R7` are gone because the old four-in-series divider chain collapsed into a single
15.4 kΩ `R4`. `D1` and `J3` were removed outright — the Schottky because the panel supply now
feeds `VSYS` directly, `J3` because the ring moved to the 5-pin `J6`. **`R2` is back in use**,
as the PIR's 100 Ω supply filter; it is not the old digit series resistor, which stays deleted
because the RGBDigit already carries 330 Ω in its `Di` line. These are explained in
[`../docs/pcb-assembly.md`](../docs/pcb-assembly.md). Nothing is missing.

**`C7` is a designator introduced here.** The design doc records the SCD4x decoupling as
"`C4` — 100 nF ∥ 10 µF", which is two physical parts under one reference. They are split
into `C4` (100 nF) and `C7` (10 µF) so each gets its own line. Rename if the schematic
numbers them differently.

### Three lines to check before ordering

**`U1` — `SC1634` is the Pico 2 W *with headers* (Pico 2 WH).** Chosen because the
existing build uses a WH and because pre-fitted headers suit a socketed, replaceable
Pico. If the Pico should instead solder flat to the board, the bare Pico 2 W is a
different SKU — confirm which before buying.

**`C1` — the schematic says 16 V, and it has no MPN.** Earlier notes shopped a 1000 µF
**6.3 V** radial (`EEU-FR0J102`, out of stock; `EEU-FM0J102` the substitute). Decide which
voltage rating is wanted and pick a part that matches the `E5-10,5` footprint — the 5 mm-pitch,
10 mm-can radial the board is drawn for.

**`BZ1` — the part itself is unsettled, not just the manufacturer.** Three names are in play:
`QMB-09B-05` in the schematic, `UDB-05B` in the prose, `CST-951AP` in `datasheets/`. There is
already a buzzer in the parts box; measure it and make the schematic, the BOM and the datasheet
agree before ordering.

### Not in the CSV, but needed to build the box

Board BOM only. The cable side needs, per connector: **JST XH housings** (`XHP-2`,
`XHP-4`, `XHP-5` — no `XHP-3`, since `J3` is gone) and **crimp terminals**
(`SXH-001T-P0.6`), plus a **CR1220 cell** for `BT1` and the **`SB-F-02` Fresnel lens** for the
PIR, which no assembler will fit. None of these mount to the board, so they are left out to keep
the file clean for the importer.

## Provenance

MPNs are taken from the design doc where it names them, and otherwise chosen here from
distributor listings. **They have been confirmed as listings, not by checking pad
geometry or stock.** Yageo reel suffixes in particular vary (`-07` vs `-13`) — paste
each into a distributor and take whichever is actually stocked.
