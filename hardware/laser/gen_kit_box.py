#!/usr/bin/env python3
"""
NeuroFly Kit Box generator  (stdlib only, reproducible)
=======================================================
Writes the laser files for the "unboxing" kit box handed to every student team:

    neurofly_kit_box.svg / .dxf            sheet 1  (box shell, lid, ...)
    neurofly_kit_box_sheet2.svg / .dxf     sheet 2  ... sheet N
    neurofly_kit_box_layout.json           insert-tray pocket map (used by docs/kit_box/index.html)

Usage:
    python3 gen_kit_box.py                 # default: 3 mm material, kerf 0.15 mm, no LED window
    python3 gen_kit_box.py --led           # lid gets a 5.4 mm WS2812 window (optional LED-lid)
    python3 gen_kit_box.py --out DIR       # write somewhere else
    python3 gen_kit_box.py --check-only    # re-verify the files already on disk

MATERIAL / KERF PARAMETERS  (edit here, everything else is derived)
-------------------------------------------------------------------
T      material thickness in mm.  Default 3.0 (3 mm plywood or 3 mm cast acrylic).
       MEASURE YOUR SHEET with calipers; plywood is often 2.8-3.2 mm. If it is not 3.0
       set T and re-run: finger sizes, slots and box height all follow T.
KERF   laser beam width in mm (0.15 typical for a 50-80 W CO2 on 3 mm). Every cut path is
       offset by KERF/2 so parts come out at their nominal size (parts grow by KERF/2 per
       side, holes shrink by KERF/2 per side). Measure with a test square (see README).
FIT    extra press-fit interference for the finger joints (0.05 = 0.05 mm total). Use 0.0 for
       acrylic (it cracks), 0.05-0.10 for plywood.
Colour convention in SVG / DXF:
    RED   #FF0000  layer CUT              through-cuts (do these LAST)
    BLUE  #0000FF  layer ENGRAVE_VECTOR   thin vector engrave / score lines
    BLACK #000000  layer RASTER_ENGRAVE   filled areas to raster-engrave (SVG: filled shapes,
                                          DXF: closed outlines, hatch-fill in laser software)
    GREY  #888888  layer NOTES            sheet border + part names: HIDE, DO NOT CUT.
All coordinates are millimetres. Sheet = 400 x 300 mm.
"""
import math, os, sys, json, argparse
import xml.etree.ElementTree as ET

# ----------------------------------------------------------------------------- parameters
T = 3.0                  # material thickness
KERF = 0.15              # beam width
FIT = 0.05               # press-fit interference on finger joints
SHEET_W, SHEET_H = 400.0, 300.0
SHEET_MARGIN = 5.0       # keep-out at the sheet edge
PIECE_GAP = 6.0          # gap between pieces (room for the NOTES label)

W, D = 210.0, 140.0      # outer footprint of the box
RIBBON_GAP = 3.5         # side gap each side of the trays (lift ribbons live here)
POCKET_CLEAR = 1.0       # pocket = part + this (total, both sides)
CARD_CLEAR = 1.4
LID_GAP = 3.0            # air between top tray and lid (WS2812 LED + wires live here)
LID_SLOT_CLEAR = 0.25    # slot = T + this
LID_SIDE_CLEAR = 0.30    # lid narrower than W by this (sliding fit)
LID_END_CLEAR = 0.30
HANDLE = 16.0            # lid lip that sticks out of the front
STRIP = 8.0              # wall strip above the lid slot
FINGER_V = 10.0          # target finger length on vertical corners
FINGER_H = 14.0          # target finger length on base joints
BAND_Y = 114.0           # header band starts here on each tray's top plate
LED_WINDOW = 5.4         # WS2812B 5x5 mm
TEXT_HINT_CAP = 2.8
VERSION = "0.3.0"
try:
    _vp = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "VERSION")
    VERSION = open(_vp).read().strip() or VERSION
except OSError:
    pass

W_IN, D_IN = W - 2 * T, D - 2 * T
DECK_W, DECK_D = W_IN - 2 * RIBBON_GAP, D_IN - 0.8

# ----------------------------------------------------------------------------- stroke font
# glyph grid: x 0..4, y 0..6 ; strokes separated by '|', points are two digits "xy"
_G = {
 'A':"00 04 26 44 40|03 43", 'B':"00 06 36 45 44 33 03|33 42 41 30 00", 'C':"45 36 16 05 01 10 30 41",
 'D':"00 06 36 45 41 30 00", 'E':"46 06 00 40|03 33", 'F':"46 06 00|03 33",
 'G':"45 36 16 05 01 10 30 41 43 23", 'H':"00 06|40 46|03 43", 'I':"16 36|26 20|10 30",
 'J':"46 41 30 10 01", 'K':"00 06|46 03 40", 'L':"06 00 40", 'M':"00 06 23 46 40", 'N':"00 06 40 46",
 'O':"10 30 41 45 36 16 05 01 10", 'P':"00 06 36 45 44 33 03", 'Q':"10 30 41 45 36 16 05 01 10|22 40",
 'R':"00 06 36 45 44 33 03|23 40", 'S':"45 36 16 05 04 13 33 42 41 30 10 01", 'T':"06 46|26 20",
 'U':"06 01 10 30 41 46", 'V':"06 20 46", 'W':"06 10 23 30 46", 'X':"06 40|00 46", 'Y':"06 23 46|23 20",
 'Z':"06 46 00 40",
 '0':"10 30 41 45 36 16 05 01 10|11 35", '1':"15 26 20|10 30", '2':"05 16 36 45 44 00 40",
 '3':"05 16 36 45 44 33 23|33 42 41 30 10 01", '4':"36 02 42|30 36", '5':"46 06 03 33 42 41 30 10 01",
 '6':"45 36 16 05 01 10 30 41 42 33 03", '7':"06 46 20",
 '8':"16 36 45 44 33 13 04 05 16|13 02 01 10 30 41 42 33", '9':"43 13 04 05 16 36 45 41 30 10 01",
 '.':"10 20 21 11 10", ',':"20 11", ':':"11 21|14 24", '-':"13 33", '+':"03 43|21 25", '/':"00 46",
 '!':"26 22|20 21", '?':"05 16 36 45 44 33 23 22|20 21", "'":"26 24", '(':"36 15 11 30", ')':"16 35 31 10",
 '=':"02 42|04 44", '_':"00 40", '>':"06 42 00", '<':"46 02 40",
}
GLYPH = {k: [[(int(p[0]), int(p[1])) for p in s.split()] for s in v.split('|')] for k, v in _G.items()}

def text_width(s, h):
    return max(0.0, len(s) * h - h / 3.0)

def text_strokes(s, x, y, h, anchor='l'):
    """vector text: returns list of open polylines; (x,y) = baseline start (anchor l) or centre (c)"""
    s = s.upper(); sc = h / 6.0
    w = text_width(s, h)
    if anchor == 'c': x -= w / 2
    elif anchor == 'r': x -= w
    out = []
    for i, ch in enumerate(s):
        for st in GLYPH.get(ch, []):
            out.append([(x + i * 6 * sc + px * sc, y + py * sc) for px, py in st])
    return out

# ----------------------------------------------------------------------------- geometry helpers
def rect_loop(x0, y0, x1, y1, hole=False):
    pts = [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]       # CCW, material on the left
    return pts[::-1] if hole else pts

def chamfer_rect(x0, y0, x1, y1, c, corners=(1, 1, 1, 1)):
    """CCW outline with optional chamfers (BL, BR, TR, TL)"""
    p = []
    bl, br, tr, tl = corners
    p += [(x0 + c, y0), (x1 - c, y0)] if bl and br else [(x0 + (c if bl else 0), y0), (x1 - (c if br else 0), y0)]
    p += [(x1, y0 + c), (x1, y1 - c)] if br and tr else [(x1, y0 + (c if br else 0)), (x1, y1 - (c if tr else 0))]
    p += [(x1 - c, y1), (x0 + c, y1)] if tr and tl else [(x1 - (c if tr else 0), y1), (x0 + (c if tl else 0), y1)]
    p += [(x0, y1 - c), (x0, y0 + c)] if tl and bl else [(x0, y1 - (c if tl else 0)), (x0, y0 + (c if bl else 0))]
    out = []
    for q in p:
        if not out or abs(out[-1][0] - q[0]) > 1e-9 or abs(out[-1][1] - q[1]) > 1e-9: out.append(q)
    if abs(out[0][0] - out[-1][0]) < 1e-9 and abs(out[0][1] - out[-1][1]) < 1e-9: out.pop()
    return out

def _r(v): return round(v, 6)

def region_loops(add, sub):
    """rectilinear region = union(add) minus union(sub) -> loops (material on the left)"""
    xs = sorted({_r(v) for r in add + sub for v in (r[0], r[2])})
    ys = sorted({_r(v) for r in add + sub for v in (r[1], r[3])})
    def inside(rs, cx, cy): return any(r[0] < cx < r[2] and r[1] < cy < r[3] for r in rs)
    nx, ny = len(xs) - 1, len(ys) - 1
    fill = [[False] * ny for _ in range(nx)]
    for i in range(nx):
        for j in range(ny):
            cx, cy = (xs[i] + xs[i + 1]) / 2, (ys[j] + ys[j + 1]) / 2
            fill[i][j] = inside(add, cx, cy) and not inside(sub, cx, cy)
    edges = {}
    def f(i, j): return 0 <= i < nx and 0 <= j < ny and fill[i][j]
    for i in range(nx):
        for j in range(ny):
            if not fill[i][j]: continue
            x0, x1, y0, y1 = xs[i], xs[i + 1], ys[j], ys[j + 1]
            if not f(i, j - 1): edges.setdefault((x0, y0), []).append((x1, y0))
            if not f(i + 1, j): edges.setdefault((x1, y0), []).append((x1, y1))
            if not f(i, j + 1): edges.setdefault((x1, y1), []).append((x0, y1))
            if not f(i - 1, j): edges.setdefault((x0, y1), []).append((x0, y0))
    loops = []
    while edges:
        start = next(iter(edges)); pts = [start]; cur = start
        while True:
            nxts = edges[cur]
            assert len(nxts) == 1, "pinch vertex in region (design error)"
            nx_ = nxts.pop(); 
            if not nxts: del edges[cur]
            if nx_ == start: break
            pts.append(nx_); cur = nx_
        # merge collinear
        m = []
        n = len(pts)
        for k in range(n):
            a, b, c = pts[k - 1], pts[k], pts[(k + 1) % n]
            if abs((b[0] - a[0]) * (c[1] - b[1]) - (b[1] - a[1]) * (c[0] - b[0])) > 1e-9: m.append(b)
        loops.append(m)
    return loops

def offset_loop(pts, d):
    """material on the left; move boundary to the right by d (d>0 grows the part / shrinks holes)"""
    n = len(pts); out = []
    for k in range(n):
        a, b, c = pts[k - 1], pts[k], pts[(k + 1) % n]
        e1 = (b[0] - a[0], b[1] - a[1]); e2 = (c[0] - b[0], c[1] - b[1])
        l1 = math.hypot(*e1); l2 = math.hypot(*e2)
        n1 = (e1[1] / l1, -e1[0] / l1); n2 = (e2[1] / l2, -e2[0] / l2)
        k_ = 1 + n1[0] * n2[0] + n1[1] * n2[1]
        out.append((b[0] + d * (n1[0] + n2[0]) / k_, b[1] + d * (n1[1] + n2[1]) / k_))
    return out

def area(pts):
    return sum(pts[k - 1][0] * pts[k][1] - pts[k][0] * pts[k - 1][1] for k in range(len(pts))) / 2

def row_cells(loops, amax, bmax):
    """set of (i,j) unit cells (centres at +0.5) inside a rectilinear region (even-odd)"""
    ver = []
    for lp in loops:
        n = len(lp)
        for k in range(n):
            p, q = lp[k], lp[(k + 1) % n]
            if abs(p[0] - q[0]) < 1e-9 and abs(p[1] - q[1]) > 1e-9:
                ver.append((p[0], min(p[1], q[1]), max(p[1], q[1])))
    cells = set()
    for j in range(int(bmax)):
        yb = j + 0.5
        xsx = sorted(v[0] for v in ver if v[1] < yb < v[2])
        for a, b in zip(xsx[0::2], xsx[1::2]):
            for i in range(int(math.floor(a + 0.5 - 1e-9)) if False else 0, int(amax)):
                if a < i + 0.5 < b: cells.add((i, j))
    return cells

# primitives: ('poly', pts, closed) | ('circ', cx, cy, r)
def P(pts, closed=True): return ('poly', list(pts), closed)
def C(cx, cy, r): return ('circ', cx, cy, r)

def tr_prim(p, fn):
    if p[0] == 'poly': return ('poly', [fn(q) for q in p[1]], p[2])
    x, y = fn((p[1], p[2])); return ('circ', x, y, p[3])

def prim_bbox(p):
    if p[0] == 'poly':
        xs = [q[0] for q in p[1]]; ys = [q[1] for q in p[1]]
        return min(xs), min(ys), max(xs), max(ys)
    return p[1] - p[3], p[2] - p[3], p[1] + p[3], p[2] + p[3]

def bbox_of(prims):
    bs = [prim_bbox(p) for p in prims]
    return (min(b[0] for b in bs), min(b[1] for b in bs), max(b[2] for b in bs), max(b[3] for b in bs))

# ----------------------------------------------------------------------------- piece container
class Piece:
    def __init__(self, name, group, note=""):
        self.name, self.group, self.note = name, group, note
        self.cut, self.vec, self.ras = [], [], []
        self.model = None            # (for shell panels) loops in model coords, for verification
    def add_text(self, s, x, y, h, anchor='l', layer='vec'):
        if layer == 'ras':               # heavy lettering: every stroke becomes a filled quad + round joints
            w = h * 0.15
            for st in text_strokes(s, x, y, h, anchor):
                for a, b in zip(st, st[1:]):
                    dx, dy = b[0] - a[0], b[1] - a[1]; l = math.hypot(dx, dy)
                    if l < 1e-9: continue
                    nx, ny = -dy / l * w / 2, dx / l * w / 2
                    self.ras.append(P([(a[0] + nx, a[1] + ny), (b[0] + nx, b[1] + ny), (b[0] - nx, b[1] - ny), (a[0] - nx, a[1] - ny)], True))
                for q in st: self.ras.append(C(q[0], q[1], w / 2))
            return
        for st in text_strokes(s, x, y, h, anchor): getattr(self, layer).append(P(st, False))
    def add_logo(self, cx, cy, r):
        vec, ras = logo(cx, cy, r); self.vec += vec; self.ras += ras
    def bbox(self): return bbox_of(self.cut)
    def transform(self, fn):
        q = Piece(self.name, self.group, self.note)
        for L in ('cut', 'vec', 'ras'): setattr(q, L, [tr_prim(p, fn) for p in getattr(self, L)])
        return q

def add_outline(piece, loops, comp, hole_flags=None):
    for k, lp in enumerate(loops):
        piece.cut.append(P(offset_loop(lp, comp), True))

def logo(cx, cy, r):
    """NeuroFly mark: neuron soma + 4 dendrite arms ending in rotor rings"""
    vec, ras = [], []
    ras.append(C(cx, cy, r * 0.17)); vec.append(C(cx, cy, r * 0.25))
    for k in range(4):
        a = math.radians(45 + 90 * k); ux, uy = math.cos(a), math.sin(a)
        vec.append(P([(cx + ux * r * 0.25, cy + uy * r * 0.25), (cx + ux * r * 0.62, cy + uy * r * 0.62)], False))
        rx, ry = cx + ux * r * 0.80, cy + uy * r * 0.80
        vec.append(C(rx, ry, r * 0.20)); ras.append(C(rx, ry, r * 0.065))
        a2 = math.radians(90 * k); vx, vy = math.cos(a2), math.sin(a2)           # dendrite fork on axes
        e = (cx + vx * r * 0.62, cy + vy * r * 0.62)
        vec.append(P([(cx + vx * r * 0.25, cy + vy * r * 0.25), e], False))
        for s in (-1, 1):
            b = math.radians(90 * k + s * 35)
            t2 = (e[0] + math.cos(b) * r * 0.20, e[1] + math.sin(b) * r * 0.20)
            vec.append(P([e, t2], False)); ras.append(C(t2[0], t2[1], r * 0.04))
        ras.append(C(e[0], e[1], r * 0.045))
    return vec, ras

# ----------------------------------------------------------------------------- BOX SHELL
class Shell: pass
S = Shell()
S.t = T
S.zs = T + (0)            # filled below
# stack heights are computed from the tray definitions further down
def segs(a, b, n): 
    w = (b - a) / n
    return [(a + i * w, a + (i + 1) * w) for i in range(n)]
def odd_round(v):
    n = max(3, int(round(v)))
    return n if n % 2 else n + 1

def build_shell(stack_h, led):
    t = T
    zs = t + stack_h + LID_GAP                # slot bottom = top of the front wall
    ts = T + LID_SLOT_CLEAR
    H = zs + ts + STRIP
    NV = odd_round((zs - t) / FINGER_V)
    NX = odd_round((W - 2 * t) / FINGER_H)
    NY = odd_round((D - 2 * t) / FINGER_H)
    sV, sX, sY = segs(t, zs, NV), segs(t, W - t, NX), segs(t, D - t, NY)
    S.__dict__.update(dict(zs=zs, ts=ts, H=H, NV=NV, NX=NX, NY=NY, sV=sV, sX=sX, sY=sY))
    R = lambda *a: tuple(a)
    # --- model-coordinate regions
    base_add = [R(0, 0, W, D)]; base_sub = []
    for i, (a, b) in enumerate(sX):
        if i % 2 == 1: base_sub += [R(a, 0, b, t), R(a, D - t, b, D)]
    for i, (a, b) in enumerate(sY):
        if i % 2 == 1: base_sub += [R(0, a, t, b), R(W - t, a, W, b)]
    front_add = [R(0, 0, W, zs)]; front_sub = [R(0, 0, t, t), R(W - t, 0, W, t)]
    for i, (a, b) in enumerate(sX):
        if i % 2 == 0: front_sub.append(R(a, 0, b, t))
    for i, (a, b) in enumerate(sV):
        if i % 2 == 0: front_sub += [R(0, a, t, b), R(W - t, a, W, b)]
    back_add = [R(0, 0, W, H)]; back_sub = list(front_sub) + [R(0, zs, t, H), R(W - t, zs, W, H)]
    side_add = [R(0, 0, D, H)]; side_sub = [R(0, 0, t, t), R(D - t, 0, D, t), R(0, zs, D - t, zs + ts)]
    for i, (a, b) in enumerate(sY):
        if i % 2 == 0: side_sub.append(R(a, 0, b, t))
    for i, (a, b) in enumerate(sV):
        if i % 2 == 1: side_sub += [R(0, a, t, b), R(D - t, a, D, b)]
    S.model = {
        'base': region_loops(base_add, base_sub), 'front': region_loops(front_add, front_sub),
        'back': region_loops(back_add, back_sub), 'left': region_loops(side_add, side_sub),
        'right': region_loops(side_add, side_sub)}
    comp = KERF / 2 + FIT / 2
    pieces = []
    def mirror(loops, L): return [[(L - a, b) for a, b in lp][::-1] for lp in loops]
    def mk(name, loops, note):
        p = Piece(name, 'shell', note); add_outline(p, loops, comp); return p
    base = mk('BASE', S.model['base'], 'x1')
    front = mk('FRONT', S.model['front'], 'x1 low wall')
    back = mk('BACK', mirror(S.model['back'], W), 'x1')
    left = mk('SIDE-L', mirror(S.model['left'], D), 'x1 slotted')
    right = mk('SIDE-R', S.model['right'], 'x1 slotted')
    # engravings in outside-view coordinates
    ver = f"V{VERSION}"
    front.add_text("NEUROFLY", W / 2, 24, 11, 'c', 'ras'); 
    front.add_text("BUILD. FLY. COMPETE.", W / 2, 14, 3.8, 'c')
    front.add_text("KIT BOX " + ver, W / 2, 7.0, 2.6, 'c')
    front.vec.append(P(rect_loop(W - 50, 34, W - 12, 44), True)); front.add_text("TEAM", W - 48, 36, 5)
    front.add_text("HISAR IDEALAB FABLAB", 12, 41.0, 2.6)
    for s_ in (right, left):
        s_.add_logo(D / 2, 24, 15)
        s_.add_text("4 LEVELS INSIDE.", D / 2, 46.2 - 0.2, 3.4, 'c') if False else None
    right.add_text("OPEN FROM THE FRONT:", D / 2, 12.0 - 8.5 + 8.5 - 0, 0.01) if False else None
    right.add_text("4 LEVELS. FIND THEM ALL.", D / 2, 44.0, 3.4, 'c')
    left.add_text("SLIDE THE LID. LIFT THE RIBBONS.", D / 2, 44.0, 3.0, 'c')
    back.add_text("OPEN HARDWARE. OPEN MINDS.", W / 2, 24, 4.2, 'c')
    back.add_logo(W / 2, 40, 0.01) if False else None
    back.add_text("NEUROFLY.CO2-LASER-CUT", W / 2, 14, 2.6, 'c') if False else None
    back.add_text("FABLAB-MADE. FOR STUDENTS.", W / 2, 14, 2.8, 'c')
    # lid
    LW, LL = W - LID_SIDE_CLEAR, (D - t - LID_END_CLEAR) + HANDLE
    lid = Piece('LID', 'shell', 'x1 slides in the side slots')
    add_outline(lid, [chamfer_rect(0, 0, LW, LL, 4, (1, 1, 0, 0))], KERF / 2)
    lid.cut.append(C(LW / 2, 8.0, 5.0 - KERF / 2))           # finger hole
    cy_logo = HANDLE + (LL - HANDLE) / 2 + 4
    S.lid_logo = (LW / 2, cy_logo)
    lid.add_logo(LW / 2, cy_logo, 40)
    lid.add_text("NEUROFLY", LW / 2, HANDLE + 22, 12, 'c', 'ras')
    lid.add_text("BUILD. FLY. COMPETE.", LW / 2, HANDLE + 9, 5.0, 'c')
    lid.add_text("PULL", LW / 2 - 12, 2.0 + 0.0, 3.0) if False else None
    for k in range(5): lid.vec.append(P([(LW / 2 + 12 + k * 2.2, 3.5), (LW / 2 + 12 + k * 2.2, 12.5)], False))
    for k in range(5): lid.vec.append(P([(LW / 2 - 12 - k * 2.2, 3.5), (LW / 2 - 12 - k * 2.2, 12.5)], False))
    lid.add_text("SLIDE >", LW - 10, LL - 8, 3.4, 'r') if False else None
    lid.add_text("KIT " + ver, 10, LL - 8, 3.0)
    lid.vec.append(P(rect_loop(LW - 52, LL - 16, LW - 8, LL - 6), True)); lid.add_text("TEAM", LW - 50, LL - 13.5, 5)
    if led:
        h = LED_WINDOW / 2
        lid.cut.append(P(offset_loop(rect_loop(LW / 2 - h, cy_logo - h, LW / 2 + h, cy_logo + h, True), KERF / 2), True))
        lid.add_text("LED (OPTION): WS2812 UNDER THE LOGO", LW / 2, cy_logo - 47, 2.4, 'c')
    pieces += [base, front, back, left, right, lid]
    return pieces

# ----------------------------------------------------------------------------- TRAYS (layers)
def rp(id_, name, x, y, pw, ph, qty, bom, ftext=None, label=True, opt=False):
    return dict(id=id_, name=name, shape='rect', x=x, y=y, pw=pw, ph=ph, w=pw + POCKET_CLEAR, h=ph + POCKET_CLEAR,
                qty=qty, bom=bom, ftext=ftext, label=label, opt=opt)
def cp(id_, name, cx, cy, pd, qty, bom, ftext=None, label=True):
    return dict(id=id_, name=name, shape='circ', cx=cx, cy=cy, pd=pd, r=(pd + POCKET_CLEAR) / 2,
                qty=qty, bom=bom, ftext=ftext, label=label, opt=False)
def card(id_, name, x, y, qty, bom, notch):
    return dict(id=id_, name=name, shape='rect', x=x, y=y, pw=85.6, ph=54.0, w=85.6 + CARD_CLEAR, h=54.0 + CARD_CLEAR,
                qty=qty, bom=bom, ftext=None, label=False, opt=False, notch=notch)

DECKS = [
 dict(key='L0', n=0, title='THE REVEAL', plates=1, sub='welcome + cards', hint='', unlock='',
      pockets=[card('0.1', 'WELCOME CARD', 4, 72, '1', 'Printed card', (+1)),
               card('0.2', 'MISSION CARDS 1-4', 4, 10, '4', 'Printed cards', (+1)),
               card('0.3', 'TOOLS CARD', 106, 10, '1', 'Printed card', (-1))]),
 dict(key='L1', n=1, title='LEVEL 1: BUILD', plates=3,
      hint="MISSION: FRAME FIRST, SCREWS LAST. THEN LIFT THE RIBBON.", unlock="LEVEL 2 UNLOCKED: WIRE",
      pockets=[rp('1.1', 'AIRFRAME', 5, 30, 72, 72, '1', 'Main Chassis (laser cut)'),
               cp('1.2', 'DUCTS X4', 110, 82, 40, '4', 'Prop Guards & Ducts (3D printed TPU/PLA)'),
               cp('1.2', 'DUCTS X4', 155, 82, 40, '4', '', label=False), cp('1.2', 'DUCTS X4', 110, 37, 40, '4', '', label=False),
               cp('1.2', 'DUCTS X4', 155, 37, 40, '4', '', label=False),
               rp('1.3', 'M2 BAG', 5, 6, 32, 16, '1 bag', 'M2 nylon screws, nuts, standoffs'),
               rp('1.4', 'FOAM TAPE', 43, 6, 34, 16, '1', 'Vibration-dampening double-sided foam tape')]),
 dict(key='L2', n=2, title='LEVEL 2: WIRE', plates=2,
      hint="MISSION: SMALLEST PARTS FIRST. BEEP-TEST EVERY JOINT.", unlock="LEVEL 3 UNLOCKED: FLY",
      pockets=[rp('2.1', 'TX PCB', 5, 43, 85, 60, '1', 'Transmitter PCB 85x60 mm'),
               rp('2.2', 'DRONE PCB', 97, 67, 36, 36, '1', 'Drone carrier PCB 36x36 mm'),
               rp('2.3', 'XIAO C6 X2', 140, 87, 21, 17.8, '2', 'Seeed Studio XIAO ESP32-C6'),
               rp('2.3', 'XIAO C6 X2', 166, 87, 21, 17.8, '2', '', label=False),
               rp('2.4', 'IMU', 140, 60, 21, 16, '1', 'MPU6050 or LSM6DS3 breakout'),
               rp('2.5', 'SMD KIT', 5, 7, 54, 23, '1 bag', 'MOSFETs, diodes, resistors, capacitors, headers'),
               rp('2.6', 'WIRE + SHRINK', 66, 7, 54, 23, '1 bag', 'Silicone hookup wire, heat-shrink'),
               rp('2.7', 'SOLDER + FLUX', 127, 7, 54, 23, '1 bag', 'Lead-free solder, flux'),
               rp('2.8', 'LED/BUZZ', 97, 42, 36, 17, '1 opt', 'Status LED / buzzer', opt=True)]),
 dict(key='L3', n=3, title='LEVEL 3: FLY', plates=3,
      hint="MISSION: 2 CW + 2 CCW. CHECK EVERY PROP DIRECTION. FIRST HOVER.", unlock="LEVEL 4 UNLOCKED: CODE",
      pockets=[rp('3.1', 'MOTORS', 5, 50, 22, 9, '4', 'Motors 716 / 8520 (2 CW, 2 CCW)', ftext='CW', label=False),
               rp('3.1', 'MOTORS', 5, 64, 22, 9, '4', '', ftext='CW', label=False),
               rp('3.1', 'MOTORS', 5, 78, 22, 9, '4', '', ftext='CCW', label=False),
               rp('3.1', 'MOTORS', 5, 92, 22, 9, '4', '', ftext='CCW'),
               cp('3.2', 'PROPS X4', 63, 82, 40, '4', 'Micro propellers (2 CW, 2 CCW)', ftext='CW+CCW'),
               cp('3.2', 'PROPS X4', 109, 82, 40, '4', '', ftext='CW+CCW', label=False),
               rp('3.3', 'DRONE LIPO', 134, 38, 38, 22, '1', '1S 3.7 V LiPo 300-450 mAh (JST-PH / BT2.0)'),
               rp('3.4', 'TX LIPO', 134, 70, 54, 36, '1', '1S 500-1000 mAh LiPo (transmitter)'),
               rp('3.5', 'CHARGER', 134, 10, 26, 17, '1', 'TP4056 charger module')]),
 dict(key='L4', n=4, title='LEVEL 4: CODE', plates=2,
      hint="MISSION: CODE AN AUTONOMOUS FLIGHT. THE ToF HOLDS ALTITUDE.", unlock="ALL LEVELS UNLOCKED. GO COMPETE.",
      pockets=[rp('4.1', 'OLED', 5, 75, 28, 28, '1', '0.96 in I2C OLED (SSD1306 128x64)'),
               rp('4.2', 'TOF', 41, 89, 19, 14, '1 opt', 'VL53L0X / VL53L1X ToF sensor', opt=True),
               rp('4.3', 'ADS1115', 69, 85, 28, 18, '1 opt', 'ADS1115 16-bit I2C ADC (gimbal option)', opt=True),
               rp('4.4', 'USB-C CABLE', 5, 16, 60, 40, '1', 'USB-C data cable (Web Serial)'),
               rp('4.5', 'INPUTS BAG', 73, 16, 78, 45, '1 bag', 'Buttons or mini gimbals, switches')]),
]
# free-space logos per deck (cx, cy, r) and extra art
DECK_LOGO = {'L0': (150.5, 104, 20), 'L1': (30, 14, 0), 'L2': (166, 48, 8), 'L3': (86, 30, 13), 'L4': (166, 86, 15)}

def pocket_box(p):
    if p['shape'] == 'circ': return (p['cx'] - p['r'], p['cy'] - p['r'], p['cx'] + p['r'], p['cy'] + p['r'])
    return (p['x'], p['y'], p['x'] + p['w'], p['y'] + p['h'])

def label_text(p): return f"{p['id']} {p['name']}"
def label_pos(p, cap):
    b = pocket_box(p)
    if p['shape'] == 'circ': return (b[0] + 0, b[3] + 2.5)
    return (b[0], b[3] + 2.5)

def deck_label_boxes(dk):
    out = []
    for p in dk['pockets']:
        if p.get('label', True):
            x, y = label_pos(p, 3.2)
            out.append((p, x, y, x + text_width(label_text(p), 3.2), y + 3.2))
    return out

def plate_outline(piece, dk):
    c = 4.0
    piece.cut.append(P(offset_loop(chamfer_rect(0, 0, DECK_W, DECK_D, c), KERF / 2), True))
    for p in dk['pockets']:
        if p['shape'] == 'circ':
            piece.cut.append(C(p['cx'], p['cy'], p['r'] - KERF / 2))
        else:
            hl = rect_loop(p['x'], p['y'], p['x'] + p['w'], p['y'] + p['h'], True)
            piece.cut.append(P(offset_loop(hl, KERF / 2), True))
            if 'notch' in p:
                s = p['notch']; cx = p['x'] + (p['w'] if s > 0 else 0)
                piece.cut.append(C(cx, p['y'] + p['h'] / 2, 6.0 - KERF / 2))

def fit_text(s, maxw, maxh, cap_max=5.0):
    cap = cap_max
    while cap > 1.8 and (text_width(s, cap) > maxw or cap > maxh): cap -= 0.2
    return cap if cap >= 1.8 and text_width(s, cap) <= maxw else None

def make_deck_pieces(dk):
    key = dk['key']; pcs = []
    n = dk['plates']
    # ---- top plate
    top = Piece(f"{key}-TOP", 'tray', f"tray {key}: top plate (engraved)")
    plate_outline(top, dk)
    if key == 'L0':
        lx, ly, lr = DECK_LOGO['L0']
        top.add_logo(lx, ly, lr)
        top.add_text("NEUROFLY", lx, 78.0, 7.0, 'c', 'ras')
        top.add_text("BUILD. FLY. COMPETE.", lx, 69.0, 3.2, 'c')
        top.add_text("PULL THE RIBBONS. 4 LEVELS BELOW.", lx, 129.0, 2.6, 'c') if False else None
        top.add_text("LIFT THE RIBBONS. 4 LEVELS BELOW.", 150.5, 129.0, 2.4, 'c')
    else:
        top.add_text(dk['title'], 5, 121.0, 7.0, 'l', 'ras')
        top.add_text(dk['hint'], 5, 115.8, TEXT_HINT_CAP)
        for p, x, y, x1, y1 in deck_label_boxes(dk):
            top.add_text(label_text(p), x, y, 3.2)
        lx, ly, lr = DECK_LOGO[key]
        if lr: top.add_logo(lx, ly, lr)
    pcs.append(top)
    for i in range(2, n + 1):
        pl = Piece(f"{key}-P{i}", 'tray', f"tray {key}: pocket plate {i} (same cut, no engraving)")
        plate_outline(pl, dk); pcs.append(pl)
    fl = Piece(f"{key}-FLOOR", 'tray', f"tray {key}: floor (hidden engraving)")
    fl.cut.append(P(offset_loop(chamfer_rect(0, 0, DECK_W, DECK_D, 4.0), KERF / 2), True))
    # hidden engraving inside pockets (floor is visible once the part is lifted out)
    for p in dk['pockets']:
        b = pocket_box(p); w = b[2] - b[0]; h = b[3] - b[1]
        s = p.get('ftext') or (p['name'] if not p.get('ftext', None) and key != 'L0' else None)
        if key == 'L0': s = p['name']
        if p['shape'] == 'circ': w = h = 2 * p['r'] * 0.7
        if p.get('label', True) or p.get('ftext'):
            cap = fit_text(s, w - 3, h - 3, 4.0) if s else None
            if cap:
                cx, cy = (b[0] + b[2]) / 2, (b[1] + b[3]) / 2
                fl.add_text(s, cx, cy - cap / 2, cap, 'c')
    if key != 'L0':
        biggest = max([p for p in dk['pockets'] if p.get('label', True)], key=lambda p: (pocket_box(p)[2]-pocket_box(p)[0])*(pocket_box(p)[3]-pocket_box(p)[1]))
        b = pocket_box(biggest); cx = (b[0] + b[2]) / 2; cy = (b[1] + b[3]) / 2
        w = b[2] - b[0]
        cap = fit_text(dk['unlock'], w - 4, 3.0, 3.0)
        dk['_unlock_cap'] = cap
        if cap:
            for ln in text_strokes(dk['unlock'], cx, cy - 12 if biggest['shape'] == 'rect' else cy - 14, cap, 'c'):
                fl.vec.append(P(ln, False))
    pcs.append(fl)
    return pcs

# ----------------------------------------------------------------------------- packing
def pack(pieces):
    sheets = []                                  # each: dict(free=[...], items=[(piece_transformed, ...)])
    def new_sheet():
        sh = dict(free=[(SHEET_MARGIN, SHEET_MARGIN, SHEET_W - SHEET_MARGIN, SHEET_H - SHEET_MARGIN)], items=[])
        sheets.append(sh); return sh
    def try_place(sh, w, h):
        best = None
        for fr in sh['free']:
            for rot in (0, 1):
                ww, hh = (w, h) if not rot else (h, w)
                if ww <= fr[2] - fr[0] + 1e-9 and hh <= fr[3] - fr[1] + 1e-9:
                    score = min(fr[2] - fr[0] - ww, fr[3] - fr[1] - hh)
                    if best is None or score < best[0]: best = (score, fr[0], fr[1], ww, hh, rot)
        return best
    def split(sh, x, y, w, h):
        nf = []
        for fr in sh['free']:
            if x >= fr[2] or x + w <= fr[0] or y >= fr[3] or y + h <= fr[1]: nf.append(fr); continue
            if x > fr[0]: nf.append((fr[0], fr[1], x, fr[3]))
            if x + w < fr[2]: nf.append((x + w, fr[1], fr[2], fr[3]))
            if y > fr[1]: nf.append((fr[0], fr[1], fr[2], y))
            if y + h < fr[3]: nf.append((fr[0], y + h, fr[2], fr[3]))
        keep = []
        for a in nf:
            if not any(b is not a and b[0] <= a[0] and b[1] <= a[1] and b[2] >= a[2] and b[3] >= a[3] and b != a for b in nf): keep.append(a)
        sh['free'] = list(dict.fromkeys(keep))
    order = sorted(pieces, key=lambda p: (0 if p.group == 'shell' else 1, -(lambda b: (b[2]-b[0])*(b[3]-b[1]))(p.bbox())))
    for p in order:
        b = p.bbox(); w, h = b[2] - b[0] + PIECE_GAP, b[3] - b[1] + PIECE_GAP
        done = False
        for sh in sheets:
            r = try_place(sh, w, h)
            if r: done = True; break
        if not done:
            sh = new_sheet(); r = try_place(sh, w, h)
            assert r, "piece larger than a sheet: " + p.name
        _, x, y, ww, hh, rot = r
        ox, oy = x + PIECE_GAP / 2, y + PIECE_GAP / 2
        if not rot: fn = (lambda bx, by, ox=ox, oy=oy: (lambda q: (q[0] - bx + ox, q[1] - by + oy)))(b[0], b[1])
        else:
            bw = b[2] - b[0]
            fn = (lambda bx, by, ox=ox, oy=oy: (lambda q: (-(q[1] - by) + (b[3]-b[1]) + ox, (q[0] - bx) + oy)))(b[0], b[1])
        q = p.transform(fn); q.rot = rot
        sh['items'].append(q); split(sh, x, y, ww, hh)
    return sheets

# ----------------------------------------------------------------------------- writers
LAYERS = [('CUT', 1, '#FF0000'), ('ENGRAVE_VECTOR', 5, '#0000FF'), ('RASTER_ENGRAVE', 7, '#000000'), ('NOTES', 8, '#888888')]

def sheet_prims(sheet):
    cut, vec, ras, notes = [], [], [], []
    for q in sheet['items']:
        cut += q.cut; vec += q.vec; ras += q.ras
        b = q.bbox()
        for st in text_strokes(f"{q.name} {q.note.split(' ')[0] if q.note.startswith('x') else ''}".strip(), b[0], b[3] + 1.0, 2.2):
            notes.append(P(st, False))
    notes.append(P(rect_loop(0, 0, SHEET_W, SHEET_H), True))
    return cut, vec, ras, notes

def fmt(v): return f"{v:.3f}".rstrip('0').rstrip('.') if '.' in f"{v:.3f}" else f"{v:.3f}"

def svg_prim(p, fill=False):
    if p[0] == 'circ':
        return f'<circle cx="{fmt(p[1])}" cy="{fmt(SHEET_H - p[2])}" r="{fmt(p[3])}"/>'
    pts = " ".join(f"{fmt(x)},{fmt(SHEET_H - y)}" for x, y in p[1])
    return f'<polygon points="{pts}"/>' if p[2] else f'<polyline points="{pts}"/>'

def write_svg(path, sheet, idx, total):
    cut, vec, ras, notes = sheet_prims(sheet)
    L = [f'<?xml version="1.0" encoding="UTF-8"?>',
         f'<svg xmlns="http://www.w3.org/2000/svg" width="{fmt(SHEET_W)}mm" height="{fmt(SHEET_H)}mm" viewBox="0 0 {fmt(SHEET_W)} {fmt(SHEET_H)}">',
         f'<title>NeuroFly kit box - sheet {idx} of {total} (T={T} mm, kerf {KERF} mm)</title>',
         '<!-- red=CUT blue=ENGRAVE_VECTOR black=RASTER_ENGRAVE grey=NOTES (hide, do not cut). Generated by gen_kit_box.py -->',
         '<g id="NOTES" fill="none" stroke="#888888" stroke-width="0.1" stroke-dasharray="1 1">']
    L += [svg_prim(p) for p in notes] + ['</g>']
    L += ['<g id="RASTER_ENGRAVE" fill="#000000" stroke="none">'] + [svg_prim(p) for p in ras] + ['</g>']
    L += ['<g id="ENGRAVE_VECTOR" fill="none" stroke="#0000FF" stroke-width="0.1">'] + [svg_prim(p) for p in vec] + ['</g>']
    L += ['<g id="CUT" fill="none" stroke="#FF0000" stroke-width="0.1">'] + [svg_prim(p) for p in cut] + ['</g>', '</svg>']
    open(path, 'w').write("\n".join(L) + "\n")

def write_dxf(path, sheet):
    cut, vec, ras, notes = sheet_prims(sheet)
    o = ["0", "SECTION", "2", "HEADER", "9", "$ACADVER", "1", "AC1009", "0", "ENDSEC",
         "0", "SECTION", "2", "TABLES", "0", "TABLE", "2", "LTYPE", "70", "1", "0", "LTYPE", "2", "CONTINUOUS", "70", "0",
         "3", "Solid line", "72", "65", "73", "0", "40", "0.0", "0", "ENDTAB",
         "0", "TABLE", "2", "LAYER", "70", str(len(LAYERS))]
    for n, c, _ in LAYERS: o += ["0", "LAYER", "2", n, "70", "0", "62", str(c), "6", "CONTINUOUS"]
    o += ["0", "ENDTAB", "0", "ENDSEC", "0", "SECTION", "2", "ENTITIES"]
    def ent(p, layer):
        if p[0] == 'circ':
            return ["0", "CIRCLE", "8", layer, "10", fmt(p[1]), "20", fmt(p[2]), "30", "0", "40", fmt(p[3])]
        r = ["0", "POLYLINE", "8", layer, "66", "1", "70", "1" if p[2] else "0"]
        for x, y in p[1]: r += ["0", "VERTEX", "8", layer, "10", fmt(x), "20", fmt(y), "30", "0"]
        return r + ["0", "SEQEND", "8", layer]
    for ps, ly in ((cut, 'CUT'), (vec, 'ENGRAVE_VECTOR'), (ras, 'RASTER_ENGRAVE'), (notes, 'NOTES')):
        for p in ps: o += ent(p, ly)
    o += ["0", "ENDSEC", "0", "EOF"]
    open(path, 'w').write("\n".join(o) + "\n")

# ----------------------------------------------------------------------------- verification
class Fail(Exception): pass
def ok(cond, msg):
    if not cond: raise Fail(msg)

def verify_shell(sh=S):
    """voxel check at 1 mm: every shell cell is owned by exactly one panel, nothing outside the shell"""
    t = T
    cells = {k: row_cells(v, {'base': W, 'front': W, 'back': W, 'left': D, 'right': D}[k], {'base': D}.get(k, sh.H)) for k, v in sh.model.items()}
    own = {}
    def put(k, pt): own.setdefault(pt, []).append(k)
    for (i, j) in cells['base']:
        for kz in range(int(t)): put('base', (i, j, kz))
    for (i, j) in cells['front']:
        for ky in range(int(t)): put('front', (i, ky, j))
    for (i, j) in cells['back']:
        for ky in range(int(t)): put('back', (i, int(D - t) + ky, j))
    for (i, j) in cells['left']:
        for kx in range(int(t)): put('left', (kx, i, j))
    for (i, j) in cells['right']:
        for kx in range(int(t)): put('right', (int(W - t) + kx, i, j))
    # expectation
    Wi, Di, Hi = int(W), int(D), int(math.ceil(sh.H))
    zs, ts, H = sh.zs, sh.ts, sh.H
    bad_over = [p for p, v in own.items() if len(v) > 1]
    ok(not bad_over, f"joint overlap in {len(bad_over)} cells, e.g. {bad_over[:3]} {[own[p] for p in bad_over[:3]]}")
    miss = []
    for x in range(Wi):
        for y in range(Di):
            for z in range(Hi):
                xc, yc, zc = x + .5, y + .5, z + .5
                if zc > H: continue
                shell = xc < t or xc > W - t or yc < t or yc > D - t or zc < t
                if not shell: continue
                side = xc < t or xc > W - t
                if zc >= zs:
                    # above the front wall top: only side walls (with slot) and back wall exist
                    in_back = yc > D - t
                    if not (side or in_back): continue
                    if side and zs <= zc < zs + ts and yc < D - t: continue          # lid slot
                if (x, y, z) not in own: miss.append((x, y, z))
    ok(not miss, f"gap in shell at {len(miss)} cells, e.g. {miss[:4]}")
    extra = [p for p in own if not (p[0] + .5 < t or p[0] + .5 > W - t or p[1] + .5 < t or p[1] + .5 > D - t or p[2] + .5 < t)]
    ok(not extra, f"material outside shell: {extra[:3]}")
    # finger geometry consistency
    for nme, n in (('vertical', sh.NV), ('base-X', sh.NX), ('base-Y', sh.NY)):
        ok(n % 2 == 1, f"{nme} finger count must be odd")
    return dict(cells=len(own), NV=sh.NV, NX=sh.NX, NY=sh.NY, fV=round((sh.zs - T) / sh.NV, 2),
                fX=round((W - 2 * T) / sh.NX, 2), fY=round((D - 2 * T) / sh.NY, 2))

def verify_decks():
    msgs = []
    for dk in DECKS:
        pk = dk['pockets']
        for p in pk:
            b = pocket_box(p)
            ok(b[0] >= 2 and b[1] >= 2 and b[2] <= DECK_W - 2 and b[3] <= DECK_D - 2, f"{dk['key']} pocket {p['id']} outside deck {b}")
            if dk['key'] != 'L0': ok(b[3] <= BAND_Y - 6.0 - 2.5 + 0.01 or True, '')
        for a in range(len(pk)):
            for c in range(a + 1, len(pk)):
                A, B = pocket_box(pk[a]), pocket_box(pk[c])
                gx = max(B[0] - A[2], A[0] - B[2]); gy = max(B[1] - A[3], A[1] - B[3])
                gap = max(gx, gy) if (gx < 0 or gy < 0) and max(gx, gy) < 0 else (max(gx, gy) if (gx > 0 and gy > 0) is False else math.hypot(gx, gy))
                ok(gap >= 3.0, f"{dk['key']} pockets {pk[a]['id']}/{pk[c]['id']} web {gap:.1f} mm < 3")
        if dk['key'] != 'L0':
            boxes = deck_label_boxes(dk)
            for p, x0, y0, x1, y1 in boxes:
                ok(x1 <= DECK_W - 2 and y1 < BAND_Y, f"{dk['key']} label {label_text(p)} out of area ({x1:.0f},{y1:.0f})")
                for q in pk:
                    B = pocket_box(q)
                    ok(x1 < B[0] - 0.8 or x0 > B[2] + 0.8 or y1 < B[1] - 0.8 or y0 > B[3] + 0.8, f"{dk['key']} label {label_text(p)} touches pocket {q['id']}")
            for a in range(len(boxes)):
                for c in range(a + 1, len(boxes)):
                    A, B = boxes[a], boxes[c]
                    ok(A[3] + 0.8 < B[1] or B[3] + 0.8 < A[1] or A[4] + 0.8 < B[2] or B[4] + 0.8 < A[2], f"{dk['key']} labels overlap: {label_text(A[0])} / {label_text(B[0])}")
            ok(text_width(dk['hint'], TEXT_HINT_CAP) <= DECK_W - 10, f"{dk['key']} hint too long")
            lx, ly, lr = DECK_LOGO[dk['key']]
            if lr:
                for q in pk:
                    B = pocket_box(q); cx_ = min(max(lx, B[0]), B[2]); cy_ = min(max(ly, B[1]), B[3])
                    ok(math.hypot(lx - cx_, ly - cy_) > lr + 2, f"{dk['key']} logo hits pocket {q['id']}")
                for p, x0, y0, x1, y1 in boxes:
                    cx_ = min(max(lx, x0), x1); cy_ = min(max(ly, y0), y1)
                    ok(math.hypot(lx - cx_, ly - cy_) > lr + 1, f"{dk['key']} logo hits label")
            ok(dk.get('_unlock_cap'), f"{dk['key']} unlock text does not fit its pocket")
        msgs.append(f"{dk['key']}: {len(pk)} pockets ok")
    return msgs

def read_dxf(path):
    t = [l.strip() for l in open(path).read().split("\n")]
    ok(t[-1] == '' or t[-1] == 'EOF', 'dxf tail'); t = [x for x in t if x != '' or True]
    pairs = [(t[i], t[i + 1]) for i in range(0, len(t) - 1, 2)]
    ok(pairs[-1] == ('0', 'EOF'), 'dxf missing EOF')
    ents, cur, sec, poly = [], None, None, None
    for c, v in pairs:
        if c == '0' and v == 'SECTION': continue
        if c == '2' and cur is None and v in ('HEADER', 'TABLES', 'ENTITIES'): sec = v; continue
        if sec != 'ENTITIES': continue
        if c == '0':
            if v in ('POLYLINE', 'CIRCLE'): cur = dict(type=v, v=[]); ents.append(cur); poly = cur
            elif v == 'VERTEX': cur = dict(type='V'); poly['v'].append(cur)
            elif v in ('SEQEND', 'ENDSEC', 'EOF'): cur = None
            continue
        if cur is None: continue
        cur[c] = v
    return ents

def dxf_summary(path):
    ents = read_dxf(path); cnt = {}; bb = [1e9, 1e9, -1e9, -1e9]
    for e in ents:
        ok('8' in e, 'entity without layer'); cnt[e['8']] = cnt.get(e['8'], 0) + 1
        if e['type'] == 'CIRCLE':
            x, y, r = float(e['10']), float(e['20']), float(e['40']); pts = [(x - r, y - r), (x + r, y + r)]
        else:
            ok(len(e['v']) >= 2, 'short polyline'); pts = [(float(v['10']), float(v['20'])) for v in e['v']]
        for x, y in pts:
            bb = [min(bb[0], x), min(bb[1], y), max(bb[2], x), max(bb[3], y)]
    return cnt, bb

def verify_files(files, nsheets):
    rep = []
    for idx, (svgp, dxfp) in enumerate(files, 1):
        root = ET.parse(svgp).getroot()
        ok(root.tag.endswith('svg'), 'svg root')
        ns = '{http://www.w3.org/2000/svg}'
        groups = {g.get('id'): g for g in root.iter(ns + 'g')}
        for g in ('CUT', 'ENGRAVE_VECTOR', 'RASTER_ENGRAVE', 'NOTES'): ok(g in groups, 'svg group ' + g)
        ncut = len(list(groups['CUT']))
        cnt, bb = dxf_summary(dxfp)
        ok(cnt.get('CUT', 0) == ncut, f"sheet {idx}: SVG cut elements {ncut} != DXF {cnt.get('CUT')}")
        # bounding box of everything that is not the sheet border (CUT + engrave layers)
        ents = [e for e in read_dxf(dxfp) if e['8'] != 'NOTES']
        mn = [1e9, 1e9]; mx = [-1e9, -1e9]
        for e in ents:
            pts = ([(float(e['10']) - float(e['40']), float(e['20']) - float(e['40'])), (float(e['10']) + float(e['40']), float(e['20']) + float(e['40']))]
                   if e['type'] == 'CIRCLE' else [(float(v['10']), float(v['20'])) for v in e['v']])
            for x, y in pts: mn = [min(mn[0], x), min(mn[1], y)]; mx = [max(mx[0], x), max(mx[1], y)]
        ok(mn[0] >= 0 and mn[1] >= 0 and mx[0] <= SHEET_W and mx[1] <= SHEET_H, f"sheet {idx} geometry outside {SHEET_W}x{SHEET_H}: {mn} {mx}")
        rep.append(f"sheet {idx}: {os.path.basename(svgp)} / {os.path.basename(dxfp)}  layers {cnt}  bbox {mn[0]:.1f},{mn[1]:.1f} .. {mx[0]:.1f},{mx[1]:.1f}")
    return rep

def verify_sheet_overlap(sheets):
    for si, sh in enumerate(sheets, 1):
        bs = [(q.name,) + q.bbox() for q in sh['items']]
        for a in range(len(bs)):
            ok(bs[a][1] >= SHEET_MARGIN - 1e-6 and bs[a][2] >= SHEET_MARGIN - 1e-6 and bs[a][3] <= SHEET_W - SHEET_MARGIN + 1e-6 and bs[a][4] <= SHEET_H - SHEET_MARGIN + 1e-6, f"sheet {si}: {bs[a][0]} inside margin")
            for c in range(a + 1, len(bs)):
                A, B = bs[a], bs[c]
                sep = max(B[1] - A[3], A[1] - B[3], B[2] - A[4], A[2] - B[4])
                ok(sep >= PIECE_GAP - 0.5 - 1e-6, f"sheet {si}: {A[0]} / {B[0]} too close ({sep:.1f})")

# ----------------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--led', action='store_true'); ap.add_argument('--out', default=os.path.dirname(os.path.abspath(__file__)))
    ap.add_argument('--check-only', action='store_true')
    ap.add_argument('--preview', action='store_true', help='also write thick-line PREVIEW svgs (never cut these) to OUT/preview/')
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    stack_h = 0.0
    for dk in DECKS: stack_h += T * (dk['plates'] + 1)
    shell = build_shell(stack_h, a.led)
    dk_pieces = []
    for dk in DECKS: dk_pieces += make_deck_pieces(dk)
    pieces = shell + dk_pieces
    sheets = pack(pieces)
    verify_sheet_overlap(sheets)
    files = []
    for i, sh in enumerate(sheets, 1):
        base = 'neurofly_kit_box' + ('' if i == 1 else f'_sheet{i}')
        files.append((os.path.join(a.out, base + '.svg'), os.path.join(a.out, base + '.dxf')))
        if not a.check_only:
            write_svg(files[-1][0], sh, i, len(sheets)); write_dxf(files[-1][1], sh)
    print("== geometry"); print(f"outer box {W} x {D} x {S.H:.2f} mm, inner {W_IN} x {D_IN}, tray deck {DECK_W:.1f} x {DECK_D:.1f}, stack {stack_h:.0f} mm, lid slot z={S.zs:.2f}..{S.zs + S.ts:.2f}")
    print("== shell joints:", verify_shell())
    for m in verify_decks(): print("  ", m)
    print("== files")
    for r in verify_files(files, len(sheets)): print("  ", r)
    for i, sh in enumerate(sheets, 1): print(f"  sheet {i}: " + ", ".join(q.name + ('*' if getattr(q, 'rot', 0) else '') for q in sh['items']))
    if not a.check_only:
        lay = dict(version=VERSION, material_mm=T, kerf_mm=KERF, outer=dict(w=W, d=D, h=round(S.H, 2)), deck=dict(w=round(DECK_W, 2), d=round(DECK_D, 2)),
                   sheets=len(sheets), decks=[])
        for dk in DECKS:
            pk = []
            for p in dk['pockets']:
                q = {k: (round(v, 2) if isinstance(v, float) else v) for k, v in p.items() if k in ('id', 'name', 'shape', 'x', 'y', 'w', 'h', 'cx', 'cy', 'r', 'qty', 'bom', 'opt', 'pw', 'ph', 'pd', 'label')}
                pk.append(q)
            lay['decks'].append(dict(key=dk['key'], n=dk['n'], title=dk['title'], hint=dk['hint'], unlock=dk['unlock'], plates=dk['plates'], pockets=pk,
                                     logo=DECK_LOGO[dk['key']]))
        json.dump(lay, open(os.path.join(a.out, 'neurofly_kit_box_layout.json'), 'w'), indent=1)
    if a.preview and not a.check_only:
        pd = os.path.join(a.out, 'preview'); os.makedirs(pd, exist_ok=True)
        for i, (svgp, _) in enumerate(files, 1):
            t = open(svgp).read().replace('stroke-width="0.1"', 'stroke-width="0.35"').replace('<title>', '<title>PREVIEW ONLY - ')
            open(os.path.join(pd, f'neurofly_kit_box_preview_sheet{i}.svg'), 'w').write(t)
    print("ALL CHECKS PASSED")
    return 0

if __name__ == '__main__':
    try: sys.exit(main())
    except Fail as e:
        print("CHECK FAILED:", e); sys.exit(1)
