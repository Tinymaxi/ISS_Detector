#!/usr/bin/env python3
"""Turn Fusion's Pick-and-Place export into a JLCPCB CPL.

Fusion Electronics has all the placement data, but not in a shape JLCPCB accept:
it writes **two** files (front and back) with no header row, and no Layer column
at all — the side is carried only by the filename. This script merges them, adds
the header and the Layer column, drops the things that are not parts, and applies
the two transformations that were previously done by hand in a spreadsheet.

Run it after every CAM export. Nothing here should ever be typed by hand again.

    python3 eda/make-jlc-cpl.py "~/Desktop/Tinymaxi/ISS Detector CO2 Alarm/CAMOutputs/Assembly"

The two hand-edits it replaces
------------------------------

**1. U1 is one part in the board and two parts on the bench.** The board carries a
single `U1` with the Pico's 40-hole footprint. What actually gets soldered is two
1x20 female socket strips (LCSC C429947) that the Pico plugs into afterwards — the
module itself is never on the assembly. A CPL has one row per placed part, so U1
has to become U1A and U1B, one per pad row, offset half the Pico's 0.7" row pitch
either side of the footprint centre. See `split_pico()`.

**2. Rotation corrections.** There is no universal zero-rotation convention. JLCPCB
align each part against *their* footprint for the LCSC code, which is not always
how ours is drawn, so some parts arrive 90 or 180 degrees out. The corrections in
`ROTATION_FIX` were established by reading JLCPCB's own assembly preview and are
keyed by package, because they are a property of the part rather than the instance
— which is why all three SOT-23s share one entry, and both SOIC parts share the
270 that JLCPCB's own tooling applies to that family.

**Two entries in that table are unverified** and are flagged at the end of every
run. Check them in the assembly preview before paying for anything.

The bottom-layer rule
---------------------

Fusion reports bottom-side parts in top-side view. JLCPCB want them as seen from
the bottom, so every bottom part gains 180 degrees before its per-package
correction is applied. This reproduces the file that was previously uploaded for
40 of 46 parts, with the six exceptions all accounted for in `ROTATION_FIX` — which
is the evidence that the rule is right rather than a coincidence.
"""

import argparse
import csv
import math
import re
import sys
from pathlib import Path

# Half the Pico's row-to-row pitch: 0.7 inch between the two pad rows, so each
# socket strip sits 0.35 inch from the footprint centre.
PICO_ROW_HALF_PITCH_MM = 8.89

# The part in the board whose footprint is the Pico module, and the two socket
# designators it becomes. Both halves must also appear in the BOM against C429947.
PICO_DESIGNATOR = "U1"
PICO_HALVES = ("U1A", "U1B")

# Degrees added to a part after the bottom-layer rule, keyed by package name as
# Fusion writes it into the export. Anything absent gets no correction.
ROTATION_FIX = {
    "SOT95P240X120-3N": 180,             # MMBT2222A  — Q1, Q3
    "SOT95P280X125-3N": 180,             # AO3400A    — Q2
    "MICS-SMD10": 180,                   # MiCS-5524  — U2
    "B4B-XH-A": 180,                     # JST XH 4-pin — J2, J5
    "JST_B5B-XH-A": 180,                 # JST XH 5-pin — J4, J6, J7
    "JST-XH-TRHU-2-PIN-FOOTPRINT": 270,  # JST XH 2-pin — J1
    "SOIC127P1032X265-16N": 90,          # DS3231 SOIC-16W — U4
    "SOIC127P600X175-14N": 90,           # SN74HCT125 SO-14 — U5
    "XDCR_SCD41-D-R2": 180,              # SCD41 — U3
}

# Corrections changed or added since the last order preview, so not yet confirmed
# against JLCPCB's assembly viewer. Printed as a warning after every run.
UNVERIFIED = {
    "SOIC127P1032X265-16N": "U4 DS3231: 270 -> 90. Pin-1 dot was diagonally opposite the silk dot.",
    "SOIC127P600X175-14N": "U5 SN74HCT125: 270 -> 90. Same fault as U4.",
    "JST_B5B-XH-A": "J4/J6/J7: +180 added — they had no correction while the 4-pin JSTs did.",
    "JST-XH-TRHU-2-PIN-FOOTPRINT": "J1: 90 -> 270. A 180 error here reverses VBUS and GND.",
}

# Confirmed correct in JLCPCB's assembly preview on 2026-09-15, both sides:
# BT1 (no correction), U3 (+180), J2/J5 (+180), Q1/Q2/Q3 (+180), U2 (+180),
# U6, U7 and every 0805. Do not change these without re-checking the preview.
#
# CAVEAT for the 2026-09-16 order (W2026091606514059): after uploading the file
# this script produced, some parts were rotated by hand in JLCPCB's placement
# editor and the list of which ones was not kept. So the placement that was
# actually approved may differ from this file by an unknown set of 180s.
# On any re-order, verify every polarised and keyed part in the preview again
# rather than assuming this file matches what was built last time.

# Designator prefixes that are in the export but must never reach the CPL.
EXCLUDE = {
    "FID": "fiducials are camera targets, not parts",
    "DIGIT": "RGBDigit modules have no LCSC part number — hand-fitted",
}


def load_pnp(path, layer):
    """Read one Fusion Pick-and-Place CSV. No header row; six fixed columns."""
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        for n, rec in enumerate(csv.reader(fh), start=1):
            if not rec or not rec[0].strip():
                continue
            if len(rec) < 6:
                sys.exit(f"{path.name}:{n}: expected 6 columns, got {len(rec)}")
            designator, x, y, rot, _value, package = (f.strip() for f in rec[:6])
            try:
                rows.append(
                    {
                        "designator": designator,
                        "x": float(x),
                        "y": float(y),
                        "rot": float(rot),
                        "package": package,
                        "layer": layer,
                    }
                )
            except ValueError:
                sys.exit(f"{path.name}:{n}: could not read coordinates from {rec!r}")
    return rows


def natural_key(designator):
    """Sort R2 before R10, and U1A/U1B before U2. Cosmetic — JLCPCB ignore row order."""
    m = re.fullmatch(r"([A-Za-z]+)(\d*)(.*)", designator)
    if not m:
        return (designator, 0, "")
    prefix, number, suffix = m.groups()
    return (prefix, int(number) if number else 0, suffix)


def excluded(designator):
    """Return the reason this designator is not a placeable part, or None."""
    for prefix, reason in EXCLUDE.items():
        if designator.startswith(prefix) and designator[len(prefix):].isdigit():
            return reason
    return None


def split_pico(row):
    """One Pico footprint -> two socket strips, one per pad row.

    The rows run along the part's own axis, so they are offset perpendicular to
    it — which is a rotation of the offset vector by the part's own angle. Each
    strip then sits at 90 degrees to the module, facing opposite ways, because a
    Pico numbers its pins 1-20 down one side and 21-40 back up the other.

    Which half gets which of those two angles does not matter: a 1x20 socket is
    symmetric about its centre, so 0 and 180 occupy the identical twenty holes and
    a plain socket strip has no pin-1 to get wrong. The assignment below is the one
    already reviewed in JLCPCB's preview, kept so the file does not churn.

    Rotations come out in Fusion's own convention. The bottom-layer rule is applied
    once, afterwards, in `final_rotation()` — not here.
    """
    theta = math.radians(row["rot"])
    dx = PICO_ROW_HALF_PITCH_MM * math.cos(theta)
    dy = PICO_ROW_HALF_PITCH_MM * math.sin(theta)
    for designator, sign, turn in ((PICO_HALVES[0], -1, -90), (PICO_HALVES[1], 1, 90)):
        half = dict(row)
        half["designator"] = designator
        half["x"] = row["x"] + sign * dx
        half["y"] = row["y"] + sign * dy
        half["rot"] = (row["rot"] + turn) % 360
        yield half


def final_rotation(row):
    """Bottom-layer rule, then the per-package correction."""
    rot = row["rot"]
    if row["layer"] == "Bottom":
        rot += 180
    rot += ROTATION_FIX.get(row["package"], 0)
    return rot % 360


def bom_designators(path):
    """Expand a JLCPCB BOM's Designator column into a set."""
    designators = set()
    with path.open(newline="", encoding="utf-8") as fh:
        for rec in csv.DictReader(fh):
            field = next(
                (v for k, v in rec.items() if k and k.strip().lower() == "designator"),
                None,
            )
            if not field:
                continue
            designators.update(d.strip() for d in field.split(",") if d.strip())
    return designators


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument(
        "assembly_dir",
        type=Path,
        help="CAMOutputs/Assembly directory holding PnP_PCB_front.csv and _back.csv",
    )
    ap.add_argument("-o", "--output", type=Path, help="where to write the CPL")
    ap.add_argument("--bom", type=Path, help="BOM to cross-check designators against")
    args = ap.parse_args()

    repo = Path(__file__).resolve().parent.parent
    assembly = args.assembly_dir.expanduser()
    out_path = args.output or repo / "eda" / "manufacturing" / "CPL.csv"
    bom_path = args.bom or repo / "eda" / "manufacturing" / "BOM_1.csv"

    rows = []
    for name, layer in (("PnP_PCB_front.csv", "Top"), ("PnP_PCB_back.csv", "Bottom")):
        path = assembly / name
        if not path.is_file():
            sys.exit(f"missing {path} — run CAM Processor > Assembly > Pick and Place")
        rows.extend(load_pnp(path, layer))

    placed, dropped = [], []
    for row in rows:
        reason = excluded(row["designator"])
        if reason:
            dropped.append((row["designator"], reason))
        elif row["designator"] == PICO_DESIGNATOR:
            placed.extend(split_pico(row))
        else:
            placed.append(row)

    placed.sort(key=lambda r: natural_key(r["designator"]))

    seen = set()
    for row in placed:
        if row["designator"] in seen:
            sys.exit(f"duplicate designator {row['designator']} — JLCPCB will reject this")
        seen.add(row["designator"])

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(["Designator", "Mid X", "Mid Y", "Layer", "Rotation"])
        for row in placed:
            rot = final_rotation(row)
            w.writerow(
                [
                    row["designator"],
                    f"{row['x']:.4f}",
                    f"{row['y']:.4f}",
                    row["layer"],
                    f"{rot:g}",
                ]
            )

    log = sys.stderr.write
    log(f"wrote {out_path}  ({len(placed)} placements)\n")
    log(f"  dropped {len(dropped)}: " + ", ".join(d for d, _ in dropped) + "\n")

    if bom_path.is_file():
        in_bom = bom_designators(bom_path)
        only_cpl = sorted(seen - in_bom)
        only_bom = sorted(in_bom - seen)
        if only_cpl or only_bom:
            log(f"\nMISMATCH against {bom_path.name} — JLCPCB require an exact match:\n")
            if only_cpl:
                log("  in CPL but not BOM: " + ", ".join(only_cpl) + "\n")
            if only_bom:
                log("  in BOM but not CPL: " + ", ".join(only_bom) + "\n")
        else:
            log(f"  designators match {bom_path.name} exactly ({len(in_bom)})\n")
    else:
        log(f"  no BOM at {bom_path} — designators not cross-checked\n")

    if UNVERIFIED:
        log("\nCHECK THESE IN THE ASSEMBLY PREVIEW BEFORE ORDERING:\n")
        for note in UNVERIFIED.values():
            log(f"  - {note}\n")


if __name__ == "__main__":
    main()
