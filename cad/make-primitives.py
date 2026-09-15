#!/usr/bin/env python3
"""Generate STEP solids for parts whose only 3D model is an OBJ mesh.

Fusion's Electronics library binds a *solid* to a footprint; an OBJ arrives as a
mesh and will not bind. These parts are simple enough that an exact extruded
solid is indistinguishable from the real geometry for enclosure work — and it
imports without the mesh-conversion detour.

Cylinders are approximated by a **circumscribed** polygon, so the model is never
smaller than the part. A clearance body should err large.

Dimensions are measured from the LCSC/EasyEDA meshes in ./lcsc and cross-checked
against the datasheets in ../datasheets. Bodies sit on z = 0, standing up, so
they drop straight onto the board plane.

Usage:  python3 make-primitives.py
"""

import math
import datetime


def polygon_solid(entities, poly, z0, z1, name):
    """Emit an extruded polygon as a MANIFOLD_SOLID_BREP. `poly` must wind CCW."""

    def add(s):
        entities.append(s)
        return len(entities)

    def point(x, y, z):
        return add(f"CARTESIAN_POINT('',({x:.6f},{y:.6f},{z:.6f}))")

    def direction(x, y, z):
        return add(f"DIRECTION('',({x:.6f},{y:.6f},{z:.6f}))")

    def edge(v1, v2, p1, d):
        dd = direction(*d)
        vec = add(f"VECTOR('',#{dd},1.0)")
        ln = add(f"LINE('',#{p1},#{vec})")
        return add(f"EDGE_CURVE('',#{v1},#{v2},#{ln},.T.)")

    def plane(px, py, pz, n, r):
        o = point(px, py, pz)
        nd = direction(*n)
        rd = direction(*r)
        ax = add(f"AXIS2_PLACEMENT_3D('',#{o},#{nd},#{rd})")
        return add(f"PLANE('',#{ax})")

    def face(loop, surf):
        oriented = [add(f"ORIENTED_EDGE('',*,*,#{e},{s})") for e, s in loop]
        lp = add("EDGE_LOOP('',(" + ",".join(f"#{o}" for o in oriented) + "))")
        fb = add(f"FACE_OUTER_BOUND('',#{lp},.T.)")
        return add(f"ADVANCED_FACE('',(#{fb}),#{surf},.T.)")

    n = len(poly)
    pb = [point(x, y, z0) for x, y in poly]
    pt = [point(x, y, z1) for x, y in poly]
    vb = [add(f"VERTEX_POINT('',#{p})") for p in pb]
    vt = [add(f"VERTEX_POINT('',#{p})") for p in pt]

    eb, et, ev = [], [], []
    for i in range(n):
        j = (i + 1) % n
        dx, dy = poly[j][0] - poly[i][0], poly[j][1] - poly[i][1]
        m = math.hypot(dx, dy)
        eb.append(edge(vb[i], vb[j], pb[i], (dx / m, dy / m, 0.0)))
        et.append(edge(vt[i], vt[j], pt[i], (dx / m, dy / m, 0.0)))
        ev.append(edge(vb[i], vt[i], pb[i], (0.0, 0.0, 1.0)))

    faces = []
    for i in range(n):
        j = (i + 1) % n
        dx, dy = poly[j][0] - poly[i][0], poly[j][1] - poly[i][1]
        m = math.hypot(dx, dy)
        # outward normal of a CCW edge
        surf = plane(poly[i][0], poly[i][1], z0, (dy / m, -dx / m, 0.0), (0.0, 0.0, 1.0))
        faces.append(face([(eb[i], ".T."), (ev[j], ".T."), (et[i], ".F."), (ev[i], ".F.")], surf))

    surf = plane(0, 0, z0, (0, 0, -1), (1, 0, 0))
    faces.append(face([(eb[i], ".F.") for i in range(n - 1, -1, -1)], surf))
    surf = plane(0, 0, z1, (0, 0, 1), (1, 0, 0))
    faces.append(face([(et[i], ".T.") for i in range(n)], surf))

    shell = add("CLOSED_SHELL('',(" + ",".join(f"#{f}" for f in faces) + "))")
    return add(f"MANIFOLD_SOLID_BREP('{name}',#{shell})")


def circle(diameter, sides=32):
    """Circumscribed polygon — never smaller than the true circle."""
    r = (diameter / 2) / math.cos(math.pi / sides)
    return [(r * math.cos(2 * math.pi * i / sides), r * math.sin(2 * math.pi * i / sides))
            for i in range(sides)]


def box(length, width, cx=0.0, cy=0.0):
    lx, wy = length / 2, width / 2
    return [(cx - lx, cy - wy), (cx + lx, cy - wy),
            (cx + lx, cy + wy), (cx - lx, cy + wy)]          # CCW


def write_step(path, name, description, solids):
    """solids: list of (poly, z0, z1, body_name)"""
    entities = []
    breps = [polygon_solid(entities, p, z0, z1, bn) for p, z0, z1, bn in solids]

    def add(s):
        entities.append(s)
        return len(entities)

    o = add("CARTESIAN_POINT('',(0.,0.,0.))")
    z = add("DIRECTION('',(0.,0.,1.))")
    x = add("DIRECTION('',(1.,0.,0.))")
    ax = add(f"AXIS2_PLACEMENT_3D('',#{o},#{z},#{x})")
    lu = add("(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT(.MILLI.,.METRE.))")
    au = add("(NAMED_UNIT(*)PLANE_ANGLE_UNIT()SI_UNIT($,.RADIAN.))")
    su = add("(NAMED_UNIT(*)SI_UNIT($,.STERADIAN.)SOLID_ANGLE_UNIT())")
    un = add(f"UNCERTAINTY_MEASURE_WITH_UNIT(LENGTH_MEASURE(1.E-07),#{lu},'','')")
    ctx = add(f"(GEOMETRIC_REPRESENTATION_CONTEXT(3)GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT((#{un}))"
              f"GLOBAL_UNIT_ASSIGNED_CONTEXT((#{lu},#{au},#{su}))REPRESENTATION_CONTEXT('',''))")
    items = ",".join([f"#{ax}"] + [f"#{b}" for b in breps])
    rep = add(f"ADVANCED_BREP_SHAPE_REPRESENTATION('',({items}),#{ctx})")
    apc = add("APPLICATION_CONTEXT('automotive design')")
    add(f"APPLICATION_PROTOCOL_DEFINITION('international standard','automotive_design',2000,#{apc})")
    pdc = add(f"PRODUCT_DEFINITION_CONTEXT('part definition',#{apc},'design')")
    pcx = add(f"PRODUCT_CONTEXT('',#{apc},'mechanical')")
    pr = add(f"PRODUCT('{name}','{name}','',(#{pcx}))")
    pdf = add(f"PRODUCT_DEFINITION_FORMATION('','',#{pr})")
    pd = add(f"PRODUCT_DEFINITION('design','',#{pdf},#{pdc})")
    pds = add(f"PRODUCT_DEFINITION_SHAPE('','',#{pd})")
    add(f"SHAPE_DEFINITION_REPRESENTATION(#{pds},#{rep})")

    ts = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S")
    body = "\n".join(f"#{i + 1}={s};" for i, s in enumerate(entities))
    open(path, "w").write(
        "ISO-10303-21;\nHEADER;\n"
        f"FILE_DESCRIPTION(('{description}'),'2;1');\n"
        f"FILE_NAME('{path}','{ts}',('ISS-Box'),(''),'','','');\n"
        "FILE_SCHEMA(('AUTOMOTIVE_DESIGN { 1 0 10303 214 1 1 1 1 }'));\nENDSEC;\nDATA;\n"
        f"{body}\nENDSEC;\nEND-ISO-10303-21;\n")
    return len(entities), len(breps)


PARTS = [
    ("buzzer-qmb-09b-05.step", "QMB-09B-05",
     "QMB-09B-05 magnetic transducer body, 9.0 dia x 5.5 mm",
     [(circle(9.00), 0.0, 5.50, "body")]),

    ("mics-5524.step", "MICS-5524",
     "MiCS-5524 gas sensor body, 7.0 x 5.0 x 1.52 mm",
     [(box(7.00, 5.00), 0.0, 1.52, "body")]),

    ("veml7700.step", "VEML7700",
     "VEML7700 ambient light sensor body, 6.8 x 2.35 x 3.0 mm",
     [(box(6.80, 2.35), 0.0, 3.00, "body")]),

    # Base flange 9.10 dia, can 8.20 dia standing 4.20 above it (datasheet p.8).
    # Two bodies so the shoulder is real — the Fresnel lens skirt keys on it.
    ("pir-bs612.step", "BS612",
     "BS612 PIR: 9.10 dia base, 8.20 dia can 4.20 tall",
     [(circle(9.10), 0.0, 0.50, "base"), (circle(8.20), 0.50, 4.70, "can")]),

    # Profile sliced out of lcsc/scd41.obj: a full-width base plate 0.88 tall,
    # then the sensing body stepping in to 7.86 square and rising to 6.30.
    # The gas-permeable membrane is the top face of that upper body — it is what
    # the enclosure vent has to sit over.
    ("scd41.step", "SCD41-D-R2",
     "SCD41 CO2 sensor: 10.1 sq base, 7.86 sq body to 6.30 mm",
     [(box(10.10, 10.10), 0.0, 0.88, "base"),
      (box(7.86, 7.86), 0.88, 6.30, "body")]),

    # Profile sliced out of lcsc/cr1220-holder.obj. The full 19.76 width is only
    # at tab level; above that the plastic steps in. Not centred on its origin —
    # the body sits 0.46 mm to +x. Locating pegs run 0.85 below the board and are
    # left out: the case never sees them.
    ("cr1220-holder.step", "CR1220-2",
     "CR1220-2 coin-cell holder: 19.76 x 14.90 tabs, body to 4.05 mm",
     [(box(19.76, 14.90, 0.46), 0.0, 0.55, "tabs"),
      (box(15.76, 14.90, 0.46), 0.55, 4.05, "body")]),
]

if __name__ == "__main__":
    for path, name, desc, solids in PARTS:
        n, b = write_step(path, name, desc, solids)
        print(f"{path:26s} {n:5d} entities  {b} solid(s)")
