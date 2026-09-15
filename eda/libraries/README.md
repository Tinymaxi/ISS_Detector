# EDA libraries

EAGLE `.lbr` files — symbol, footprint and device for parts that are not in Fusion's
built-in libraries. **Fusion Electronics is EAGLE underneath**, so `.lbr` imports
directly: Library Manager → import, then mark the library in use.

Committed rather than gitignored, for the same reason as `../../datasheets/`: these
are small, hand-picked, and the download links rot.

| File | Part | Package | Checked against |
|---|---|---|---|
| `MICS-5524.lbr` | SGX MiCS-5524 gas sensor | `MICS-SMD10` — 10 pads, 5 × 7 mm | [`mics-5524.pdf`](../../datasheets/mics-5524.pdf) p. 4 |
| `AO3400A.lbr` | Alpha & Omega AO3400A N-channel MOSFET | `SOT95P280X125-3N` — SOT-23 | — |
| `DS3231SN.lbr` | Analog Devices DS3231SN# RTC | `SOIC127P1032X265-16N` — SO wide | [`ds3231.pdf`](../../datasheets/ds3231.pdf) p. 18 |
| `SCD41-D-R2.lbr` | Sensirion SCD41 CO₂ sensor | `XDCR_SCD41-D-R2` — 21-pad LGA | [`scd4x.pdf`](../../datasheets/scd4x.pdf) §2.3, §4.2 |
| `MMBT2222A.lbr` | NPN, buzzer driver | `SOT95P240X120-3N` — SOT-23 | pads B=1, E=2, C=3 |
| `CST-951AP.lbr` | Same Sky magnetic buzzer | `CUI_CST-951AP` — THT, 4.0 mm pitch | [`buzzer-CST-951AP.pdf`](../../datasheets/buzzer-CST-951AP.pdf) |
| `CR1220-2.lbr` | Q&J CR1220-2 coin-cell holder | `BAT-SMD_CR1220-2` — SMD, asymmetric tabs | Manufacturer drawing `BS-12-B3AA004` (LCSC `C70381`) |
| `QMB-09B-05.lbr` | Same Sky QMB-09B-05 magnetic transducer | `BUZ-TH_BD9.0-P4.00` — THT, 4.0 mm pitch | EasyEDA library for LCSC `C161819` |
| `BS612.lbr` | Senba BS612 PIR sensor | `SENSORM-TH_BS612` — THT, TO-5 6-pin | EasyEDA `C389589` + [`pir-BS612.pdf`](../../datasheets/pir-BS612.pdf) p.8 |
| `VEML7700.lbr` | Vishay VEML7700-TR ambient light | `LED-SMD_4P-L6.8-W2.4` — SMD, 1.27 mm pitch | EasyEDA library for LCSC `C504893` |
| `FIDUCIAL.lbr` | Pick-and-place fiducial | `FIDUCIAL_1MM` — 1 mm copper, 2 mm mask | JLCPCB PCBA capabilities |

## What was verified

**`AO3400A`** — pad mapping is **G = 1, S = 2, D = 3**, confirmed from the `<connect>`
elements in the library itself. That is the standard SOT-23 MOSFET order and it is what
the buzzer's `Q1` footprint will *not* share, since SOT-23 BJTs run E-B-C. Two SOT-23
parts on this board, two different pinouts — worth a second look at both.

**`MICS-5524`** — the symbol exposes all ten pads and names the four live ones exactly as
the datasheet's connection table does: `C/RH1`, `D/RS1`, `F/RH2`, `G/RS2`. The other six
(`A`, `B`, `E`, `H`, `J`, `K`) are genuinely unconnected on this part — unlike the
DS3231, whose N.C. pins must be grounded.

**Pin directions were added here**, since the download shipped with none set and EAGLE
defaults those to `io` — the noisiest case for ERC. The six dead pins are now `nc` and
the four live ones `pas`. That inverts ERC usefully: it stays quiet about them being
unconnected, and warns if anything is ever *connected* to one, which is the mistake
worth catching.

> **Note:** this file is the archived original. Once imported, Fusion keeps its own
> copy, and that copy is what the design actually uses. Edits made in Fusion's library
> editor do not flow back here — if the two drift, Fusion is authoritative.

**`DS3231SN`** — from SnapEDA, Analog Devices listing. The footprint is the **wide body**,
which is the thing worth checking on this part: pad rows sit **9.31 mm** apart
centre-to-centre and the name encodes a 10.32 mm lead span. A narrow SOIC-16 would read
~5.4 mm and `600`. The part is wide because it contains a quartz tuning-fork crystal, so
a slim-looking footprint is always the wrong one.

The eight mapped pins match p. 18's numbering exactly (`SQW/!INT` is SnapEDA's name for
the datasheet's `INT/SQW`, same pin 3).

> **This library shipped incomplete.** The symbol has 8 pins and **pads 5–12 were mapped
> to nothing at all** — not bundled, simply absent. As downloaded, the eight `N.C.` pads
> would have become unconnected copper islands, which the datasheet forbids: *"N.C. Must
> be connected to ground"*, and *"SUBSTRATE CONNECTED TO GROUND"*. Fixed by appending
> pads 5–12 to the `GND` connection in the device, so grounding pin 13 grounds all nine.

**`SCD41-D-R2`** — the one that arrived correct, and visibly built from Figure 4 rather
than generated. Every pad carries `cream="no" stop="no"` with hand-drawn paste (layer 31)
and mask (layer 29) geometry, exactly as the datasheet's three separate drawings require.

The detail worth having checked: the 4.8 × 4.8 mm centre pad's **paste is split into four
1.8 mm quadrants**, leaving a 0.7 mm clear cross through the middle. That comfortably
contains the **Ø 0.6 mm solder-and-flux keep-free area** Sensirion says "must be
respected". A generated footprint would have pasted the whole pad.

All **21 pads** are mapped, and every one matches Table 6 of the datasheet: `VDD`→7,
`VDDH`→19, `GND`→6/20/**21**, `SCL`→9, `SDA`→10, and `DNC`→1–5, 8, 11–18.

> **The Ultra Librarian version of this part is wrong — do not use it.** Two faults,
> both found by comparison:
>
> 1. Its centre-pad paste is a 3 × 3 grid whose middle square spans x[−0.70, 0.70],
>    y[−0.70, 0.70] — directly over the Ø 0.6 mm keep-free area. Every sampled point in
>    that zone falls under paste.
> 2. It maps pad 21 to a separate `EPAD` pin rather than to `GND`. Table 6 labels the
>    centre pads `21 GND`.
>
> This is the concrete case for the rule below: two libraries for the same part, one
> correct, one not, and nothing but the datasheet to tell them apart.

**`CR1220-2`** — written here rather than downloaded, because nothing on SnapMagic
matches a part JLCPCB actually stock. It replaces the **Keystone 2894**, whose footprint
has a **centre pad under the body** — unreachable with an iron, and out of stock at
JLCPCB anyway, so neither of us could fit it.

Dimensions were originally transcribed from **EasyEDA's own library for `C70381`**. They
have since been **corrected against the manufacturer's drawing** — Q&J `BS-12-B3AA004`,
now in [`../../datasheets/C70381.pdf`](../../datasheets/C70381.pdf), whose *PCB Layout
Diagram (Top View)* is the authority:

| | EasyEDA `C70381` | QJ drawing | **CR1220-2 (in use)** |
|---|---|---|---|
| Pad 1 (−) | −8.60, 4.50 × 2.30 | −8.15, 4.04 × 2.30 | **−8.49, 4.72 × 2.30** |
| Pad 2 (+) | +8.60, 4.50 × 2.30 | +7.815, 4.35 × 2.30 | **+8.245, 5.21 × 2.30** |
| Locating holes | Ø1.20 at −3.75, Ø1.00 at +3.75 | both Ø1.20 at ±3.75 | **both Ø1.20 at ±3.75** |
| Polarity | *not stated — pins are named `1` and `2`* | `−` left, `+` right | **`−` = pad 1, `+` = pad 2** |
| Body | 19.76 × 14.90 mm | 19.46 mm long | 19.76 × 14.90 (EasyEDA) |

The pads are a **deliberate superset** of the two sources. They disagree by 0.4–0.8 mm,
the drawing is a scan and the EasyEDA pattern is what JLCPCB's process actually runs
against, so rather than pick a winner the pad spans the union — the tab lands fully on
copper whichever is right. On an isolated 5 mm pad with nothing adjacent to bridge to,
extra copper costs nothing.

> ⚠️ **EasyEDA carries no polarity for this part at all.** Its symbol pins are named `1`
> and `2` — checked against the live API, not from memory. The `+` on pad 1 in the first
> version of this library was *inferred* from the drawn battery glyph, and it was wrong.
> The QJ drawing marks the land pattern explicitly in red: **`−` on the left pad, `+` on
> the right.** That is the only actual statement of polarity from either source, so it
> wins. It is not cosmetic — `VBAT` on the DS3231 is rated −0.3 to +6.0 V, and a reversed
> cell sits outside that.
>
> **The holes do not polarise the part.** An earlier note here said they did. EasyEDA
> draws them Ø1.20 and Ø1.00; the manufacturer specifies **both Ø1.20**. The library now
> uses 1.20 for both — the safe choice, since the larger locating peg is itself Ø1.00 and
> would interfere with a Ø1.00 hole. Orientation on the finished board comes from the
> **tab widths**: narrower is negative.

Losing the centre pad is not just a footprint change: the net that ran to `N` has to move
onto one of the two clips, so **`BT1` must be deleted and re-placed in the schematic**,
not swapped in the board.

> **How to push a library edit into Fusion.** Do *not* re-import from disk — Fusion refuses
> to reuse the name and offers only "Remove from view", which orphans the old copy. Instead,
> in the **Data Panel**, click the ▾ on the library item and choose **Import New Change**,
> then point it at the `.lbr`. That uploads a new version into the same cloud item, so the
> library keeps its name and identity.
>
> That updates the *library*. It still does not reach parts already placed — the schematic
> embeds its own copy of a device at placement time. Delete and re-place the part, then
> confirm by reading the embedded package back out of the `.fsch`.

**`QMB-09B-05`** — the in-stock buzzer. JLCPCB *do* list the CST-951AP (`C22359731`), but
they hold **zero** of it: pre-order only, minimum 12, $0.74 each. The QMB-09B-05 is the
same manufacturer, 36,548 in stock at $0.15.

The land pattern is the decisive part, and it needs no board work: JLCPCB build both
parts on the **same package**, `BUZ-TH_BD9.0-P4.00-D0.6-FD` — pads at ±2.00 mm, body
Ø9.0 mm, identical to what `CST-951AP.lbr` already draws.

> **The one real difference is the drill: 1.0 mm, not our 0.8 mm.** A 0.8 mm hole leaves
> 0.1 mm of radial clearance on a 0.6 mm lead. That is fine when a human aims the part,
> and tight when a machine inserts it and solder has to wick the barrel. JLCPCB drill
> theirs at 1.0 mm, so match them. This applies whichever of the two buzzers we end up
> with — it is worth fixing in `CST-951AP.lbr` too if that part ever comes back in stock.

Everything electrical moves the right way, given we had already decided the CST-951AP was
louder than we wanted:

| | CST-951AP | QMB-09B-05 |
|---|---|---|
| SPL | 85–92 dBA | **80 dB** |
| Current | 80 mA | **60 mA** |
| Frequency | 2.73 kHz | 2.7 kHz |
| Supply | 4–6 V, 5 V rated | 3–7 V, 5 V rated |
| Height | 4.5 mm | **5.7 mm** |

Same resonance, so `R14`'s volume trim behaves the same. Both are passive/externally
driven, so the PWM drive and `Q2` are unchanged. The 1.2 mm extra height is the only
thing to check against the case.

**`BS612` and `VEML7700`** — both written here, both transcribed from EasyEDA's libraries
for the LCSC part numbers, so the land patterns are the ones JLCPCB build against.

The BS612's pin numbering was the part worth being careful about. EasyEDA's pad records
carry the number in a different field for `ELLIPSE` pads than for `RECT` ones, and a first
read of the package came back with **six unnumbered pads** — six holes on a TO-5 circle with
two 90° gaps and no way to tell which was pin 1. Guessing would have been a dead board.
Reading the raw records gave the mapping, and it agrees with p.8 of the datasheet:

| Pin | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| | `SENS` | `OEN` | `VSS` | `VDD` | `REL` | `ONTIME` |

**The VEML7700 has `SCL` on pin 1 and `SDA` on pin 4 — opposite ends of the part.** Worth
knowing before routing, since the obvious placement puts one of them the long way round.

Neither part's rules live only in the library. `pcb-assembly.md` carries the BS612's ground
island, the mandatory 100 nF, the lens, and the two order remarks (no washing, hand-solder
≤350 °C/≤3 s); the same file carries why the light sensor may sit on the digit face after
all, provided it gets a light-tight collar.

**`FIDUCIAL`** — written because Fusion's managed libraries have none. `holes.lbr` covers
mounting holes and nothing else, and JLCPCB list fiducials as *Necessary* for Standard PCBA.

1 mm bare copper, 2 mm mask opening, **no paste** (`cream="no"` — a pasted fiducial is a
solder blob and the camera cannot find it), and a 3 mm copper keep-out so a ground pour
cannot creep up to the mark.

> **The placement rule matters more than the pad.** Three per board, near the corners, and
> **asymmetrically arranged**. Three fiducials in a symmetric pattern are worse than none:
> they let the machine align a mirrored or rotated board and be confident about it.

The symbol has **no pins**, so a fiducial never joins a net and never trips ERC. It does
still appear as a part, so `FID1`–`FID3` want excluding from the BOM and CPL, the same way
`DIGIT1`–`4` are — there is nothing for a machine to place. The device carries a
`BOM=EXCLUDE` attribute to make that decision visible in the file rather than only in the
generator script.

## The one thing still to check

The library draws the top and bottom pad rows at **1.25 mm** pitch. The datasheet
dimension is **1.27 mm** (`1.27 × 4 = 5.08`). A 0.02 mm error against a 0.6 mm pad is
harmless in itself, but it means the footprint was redrawn rather than lifted from the
drawing — so overlay it against the package outline on p. 4 once before routing, rather
than trusting it wholesale. The overall envelope does check out: side pads at x = ±3.5
give the 7 mm axis, end pads at y = ±2.54 give the 5 mm axis.

This is the check [`../../docs/pcb-assembly.md`](../../docs/pcb-assembly.md) asks for by
name — vendor- and community-submitted footprints are "occasionally stale", and thirty
seconds against the datasheet drawing is worth it.
