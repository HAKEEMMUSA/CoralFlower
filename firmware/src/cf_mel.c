/* SPDX-License-Identifier: GPL-3.0-only
 * Leviathan: the melody engine (the ARP button's Melody mode), in the audio ISR. Included by cf_arp.c.
 *
 * The chord the keys hold keeps sounding on part 0; over it this plays a melody on the LAYER (part 2, its own sound),
 * in time: a 16th a step, locked to the beat while the clock runs (a rhythm, a pattern, the metronome), else from the
 * first chord. It is built the way a synth is (the owner, 2026-10-30: "dual-layer architecture or oscillator algorithms
 * or modulation matrix ... like Electra X, Serum and Surge"):
 *   - two layers: a RHYTHM layer (when a note starts) and a PITCH layer (which note), each an oscillator with
 *     algorithms: the rhythm's EUCLID BOUNCE DRIVE SYNC RAGE RANDOM, at a DENSITY; the pitch's contour SINE TRI RISE
 *     FALL SQUARE STEPS WALK ARP, at a RATE (cycles a phrase), over a RANGE above the chord's root, its value
 *     quantised to the chord's tones (on the beats, and TONES % of the rest) or the scale's;
 *   - two LFOs (SIN TRI SAW SQR S&H, a quarter bar to eight bars a cycle, on the phrase's clock);
 *   - a four-slot modulation matrix: a source (LFO 1, LFO 2, RAMP the phrase's position, CHORD a fall from each new
 *     chord over a bar, RANDOM a step's own, BEAT the beats, HEIGHT the last note's place in the range) times an amount
 *     (-100 .. +100) onto a destination (DENSITY PITCH RANGE OCTAVE GATE VELO SLIDE TONES ROTATE RATE).
 * A phrase is BARS long and plays again (VARY % of its steps change on a repeat); SEED picks another phrase; the same
 * settings, seed and chord give the same notes (a hash of seed and step, no state), so the screen shows what will
 * play. Integer only (Q12: 4096 = 1.0); no 64-bit division (the device's toolchain has none). */

#define ML_Q 4096                                    /* Q12 */
static const uint8_t ML_RATE_X2[] = {1, 2, 4, 6, 8, 16};          /* the contour's cycles a phrase x 2: 1/2 .. 8 */
#define ML_NRATE ((uint32_t)sizeof ML_RATE_X2)
static const uint8_t ML_LRATE[] = {4, 8, 16, 32, 64, 128};        /* an LFO's cycle in 16ths: 1/4 bar .. 8 bars */
#define ML_NLRATE ((uint32_t)sizeof ML_LRATE)

/* the rhythms' steps by priority (DENSITY takes the first so many), a bar of 16: the strong steps first */
static const uint8_t ML_RANK[ML_NR][16] = {
    {0},                                                                  /* EUCLID: computed */
    {0, 6, 12, 3, 8, 14, 10, 4, 2, 15, 9, 13, 1, 5, 7, 11},              /* BOUNCE: 3-3-2 and its pickups */
    {0, 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15},              /* DRIVE: on the beat, then 8ths, 16ths */
    {2, 6, 10, 14, 3, 11, 7, 15, 0, 9, 5, 13, 1, 4, 8, 12},              /* SYNC: off the beat */
    {0, 3, 6, 10, 12, 14, 8, 2, 11, 4, 15, 7, 5, 9, 13, 1},              /* RAGE: the rage leads' stabs */
    {0},                                                                  /* RANDOM: a coin a step */
};

/* the notes the pitch layer picks from: the chord's tones, the scale's (the chord's in it too), over the range */
typedef struct {
    uint8_t lo, hi;                                  /* the range, MIDI notes */
    uint8_t nc, ns, ct[40], sc[40];                  /* chord tones, scale tones in it, low to high */
} ml_pool_t;

static uint32_t ml_hash(uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t h = a * 0x9E3779B1u ^ (b + 0x7F4A7C15u) * 0x85EBCA77u ^ (c + 0x165667B1u) * 0xC2B2AE3Du;
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    h *= 0x297A2D39u;
    return h ^ (h >> 15);
}
static int32_t ml_bip(uint32_t h) { return (int32_t)(h >> 19) - ML_Q; }   /* a hash as -1 .. +1 (Q12) */
static int32_t ml_clamp(int32_t v, int32_t a, int32_t b) { return v < a ? a : v > b ? b : v; }

/* a wave at phase ph (0 .. 4095 a cycle), -1 .. +1; S&H: a hash of the cycle */
static int32_t ml_wave(uint32_t w, uint32_t ph, uint32_t cycle, uint32_t seed)
{
    ph &= 4095u;
    switch (w) {
    case ML_W_TRI: return ph < 2048u ? (int32_t)ph * 4 - ML_Q : 3 * ML_Q - (int32_t)ph * 4;
    case ML_W_SAW: return (int32_t)ph * 2 - ML_Q;
    case ML_W_SQR: return ph < 2048u ? ML_Q : -ML_Q;
    case ML_W_SH: return ml_bip(ml_hash(seed, cycle, 0x5348u));
    default: return sine_i(ph << 20) >> 3;           /* (Q15 -> Q12) */
    }
}

/* the pool: the chord (low to high, its root's pitch class), the key, the settings, the range moved by mods */
static void ml_pool(ml_pool_t *P, const uint8_t *note, uint32_t n, uint32_t root_pc, const cf_mel_t *M,
                    int32_t range_mod, int32_t oct_mod)
{
    uint32_t cm = 0, sm, i;
    int32_t lo, span, o;
    for (i = 0; i < n; i++)
        cm |= 1u << (note[i] % 12u);
    if (!cm)
        cm = 1u << (root_pc % 12u);
    if (cfc.cfg.quantize != CF_Q_OFF) {              /* the key's scale, turned to its tonic */
        uint32_t m = CF_SCALES[cfc.cfg.scale < CF_NSCALES ? cfc.cfg.scale : 0].mask, t = cfc.cfg.tonic % 12u;
        sm = ((m << t) | (m >> (12u - t))) & 0xFFFu;
    } else {                                         /* (no key: the chord's own minor or major) */
        uint32_t m = (cm >> ((root_pc + 3u) % 12u)) & 1u ? 0x5ADu : 0xAB5u, t = root_pc % 12u;
        sm = ((m << t) | (m >> (12u - t))) & 0xFFFu;
    }
    sm |= cm;
    o = ml_clamp(M->oct + (oct_mod >= 0 ? (oct_mod + ML_Q / 2) / ML_Q : -((-oct_mod + ML_Q / 2) / ML_Q)), -3, 3);
    lo = 60 + (int32_t)(root_pc % 12u) + 12 * o;
    span = ml_clamp((int32_t)M->range + (range_mod * 24) / ML_Q, 5, 40);
    lo = ml_clamp(lo, 24, 100);
    P->lo = (uint8_t)lo;
    P->hi = (uint8_t)ml_clamp(lo + span, lo, 120);
    P->nc = P->ns = 0;
    for (i = P->lo; i <= P->hi; i++) {
        if (((cm >> (i % 12u)) & 1u) && P->nc < sizeof P->ct)
            P->ct[P->nc++] = (uint8_t)i;
        if (((sm >> (i % 12u)) & 1u) && P->ns < sizeof P->sc)
            P->sc[P->ns++] = (uint8_t)i;
    }
    if (!P->nc)
        P->ct[P->nc++] = P->lo;
    if (!P->ns)
        P->sc[P->ns++] = P->lo;
}

/* the matrix's sum at step k (16ths since the phrase began), Q12 per destination. chord_age: 16ths since the chord
 * changed; height: the last note's place in the range (Q12, 0 .. 1) */
static void ml_mods(const cf_mel_t *M, uint32_t k, uint32_t chord_age, int32_t height, int32_t *d)
{
    uint32_t L = 16u * (M->bars ? M->bars : 1u), s = k % L, j;
    int32_t src[ML_NS];
    for (j = 0; j < ML_ND; j++)
        d[j] = 0;
    src[ML_S_OFF] = 0;
    for (j = 0; j < 2u; j++) {
        uint32_t per = ML_LRATE[M->lfo_r[j] < ML_NLRATE ? M->lfo_r[j] : 2u];
        src[ML_S_LFO1 + j] = ml_wave(M->lfo_w[j], (k % per) * 4096u / per, k / per, M->seed + 77u * j);
    }
    src[ML_S_RAMP] = (int32_t)(s * 2u * ML_Q / L) - ML_Q;
    src[ML_S_CHORD] = chord_age < 16u ? ML_Q - (int32_t)chord_age * (ML_Q / 16) : 0;
    src[ML_S_RAND] = ml_bip(ml_hash(M->seed, s, 0x52u));
    src[ML_S_BEAT] = (s & 3u) == 0u ? ML_Q : (s & 1u) == 0u ? 0 : -ML_Q;
    src[ML_S_HEIGHT] = height * 2 - ML_Q;
    for (j = 0; j < ML_NMOD; j++)
        if (M->src[j] && M->src[j] < ML_NS && M->dst[j] && M->dst[j] < ML_ND)
            d[M->dst[j]] += src[M->src[j]] * M->amt[j] / 100;
}

/* the rhythm layer: does step k (16ths since the phrase began) start a note */
static int ml_hit(const cf_mel_t *M, uint32_t k, const int32_t *d)
{
    uint32_t L = 16u * (M->bars ? M->bars : 1u), s = k % L, bar = s / 16u, i = s % 16u, rep = k / L, var, key;
    int32_t den = ml_clamp((int32_t)M->density * ML_Q / 100 + d[ML_D_DENSITY] / 2, 0, ML_Q);
    uint32_t n = (uint32_t)((den * 16 + ML_Q / 2) / ML_Q), alg = M->rhythm < ML_NR ? M->rhythm : 0u, r;
    int32_t rot = d[ML_D_ROTATE] * 4 / ML_Q;
    var = ml_hash(M->seed, s, 0x56u) % 100u < M->vary ? rep : 0u;   /* (VARY: this step changes on a repeat) */
    key = M->seed * 131u + var;
    i = (i + 16u - (uint32_t)(rot & 15)) % 16u;
    if (!n)
        return 0;
    if (alg == ML_R_RANDOM)
        return ml_hash(key, s, 0x48u) % 16u < n;
    if (alg == ML_R_EUCLID) {                        /* (n hits spread over 16, turned by the seed and the bar) */
        uint32_t t = (i + ml_hash(M->seed, bar, 0x45u) % 4u) % 16u;
        return (t * n) % 16u < n;
    }
    for (r = 0; r < 16u; r++) {                      /* its place in the priority, the seed swapping neighbours */
        uint32_t p = ML_RANK[alg][r];
        if (r + 1u < 16u && (ml_hash(key, bar * 16u + r, 0x53u) & 3u) == 0u) {
            if (p == i)
                return r + 1u < n;
            if (ML_RANK[alg][r + 1u] == i)
                return r < n;
        }
        if (p == i)
            return r < n;
    }
    return 0;
}

/* the pitch layer's contour at step k, 0 .. 1 (Q12) */
static int32_t ml_contour(const cf_mel_t *M, uint32_t k, const int32_t *d, uint32_t key)
{
    uint32_t L = 16u * (M->bars ? M->bars : 1u), s = k % L;
    int32_t cyc2 = ML_RATE_X2[M->rate < ML_NRATE ? M->rate : 2u] + d[ML_D_RATE] * 4 / ML_Q, c;
    uint32_t ph;
    cyc2 = ml_clamp(cyc2, 1, 32);
    ph = s * 4096u * (uint32_t)cyc2 / (2u * L);
    switch (M->contour < ML_NC ? M->contour : 0u) {
    case ML_C_TRI: c = ml_wave(ML_W_TRI, ph, 0, 0); break;
    case ML_C_RISE: c = ml_wave(ML_W_SAW, ph, 0, 0); break;
    case ML_C_FALL: c = -ml_wave(ML_W_SAW, ph, 0, 0); break;
    case ML_C_SQUARE: c = ml_wave(ML_W_SQR, ph, 0, 0) * 3 / 5; break;
    case ML_C_STEPS: c = ml_bip(ml_hash(key, (s * (uint32_t)cyc2 * 2u) / L, 0x43u)); break;
    case ML_C_WALK: {                                /* (a walk of the phrase's steps so far, kept in the range) */
        uint32_t j;
        c = 0;
        for (j = 0; j <= s; j++)
            c = ml_clamp(c + ml_bip(ml_hash(key, j, 0x57u)) / 3, -ML_Q, ML_Q);
        break;
    }
    case ML_C_ARP: c = (int32_t)((s * (uint32_t)cyc2) % 8u) * 2 * ML_Q / 7 - ML_Q; break;
    default: c = ml_wave(ML_W_SIN, ph, 0, 0); break;
    }
    return ml_clamp((c + ML_Q) / 2 + d[ML_D_PITCH] / 2, 0, ML_Q);
}

/* a note of the melody */
typedef struct { uint8_t on, note, vel, slide; uint16_t len24; } ml_ev_t;

/* step k: a note or a rest. note / n / root_pc: the chord; vel: the keys' velocity; prev: the last note (0 none) */
static void ml_gen(const cf_mel_t *M, const uint8_t *note, uint32_t n, uint32_t root_pc, uint32_t k,
                   uint32_t chord_age, uint32_t prev, uint32_t vel, ml_ev_t *e)
{
    int32_t d[ML_ND], c, tgt, best, g, v;
    ml_pool_t P;
    uint32_t L = 16u * (M->bars ? M->bars : 1u), s = k % L, rep = k / L, var, key, gap, j, strong, chord_tone;
    const uint8_t *pool;
    uint32_t np;
    e->on = 0;
    ml_mods(M, k, chord_age, ML_Q / 2, d);
    if (!ml_hit(M, k, d))
        return;
    var = ml_hash(M->seed, s, 0x56u) % 100u < M->vary ? rep : 0u;
    key = M->seed * 131u + var;
    ml_mods(M, k, chord_age, prev ? 0 : ML_Q / 2, d);
    ml_pool(&P, note, n, root_pc, M, d[ML_D_RANGE], d[ML_D_OCTAVE]);
    if (prev && P.hi > P.lo)                         /* (HEIGHT: the last note's place, now the range is known) */
        ml_mods(M, k, chord_age, ml_clamp(((int32_t)prev - P.lo) * ML_Q / (P.hi - P.lo), 0, ML_Q), d);
    c = ml_contour(M, k, d, key);
    tgt = P.lo * ML_Q + c * (P.hi - P.lo);
    strong = (s & 3u) == 0u;
    chord_tone = strong || ml_hash(key, s, 0x54u) % 100u <
                           (uint32_t)ml_clamp(M->tones + d[ML_D_TONES] * 100 / ML_Q, 0, 100);
    pool = chord_tone ? P.ct : P.sc;
    np = chord_tone ? P.nc : P.ns;
    best = pool[0];
    for (j = 1; j < np; j++) {                       /* (the tone nearest the contour) */
        int32_t a = (int32_t)pool[j] * ML_Q - tgt, b = best * ML_Q - tgt;
        if ((a < 0 ? -a : a) < (b < 0 ? -b : b))
            best = pool[j];
    }
    /* its length: to the next note (16 steps at most), GATE % of it */
    for (gap = 1; gap < 16u; gap++) {
        int32_t d2[ML_ND];
        ml_mods(M, k + gap, chord_age + gap, ML_Q / 2, d2);
        if (ml_hit(M, k + gap, d2))
            break;
    }
    g = ml_clamp((int32_t)M->gate + d[ML_D_GATE] * 100 / ML_Q, 5, 100);
    e->len24 = (uint16_t)ml_clamp((int32_t)(gap * 6u) * g / 100, 1, 96);
    v = (int32_t)vel + (strong ? M->accent : 0) + d[ML_D_VELO] * 40 / ML_Q;
    e->vel = (uint8_t)ml_clamp(v, 1, 127);
    e->slide = (uint8_t)(ml_hash(key, s, 0x4Cu) % 100u <
                         (uint32_t)ml_clamp(M->slide + d[ML_D_SLIDE] * 100 / ML_Q, 0, 100));
    e->note = (uint8_t)best;
    e->on = 1;
}

/* ------------------------------------------------------------- the player --- */
static struct {
    uint8_t n, note[CF_MAX_NOTES], root_pc;          /* the chord it plays over (0 notes: none) */
    uint8_t latched;                                 /* the keys are up, LATCH keeps it */
    uint8_t cur;                                     /* the note sounding (0: none) */
    uint32_t left;                                   /* samples until it is let go */
    uint32_t k;                                      /* the next step (16ths since the melody began) */
    uint64_t acc;                                    /* free: samples x BPM x 2400 since the last step */
    uint8_t locked;                                  /* locked to the beat: the next step's place (1/24 beats) */
    uint32_t next24, gen;
    uint32_t chord_k;                                /* the step the chord last changed at */
    uint8_t prev;                                    /* the last note played */
    uint8_t steps, last;                             /* for the screen: bumped at every note; the last note */
    uint32_t shown_k;                                /* .. the step it played last */
} mel;

static void ml_off(void)
{
    if (mel.cur)
        cfp_off(2, mel.cur);
    mel.cur = 0;
}
static void mel_stop(void)
{
    ml_off();
    mel.n = 0;
    mel.latched = 0;
}
/* a 16th's place in 1/24 beats: every second one late by SWING (the ARP's) */
static uint32_t ml_second(void)
{
    uint32_t sw = cfp_cfg.arp.swing >= 50u && cfp_cfg.arp.swing <= 75u ? cfp_cfg.arp.swing : 50u;
    return (12u * sw + 50u) / 100u;
}
static uint32_t ml_place(uint32_t k) { return (k / 2u) * 12u + ((k & 1u) ? ml_second() : 0u); }
static uint32_t ml_step_at(uint32_t g)               /* the first step whose place is at g or after */
{
    uint32_t m = g / 12u, r = g % 12u;
    return r == 0u ? 2u * m : r <= ml_second() ? 2u * m + 1u : 2u * m + 2u;
}

static void ml_play(uint32_t k)
{
    ml_ev_t e;
    ml_gen(&cfp_cfg.mel, mel.note, mel.n, mel.root_pc, k, k - mel.chord_k, mel.prev, cfp_cfg.vel, &e);
    if (!e.on)
        return;
    if (e.slide && mel.cur && mel.cur != e.note) {   /* a slide: the new note under the old one's gate, legato */
        uint8_t old = mel.cur;
        cfp_on(2, e.note, e.vel);
        cfp_off(2, old);
    } else {
        ml_off();
        cfp_on(2, e.note, e.vel);
    }
    mel.cur = e.note;
    mel.left = cfa_div64((uint64_t)e.len24 * FS * 60u, cfa_bpm() * 24u);   /* (1/24 beats -> samples) */
    mel.prev = e.note;
    mel.steps++;
    mel.last = e.note;
    mel.shown_k = k;
}

/* the chord the keys ask for (cfa_chord, ARP on in Melody): m notes, or none */
static void mel_chord(const cf_chord_out_t *o, int m)
{
    int i, fresh;
    if (cfp_cp == 2u)                                /* (the keys play the LAYER: no melody over it) */
        m = 0;
    if (!m) {
        if (cfp_cfg.arp.latch && mel.n) {
            mel.latched = 1;
            return;
        }
        mel_stop();
        return;
    }
    fresh = !mel.n || mel.latched;
    mel.latched = 0;
    if (mel.n && (mel.root_pc != o->root_pc || mel.n != (uint8_t)m))
        mel.chord_k = mel.k;
    mel.n = (uint8_t)(m > CF_MAX_NOTES ? CF_MAX_NOTES : m);
    for (i = 0; i < mel.n; i++)
        mel.note[i] = o->note[i];
    mel.root_pc = o->root_pc;
    if (!fresh)
        return;
    mel.chord_k = 0;
    mel.prev = 0;
    if (cf_grid.on) {                                /* locked: the step on the beat's grid, now if it falls now */
        mel.k = ml_step_at(cf_grid.g24);
        mel.next24 = ml_place(mel.k);
        mel.gen = cf_grid.gen;
        mel.locked = 1;
        mel.chord_k = mel.k;
        if (mel.next24 <= cf_grid.g24 + 1u) {
            ml_play(mel.k);
            mel.k++;
            mel.next24 = ml_place(mel.k);
        }
    } else {
        mel.locked = 0;
        mel.k = 0;
        mel.acc = 0;
        ml_play(mel.k++);
    }
}

/* every audio block: the gate, the next step */
static void mel_tick(uint32_t n)
{
    if (mel.cur) {
        if (mel.left <= n)
            ml_off();
        else
            mel.left -= n;
    }
    if (!mel.n)
        return;
    if (cf_grid.on) {
        uint32_t g = cf_grid.g24;
        if (!mel.locked || mel.gen != cf_grid.gen || mel.next24 > g + 48u) {   /* (the grid started again) */
            mel.k = ml_step_at(g);
            mel.next24 = ml_place(mel.k);
            mel.gen = cf_grid.gen;
            mel.locked = 1;
            return;
        }
        if (g >= mel.next24) {
            ml_play(mel.k);
            mel.k++;
            mel.next24 = ml_place(mel.k);
        }
        return;
    }
    if (mel.locked) {                                /* (the clock stopped: free from here) */
        mel.locked = 0;
        mel.acc = 0;
    }
    mel.acc += (uint64_t)n * cfa_bpm() * 2400u;
    {
        /* the step just played (k - 1) lasts its place to the next one's, in 1/24 beats; samples x BPM x 2400 */
        uint32_t u = ml_place(mel.k) - ml_place(mel.k - 1u);
        uint64_t len = (uint64_t)FS * 60u * u * 100u;
        if (mel.k && mel.acc >= len) {
            mel.acc -= len;
            ml_play(mel.k++);
        }
    }
}
