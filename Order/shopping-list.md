# ISS-Box — what to buy, and where

As of **2026-09-16**. Prices are what the shop showed that day and will drift.

## Already ordered

| Shop | Order | What | Status |
|---|---|---|---|
| JLCPCB | `W2026091606514059` | 5 boards, all 5 assembled — $376.71 | Paid. Ships **straight to Switzerland**: the redirect to LCSC's Hong Kong warehouse was cancelled in chat (agent Kaihong). Waiting for the *Confirm Parts Placement* email. |
| LCSC | `WM2609160656` | Loose parts, list below | Placed. Ships **on its own**; Chloe at LCSC support was told by email. |
| LCSC | ~~`WM2609160181`~~ | — | Cancelled, replaced by `WM2609160656` |

Combining the LCSC parts with the JLCPCB boards was dropped. JLCPCB will not ship
loose parts with an assembled order, and LCSC advised against the manual merge.

### LCSC `WM2609160656`

Export: [`LCSC__WM2609160656_20260916214838.csv`](LCSC__WM2609160656_20260916214838.csv)

| LCSC | Part | Qty | For |
|---|---|---|---|
| `C89924` | Senba SB-F-02 Fresnel lens | 10 | PIR `U6` — mandatory, and only LCSC has it |
| `C140573` | JST SXH-001T-P0.6 crimp, 28–22 AWG | 200 | every plug |
| `C144404` | JST XHP-5, white | 20 | `J4` encoder, `J6` ring, `J7` debug |
| `C20417874` | HCTL HC-XH-2Y-R-05, **red** 2-pin | 50 | `J1` power |
| `C20417882` | HCTL HC-XH-4Y-L-05, **blue** 4-pin | 20 | `J2` LCD |

## Plugs and cables

| Header | Pins | Goes to | Housing |
|---|---|---|---|
| `J1` | 2 | USB-C breakout | red |
| `J2` | 4 | LCD | blue |
| `J5` | 4 | arcade button | white — 20 already in the drawer |
| `J4` | 5 | encoder | white + coloured heat-shrink |
| `J6` | 5 | NeoPixel ring | white + a different heat-shrink colour |
| `J7` | 5 | debug, bench only | white |

- **The red and blue housings are HCTL clones, not JST.** They normally take genuine
  JST crimps and mate with JST headers. Crimp and plug one before doing the rest.
- **Coloured XH housings on LCSC come in 2- and 4-pin only**, which is why the 5-pin
  plugs are told apart with heat-shrink.
- **Signal cables:** Adafruit `3890` silicone ribbon, 10 × 28 AWG, 1.2 mm over the
  insulation. The crimp takes 0.9–1.9 mm. Peel off 4 or 5 wires per plug.
- **`J1` power: 22 AWG single wire from stock, not ribbon.** All the current comes in
  here, and 22 AWG is the heaviest the crimp takes.
- Board-side crimps per box: 2 + 4 + 4 + 5 + 5 + 5 = **25**.

## Rework for rev A, found at bring-up (2026-10-10)

The full story, both sensors, with the wiring: [`../docs/rev-a-rework.md`](../docs/rev-a-rework.md).

`U7` was fitted as **`VEML7700-TR`, the side-view part**, which senses through its
upright face. The design needs the window facing away from the board, which is the
**top-view `VEML7700-TT`**. Both use the same four pads at 1.27 mm pitch — Vishay's land
patterns differ only in pad length, 1.4 vs 1.6 mm, and ours are 1.8 — so the `-TT`
drops onto the existing pads with no board change.

The parts fitted are also **180° out**, which clamped the I²C bus through their
protection diodes and left every sensor unreachable. Both faults go away with the swap.

| Part | Where | Qty |
|---|---|---|
| Vishay `VEML7700-TT`, top view | DigiKey `751-VEML7700-TTCT-ND`, CHF 1.27 | 5 boards + spares |

Fit with **pin 1 on the left-hand pad**, the one by the silk dot, going to SCL. Check
the body clears `C13`: lying flat it is 3 mm deep and there is about 3.5 mm.

For a rev B: specify `-TT` in the BOM, draw the body outline to one side of the pad row
rather than centred, and add the package to `ROTATION_FIX` in `eda/make-jlc-cpl.py`.

## Later — once the boards arrive and work

Box one is built from parts already in stock. Only buy for more boxes once a board
has been powered up and checked.

### DigiKey CH — the modules

Per box. CHF excluding VAT. DigiKey deliver duty-paid (DDP), so there is nothing to
pay on arrival. **Free delivery from CHF 70, otherwise CHF 23** — two boxes' worth
just clears it.

| Part | DigiKey | CHF | Per box |
|---|---|---|---|
| Raspberry Pi Pico 2 WH (headers fitted) — `SC1634` | `2648-SC1634-ND` | 6.42 | 1 |
| DFRobot LCD1602 RGB — `DFR0464` | `1738-1313-ND` | 9.54 | 1 |
| Adafruit NeoPixel Ring 24 — `1586` | `1528-1099-ND` | 13.59 | 1 |
| Adafruit USB-C breakout — `4090` | `1528-2873-ND` | 2.37 | 1 |
| Adafruit silicone ribbon 10 × 28 AWG, 1 m — `3890` | `1528-3890-ND` | 3.17 | 1 per ~2 boxes |

| Boxes | Parts | Delivery | Total |
|---|---|---|---|
| 1 | 35.09 | 23.00 | **58.09** |
| 2 | 70.18 (two ribbons) | free | **70.18** |

The 26 AWG ribbon, `1528-6182-ND` (CHF 2.01), was out of stock, and its insulation
diameter against the crimp has not been checked.

### BerryBase

| Part | Price | Note |
|---|---|---|
| Arcade button AB30L5-B, illuminated | — | The one the case is drawn for: **28 mm** panel hole |
| "Drehregler / Rotary Encoder mit Breakoutboard" (KY-040) | ~€1.60 | Must be the breakout — the firmware and `J4` expect it |

### Elsewhere

| Part | Where | Per box |
|---|---|---|
| RGBDigit 0.8" | rgbdigit.com — nobody else | 4 |
| CR1220 cell for `BT1` | any local electronics shop — LCSC had none | 1 |
| More lenses, crimps, housings | LCSC again — part numbers above | as needed |

## Ruled out, and why

- **LCSC** sells no arcade buttons, no KY-040 breakouts and no CR1220 cells. Their
  EC11 encoders are bare parts, not breakouts.
- **DigiKey** has no Senba lens. Their Fresnel Factory lenses are a different shape,
  and the case is drawn around the SB-F-02.
- **Adafruit's arcade buttons (`3487`–`3491`)** need a **30 mm** hole; the case has 28 mm.
- **Adafruit `4991` encoder** talks I2C and does not fit `J4`.
