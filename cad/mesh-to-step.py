#!/usr/bin/env python3
"""Convert an OBJ display mesh into a STEP faceted solid.

The LCSC/EasyEDA models in ./lcsc are display meshes: they have open boundaries
(the CR1220 holder has 211 of them), so nothing will treat them as solids and
Fusion's Electronics library will not bind them to a footprint.

This does what "Convert Mesh to Solid" does: weld coincident vertices, chain the
open boundary half-edges into loops, fan-fill each loop, verify the result is a
closed oriented 2-manifold, then emit it as a MANIFOLD_SOLID_BREP.

Usage:  python3 mesh-to-step.py lcsc/cr1220-holder.obj cr1220-holder-detailed.step "CR1220-2"
"""

import sys
import math
import datetime
from collections import defaultdict


def load_obj(path):
    verts, faces = [], []
    for line in open(path, errors="ignore"):
        p = line.split()
        if not p:
            continue
        if p[0] == "v":
            verts.append(tuple(round(float(x), 5) for x in p[1:4]))
        elif p[0] == "f":
            idx = [int(t.split("/")[0]) for t in p[1:]]
            idx = [i - 1 if i > 0 else len(verts) + i for i in idx]
            for k in range(1, len(idx) - 1):            # fan-triangulate n-gons
                faces.append((idx[0], idx[k], idx[k + 1]))
    return verts, faces


def weld(verts, faces):
    uniq, remap = {}, []
    for v in verts:
        if v not in uniq:
            uniq[v] = len(uniq)
        remap.append(uniq[v])
    out = [None] * len(uniq)
    for v, i in uniq.items():
        out[i] = v
    kept = [tuple(remap[i] for i in f) for f in faces]
    return out, [f for f in kept if len(set(f)) == 3]


def split_tjunctions(verts, faces, tol=1e-6):
    """Repair T-junctions: an edge divided on one side of the mesh but not the
    other. These show up as boundary 'loops' whose points are all collinear —
    they are not holes, and filling them only makes slivers. The fix is to split
    the long edge at the vertices sitting on it."""
    half = set()
    for a, b, c in faces:
        half |= {(a, b), (b, c), (c, a)}
    boundary = [(a, b) for (a, b) in half if (b, a) not in half]
    if not boundary:
        return faces, 0

    owner = {}
    for fi, f in enumerate(faces):
        a, b, c = f
        for e in ((a, b), (b, c), (c, a)):
            owner[e] = fi

    replaced, out, nsplit = {}, [], 0
    for (a, b) in boundary:
        pa, pb = verts[a], verts[b]
        d = tuple(pb[i] - pa[i] for i in range(3))
        L2 = sum(x * x for x in d)
        if L2 < tol:
            continue
        hits = []
        for vi, p in enumerate(verts):
            if vi in (a, b):
                continue
            t = sum((p[i] - pa[i]) * d[i] for i in range(3)) / L2
            if not (tol < t < 1 - tol):
                continue
            proj = tuple(pa[i] + t * d[i] for i in range(3))
            if sum((p[i] - proj[i]) ** 2 for i in range(3)) < tol:
                hits.append((t, vi))
        if not hits:
            continue
        fi = owner.get((a, b))
        if fi is None or fi in replaced:
            continue
        hits.sort()
        chain = [a] + [vi for _, vi in hits] + [b]
        apex = [v for v in faces[fi] if v not in (a, b)][0]
        replaced[fi] = [(chain[k], chain[k + 1], apex) for k in range(len(chain) - 1)]
        nsplit += 1

    for fi, f in enumerate(faces):
        out.extend(replaced[fi] if fi in replaced else [f])
    return out, nsplit


def close_holes(verts, faces):
    """Chain open boundary half-edges into loops and fan-fill them."""
    half = set()
    for a, b, c in faces:
        half |= {(a, b), (b, c), (c, a)}
    boundary = [(a, b) for (a, b) in half if (b, a) not in half]
    if not boundary:
        return verts, faces, 0

    nxt = defaultdict(list)
    for a, b in boundary:
        nxt[a].append(b)

    verts = list(verts)
    faces = list(faces)
    used, loops = set(), []
    for a0, b0 in boundary:
        if (a0, b0) in used:
            continue
        loop, a, b = [a0], a0, b0
        while True:
            used.add((a, b))
            if b == a0:
                break
            loop.append(b)
            cands = [x for x in nxt[b] if (b, x) not in used]
            if not cands:
                break
            a, b = b, cands[0]
        if len(loop) >= 3:
            loops.append(loop)

    # A boundary loop can pinch — pass through the same vertex twice, like a
    # figure eight. Fanning that as one loop puts two triangles on the same
    # edge, giving an edge shared by four faces. Split at every repeat.
    def split_pinches(loop):
        out, stack, pos = [], [], {}
        for v in loop:
            if v in pos:
                i = pos[v]
                sub = stack[i:]
                if len(sub) >= 3:
                    out.append(sub)
                for u in stack[i:]:
                    pos.pop(u, None)
                stack = stack[:i]
            pos[v] = len(stack)
            stack.append(v)
        if len(stack) >= 3:
            out.append(stack)
        return out

    loops = [sub for loop in loops for sub in split_pinches(loop)]

    for loop in loops:
        if len(loop) == 3:
            # A triangular hole is already a triangle. Adding a centroid here
            # makes three slivers and can land the new vertex on top of an
            # existing one, which is how you get an edge shared by four faces.
            faces.append((loop[2], loop[1], loop[0]))
            continue
        cx = sum(verts[i][0] for i in loop) / len(loop)
        cy = sum(verts[i][1] for i in loop) / len(loop)
        cz = sum(verts[i][2] for i in loop) / len(loop)
        ci = len(verts)
        verts.append((cx, cy, cz))
        n = len(loop)
        for k in range(n):
            a, b = loop[k], loop[(k + 1) % n]
            faces.append((b, a, ci))                    # reversed: patch faces outward
    return verts, faces, len(loops)


def area(verts, f):
    pa, pb, pc = verts[f[0]], verts[f[1]], verts[f[2]]
    u = (pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2])
    w = (pc[0] - pa[0], pc[1] - pa[1], pc[2] - pa[2])
    nx = u[1] * w[2] - u[2] * w[1]
    ny = u[2] * w[0] - u[0] * w[2]
    nz = u[0] * w[1] - u[1] * w[0]
    return 0.5 * math.sqrt(nx * nx + ny * ny + nz * nz)


def drop_slivers(verts, faces, eps=1e-12):
    """Zero-area triangles carry no geometry but do carry edges. Emitting the
    solid skips them, which would silently unseal the shell — so remove them
    here and let the next hole-closing pass repair whatever they were holding."""
    keep = [f for f in faces if area(verts, f) > eps]
    return keep, len(faces) - len(keep)


def check(faces):
    ec = defaultdict(int)
    for a, b, c in faces:
        for e in ((a, b), (b, c), (c, a)):
            ec[tuple(sorted(e))] += 1
    return sum(1 for v in ec.values() if v != 2), len(ec)


def emit_step(verts, faces, path, name, description):
    E = []

    def add(s):
        E.append(s)
        return len(E)

    pts = [add("CARTESIAN_POINT('',({:.5f},{:.5f},{:.5f}))".format(*v)) for v in verts]
    vps = [add(f"VERTEX_POINT('',#{p})") for p in pts]

    edge_id = {}

    def get_edge(a, b):
        key = (a, b) if a < b else (b, a)
        if key not in edge_id:
            p, q = verts[key[0]], verts[key[1]]
            d = (q[0] - p[0], q[1] - p[1], q[2] - p[2])
            m = math.sqrt(d[0] ** 2 + d[1] ** 2 + d[2] ** 2) or 1.0
            dd = add("DIRECTION('',({:.6f},{:.6f},{:.6f}))".format(*(x / m for x in d)))
            vec = add(f"VECTOR('',#{dd},1.0)")
            ln = add(f"LINE('',#{pts[key[0]]},#{vec})")
            edge_id[key] = add(f"EDGE_CURVE('',#{vps[key[0]]},#{vps[key[1]]},#{ln},.T.)")
        return edge_id[key], (a < b)

    face_ids = []
    for a, b, c in faces:
        pa, pb, pc = verts[a], verts[b], verts[c]
        u = (pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2])
        w = (pc[0] - pa[0], pc[1] - pa[1], pc[2] - pa[2])
        nx = u[1] * w[2] - u[2] * w[1]
        ny = u[2] * w[0] - u[0] * w[2]
        nz = u[0] * w[1] - u[1] * w[0]
        m = math.sqrt(nx * nx + ny * ny + nz * nz)
        if m < 1e-12:
            continue
        nx, ny, nz = nx / m, ny / m, nz / m
        m2 = math.sqrt(u[0] ** 2 + u[1] ** 2 + u[2] ** 2) or 1.0
        rx, ry, rz = u[0] / m2, u[1] / m2, u[2] / m2
        o = add("CARTESIAN_POINT('',({:.5f},{:.5f},{:.5f}))".format(*pa))
        nd = add(f"DIRECTION('',({nx:.6f},{ny:.6f},{nz:.6f}))")
        rd = add(f"DIRECTION('',({rx:.6f},{ry:.6f},{rz:.6f}))")
        ax = add(f"AXIS2_PLACEMENT_3D('',#{o},#{nd},#{rd})")
        pl = add(f"PLANE('',#{ax})")
        oes = []
        for x, y in ((a, b), (b, c), (c, a)):
            eid, fwd = get_edge(x, y)
            oes.append(add(f"ORIENTED_EDGE('',*,*,#{eid},{'.T.' if fwd else '.F.'})"))
        lp = add("EDGE_LOOP('',(" + ",".join(f"#{o}" for o in oes) + "))")
        fb = add(f"FACE_OUTER_BOUND('',#{lp},.T.)")
        face_ids.append(add(f"ADVANCED_FACE('',(#{fb}),#{pl},.T.)"))

    shell = add("CLOSED_SHELL('',(" + ",".join(f"#{f}" for f in face_ids) + "))")
    brep = add(f"MANIFOLD_SOLID_BREP('{name}',#{shell})")
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
    rep = add(f"ADVANCED_BREP_SHAPE_REPRESENTATION('',(#{ax},#{brep}),#{ctx})")
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
    body = "\n".join(f"#{i + 1}={s};" for i, s in enumerate(E))
    open(path, "w").write(
        "ISO-10303-21;\nHEADER;\n"
        f"FILE_DESCRIPTION(('{description}'),'2;1');\n"
        f"FILE_NAME('{path}','{ts}',('ISS-Box'),(''),'','','');\n"
        "FILE_SCHEMA(('AUTOMOTIVE_DESIGN { 1 0 10303 214 1 1 1 1 }'));\nENDSEC;\nDATA;\n"
        f"{body}\nENDSEC;\nEND-ISO-10303-21;\n")
    return len(E), len(face_ids)


if __name__ == "__main__":
    src, dst, name = sys.argv[1], sys.argv[2], sys.argv[3]
    V, F = load_obj(src)
    print(f"loaded   {len(V):6d} verts {len(F):6d} tris")
    V, F = weld(V, F)
    print(f"welded   {len(V):6d} verts {len(F):6d} tris")
    bad, ne = check(F)
    print(f"before   {bad:6d} non-manifold edges of {ne}")

    # Filling can produce slivers, and dropping slivers can reopen holes.
    # Alternate until both are clean.
    for it in range(1, 12):
        F, nsplit = split_tjunctions(V, F)
        F, ndrop = drop_slivers(V, F)
        bad, ne = check(F)
        nloops = 0
        if bad and nsplit == 0:                 # only genuine holes remain
            V, F, nloops = close_holes(V, F)
            F, extra = drop_slivers(V, F)
            ndrop += extra
            bad, ne = check(F)
        print(f"pass {it}   split {nsplit:4d} T-junctions, dropped {ndrop:4d} slivers, "
              f"filled {nloops:3d} loops -> {len(F)} tris, {bad} non-manifold")
        if nsplit == 0 and ndrop == 0 and nloops == 0 and bad == 0:
            break

    bad, ne = check(F)
    print(f"final    {bad:6d} non-manifold edges of {ne}   Euler V-E+F = {len(V) - ne + len(F)}")
    if bad:
        print("!! not watertight — the result will not be a valid solid")
        sys.exit(1)
    n, nf = emit_step(V, F, dst, name, f"{name} — faceted solid from LCSC mesh")
    if nf != len(F):
        print(f"!! emitted {nf} faces from {len(F)} triangles — shell is not sealed")
        sys.exit(1)
    print(f"wrote    {dst}: {n} entities, {nf} faces (all triangles emitted)")
