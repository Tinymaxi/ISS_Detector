#!/usr/bin/env python3
"""Generate the Senba SB-F-02 Fresnel lens as a STEP solid.

No public 3D model exists: LCSC hold it as C89924 with no CAD data, and Senba
publish only a datasheet. This builds it as a solid of revolution from the
dimensioned drawing in ../datasheets/lens-SB-F-02.pdf, which makes it the
manufacturer's own geometry rather than someone's approximation.

Dimensioned values used as-is:
    dome            R11.5, standing 11.5 above the flange top face
    flange          Ø25.75 outside, 1.5 thick
    bore            Ø8.10
    overall height  16.9

Two things are *not* dimensioned on the drawing and are read off it:
    skirt outer     Ø12.30  — the boss below the flange, scaled from the
                      side view against the 8.10 bore. Adjust in Fusion if it
                      matters; nothing else depends on it.
    bore depth      4.1 mm blind, from the underside up to the flange level.

And one inconsistency in the drawing itself: 11.5 above the flange plus 5.6
below comes to 17.1, but the overall is given as 16.9. Both carry ±0.15. This
favours **R11.5 and the 11.5 protrusion** — the numbers the enclosure actually
needs — and takes the skirt to −5.4 so the overall lands on 16.9.

The Fresnel groove pattern on the inner surface is not modelled. It is the
optical point of the part and mechanically irrelevant to a housing.

Usage:  python3 make-lens.py
"""

import math

exec(open("mesh-to-step.py").read().split("if __name__")[0])   # emit_step, check

N_ANG = 64          # angular divisions
N_ARC = 28          # dome arc divisions

R_DOME = 11.5
Z_APEX = 11.5
R_FLANGE = 12.875   # Ø25.75
T_FLANGE = 1.5
R_SKIRT = 6.15      # Ø12.30 — read off the drawing, not dimensioned
Z_BOTTOM = -5.4     # so the overall height is 16.9
R_BORE = 4.05       # Ø8.10
Z_BORE_TOP = -1.5   # blind bore, 3.9 mm deep


def profile():
    """(r, z) from the top apex, out and down, back to the axis."""
    p = []
    for i in range(N_ARC + 1):                      # dome, apex first
        a = (math.pi / 2) * i / N_ARC
        p.append((R_DOME * math.sin(a), R_DOME * math.cos(a)))
    p += [(R_FLANGE, 0.0),
          (R_FLANGE, -T_FLANGE),
          (R_SKIRT, -T_FLANGE),
          (R_SKIRT, Z_BOTTOM),
          (R_BORE, Z_BOTTOM),
          (R_BORE, Z_BORE_TOP),
          (0.0, Z_BORE_TOP)]
    return p


def revolve(prof, n=N_ANG):
    verts, rings = [], []
    for r, z in prof:
        if r < 1e-9:
            verts.append((0.0, 0.0, z))
            rings.append(("apex", len(verts) - 1))
        else:
            base = len(verts)
            for j in range(n):
                t = 2 * math.pi * j / n
                verts.append((r * math.cos(t), r * math.sin(t), z))
            rings.append(("ring", base))

    faces = []
    for i in range(len(rings) - 1):
        (k0, a0), (k1, a1) = rings[i], rings[i + 1]
        if k0 == "apex":
            for j in range(n):
                faces.append((a0, a1 + j, a1 + (j + 1) % n))
        elif k1 == "apex":
            for j in range(n):
                faces.append((a1, a0 + (j + 1) % n, a0 + j))
        else:
            for j in range(n):
                k = (j + 1) % n
                faces.append((a0 + j, a1 + j, a1 + k))
                faces.append((a0 + j, a1 + k, a0 + k))
    return verts, faces


def signed_volume(verts, faces):
    v = 0.0
    for a, b, c in faces:
        pa, pb, pc = verts[a], verts[b], verts[c]
        v += (pa[0] * (pb[1] * pc[2] - pb[2] * pc[1])
              - pa[1] * (pb[0] * pc[2] - pb[2] * pc[0])
              + pa[2] * (pb[0] * pc[1] - pb[1] * pc[0])) / 6.0
    return v


if __name__ == "__main__":
    V, F = revolve(profile())
    bad, ne = check(F)
    vol = signed_volume(V, F)
    if vol < 0:                                     # normals pointing inward
        F = [(a, c, b) for a, b, c in F]
        vol = -vol
    print(f"verts {len(V)}  faces {len(F)}  non-manifold edges {bad}  "
          f"Euler {len(V) - ne + len(F)}  volume {vol:.1f} mm^3")
    if bad:
        raise SystemExit("!! not watertight")
    n, nf = emit_step(V, F, "lens-sb-f-02.step", "SB-F-02",
                      "Senba SB-F-02 Fresnel lens, revolved from the datasheet drawing")
    zs = [p[2] for p in V]
    xs = [p[0] for p in V]
    print(f"wrote lens-sb-f-02.step: {n} entities, {nf} faces "
          f"({'all emitted' if nf == len(F) else 'FACES DROPPED'})")
    print(f"  Ø{max(xs) * 2:.2f}  z {min(zs):.2f}..{max(zs):.2f}  overall {max(zs) - min(zs):.2f} mm")
