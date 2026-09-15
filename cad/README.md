# CAD models

Third-party STEP models for enclosure design. Run `./fetch.sh` to download them;
the binaries are gitignored, the script is not.

| File | Part | Source |
|---|---|---|
| `pico-w/` | Raspberry Pi Pico W | [Raspberry Pi datasheets](https://datasheets.raspberrypi.com/picow/PicoW-step.zip) — official |
| `neopixel-ring-24.step` | NeoPixel Ring 24 × 5050 | [Adafruit 1586](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/1586%20NeoPixel%20Ring%2024x) |
| `scd40.step` | SCD-40 CO₂ / temp / humidity | [Adafruit 5187](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/5187%20SCD-40%20C02%20Sensor) |
| `permaproto-half.step` | Perma-Proto Half-Size — 81.3 × 50.8 × 1.6 mm | [Adafruit 1609](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/1609%20Perma-Proto%20HalfSize) |
| `permaproto-quarter.step` | Perma-Proto Quarter-Size — 50.8 × 43.2 × 1.6 mm | [Adafruit 1608](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/1608%20perma-proto%20quarter) |
| `minty-permaproto.step` | Perma-Proto Mint Tin size — 91.4 × 56.9 × 1.6 mm | [Adafruit 723](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/723%20Minty%20Perma-Proto) |
| `arcade-button-30mm.step` | Arcade button, 30 mm | [Adafruit 471](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/471%20Arcade%20Button%2030mm) |
| `usb-c-breakout-4090.step` | USB-C breakout — 24.79 × 14.38 × **4.20** mm | [Adafruit 4090](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/4090%20USB%20C%20Breakout) |
| `ds3231-rtc-3013.step` | DS3231 precision RTC — see the note below on its size | [Adafruit 3013](https://github.com/adafruit/Adafruit_CAD_Parts/tree/main/3013%20DS3231%20RTC) |
| `sparkfun-solderable-breadboard-12070.stl` | Solder-able Breadboard — **93.98 × 63.50 × 1.60 mm** | [SparkFun 3D_Models](https://github.com/sparkfun/3D_Models/tree/master/products/12070) — **STL, not STEP** |

### Bare-part models — the fabricated board

Added when the build moved from breakouts to bare parts. Everything above is a
breakout or an off-board item; these are the parts on the PCB itself.

| File | Part | Size (mm) | Source |
|---|---|---|---|
| `jst-xh-b2b.step` | `J1` — JST XH 2-way | 7.40 × 5.75 × **7.00** above board | [KiCad 3D library](https://gitlab.com/kicad/libraries/kicad-packages3D) |
| `jst-xh-b4b.step` | `J2` `J5` — XH 4-way | 12.40 × 5.75 × **7.00** | KiCad |
| `jst-xh-b5b.step` | `J4` `J6` `J7` — XH 5-way | 14.90 × 5.75 × **7.00** | KiCad |
| `soic-16w-ds3231.step` | `U4` — DS3231SN | 10.30 × 10.30 × 2.75 | KiCad |
| `soic-14-sn74hct125.step` | `U5` — SN74HCT125D | 6.00 × 8.70 × 1.75 | KiCad |
| `sot-23.step` | `Q1` `Q2` `Q3` | 2.50 × 3.00 × 1.20 | KiCad |
| `lcsc/scd41.obj` | `U3` — SCD41-D-R2 | 10.10 × 10.10 × **6.32** | LCSC `C3659294` |
| `lcsc/mics-5524.obj` | `U2` — MiCS-5524 | 7.00 × 5.00 × 1.52 | LCSC `C3659348` |
| `lcsc/pir-bs612.obj` | `U6` — BS612 | 9.85 × 9.10 × 9.20 *(incl. leads)* | LCSC `C389589` |
| `lcsc/veml7700.obj` | `U7` — VEML7700 | 6.80 × 2.35 × 3.00 | LCSC `C504893` |
| `lcsc/buzzer-qmb-09b-05.obj` | `BZ1` | Ø9.00 × **5.5** above board | LCSC `C161819` |
| `lcsc/cr1220-holder.obj` | `BT1` | 19.76 × 14.90 × 4.90 | LCSC `C70381` |

### The Fresnel lens — `make-lens.py`

**No public model exists for the `SB-F-02`.** LCSC hold it as `C89924` with no CAD data,
Senba publish only a datasheet, and what turns up on GrabCAD and Sketchfab is generic
Fresnel geometry. [`make-lens.py`](make-lens.py) revolves it from the dimensioned drawing
in [`../datasheets/lens-SB-F-02.pdf`](../datasheets/lens-SB-F-02.pdf), which makes it the
manufacturer's own geometry rather than someone's approximation.

| File | Result |
|---|---|
| `lens-sb-f-02.step` | **Ø25.75 × 16.90 mm**, z −5.40…+11.50, 4352 faces, 0 non-manifold edges, χ = 2 |

Dimensioned values used as-is: dome **R11.5** standing **11.5** above the flange, flange
**Ø25.75 × 1.5** thick, bore **Ø8.10**, overall **16.9**.

**Two numbers are not on the drawing** and were scaled off the side view against the 8.10
bore: the skirt outer diameter (**Ø12.30**) and the bore depth (3.9 mm blind). Nothing else
depends on either — change them at the top of the script if a real part says otherwise.

> ⚠️ **The drawing does not reconcile with itself.** 11.5 above the flange plus 5.6 below
> is 17.1, but the overall is given as 16.9. Both carry ±0.15. The model favours **R11.5 and
> the 11.5 protrusion** — the two the enclosure actually needs — and takes the skirt to
> −5.4 so the overall lands on 16.9 exactly. If you seat the lens off its *underside*
> instead, add the missing 0.2 mm.

The Fresnel groove pattern on the inner surface is not modelled: it is the optical point of
the part and mechanically irrelevant to a housing.

### Generated solids — `make-primitives.py`

Fusion's Electronics library binds a **solid** to a footprint. STEP arrives as one; an
OBJ arrives as a mesh and will not bind. Rather than mesh-convert each part by hand,
[`make-primitives.py`](make-primitives.py) writes exact STEP B-reps for the parts whose
shape is simple enough that a solid is indistinguishable from the real geometry:

| File | Part | Size (mm) | Bodies |
|---|---|---|---|
| `buzzer-qmb-09b-05.step` | `BZ1` | Ø9.044 × 5.50 | 1 |
| `mics-5524.step` | `U2` | 7.000 × 5.000 × 1.52 | 1 |
| `veml7700.step` | `U7` | 6.800 × 2.350 × 3.00 | 1 |
| `pir-bs612.step` | `U6` | Ø9.144 base × 0.50, Ø8.20 can to 4.70 | 2 |
| `scd41.step` | `U3` | 10.10 sq base × 0.88, 7.86 sq body to 6.30 | 2 |
| `cr1220-holder.step` | `BT1` | 19.76 × 14.90 tabs × 0.55, body to 4.05 | 2 |

Cylinders use a **circumscribed** 32-gon, so the model is never smaller than the part —
a clearance body should err large. Everything sits on **z = 0** standing up, so it drops
straight onto the board plane. Dimensions are measured from the LCSC meshes in `lcsc/`
and cross-checked against the datasheets.

The PIR is deliberately two bodies rather than one: the shoulder between the Ø9.10 base
and the Ø8.20 can is what the Fresnel lens skirt keys on, and that relationship is still
unresolved — see the PIR section of [`../docs/pcb-assembly.md`](../docs/pcb-assembly.md).

### Mesh-derived solids — `mesh-to-step.py`

The LCSC meshes are *display* models: the holder had **211 open boundary edges**, so
nothing will treat it as a solid. [`mesh-to-step.py`](mesh-to-step.py) does what Fusion's
Convert Mesh does — welds coincident vertices, chains the open boundaries into loops, fills
them, and emits a `MANIFOLD_SOLID_BREP`:

```
python3 mesh-to-step.py lcsc/cr1220-holder.obj cr1220-holder-detailed.step "CR1220-2"
```

| File | Part | Result |
|---|---|---|
| `scd41-detailed.step` | `U3` | 10.10 × 10.10 × 6.32, 2540 faces, χ = 2 |
| `cr1220-holder-detailed.step` | `BT1` | 19.76 × 14.90 × 4.90, 4418 faces, χ = −2 |

Both come out **watertight — 0 non-manifold edges** — and their bounding boxes match the
source meshes exactly. The Euler characteristics are a sanity check on shape: χ = 2 is a
simple closed body, which the SCD41 is; χ = −2 is genus 2, which the holder is, because the
cell aperture and peg holes pass through it.

This one keeps the spring clips, the cell recess and the locating pegs — everything the
box version throws away. Use it when you want the holder to *look* like a holder; the
primitive is still there if you only need the envelope.

**Most "holes" in these meshes are not holes.** Both models are riddled with
**T-junctions** — an edge divided on one side of the surface but not the other. They present
as boundary loops, but every such loop is *collinear*: a line out and back, enclosing no
area. Fan-filling one produces nothing but zero-area slivers, and dropping those slivers
reopens the boundary, so the naive repair oscillates forever. The fix is to split the long
edge at the vertices sitting on it. The SCD41 turned out to have **no holes at all** — all
90 of its boundary edges were T-junctions.

Genuine holes do exist (the holder had 6 after repair) and get fan-filled, with two further
traps handled: a triangular hole is filled with one triangle rather than a fan, because the
centroid can land on an existing vertex and give an edge shared by four faces; and a
boundary loop can pinch through the same vertex twice, like a figure eight, which is split
before filling.

**The two numbers to trust** are printed at the end: non-manifold edges must be **0**, and
the emitted face count must equal the triangle count. The second matters because a
zero-area triangle cannot become a STEP face — it gets skipped, and a skipped face silently
unseals a shell that the mesh check said was closed.

**The SCD41 and the holder are stepped, not boxed.** Their profiles were sliced out of the
LCSC meshes rather than guessed: the SCD41 has a full-width base plate 0.88 tall with the
sensing body stepping in to 7.86 square above it — **that upper face is the gas-permeable
membrane, and it is what the enclosure vent must sit over.** The holder reaches its full
19.76 mm only at tab level; the plastic steps in above that, and it is **not centred on its
own origin** — the body sits 0.46 mm toward +x. Its locating pegs run 0.85 mm below the
board and are omitted, since the case never sees them.

With these two, **every part on the board now has a solid.** The fiducials are the only
things without one, and they are flat copper — there is nothing to model.


**The KiCad models are STEP solids; the LCSC ones are OBJ meshes.** Fusion imports
both, but a mesh cannot be used as a cutting tool — for a clearance pocket, model a
primitive from the dimensions above and keep the mesh only as a visual check.

**Still missing: the `SB-F-02` Fresnel lens.** It is a mechanical part with no LCSC
entry and no public model. It is also a simple revolve — flange Ø25.75 × 1.5 thick,
dome R11.5 standing 11.5 above the flange, skirt 5.6 below with a Ø8.10 bore, 16.9
overall. See the PIR section of [`../docs/pcb-assembly.md`](../docs/pcb-assembly.md).

**Two heights that surprised us**, both confirmed against the models: the buzzer stands
**5.5 mm**, not the 4.5 the CST-951AP datasheet quotes for its sibling; and the JST
headers are **7.00 mm** before any allowance for cable bend radius. Those two set the
depth of the rear cavity, along with `C1`.

**The quarter-size model is an outline only.** It has 6 cylindrical surfaces against the
half-size model's 426, so the mounting holes are there but the perforated grid is not. Fine for
checking fit and clearance in an enclosure; useless for visualising where parts sit on the grid.
The half-size and mint-tin models are fully detailed.

**Pico WH** is mechanically a Pico W with headers fitted, so the Pico W model is the
right outline — but check whether the headers are modelled before relying on the
height.

## Not found as STEP

| Part | Where to look |
|---|---|
| MiCS-5524 gas sensor | No 3D model, but the board outline is exact: **12.70 × 19.05 mm**, read out of the dimension layer of Adafruit's own [EAGLE .brd](https://github.com/adafruit/Adafruit-MiCS-5524-Gas-Sensor-Breakout-PCB). More authoritative than any community model. |
| Generic SCD40 module (AliExpress) | No model exists. `scd40.step` here is **Adafruit's 5187 board, 30.5 × 27.9 mm — not the same board**. Measure yours. The SCD4x sensor package itself is drawn in Figure 3 (p. 24) of the [Sensirion datasheet](https://sensirion.com/media/documents/48C4B7FB/67FE0194/CD_DS_SCD4x_Datasheet_D1.pdf). |
| DFR0464 LCD1602 RGB | [SnapMagic](https://www.snapeda.com/parts/DFR0464/DFRobot/view-part/) has a 3D model; DFRobot's wiki has dimensions. |
| KY-040 rotary encoder | Generic part — [GrabCAD](https://grabcad.com/library/tag/ky-040) has several. |
| Piezo buzzer | Generic — pick a GrabCAD model matching your disc diameter. |
| RGB digit module | No CAD model. **Measure, do not trust the datasheet** — [`rgbdigit.pdf`](../datasheets/rgbdigit.pdf) is the 1" module and ours is the 0.8". Callipers give **19.9 × 27.6 mm**, 2.54 mm pin pitch, 3-in/3-out. |


## DS3231 RTC 3013 — the model is bigger than the board

Adafruit list the PCB as **25.4 × 17.8 mm**. The STEP's bounding box is
**32.36 × 24.68 × 3.20 mm** — about 7 mm larger in both axes, from features
overhanging the board outline.

Unlike the 4090 there is no stray-geometry artifact here: the Z distribution is
smooth from 0 to 3.20 mm with no isolated cluster. But **3.20 mm cannot include
the CR1220 holder** — the cell alone is 2.0 mm thick and it sits in a holder on
the underside. Treat the model as the board and its topside parts only, and
measure the real thing for the battery clearance.

## USB-C breakout 4090 — the height is not 11.7 mm

Adafruit's own STEP, and a good model — but its raw bounding box reads
24.79 × 14.38 × **11.66** mm, and that last figure is wrong for clearance.

Ten stray points sit at exactly Z = −8.459, in a 0.1 mm sliver of Y, with
nothing at all between them and the rest of the model. A flat artifact, not a
feature. Drop them and the real envelope is:

| | |
|---|---|
| Board + connector | **24.79 × 14.38 × 4.20 mm** |
| PCB plane | Z −1.00 to +0.80 |
| Connector shell | up to Z +3.20 |
| Socket face | X −4.47, protruding past the board edge |

Same failure mode as the arcade button model reaching Z = 228 mm for its wire
harness: **measure a bounding box, then look at what is setting it.**

## The 63.5 mm protoboard — SparkFun 12070

The only stock board found at exactly 63.5 mm wide. Dimensions confirmed twice
over: the dimension layer of SparkFun's own
[Eagle `.brd`](https://github.com/sparkfun/Solder-able_Breadboard) gives
93.980 × 63.500 mm, and the STL's bounding box agrees to 0.005 mm at
1.600 mm thick.

Layout, read out of the same file: **60 nets of five holes** (30 columns × two
banks) plus **four power rails of 29 holes**. That is the same column count as
the Perma-Proto Half — the extra size is board margin, not more strips.

Two things to know before choosing it:

- **No mounting holes.** The `.brd` has no `<hole>` elements at all; it is a
  plain rectangle. The Perma-Proto boards do have them.
- **STL, not STEP** — a mesh, and a 4 MB one. For enclosure clearance work a
  93.98 × 63.5 × 1.6 mm box drawn by hand is a better model than the mesh; keep
  the STL only for checking how it looks.

## Panel cutouts

For the parts where no STEP file exists, these are the numbers that actually
matter — a hole diameter beats a mesh for enclosure work.

| Part | Panel hole | Body | Source |
|---|---|---|---|
| Arcade button AB30L5-B | **Ø 28.0 mm (M28)** | ~33 mm ⌀, ~29 mm deep | [datasheet](../datasheets/arcade-button-AB30L5-B.pdf) |
| KY-040 encoder | **Ø 7 mm** threaded bush | PCB ~26 × 19 mm; knob 15 mm ⌀ × 16.5 mm | Supplier listings — **varies, measure yours** |
| Buzzer UDB-05B | Ø ~9.5 mm if front-mounted | **Ø 9.2 × 9 mm**, 4 mm pin pitch | [datasheet](../datasheets/buzzer-UDB-05B.pdf) |
| RGBDigit **0.8"** | **19.9 × 27.6 mm** per digit, measured | Fusion footprint `RGBDIGIT_08"_6PIN` is 20.32 × 27.94 nominal | ⚠️ [datasheet](../datasheets/rgbdigit.pdf) is the **1"** part — mechanical figures in it do not apply |

**KY-040 dimensions vary between suppliers** — 15 × 16.5, 26 × 19, 30 × 18 and
32 × 19 mm are all listed by different sellers for the "same" module. Measure the
one in your hand rather than trusting any of them.

The Adafruit 471 model is a usable stand-in for the BerryBase arcade button
(34.4 mm body against the datasheet's ~33 mm), but note its Z axis extends to
228 mm because the wire harness is modelled at full length. Use the datasheet's
Ø 28 mm for the panel hole, not a measurement off the model.

## No STEP available

The KY-040 and the buzzer are commodity parts with no manufacturer CAD.
[GrabCAD](https://grabcad.com/library) has community models of both if a solid is
really wanted, though for a cylinder 9.2 mm across it is quicker to draw one.


## Buzzer — UDB-05B

[Datasheet](../datasheets/buzzer-UDB-05B.pdf). It is a **magnetic** transducer, not piezo:

| | |
|---|---|
| Rated voltage | 5.0 V (range 3.0–7.0) |
| Max rated current | **30 mA** |
| Resonant frequency | **2300 ± 400 Hz** |
| Output | 83 dBA at 10 cm, at resonance |

Two consequences, both acted on in `drivers/buzzer.cpp`:

**Melodies must live near 2300 Hz.** A magnetic buzzer away from resonance is
quiet *and* current-hungry — its impedance is mostly inductive reactance, which
falls with frequency, so low notes draw more current to make less noise. The
melodies now sit in A6..F7 (1760–2794 Hz) rather than the 260–1046 Hz they
started in.

**30 mA exceeds the RP2040's 12 mA per-pin rating**, and off-resonance notes
would exceed even that. So the buzzer is driven through the 2N2222A that the
arcade lamp turned out not to need: buzzer between 5 V and the collector, 1 kΩ
from GP9 to the base, emitter to ground. No code change — PWM drives a base fine
— and it runs the buzzer at 5 V where the 83 dBA figure applies.

The full wiring, including this, is in [../README.md](../README.md) and in the
printable wiring sheet.
