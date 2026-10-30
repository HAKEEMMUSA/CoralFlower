/* SPDX-License-Identifier: GPL-3.0-only
 * Leviathan: the fallen angel (angel.h). Replaces flora.c in the unit (coralflower.c, the emulator), on the same
 * canvas primitives (flora_canvas.c) and palette (flora_pal.h). */
#include "angel.h"
#include "flora_lut.h"
#include "flora_pal.h"

/* ---- integer helpers ---------------------------------------------------------------------------------------- */
static int32_t an_sin(int32_t a)                 /* a: 1024 a turn; Q14 */
{
    a &= 1023;
    if (a < 256)
        return fl_qsin[a];
    if (a < 512)
        return fl_qsin[512 - a];
    if (a < 768)
        return -fl_qsin[a - 512];
    return -fl_qsin[1024 - a];
}
static int32_t an_cos(int32_t a) { return an_sin(a + 256); }
static uint32_t an_hash(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}
static uint32_t an_rnd(flora_t *f)
{
    uint32_t x = f->rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return f->rng = x;
}
static int32_t an_clamp(int32_t v, int32_t lo, int32_t hi) { return v < lo ? lo : v > hi ? hi : v; }
/* v toward t, a fraction dt / tau of the gap a step (at least a unit) */
static int32_t an_ease(int32_t v, int32_t t, uint32_t dt, int32_t tau)
{
    int32_t d = t - v, s;
    if (!d)
        return v;
    s = (int32_t)((int64_t)d * (int32_t)dt / (tau + (int32_t)dt));
    if (!s)
        s = d > 0 ? 1 : -1;
    return v + s;
}
static int32_t an_lerp(int32_t a, int32_t b, int32_t q12) { return a + (int32_t)(((int64_t)(b - a) * q12) >> 12); }

static const uint8_t an_bayer[16] = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5};
static int an_by(int x, int y) { return an_bayer[(y & 3) * 4 + (x & 3)]; }

/* ---- the fall, as colours ----------------------------------------------------------------------------------- */
/* each part of the picture turns in its own window of the fall (Q16 lo .. hi): the sky first, her skin last */
enum { AW_SKY, AW_WING, AW_ROBE, AW_HAIR, AW_SKIN, AW_EYES, AW_N };
static const int32_t AN_WIN[AW_N][2] = {
    {0, 40000}, {6000, 46000}, {14000, 52000}, {20000, 54000}, {28000, 62000}, {30000, 50000},
};
/* how far a window has turned at this fall: 0 .. 16 (a pixel turns when its dither threshold is under it) */
static int an_k(const flora_t *f, int w)
{
    int32_t lo = AN_WIN[w][0], hi = AN_WIN[w][1];
    if (f->corr <= lo)
        return 0;
    if (f->corr >= hi)
        return 16;
    return (int)((f->corr - lo) * 16 / (hi - lo));
}
/* where a pixel turns: the fall spreads in patches (a 3 px cell's own threshold, its edge dithered), and the sky
 * burns from the ground up */
static int an_thr(int w, int x, int y)
{
    uint32_t h;
    if (w == AW_SKY)
        return (an_clamp(70 - y, 0, 70) * 12 / 70 + an_by(x, y)) / 2 + 1;
    h = an_hash((uint32_t)(x / 3) * 73856093u ^ (uint32_t)(y / 3) * 19349663u ^ (uint32_t)w * 83492791u);
    return (int)((h % 16u) * 3u + (uint32_t)an_by(x, y)) / 4;
}
static int an_fallen(const flora_t *f, int w, int x, int y) { return an_thr(w, x, y) < an_k(f, w); }

/* the materials, four shades each (0 light .. 3 dark), calm and fallen */
enum { AM_SKIN, AM_HAIR, AM_ROBE, AM_WING, AM_N };
static const uint8_t AN_CALM[AM_N][4] = {
    {FL_C(FL_R_ROSE, 0), FL_C(FL_R_PEACH, 0), FL_C(FL_R_ROSE, 1), FL_C(FL_R_SAND, 1)},       /* fair skin, a blush */
    {FL_C(FL_R_FIRE, 0), FL_C(FL_R_COPPER, 0), FL_C(FL_R_COPPER, 1), FL_C(FL_R_COPPER, 2)}, /* golden hair */
    {FL_C(FL_R_SAND, 0), FL_C(FL_R_SAGE, 0), FL_C(FL_R_SAND, 1), FL_C(FL_R_SAND, 2)},       /* a white robe */
    {FL_C(FL_R_SAND, 0), FL_C(FL_R_SAGE, 0), FL_C(FL_R_SAGE, 1), FL_C(FL_R_SAGE, 2)},       /* white feathers */
};
static const uint8_t AN_HELL[AM_N][4] = {
    {FL_C(FL_R_ROSE, 1), FL_C(FL_R_ROSE, 2), FL_C(FL_R_FIRE, 2), FL_C(FL_R_ROSE, 3)},       /* red skin */
    {FL_C(FL_R_ROSE, 3), FL_C(FL_R_SAGE, 3), FL_BLACK, FL_BLACK},                          /* black hair, a red sheen */
    {FL_C(FL_R_ROSE, 3), FL_C(FL_R_SAGE, 3), FL_C(FL_R_FIRE, 3), FL_BLACK},                /* a black robe */
    {FL_C(FL_R_SAGE, 3), FL_BG2, FL_BLACK, FL_BLACK},                                      /* black feathers */
};
static const uint8_t AM_WINDOW[AM_N] = {AW_SKIN, AW_HAIR, AW_ROBE, AW_WING};
static uint8_t an_mat(const flora_t *f, int m, int shade, int x, int y)
{
    shade = an_clamp(shade, 0, 3);
    return an_fallen(f, AM_WINDOW[m], x, y) ? AN_HELL[m][shade] : AN_CALM[m][shade];
}

static void an_put(flc_t *c, int x, int y, uint8_t col)
{
    if ((unsigned)x < (unsigned)c->w && (unsigned)y < (unsigned)c->h)
        c->px[y * c->w + x] = col;
}

/* a tapered stroke of material m, lit from the upper left; shade s0 at the first end .. s1 at the second (Q8), the
 * widths Q8 px. bright: added light, Q8 shades */
static const int8_t AN_DISC[4][4] = {{0, -1, -1, -1}, {1, 0, -1, -1}, {1, 1, 0, -1}, {2, 2, 1, 0}};
static void an_stroke(const flora_t *f, flc_t *c, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t w0,
                      int32_t w1, int m, int32_t s0, int32_t s1)
{
    int32_t dx = x1 - x0, dy = y1 - y0, ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    int32_t n = (ax > ay ? ax : ay) >> 7, i;
    if (n < 1)
        n = 1;
    for (i = 0; i <= n; i++) {
        int32_t x = x0 + dx * i / n, y = y0 + dy * i / n, w = w0 + (w1 - w0) * i / n, s = s0 + (s1 - s0) * i / n;
        int px = (int)(x >> 8), py = (int)(y >> 8), r = (int)((w + 128) >> 9), ddy;
        if (r > 3)
            r = 3;
        for (ddy = -r; ddy <= r; ddy++) {
            int hw = AN_DISC[r][ddy < 0 ? -ddy : ddy], xx;
            for (xx = px - hw; xx <= px + hw && hw >= 0; xx++) {
                int32_t lit = s + ((xx - px) + ddy) * 80;          /* (the lower right a shade darker) */
                int sh = (int)(lit >> 8);
                if ((lit & 255) > 160 || ((lit & 255) > 80 && ((xx ^ (py + ddy)) & 1)))
                    sh++;
                an_put(c, xx, py + ddy, an_mat(f, m, sh, xx, py + ddy));
            }
        }
    }
}

/* a filled triangle (Q8 corners) in one colour, only on the pixels the dither lets through (k of 16: 16 all) */
static void an_tri(flc_t *c, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t col, int k)
{
    int32_t minx = x0, maxx = x0, miny = y0, maxy = y0, area, px, py;
    if (x1 < minx) minx = x1;
    if (x2 < minx) minx = x2;
    if (x1 > maxx) maxx = x1;
    if (x2 > maxx) maxx = x2;
    if (y1 < miny) miny = y1;
    if (y2 < miny) miny = y2;
    if (y1 > maxy) maxy = y1;
    if (y2 > maxy) maxy = y2;
    area = (x1 - x0) * (y2 - y0) / 256 - (x2 - x0) * (y1 - y0) / 256;
    if (!area)
        return;
    for (py = miny >> 8; py <= maxy >> 8; py++)
        for (px = minx >> 8; px <= maxx >> 8; px++) {
            int32_t cx = (px << 8) + 128, cy = (py << 8) + 128;
            int32_t e0 = (x1 - x0) * (cy - y0) / 256 - (y1 - y0) * (cx - x0) / 256;
            int32_t e1 = (x2 - x1) * (cy - y1) / 256 - (y2 - y1) * (cx - x1) / 256;
            int32_t e2 = (x0 - x2) * (cy - y2) / 256 - (y0 - y2) * (cx - x2) / 256;
            if (area < 0) {
                e0 = -e0;
                e1 = -e1;
                e2 = -e2;
            }
            if (e0 >= 0 && e1 >= 0 && e2 >= 0 && an_by((int)px, (int)py) < k)
                an_put(c, (int)px, (int)py, col);
        }
}

/* ---- her figure --------------------------------------------------------------------------------------------- */
/* kneeling, her hands together at her chest, her hair long down her back. 'k' 'h' 'H' hair (dark .. light), 'S' 's'
 * 'd' skin (light .. shadow), 'R' 'r' 'q' 'Q' robe (light .. deep shadow); the eyes, the mouth, the halo and the
 * horns are drawn over it (an_face) */
#define AN_BW 26
#define AN_BH 34
#define AN_HEAD 12                               /* rows 0 .. 11: the head (it lifts and bows) */
static const char *const AN_BODY[AN_BH] = {
    "..........kkhhkk..........",
    "........khhHHHHhhk........",
    ".......khHHHHHHHHhk.......",
    "......khHHHHHHHHHHhk......",
    "......khHHSSSSSSHHhk......",
    ".....kkhHSSSSSSSSHhkk.....",
    ".....khhSSSSSSSSSShhk.....",
    ".....khhSSSSSSSSSShhk.....",
    ".....khhSSSSSdSSSShhk.....",
    ".....khhhSSSSSSSShhhk.....",
    "....kkhhhdSSSSSSdhhhkk....",
    "....khhhhkddSSddkhhhhk....",
    "....khhhhhhsssshhhhhhk....",
    "...khhhhRRRssssRRRhhhhk...",
    "...khhhRRRRRRRRRRRRhhhk...",
    "..khhhRRRRRRRRRRRRRrhhhk..",
    "..khhRRRRRRRSSRRRRRrrhhk..",
    "..khhRRRRRRSSSsRRRRrrhhk..",
    "..khhRRRRRrSSsdrRRRrrhhk..",
    "..khhRRRRRrSSsdrRRRrrhhk..",
    "..kh.RRRRRrrSdrrRRRrr.hk..",
    "..k..RRRRrrrrrrrrRRrr..k..",
    ".....RRRRrrqqqqrrRrrq.....",
    ".....RRRRrrqqqqrrRrrq.....",
    "....RRRRrrqqqqqqrrrrqq....",
    "....RRRRrrqqqqqqrrrrqq....",
    "...RRRRrrrqqqqqqrrrrrqq...",
    "...RRRRrrqqqQQqqqrrrrqq...",
    "..RRRRRrrqqQQQQqqrrrrrqq..",
    "..RRRRrrrqqQQQQqqrrrrrqq..",
    ".RRRRRrrqqqQQQQqqqrrrrrqq.",
    ".RRRRrrrqqQQQQQQqqrrrrrqq.",
    "RRRRRrrqqqQQQQQQqqqrrrrrqq",
    "RRRRrrrqqQQQQQQQQqqrrrrrqq",
};
#define AN_GROUND_Y 62                           /* her knees' line (the cloud's top), canvas px */

static void an_body_row(const flora_t *f, flc_t *c, int x0, int y, const char *row)
{
    int i;
    for (i = 0; i < AN_BW; i++) {
        int m, s, x = x0 + i;
        switch (row[i]) {
        case 'H': m = AM_HAIR, s = 0; break;
        case 'h': m = AM_HAIR, s = 1; break;
        case 'k': m = AM_HAIR, s = 3; break;
        case 'S': m = AM_SKIN, s = 0; break;
        case 's': m = AM_SKIN, s = 1; break;
        case 'd': m = AM_SKIN, s = 3; break;
        case 'R': m = AM_ROBE, s = 0; break;
        case 'r': m = AM_ROBE, s = 1; break;
        case 'q': m = AM_ROBE, s = 2; break;
        case 'Q': m = AM_ROBE, s = 3; break;
        default: continue;
        }
        an_put(c, x, y, an_mat(f, m, s, x, y));
    }
}
static void an_body(const flora_t *f, flc_t *c, int x0, int y0, int hdy)
{
    int j;
    if (hdy < 0)                                 /* (a lifted head: its chin's row also where it was) */
        an_body_row(f, c, x0, y0 + AN_HEAD - 1, AN_BODY[AN_HEAD - 1]);
    for (j = 0; j < AN_BH; j++)
        an_body_row(f, c, x0, y0 + j + (j < AN_HEAD ? hdy : 0), AN_BODY[j]);
}

/* her face: closed eyes calm; open, then glowing as she falls; the brows slant, the mouth opens on fangs */
static void an_face(const flora_t *f, flc_t *c, int x0, int y0)
{
    int32_t q = f->corr, flash = f->thump > 2000;
    int ey = y0 + 7, my = y0 + 9, i;
    static const int8_t EX[4] = {9, 10, 15, 16};
    if (q < 26000) {                                             /* closed: the lashes */
        for (i = 0; i < 4; i++)
            an_put(c, x0 + EX[i], ey, an_mat(f, AM_SKIN, 3, x0 + EX[i], ey));
    } else if (q < 44000) {                                      /* open, dark */
        an_put(c, x0 + 9, ey, FL_INK);
        an_put(c, x0 + 10, ey, FL_BLACK);
        an_put(c, x0 + 15, ey, FL_BLACK);
        an_put(c, x0 + 16, ey, FL_INK);
    } else {                                                     /* glowing */
        uint8_t hot = flash ? FL_C(FL_R_SAND, 0) : FL_C(FL_R_FIRE, 0), rim = FL_C(FL_R_FIRE, 1);
        an_put(c, x0 + 9, ey, rim);
        an_put(c, x0 + 10, ey, hot);
        an_put(c, x0 + 15, ey, hot);
        an_put(c, x0 + 16, ey, rim);
    }
    if (q >= 36000) {                                            /* the brows, slanting in */
        uint8_t b = an_mat(f, AM_HAIR, 3, x0 + 9, ey - 1);
        an_put(c, x0 + 8, ey - 2, b);
        an_put(c, x0 + 9, ey - 2, b);
        an_put(c, x0 + 10, ey - 1, b);
        an_put(c, x0 + 17, ey - 2, b);
        an_put(c, x0 + 16, ey - 2, b);
        an_put(c, x0 + 15, ey - 1, b);
    }
    if (q < 40000) {                                             /* a small mouth */
        an_put(c, x0 + 12, my, an_mat(f, AM_SKIN, 2, x0 + 12, my));
        an_put(c, x0 + 13, my, an_mat(f, AM_SKIN, 2, x0 + 13, my));
    } else {                                                     /* open, the fangs under it */
        for (i = 11; i <= 14; i++)
            an_put(c, x0 + i, my, FL_BLACK);
        if (q >= 48000) {
            an_put(c, x0 + 11, my + 1, FL_C(FL_R_SAND, 0));
            an_put(c, x0 + 14, my + 1, FL_C(FL_R_SAND, 0));
        }
    }
}

/* the horns: out of her hair as she falls, curling up and out, bone darkening to black at the tips */
static void an_horns(const flora_t *f, flc_t *c, int x0, int y0)
{
    int32_t len = an_clamp((f->corr - 18000) * 4096 / 30000, 0, 4096), side;
    if (!len)
        return;
    for (side = -1; side <= 1; side += 2) {
        int32_t bx = ((int32_t)x0 * 256) + (side < 0 ? 7 * 256 : 18 * 256 + 255), by = ((int32_t)y0 + 3) * 256;
        /* three bends: up and out, up, then curling back in */
        static const int16_t P[4][2] = {{0, 0}, {-3, -3}, {-4, -7}, {-2, -10}};
        int32_t px = bx, py = by, k;
        for (k = 1; k < 4; k++) {
            int32_t nx = bx + side * -P[k][0] * 256 * len / 4096, ny = by + P[k][1] * 256 * len / 4096;
            int32_t w0 = k == 1 ? 640 : k == 2 ? 448 : 256, w1 = k == 1 ? 448 : k == 2 ? 256 : 128;
            int32_t s0 = k * 180, s1 = k * 180 + 180;
            int32_t dx = nx - px, dy = ny - py, ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
            int32_t n = (ax > ay ? ax : ay) >> 7, i;
            if (n < 1)
                n = 1;
            for (i = 0; i <= n; i++) {
                int32_t x = px + dx * i / n, y = py + dy * i / n, w = w0 + (w1 - w0) * i / n, s = s0 + (s1 - s0) * i / n;
                int qx = (int)(x >> 8), qy = (int)(y >> 8), r = (int)((w + 128) >> 9), sh = (int)(s >> 8), dd;
                static const uint8_t BONE[4] = {FL_C(FL_R_SAND, 1), FL_C(FL_R_SAND, 2), FL_C(FL_R_SAND, 3), FL_BLACK};
                for (dd = -r; dd <= r; dd++) {
                    an_put(c, qx + dd, qy, BONE[an_clamp(sh + (dd > 0), 0, 3)]);
                    if (r && dd == 0)
                        an_put(c, qx, qy - 1, BONE[an_clamp(sh, 0, 3)]);
                }
            }
            px = nx;
            py = ny;
        }
    }
}

/* the halo: a gold ring over her head, fading, flickering and breaking as she falls (gone half way) */
static void an_halo(const flora_t *f, flc_t *c, int x0, int y0)
{
    int32_t k = 16 - an_clamp(f->corr * 16 / 30000, 0, 16), a;
    int cx = x0 * 2 + 25, cy = y0 - 4;                           /* (x in half pixels: the head's centre is 12.5) */
    if (k <= 0)
        return;
    for (a = 0; a < 1024; a += 16) {
        int x = (cx + (int)((an_cos(a) * 13) >> 14)) >> 1, y = cy + (int)((an_sin(a) * 2 + 8192) >> 14);
        int crack = f->corr > 9000 && ((a >> 6) == 3 || (a >> 6) == 11);
        if (crack || an_by(x, y) >= k)
            continue;
        if ((f->t / 90u + (uint32_t)a) % 23u == 0u && f->corr > 6000)   /* (it flickers) */
            continue;
        an_put(c, x, y, (a & 64) ? FL_C(FL_R_FIRE, 0) : FL_C(FL_R_SAND, 0));
    }
}

/* ---- the wings ---------------------------------------------------------------------------------------------- */
/* a wing's bones (Q8, from the shoulder): elbow, wrist, tip; folded around her (open 0) or spread (open 1, then
 * turned up or down by the lift). side +1 her left wing (the screen's right), -1 the other */
typedef struct { int32_t x[4], y[4]; } an_bones_t;
static const int16_t AN_FOLD[3][2] = {{8, -9}, {12, 5}, {3, 21}};   /* (up behind her shoulder, round to the front) */
static const int16_t AN_SPREAD[3][2] = {{13, -11}, {27, -19}, {44, -15}};
static void an_bones(const flora_t *f, int side, int32_t sx, int32_t sy, an_bones_t *b)
{
    int32_t o = f->open[side > 0 ? 0 : 1], e = (o * o) >> 12, rot = (f->lift * 70) >> 12, i;
    /* (eased: slow at the start, as a wing pulls free of her) */
    e = (e + o) / 2;
    b->x[0] = sx;
    b->y[0] = sy;
    for (i = 0; i < 3; i++) {
        int32_t fx = AN_FOLD[i][0] * 256, fy = AN_FOLD[i][1] * 256;
        int32_t ox = AN_SPREAD[i][0] * 256, oy = AN_SPREAD[i][1] * 256;
        int32_t r = -rot;                                        /* (up: the wing turns about the shoulder) */
        int32_t rx = (ox * an_cos(r) - oy * an_sin(r)) >> 14, ry = (ox * an_sin(r) + oy * an_cos(r)) >> 14;
        rx = rx * (4096 - f->tight / 4) >> 12;
        b->x[i + 1] = sx + side * an_lerp(fx, rx, e);
        b->y[i + 1] = sy + an_lerp(fy, ry, e);
    }
}
/* a feather's root (along the bones) and its direction (1024 a turn, screen y down), length (px) */
static void an_feather(const flora_t *f, int side, const an_bones_t *b, int i, int32_t *rx, int32_t *ry, int32_t *dir,
                       int32_t *len)
{
    int32_t o = f->open[side > 0 ? 0 : 1], a_open, a_fold = 256 - 16, seg, u;
    if (i < 5) {                                 /* secondaries: along the shoulder .. elbow .. wrist, hanging */
        seg = i < 2 ? 0 : 1;
        u = i < 2 ? 1400 + i * 1500 : (i - 2) * 1365;
        a_open = 250 - i * 10;
        *len = 13 + (i & 1);
    } else {                                     /* primaries: along the wrist .. tip, fanning out */
        seg = 2;
        u = (i - 5) * 4096 / 6;
        a_open = 175 - (i - 5) * 23;
        *len = 18 - (i - 5) / 2;
    }
    *rx = an_lerp(b->x[seg], b->x[seg + 1], u);
    *ry = an_lerp(b->y[seg], b->y[seg + 1], u);
    a_open -= (f->lift * 40) >> 12;              /* (lifted wings fan up) */
    a_open += (f->tight * 30) >> 12;             /* (diminished: close) */
    a_fold += i * 2;
    {
        int32_t a = an_lerp(a_fold, a_open, o);
        *dir = side > 0 ? a : 512 - a;
    }
    if (o < 1200)                                /* (folded, the feathers reach down over her robe) */
        *len += (1200 - o) * 4 / 1200;
}
static void an_wing(const flora_t *f, flc_t *c, int side, int32_t sx, int32_t sy, int membrane_only)
{
    an_bones_t b;
    int32_t rx[AN_NFEATH], ry[AN_NFEATH], tx[AN_NFEATH], ty[AN_NFEATH];
    int i, w = side > 0 ? 0 : 1, km = (int)an_clamp((f->corr - 32000) * 16 / 24000, 0, 16);
    int32_t o = f->open[w], dark = o < 2000 ? (2000 - o) * 200 / 2000 : 0;   /* (folded over her robe: a shade darker) */
    int32_t torn = an_clamp((f->corr - 40000) * 4096 / 20000, 0, 4096);
    an_bones(f, side, sx, sy, &b);
    for (i = 0; i < AN_NFEATH; i++) {
        int32_t dir, len;
        an_feather(f, side, &b, i, &rx[i], &ry[i], &dir, &len);
        if (torn && (i % 3) == 1)                /* (fallen: the feathers torn short) */
            len = len * (4096 - torn / 2) >> 12;
        len += (f->glow[w][i] * 3) >> 12;        /* (a lit feather reaches out) */
        tx[i] = rx[i] + ((an_cos(dir) * len) >> 6);
        ty[i] = ry[i] + ((an_sin(dir) * len) >> 6);
    }
    if (km) {                                    /* the membrane, red between the fingers, dark by the arm */
        for (i = 5; i < AN_NFEATH - 1; i++)
            an_tri(c, b.x[3], b.y[3], tx[i], ty[i], tx[i + 1], ty[i + 1], FL_C(FL_R_ROSE, 3), km);
        an_tri(c, b.x[2], b.y[2], b.x[3], b.y[3], tx[5], ty[5], FL_C(FL_R_FIRE, 3), km);
        for (i = 0; i < 4; i++)
            an_tri(c, rx[i], ry[i], rx[i + 1], ry[i + 1], tx[i + 1], ty[i + 1], i & 1 ? FL_C(FL_R_FIRE, 3) : FL_C(FL_R_ROSE, 3), km);
        if (km > 8)                              /* (the finger bones) */
            for (i = 6; i < AN_NFEATH; i += 2)
                an_stroke(f, c, b.x[3], b.y[3], tx[i], ty[i], 384, 128, AM_HAIR, 768, 768);
    }
    if (membrane_only)
        return;
    for (i = 0; i < AN_NFEATH; i++) {            /* far feathers first: the primaries under the secondaries */
        int k = AN_NFEATH - 1 - i;
        int32_t g = f->glow[w][k], s0 = 128 - (g >> 4), s1 = 560 - (g >> 3);
        if (torn > 3000 && (k % 4) == 2)         /* (torn out) */
            continue;
        an_stroke(f, c, rx[k], ry[k], tx[k], ty[k], 704, 192, AM_WING, s0 + dark, s1 + dark);
    }
    /* the coverts: the arm, thick and light */
    an_stroke(f, c, b.x[0], b.y[0], b.x[1], b.y[1], 1024, 768, AM_WING, 0, 64);
    an_stroke(f, c, b.x[1], b.y[1], b.x[2], b.y[2], 768, 512, AM_WING, 64, 160);
    an_stroke(f, c, b.x[2], b.y[2], b.x[3], b.y[3], 512, 256, AM_WING, 160, 300);
}

/* ---- the scene ---------------------------------------------------------------------------------------------- */
static const uint8_t AN_NIGHT[5] = {FL_BG, FL_L0, FL_L1, FL_L2, FL_L3};
static const uint8_t AN_FIRE[5] = {FL_BG, FL_S0, FL_S1, FL_S2, FL_S3};
static uint8_t an_grad(const uint8_t *st, int32_t t, int x, int y)
{
    int32_t i;
    if (t <= 0)
        return st[0];
    if (t >= 1024)
        return st[4];
    i = t >> 8;
    return (int32_t)((t & 255) >> 4) > an_by(x, y) ? st[i + 1] : st[i];
}
static int32_t an_sky_t(int y, int hz) { return an_clamp(y * y * 1024 / (hz * hz), 0, 1024); }
/* the cloud bank she kneels on (calm), the burning ground (fallen): its top line */
static int an_cloud_y(int x, uint32_t t)
{
    return AN_GROUND_Y - 1 + ((an_sin(x * 26 + (int32_t)(t >> 7)) * 2) >> 14) + ((an_sin(x * 61 + 300) * 2) >> 14) +
           (x < 34 || x > 86 ? 2 : 0);
}

static void an_sky(flora_t *f, flc_t *c, int scene)
{
    int W = c->w, H = c->h, x, y, i, hz = scene ? 66 : H + 6;
    uint8_t *px = c->px;
    int32_t flare = f->thump * an_k(f, AW_SKY) / 16 / 24;       /* (fallen: an 808 hit flashes the sky) */
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++) {
            int32_t t = an_sky_t(y, hz);
            px[y * W + x] = an_fallen(f, AW_SKY, x, y) ? an_grad(AN_FIRE, t + flare, x, y) : an_grad(AN_NIGHT, t, x, y);
        }
    if (!scene)
        return;
    for (i = 0; i < 22; i++) {                                   /* stars (calm), embers in the air (fallen) */
        uint32_t h = an_hash((uint32_t)i * 7919u + 13u);
        int sx = (int)(h % (uint32_t)W), sy = (int)((h >> 8) % 46u), tw = (int)(((f->t >> 6) + h) % 48u);
        if ((sx < 38 && sy < 20) || (sx > 84 && sy < 8) || (sx > 84 && sy > 14 && sy < 34))
            continue;
        if (an_fallen(f, AW_SKY, sx, sy))
            continue;
        px[sy * W + sx] = tw < 2 ? FL_C(FL_R_SAND, 0) : (h >> 20) & 1 ? FL_C(FL_R_SAND, 2) : FL_C(FL_R_SAGE, 2);
    }
    {                                                            /* the moon: a crescent, filling to a blood moon */
        int mx = W * 4 / 5 + 2, my = 22;
        int32_t sh = 3 + an_clamp(f->corr * 7 / 40000, 0, 7);    /* (the shadow slides off) */
        for (y = my - 10; y <= my + 10; y++)
            for (x = mx - 10; x <= mx + 10; x++) {
                int dx = x - mx, dy = y - my, d2 = dx * dx + dy * dy;
                int ex = dx - (int)sh, ey = dy + 2, e2 = ex * ex + ey * ey, red = an_fallen(f, AW_SKY, x, y);
                if (d2 <= 36) {
                    if (e2 > 25 || red)
                        an_put(c, x, y, red ? (d2 > 20 || ((dx * 3 + dy * 5) & 7) == 0 ? FL_C(FL_R_ROSE, 2) : FL_C(FL_R_FIRE, 2))
                                            : (d2 > 25 || e2 < 34 ? FL_C(FL_R_SAND, 1) : FL_C(FL_R_SAND, 0)));
                } else if (d2 <= 110) {
                    int32_t t = an_sky_t(y, hz) + (110 - d2) * 3;
                    an_put(c, x, y, red ? an_grad(AN_FIRE, t, x, y) : an_grad(AN_NIGHT, t, x, y));
                }
            }
    }
}

/* the ground: soft cloud tops (calm), black rock with fire licking up (fallen); behind her (front 0) or the flames'
 * tongues in front of her robe's hem (front 1) */
static void an_ground(flora_t *f, flc_t *c, int front)
{
    int W = c->w, H = c->h, x, y;
    int32_t fire = an_clamp((f->corr - 12000) * 4096 / 40000, 0, 4096), boost = f->thump / 2;
    for (x = 0; x < W; x++) {
        int top = an_cloud_y(x, f->t);
        uint32_t h = an_hash((uint32_t)x * 2246822519u + (f->t / 70u) * 3266489917u);
        int32_t flame = (int32_t)((fire + boost) * (int32_t)(4 + (h % 11u)) >> 12);
        if (!front)
            for (y = top; y < H; y++) {
                int fall = an_fallen(f, AW_SKY, x, y), d = y - top;
                uint8_t col;
                if (fall)
                    col = d == 0 ? FL_C(FL_R_FIRE, 2) : ((x * 7 + y * 3) % 13 == 0 ? FL_C(FL_R_FIRE, 3) : FL_BLACK);
                else
                    col = d == 0 ? FL_C(FL_R_SAGE, 0) : d < 3 ? FL_C(FL_R_SAGE, 1) : d < 6 ? FL_C(FL_R_SAGE, 2)
                                                                                     : FL_C(FL_R_SAGE, 3);
                c->px[y * W + x] = col;
            }
        if (flame > 0 && (!front || (x > 46 && x < 74 && (h >> 12) % 3u == 0u)))
            for (y = top - (int)flame; y < top + (front ? 0 : 2); y++) {
                int d = top - y;
                if (y < 0 || an_by(x, y) >= an_k(f, AW_SKY) + (int)(boost >> 9))
                    continue;
                an_put(c, x, y, d * 3 > flame * 2 ? FL_C(FL_R_FIRE, 0) : d * 3 > flame ? FL_C(FL_R_FIRE, 1)
                                                                                         : FL_C(FL_R_FIRE, 2));
            }
    }
}

static void an_rings(flora_t *f, flc_t *c)
{
    int i, cx = f->cx, cy = AN_GROUND_Y - 20;
    for (i = 0; i < 4; i++) {
        uint32_t age = f->t - f->ring_t[i];
        int32_t r, a, steps, fade;
        if (!f->ring_t[i] || age > 2400u)
            continue;
        r = (int32_t)age * 36 / 1000 + 6;
        fade = 768 - (int32_t)age * 768 / 2400;
        steps = r * 7 + 16;
        for (a = 0; a < steps; a++) {
            int32_t ang = a * 1024 / steps;
            int x = cx + ((an_cos(ang) * r) >> 14), y = cy + ((an_sin(ang) * r) >> 14);
            if ((unsigned)x < (unsigned)c->w && (unsigned)y < (unsigned)c->h && an_by(x, y) * 24 < fade)
                c->px[y * c->w + x] = an_fallen(f, AW_SKY, x, y) ? FL_S3 : FL_L3;
        }
    }
}

static void an_draw_part(flora_t *f, flc_t *c, const an_part_t *p)
{
    int x = (int)(p->x >> 8), y = (int)(p->y >> 8);
    if (p->kind == 0) {                          /* a feather: a short stroke, light */
        uint8_t a = p->shade ? FL_C(FL_R_SAGE, 3) : FL_C(FL_R_SAND, 0), b = p->shade ? FL_BLACK : FL_C(FL_R_SAGE, 1);
        an_put(c, x, y, a);
        an_put(c, x + ((p->life / 300u) & 1u ? 1 : -1), y + 1, b);
    } else if (p->kind == 1) {                   /* an ember: one pixel, flickering */
        an_put(c, x, y, (p->life / 60u) & 1u ? FL_C(FL_R_FIRE, 0) : FL_C(FL_R_FIRE, 1));
    } else {                                     /* a spark of light */
        an_put(c, x, y, p->life > 300u ? FL_C(FL_R_SAND, 0) : FL_C(FL_R_FIRE, 0));
    }
}

/* ---- the API ------------------------------------------------------------------------------------------------ */
void flora_init(flora_t *f, uint32_t seed, int cx, int cy)
{
    uint32_t i;
    uint8_t *b = (uint8_t *)f;
    for (i = 0; i < sizeof *f; i++)
        b[i] = 0;
    f->rng = seed ? seed : 0x9E3779B9u;
    f->cx = (int16_t)cx;
    f->cy = (int16_t)cy;
    f->nseg = 1;
    f->head = f->head_t = -2048;
}
void flora_set_species(flora_t *f, int species) { f->species = (uint8_t)species; }
void flora_set_key(flora_t *f, int tonic_pc) { f->home = (uint8_t)(tonic_pc % 12); }
void flora_set_playing(flora_t *f, int on) { f->playing = (uint8_t)(on != 0); }

static void an_spawn(flora_t *f, int32_t x, int32_t y, int kind)
{
    uint32_t i, best = 0, life = 0xFFFF;
    for (i = 0; i < AN_NPART; i++)
        if (f->part[i].life < life) {
            life = f->part[i].life;
            best = i;
        }
    {
        an_part_t *p = &f->part[best];
        uint32_t r = an_rnd(f);
        p->x = x;
        p->y = y;
        p->kind = (uint8_t)kind;
        p->shade = (uint8_t)(f->corr > 30000);
        p->vx = (int16_t)((int32_t)(r % 61u) - 30);
        p->vy = (int16_t)(kind == 0 ? 20 + (int32_t)((r >> 8) % 20u) : -(40 + (int32_t)((r >> 8) % 50u)));
        p->life = (uint16_t)(kind == 0 ? 2600 + (r >> 16) % 1400u : kind == 1 ? 1400 + (r >> 16) % 1200u : 500u);
    }
}

void flora_set_chord(flora_t *f, const fl_chord_t *c)
{
    int changed = c->n != f->chord.n || c->quality != f->chord.quality || c->root != f->chord.root, i;
    if (c->n && changed) {
        f->chords++;
        f->ring_t[f->ring_i & 3u] = f->t ? f->t : 1u;
        f->ring_i++;
        if (f->playing)                          /* (each chord of the song pulls her down a little) */
            f->corr = an_clamp(f->corr + 120, 0, 65536);
    }
    f->chord = *c;
    for (i = 0; i < AN_NFEATH; i++)
        f->lit[0][i] = f->lit[1][i] = 0;
    for (i = 0; i < c->n && i < 8; i++) {
        int pc = c->note[i] % 12;
        f->lit[pc < 6 ? 0 : 1][(pc % 6) * 2 + (c->note[i] / 12) % 2] = 1;
    }
}

void flora_note_on(flora_t *f, uint8_t note, uint8_t vel)
{
    int pc = note % 12, w = pc < 6 ? 0 : 1, k = (pc % 6) * 2 + (note / 12) % 2, j;
    int32_t p = 2400 + vel * 13;
    for (j = -1; j <= 1; j++) {
        int kk = k + j;
        if (kk >= 0 && kk < AN_NFEATH)
            f->glow[w][kk] = (uint16_t)an_clamp(f->glow[w][kk] + (j ? p / 2 : p), 0, 4096);
    }
    {                                            /* shed: a feather (calm) or an ember (fallen) from its tip */
        int side = w ? -1 : 1;
        int32_t sx = ((int32_t)f->cx + side * 6) * 256, sy = (AN_GROUND_Y - 22) * 256, rx, ry, dir, len;
        an_bones_t b;
        an_bones(f, side, sx, sy, &b);
        an_feather(f, side, &b, k, &rx, &ry, &dir, &len);
        an_spawn(f, rx + ((an_cos(dir) * len) >> 6), ry + ((an_sin(dir) * len) >> 6),
                 (int32_t)(an_rnd(f) % 65536u) < f->corr ? 1 : 0);
    }
}

void flora_set_level(flora_t *f, uint16_t level) { f->level_in = level; }

void flora_thump(flora_t *f, uint8_t vel)
{
    f->thump = an_clamp(1500 + vel * 20, 0, 4096);
    f->thump_t = f->t ? f->t : 1u;
    if (f->corr > 20000) {                        /* (the fire spits embers) */
        uint32_t r = an_rnd(f);
        an_spawn(f, (int32_t)(30 + r % 60u) * 256, (AN_GROUND_Y - 1) * 256, 1);
    }
}

void flora_tick(flora_t *f, uint32_t dt)
{
    const fl_chord_t *c = &f->chord;
    int32_t base, ow[2], lift = 0, head = -2048, tight = 0, lv, i, w;
    if (dt > 200u)
        dt = 200u;
    f->t += dt;
    f->level = an_ease(f->level, f->level_in, dt, 120);
    lv = f->level >> 4;                                          /* Q12 */
    /* the pose the chord asks for */
    if (!c->n) {
        ow[0] = ow[1] = 0;
    } else {
        base = an_clamp(1900 + (c->n - 1) * 520, 0, 4096);
        ow[0] = ow[1] = base;
        head = 0;
        switch (c->quality) {
        case FL_Q_MAJOR: lift = 2400, head = 2048; break;
        case FL_Q_MINOR: lift = -2000, head = -1200; break;
        case FL_Q_DIM: tight = 4096, ow[0] = ow[1] = base * 3 / 4, lift = -800; break;
        case FL_Q_AUG: ow[0] = ow[1] = base + 600, lift = 1200; break;
        case FL_Q_SUS2: ow[1] = base * 2 / 3, lift = 800; break;
        case FL_Q_SUS4: ow[0] = base * 2 / 3, lift = 800; break;
        case FL_Q_SINGLE: {
            int n0 = 0, n1 = 0;
            for (i = 0; i < c->n; i++)
                *(c->note[i] % 12 < 6 ? &n0 : &n1) += 1;
            ow[0] = n0 ? 1500 + n0 * 700 : 500;
            ow[1] = n1 ? 1500 + n1 * 700 : 500;
            head = -600;
            break;
        }
        default: break;
        }
        ow[0] += (lv * 600) >> 12;
        ow[1] += (lv * 600) >> 12;
    }
    {                                                            /* breathing, slow */
        int32_t br = (an_sin((int32_t)(f->t / 7u)) * 160) >> 14;
        ow[0] += br;
        ow[1] += br;
    }
    ow[0] += f->thump / 10;
    ow[1] += f->thump / 10;
    for (w = 0; w < 2; w++) {
        f->open_t[w] = an_clamp(ow[w], 0, 4096);
        f->open[w] = an_ease(f->open[w], f->open_t[w], dt, f->open_t[w] > f->open[w] ? 260 : 520);
    }
    f->lift_t = lift;
    f->lift = an_ease(f->lift, lift, dt, 300);
    f->head_t = head;
    f->head = an_ease(f->head, head, dt, 350);
    f->tight = an_ease(f->tight, tight, dt, 300);
    /* the feathers' light: a chord tone's glows, a pulse fades */
    for (w = 0; w < 2; w++)
        for (i = 0; i < AN_NFEATH; i++) {
            int32_t g = f->glow[w][i], t = f->lit[w][i] ? 1800 : 0;
            f->glow[w][i] = (uint16_t)(g > t ? an_ease(g, t, dt, 380) : an_ease(g, t, dt, 160));
        }
    f->thump = an_ease(f->thump, 0, dt, 160);
    /* the fall: while the song plays, about two and a half minutes to the bottom (a track's length); chords alone pull
     * slowly; quiet and stopped for 8 s, she heals back in 25 s */
    if (f->playing) {
        f->corr += (int32_t)(dt * 65536u / 150000u);
        f->quiet_ms = 0;
    } else if (c->n) {
        f->corr += (int32_t)(dt * 65536u / 360000u);
        f->quiet_ms = 0;
    } else {
        f->quiet_ms += dt;
        if (f->quiet_ms > 8000u)
            f->corr -= (int32_t)(dt * 65536u / 25000u);
    }
    f->corr = an_clamp(f->corr, 0, 65536);
    /* the particles */
    for (i = 0; i < AN_NPART; i++) {
        an_part_t *p = &f->part[i];
        if (!p->life)
            continue;
        p->life = (uint16_t)(p->life > dt ? p->life - dt : 0u);
        p->x += p->vx * (int32_t)dt / 64;
        p->y += p->vy * (int32_t)dt / 64;
        if (p->kind == 0)                        /* (a feather sways as it falls) */
            p->x += (an_sin((int32_t)((f->t + i * 97u) / 3u)) * (int32_t)dt) >> 18;
        if (p->x < 0 || p->x >= 120 * 256 || p->y < -512 || p->y >= 72 * 256)
            p->life = 0;
    }
    f->part_acc += (uint16_t)dt;                 /* (fallen: embers rise from the fire all the time) */
    if (f->part_acc > 160u) {
        f->part_acc = 0;
        if ((int32_t)(an_rnd(f) % 65536u) < f->corr - 16000)
            an_spawn(f, (int32_t)(20 + an_rnd(f) % 80u) * 256, (AN_GROUND_Y - 1) * 256, 1);
        else if (!c->n && f->corr < 20000 && an_rnd(f) % 40u == 0u)   /* (calm: now and then a feather) */
            an_spawn(f, (int32_t)(f->cx - 20 + (int)(an_rnd(f) % 40u)) * 256, 30 * 256, 0);
    }
}

void flora_render(flora_t *f, flc_t *c)
{
    int i, x0 = f->cx - AN_BW / 2, bob, hdy, front;
    int32_t sxr, sxl, sy;
    if (f->backdrop == FL_BD_NONE)
        flc_clear(c, FL_BG);
    else
        an_sky(f, c, f->backdrop == FL_BD_SCENE);
    if (f->backdrop == FL_BD_RIPPLES)
        an_rings(f, c);
    if (f->backdrop != FL_BD_NONE)
        an_ground(f, c, 0);
    for (i = 0; i < AN_NPART; i++)               /* far particles */
        if (f->part[i].life && (i & 1))
            an_draw_part(f, c, &f->part[i]);
    bob = (an_sin((int32_t)(f->t / 6u)) > 9000) + (f->thump > 2600);
    hdy = f->head > 1000 ? -1 : f->head < -1000 ? 1 : 0;
    {
        int y0 = AN_GROUND_Y - AN_BH + 2 + bob;
        sxr = ((int32_t)x0 + 19) * 256 + 128;
        sxl = ((int32_t)x0 + 6) * 256 + 128;
        sy = ((int32_t)y0 + 15) * 256;
        front = f->open[0] + f->open[1] < 2 * 1500;              /* (folded: the wings are wrapped in front of her) */
        an_wing(f, c, 1, sxr, sy, front);
        an_wing(f, c, -1, sxl, sy, front);
        if (!front) {
            an_wing(f, c, 1, sxr, sy, 0);
            an_wing(f, c, -1, sxl, sy, 0);
        }
        an_body(f, c, x0, y0, hdy);
        an_face(f, c, x0, y0 + hdy);
        an_horns(f, c, x0, y0 + hdy);
        if (front) {
            an_wing(f, c, 1, sxr, sy, 0);
            an_wing(f, c, -1, sxl, sy, 0);
            {                                    /* (folded: her face and hands still show over the feathers) */
                int j, k;
                for (j = 0; j < 12; j++)
                    for (k = 6; k < 20; k++) {
                        char ch = AN_BODY[j][k];
                        int m = ch == 'S' ? AM_SKIN : ch == 's' ? AM_SKIN : ch == 'd' ? AM_SKIN : -1;
                        if (m >= 0)
                            an_put(c, x0 + k, y0 + j + hdy, an_mat(f, m, ch == 'S' ? 0 : ch == 's' ? 1 : 3, x0 + k,
                                                                   y0 + j + hdy));
                    }
                an_face(f, c, x0, y0 + hdy);
            }
        }
        an_halo(f, c, x0, y0 + hdy);
    }
    if (f->backdrop != FL_BD_NONE)
        an_ground(f, c, 1);
    for (i = 0; i < AN_NPART; i++)               /* near particles */
        if (f->part[i].life && !(i & 1))
            an_draw_part(f, c, &f->part[i]);
}
