/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 ChoralRoot FM-1 contributors (a fork of Felucca) */
/* 808: the trap bass (TrapRoot, docs/TRAP.md). Engine 11, the slot NOISE held until 0.18 (FELUCCA_808; with
 * FELUCCA_NOISE the slot is NOISE's and this file is not built).
 *
 * One voice is the classic tuned kick-bass: a sine whose pitch drops from PUNCH semitones above the note to the note
 * in SNAP's time, under an exponential DECAY (the 808's long tail), driven into a waveshaper (MODEL, DRIVE), a two-pole
 * low-pass (TONE), with a short CLICK on the hit. GLIDE is the slide: a note struck while the last one still sounds
 * (overlapping keys, or a slide step of a rhythm's 808 line, cf_drum.c dm8_step) bends from the old pitch to the new one in GLIDE's time,
 * the tail and the drive going on without a new hit, as the trap 808 slides do.
 *
 * MODEL, the waveshapers (after Surge XT's waveshaper set, surge-synthesizer.github.io: the same curves written afresh in fixed point,
 * no code copied; LICENSING.md):
 *   CLEAN  the sine; DRIVE only lifts it into the output's soft knee (a round sub)
 *   WARM   tanh (Surge "Soft"): the round, speaker-friendly 808
 *   HARD   a hard clip (Surge "Hard"): the square-edged, distorted trap 808
 *   GROWL  an asymmetric tanh (Surge "Asym"): even harmonics, a growl; the DC blocked after it
 *   FOLD   a sine wavefolder (Surge "Sine"): DRIVE folds the sine over itself, a metallic, talking 808
 *   RAGE   tanh into a hard clip with a full-wave rectified share mixed in (an octave of fuzz over the note):
 *          the blown-out rage 808 (Ken Carson, Destroy Lonely); loud, so it is brought down by its own trim
 * Each model is levelled by ear and RMS (E8_COMP), so a model change keeps the loudness.
 *
 * What applies: MONO (the presets; POLY works too), LEVEL, PAN, the sends, DIST, the ADSR as the gate (the presets: A 0,
 * S full: the shape is DECAY's) and its release, velocity, the LFO and the matrix's PITCH / AMP. The platform's GLIDE
 * (P_GLIDE) is the engine's own: E7 GLIDE (voice.c's glide is cancelled at the note, e8_note_on). FILTER, SHAPE and
 * ENV DEST do nothing.
 *
 * State: per part and voice (e8_vs, POOL). Cost: a voice per sample is one interpolated sine, the shaper (one table
 * read at most), two one-poles and the click when it rings: a light engine (cpu/808 in tests/cpu_baseline.txt). */

enum { E8_CLEAN, E8_WARM, E8_HARD, E8_GROWL, E8_FOLD, E8_RAGE, E8_HEAVY, E8_NMODEL };   /* (append only: stored) */
static const char *const N_E8_MODEL[E8_NMODEL] = {"CLEAN", "WARM", "HARD", "GROWL", "FOLD", "RAGE", "HEAVY"};
/* output trim per model (Q12): the models level with WARM at DRIVE 70 by RMS (measured by TrapRoot's ChoralRoot-based tests);
 * CLEAN's falls with DRIVE (a sine at 0, nearly a square at the knee) */
static const uint16_t E8_COMP[E8_NMODEL] = {4096, 3700, 4140, 3800, 5570, 4240, 4096};

/* HEAVY: a shaping curve made from the owner's reference 808 (a heavy, saturated C1 808, TrapRoot 2026-10-30): its
 * steady tone's first 16 harmonics measured (levels, and their signs against the fundamental's phase), then summed as
 * Chebyshev polynomials, sum a_n T_n(x), so a full-scale sine through the curve gives exactly those levels: the third
 * harmonic 10 dB over the fundamental, the second and fourth near it, the rest falling to -35 dB by the 16th. DRIVE
 * sets how hard the sine hits it (0: 30 % of the curve, a rounder, cleaner sub; 127: all of it, the reference's tone).
 * 257 points, x -1 .. +1, Q15 (DC removed for the full sine; the DC blocker takes the rest) */
static const int16_t E8_HEAVY_TAB[257] = {
    7024, 6357, 5416, 4299, 3083, 1825, 568, -656, -1826, -2928, -3954, -4900,
    -5766, -6554, -7266, -7909, -8488, -9008, -9475, -9895, -10273, -10613, -10922, -11202,
    -11458, -11693, -11909, -12109, -12296, -12472, -12638, -12796, -12947, -13092, -13234, -13371,
    -13506, -13640, -13773, -13905, -14038, -14172, -14308, -14445, -14585, -14727, -14872, -15020,
    -15170, -15323, -15478, -15636, -15795, -15956, -16117, -16279, -16440, -16599, -16756, -16909,
    -17058, -17202, -17340, -17470, -17591, -17704, -17805, -17896, -17973, -18038, -18088, -18124,
    -18143, -18147, -18133, -18102, -18054, -17987, -17902, -17799, -17678, -17538, -17380, -17204,
    -17010, -16799, -16571, -16326, -16065, -15789, -15497, -15191, -14872, -14538, -14192, -13834,
    -13464, -13084, -12692, -12291, -11880, -11460, -11031, -10594, -10148, -9695, -9235, -8768,
    -8293, -7812, -7324, -6830, -6329, -5822, -5309, -4791, -4266, -3737, -3202, -2661,
    -2117, -1567, -1014, -456, 104, 667, 1233, 1800, 2368, 2936, 3503, 4069,
    4633, 5193, 5749, 6300, 6845, 7382, 7911, 8430, 8939, 9436, 9921, 10392,
    10849, 11290, 11715, 12124, 12514, 12887, 13241, 13576, 13893, 14190, 14468, 14728,
    14969, 15192, 15398, 15588, 15762, 15921, 16067, 16202, 16326, 16441, 16548, 16650,
    16749, 16845, 16941, 17040, 17141, 17248, 17363, 17487, 17621, 17767, 17928, 18103,
    18294, 18502, 18728, 18972, 19235, 19516, 19816, 20134, 20469, 20820, 21186, 21566,
    21958, 22360, 22769, 23183, 23601, 24018, 24432, 24839, 25238, 25623, 25993, 26344,
    26672, 26975, 27249, 27491, 27698, 27868, 27999, 28087, 28132, 28132, 28084, 27989,
    27845, 27652, 27410, 27118, 26778, 26389, 25953, 25470, 24941, 24367, 23749, 23087,
    22382, 21635, 20844, 20010, 19130, 18203, 17227, 16197, 15109, 13958, 12738, 11442,
    10061, 8586, 7008, 5317, 3501, 1549, -549, -2805, -5227, -7824, -10603, -13567,
    -16717, -20048, -23551, -27209, -31000,
};

typedef struct {
    int32_t env;                 /* the tail, Q24 (1 << 24 = full) */
    int32_t pen;                 /* the punch's envelope, Q24 */
    int32_t glo;                 /* the slide's offset, p16 << 8 (the pitch still to bend, toward 0) */
    int32_t cur16;               /* the pitch it sounded at, p16 << 8 (the note plus the offsets): where a slide starts */
    int32_t lp1, lp2, dc;        /* the tone filter, the DC blocker (GROWL, RAGE) */
    int32_t cle;                 /* the click's envelope, Q15 */
    uint32_t cph, inc;           /* the click's phase; the last block's increment (the ramp's start) */
    uint8_t atk;                 /* blocks of a retriggered tail's ramp still to go (no click then) */
    uint16_t age;                /* blocks since the hit (HEAVY's hold) */
} e8_vs_t;
static e8_vs_t e8_vs[ENG_STATE_NPART][NPOLY] __attribute__((section(".pool")));
#define E8_HOLD ((uint16_t)(FS * 18u / 10u / CTL))   /* 1.8 s in control blocks */
static e8_vs_t *e8_of(const track_t *t, const voice_t *v)
{
    uint32_t part = (uint32_t)(t - trk);
    return part < ENG_STATE_NPART ? &e8_vs[part][(uint32_t)(v - t->v) % NPOLY] : 0;
}

/* 2^(x / 4096) for x in -4..+4 octaves (Q12), Q16 (the fraction by dv_exp2's cubic: within 0.01 %) */
static uint32_t e8_exp2(int32_t x)
{
    uint32_t f, m;
    int32_t k = 0;
    x = clamp(x, -4 * 4096, 4 * 4096);
    while (x < 0) {
        x += 4096;
        k++;
    }
    f = (uint32_t)x & 4095u;
    m = 65536u + ((f * (45594u + ((f * (14851u + ((f * 5092u) >> 12))) >> 12))) >> 12);
    m <<= (uint32_t)x >> 12;
    return m >> k;
}
/* an increment times 2^(off / 192) (off: p16 << 8, 1/16 semitone in Q8) */
static uint32_t e8_bend(uint32_t inc, int32_t off)
{
    uint32_t r = e8_exp2(off / 12);                  /* (an octave is 192 x 256: off / 12 is Q12 octaves) */
    return (uint32_t)(((uint64_t)inc * r) >> 16);
}
/* p16 -> a one-pole's coefficient, Q15 (w / (1 + w), w = 2 pi f / fs): drum_voice.c's dv_pole */
static int32_t e8_pole(int32_t p16)
{
    int32_t w = (int32_t)(((pitch_inc((uint32_t)clamp(p16, 0, 2047)) >> 17) * 205887u) >> 15);
    return (w << 15) / (32768 + w);
}

/* the waveshaper: x Q15 (the sine at DRIVE's gain, up to ~20x), out Q15 */
static inline int32_t e8_shape(uint32_t model, int32_t x, int32_t *dc)
{
    int32_t y;
    switch (model) {
    case E8_WARM:
        return softclip(x);
    case E8_HARD:
        return clamp(x, -26000, 26000) + (clamp(x, -26000, 26000) >> 4);
    case E8_GROWL:                                   /* the positive half pushed twice as hard as the negative */
        y = x > 0 ? softclip(x << 1) : softclip(x >> 1);
        break;
    case E8_FOLD:                                    /* sin(x): x Q15 of a quarter turn (32768 = pi / 2) */
        return sine_i((uint32_t)x << 15) - (sine_i((uint32_t)x << 15) >> 3);
    case E8_HEAVY: {                                 /* the curve, interpolated */
        int32_t u = clamp(x, -32767, 32767) + 32768, i = u >> 8, f = u & 255;
        y = E8_HEAVY_TAB[i] + (((E8_HEAVY_TAB[i + 1] - E8_HEAVY_TAB[i]) * f) >> 8);
        break;
    }
    case E8_RAGE: {
        int32_t s = softclip(x), a = s < 0 ? -s : s;
        y = clamp(s + (s >> 1), -27000, 27000) + ((a - 18000) >> 2);   /* tanh, clipped, a quarter of |x| over it */
        break;
    }
    default:
        return x;
    }
    *dc += (y - *dc) >> 9;                           /* a ~14 Hz DC blocker (the asymmetric shapes) */
    return y - *dc;
}

/* a note on the voice. was: voice_was (0 the voice was free, 1 releasing, 2 its key still down); slide: the note
 * bends from the sounding pitch (keys overlapping, a slide step, LEGATO) */
static void e8_hit(track_t *t, voice_t *v, e8_vs_t *S, uint32_t was, int slide)
{
    const int16_t *p = t->p;
    int32_t to = v->pitch16 << 8;
    v->pitch_cur = v->pitch16;                       /* the slide is the engine's (voice.c's glide cancelled) */
    v->gstep = 0;
    if (slide && was && p[P_E7] > 0) {               /* a slide: from where it sounds, the tail and drive go on */
        S->glo = clamp(S->cur16 - to, -24 * 16 * 256, 24 * 16 * 256);
        return;
    }
    S->glo = 0;
    S->age = 0;
    S->pen = p[P_E2] > 0 ? 1 << 24 : 0;
    S->atk = was && S->env > (1 << 20) ? 2u : 0u;    /* struck while it rings: a 2-block ramp up, no click */
    if (!S->atk) {
        S->env = 1 << 24;
        S->cle = p[P_E6] * 258;
        S->cph = 0;
        S->lp1 = S->lp2 = S->dc = 0;
        S->inc = 0;
        v->ph[0] = 0;                                /* the sine from its zero crossing, rising: the same punch every
                                                      * hit (a fresh voice: voice_start keeps no phase) */
    }
}
static void e8_note_on(track_t *t, voice_t *v)
{
    e8_vs_t *S = e8_of(t, v);
    if (S)
        e8_hit(t, v, S, voice_was, voice_was == 2u || (voice_was == 1u && t->slide_glide));
}
static void e8_legato(track_t *t, voice_t *v)        /* LEGATO: a new note on the sounding voice: a slide */
{
    e8_vs_t *S = e8_of(t, v);
    if (S)
        e8_hit(t, v, S, 2u, 1);
}

/* the voice's level: the tail times the ADSR (the gate and its release); once a block */
static int32_t e8_amp(track_t *t, voice_t *v, int32_t adsr)
{
    const int16_t *p = t->p;
    e8_vs_t *S = e8_of(t, v);
    if (!S)
        return 0;
    if (S->atk) {
        S->env += ((1 << 24) - S->env) >> 1;
        S->atk--;
    } else if (p[P_E4] < 127) {                      /* DECAY: ~77 ms .. 10 s to -40 dB; 127: held */
        S->env -= mulq16(S->env, ENV_EXP[60 + (uint32_t)p[P_E4] * 66u / 126u]);
    } else if (p[P_E0] == E8_HEAVY) {                /* HEAVY at 127: held 1.8 s, then -14 dB a second (the reference
                                                  * one-shot's tail, measured), the key down or not */
        if (S->age < E8_HOLD)
            S->age++;
        else
            S->env -= mulq16(S->env, 77);            /* (1 - 10^(-0.7 / 1378)) in Q16: -14 dB over 1378 blocks */
    }
    return mulq15(S->env >> 9, adsr);
}

static void e8_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    e8_vs_t *S = e8_of(t, v);
    uint32_t model = (uint32_t)clamp(p[P_E0], 0, E8_NMODEL - 1), i, ph = v->ph[0], inc0, inc1, cinc;
    int32_t g, comp, a1, a2, cle, kc, punch, dinc, lp1, lp2, dc, hg;
    if (!S)
        return;
    if (n > CTL)
        n = CTL;
    /* the pitch: the note (m->inc) bent by the punch and the slide still to go */
    punch = (int32_t)(((int64_t)(p[P_E2] * 774) * (S->pen >> 9)) >> 15);   /* PUNCH: 0 .. 24 semitones (p16 << 8) */
    S->pen -= mulq16(S->pen, ENV_EXP[8 + (uint32_t)p[P_E3] * 58u / 127u]);   /* SNAP: ~2 .. 140 ms */
    if (S->glo) {                                    /* GLIDE: ~6 ms .. 1.2 s (99 % of the way) */
        S->glo -= mulq16(S->glo, ENV_EXP[24 + (uint32_t)p[P_E7] * 64u / 127u]);
        if (S->glo > -64 && S->glo < 64)
            S->glo = 0;
    }
    S->cur16 = (v->pitch16 << 8) + S->glo;
    inc1 = e8_bend(m->inc, punch + S->glo);
    inc0 = S->inc ? S->inc : inc1;
    S->inc = inc1;
    dinc = ((int32_t)(inc1 >> 1) - (int32_t)(inc0 >> 1)) >> (CTL_LOG2 - 1);
    /* DRIVE: x1 .. x20 (exponential), the model's trim */
    g = (int32_t)e8_exp2((int32_t)p[P_E1] * 4096 * 43 / (127 * 10));   /* Q16: 2^(4.3 d) */
    comp = model == E8_CLEAN ? 4096 - p[P_E1] * 12 : E8_COMP[model];
    hg = 19661 + p[P_E1] * 45875 / 127;              /* HEAVY: 30 % .. 100 % of its curve (Q16) */
    hg -= (int32_t)(((int64_t)(hg * 3 / 5) * (S->pen >> 9)) >> 15);   /* (.. less of it while the punch falls: the
                                                  * reference swells from a round sweep into its full tone, ~70 ms) */
    /* TONE: two one-poles at ~120 Hz .. 12 kHz */
    a1 = e8_pole(749 + p[P_E5] * 1300 / 127);
    a2 = a1;
    cle = S->cle;
    kc = 32768 - 1350;                               /* the click: tau ~0.5 ms */
    cinc = 0x8F5C28Fu;                               /* 1.54 kHz */
    lp1 = S->lp1;
    lp2 = S->lp2;
    dc = S->dc;
    for (i = 0; i < n; i++) {
        int32_t x, y;
        ph += inc0 + (uint32_t)(dinc * (int32_t)i);
        x = sine_i(ph);
        if (model == E8_HEAVY)                       /* (HEAVY: DRIVE is how much of the curve, not a gain) */
            x = (int32_t)(((int64_t)x * hg) >> 16);
        else if (model != E8_CLEAN || p[P_E1])
            x = (int32_t)(((int64_t)x * g) >> 16);
        y = e8_shape(model, model == E8_CLEAN ? soft_knee(x, 24000) : x, &dc);
        if (cle > 40) {                              /* CLICK: a damped 1.5 kHz burst on the hit */
            S->cph += cinc;
            y += mulq15(sine_i(S->cph), cle);
            cle = mulq15(cle, kc);
        }
        lp1 += mulq15(y - lp1, a1);
        lp2 += mulq15(lp1 - lp2, a2);
        y = (lp2 * comp) >> 12;
        out[i] += voice_amp(y, m, i);
    }
    v->ph[0] = ph;
    S->cle = cle;
    S->lp1 = lp1;
    S->lp2 = lp2;
    S->dc = dc;
}

/* the factory 808s: {MODEL DRIVE PUNCH SNAP DECAY TONE CLICK GLIDE}, {A D S R} (the gate: A 0, S full), mono */
static const preset_t E8_PRESETS[] = {
    /* HEAVY: the owner's reference, matched by measurement (its tone through E8_HEAVY_TAB, within 0.5 dB on the first
     * eight harmonics at C1; the punch sweeping two octaves down in ~70 ms with the curve swelling in; the tail held, then
     * held 1.8 s, then falling about 14 dB a second, as the one-shot's tail: e8_amp) */
    {"HEAVY", {E8_HEAVY, 127, 127, 105, 127, 110, 90, 60}, {0, 64, 127, 60}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"CARSON", {E8_RAGE, 70, 60, 40, 100, 100, 30, 60}, {0, 64, 127, 48}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"SLIDE", {E8_WARM, 50, 45, 36, 108, 90, 20, 72}, {0, 64, 127, 50}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"CLEAN", {E8_CLEAN, 0, 40, 34, 104, 70, 12, 56}, {0, 64, 127, 50}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"HARD", {E8_HARD, 60, 55, 40, 96, 104, 36, 56}, {0, 64, 127, 46}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"BLOWN", {E8_RAGE, 104, 70, 46, 98, 112, 40, 64}, {0, 64, 127, 46}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"GROWL", {E8_GROWL, 80, 50, 40, 102, 96, 24, 60}, {0, 64, 127, 48}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"TALK", {E8_FOLD, 58, 50, 44, 100, 108, 18, 64}, {0, 64, 127, 48}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"SHORT", {E8_WARM, 44, 64, 30, 70, 90, 44, 40}, {0, 64, 127, 40}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"ROUND SUB", {E8_WARM, 22, 24, 30, 116, 60, 8, 60}, {0, 64, 127, 54}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"DIRTY", {E8_HARD, 92, 60, 42, 104, 120, 32, 66}, {0, 64, 127, 48}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
};

static const engine_t ENG_808 = {
    .name = "808",
    .page_title = {"808", "SHAPE"},
    .edit = {
        {"MODEL", F_ENUM, 0, E8_NMODEL - 1, E8_WARM, N_E8_MODEL, 0},
        {"DRIVE", F_PCT, 0, 127, 50, 0, 0},
        {"PUNCH", F_PCT, 0, 127, 45, 0, 0},
        {"SNAP", F_PCT, 0, 127, 36, 0, 0},
        {"DECAY", F_PCT, 0, 127, 104, 0, 0},
        {"TONE", F_PCT, 0, 127, 90, 0, 0},
        {"CLICK", F_PCT, 0, 127, 20, 0, 0},
        {"GLIDE", F_PCT, 0, 127, 60, 0, 0},
    },
    .presets = E8_PRESETS,
    .npresets = NELEM(E8_PRESETS),
    .knob = {P_E1, P_E4, P_E5, P_E7},
    .poly = 2,
    .note_on = e8_note_on,
    .render = e8_render,
    .amp = e8_amp,
    .legato = e8_legato,
};
