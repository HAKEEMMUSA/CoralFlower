/* SPDX-License-Identifier: GPL-3.0-only
 * CoralFlower: the drum machine (0.6; the owner, 2026-10-28: "Да, подходят"). Ready rhythms of 1 or 2 bars, each for
 * one TIME SIG, played on the drums' part (the DRUM engine and the second one's kits, as a pattern's DRUMS) while the
 * keys play chords on HOME. The audio side: the rhythms and their player. Included by cf_out.c after cf_seqp.c: a
 * rhythm and the sequencer never play together (one thing plays: the player stops the other as it starts).
 *
 * A rhythm is its drums' hits as text, one string a drum, its bars one after another ('x' a hit, '.' none): the
 * drums' grid of the metronome's beats x 4 steps (x 2 in a bar of more than 4 beats, sq_dsteps), as a pattern's
 * DRUMS. Written for CoralFlower from the common grooves of each style, no other instrument's patterns. */

enum { DM_K, DM_S, DM_C, DM_H, DM_O, DM_LT, DM_MT, DM_HT, DM_R, DM_B, DM_X };   /* (the lanes: cf_seqd.c SQ_NLANE) */
typedef struct {
    const char *name;            /* 9 letters at most: "Breakcore" fits the page's title and the plaque */
    uint8_t id;                  /* its own number, never reused: what the user's KIT and SWING and the rhythm stood on
                                  * are kept by (cf_dmui.c); rc3's rhythms kept theirs (0 .. 28), new ones go on from 29 */
    uint8_t beats, bars;         /* its TIME SIG (cs.beats), its length */
    uint8_t kit, swing;          /* its own kit (SQ_KITS) and SWING (0..100 %, as a pattern's) */
    uint8_t bpm;                 /* the tempo its style lives at (shown, never set: the tempo is the user's) */
    const char *lane[SQ_NLANE];  /* each drum's hits, bar after bar; 0: the drum is not in it */
    uint8_t lev[SQ_NLANE];       /* .. its level, 0..100 % (a pattern's dlev) */
    uint8_t line;                /* TrapRoot: its 808 pattern (DM8_LINES index + 1; 0 none): while it plays the bass part
                                  * plays it (cf_play.c cfp_sync: the keys only set its root) */
} dm_rhythm_t;

/* TrapRoot: the 808 patterns (dm8_lines.h, generated from the owner's MIDI files by tools/trap/mid2lines.py; Rage's
 * one hit from their FL Studio pattern): each note at its own tick, its velocity, a slide or not, one-shot */
#include "dm8_lines.h"
/* TrapRoot: a hit's character in a lane: 'x' a hit, 'X' an accent, 'o' a ghost note, '2' '3' '4' '6' '8' a roll of
 * that many hits inside the step (the trap hat rolls; the pattern's DRUMS keep its first hit) */

/* the kits by name (SQ_KITS' order) */
enum { DMK_STD, DMK_808, DMK_909, DMK_TIGHT, DMK_BOOM, DMK_LOFI, DMK_HAND, DMK_CRUSH, DMK_XCLUB, DMK_XDEEP, DMK_XLOFI,
       DMK_XLIVE, DMK_XTECH, DMK_XBREAK, DMK_XRAGE };

/* CoralFlower's own rhythms are out of TrapRoot (ids 0 .. 47 are the 808 patterns' now: a KIT or SWING a CoralFlower
 * user kept on one of theirs lands on an 808 pattern, where neither does anything) */
static const dm_rhythm_t DM_RHY[] = {
    /* TrapRoot (the owner, 2026-10-30: "Aside from the rage lets remove the other ones ... the rhythms should only be
     * just 808 patterns"): Rage, its drums and its 808; then the owner's 808 patterns, the 808 alone (DM8_RHYTHMS) */
    /* Rage: the owner's pick (an FL Studio pattern at 180 BPM, read from its channel rack): no kick, the 808 is the
     * low end and hits once every two bars; the snare on 1, 7, 13 and 21, 27 (the rage bounce), the clap on 3 of each
     * bar, one open hat (step 13), one shaker (bar 2's step 3, on the hat lane: the kits have no shaker) */
    {"Rage", 48, 4, 2, DMK_XRAGE, 0, 180,
     {[DM_S] = "x.....x.....x..." "....x.....x.....", [DM_C] = "........x......." "........x.......",
      [DM_O] = "............x..." "................", [DM_H] = "................" "..x............."},
     {[DM_S] = 100, [DM_C] = 85, [DM_O] = 55, [DM_H] = 50},
     DL_RAGE},
    DM8_RHYTHMS
};
#define DM_N ((uint32_t)(sizeof DM_RHY / sizeof DM_RHY[0]))

/* the rhythms of a TIME SIG: how many, the k-th (DM_N: none) */
static uint32_t dm_count(uint32_t beats)
{
    uint32_t i, n = 0;
    for (i = 0; i < DM_N; i++)
        n += DM_RHY[i].beats == beats;
    return n;
}
static uint32_t dm_nth(uint32_t beats, uint32_t k)
{
    uint32_t i;
    for (i = 0; i < DM_N; i++)
        if (DM_RHY[i].beats == beats && !k--)
            return i;
    return DM_N;
}
static uint32_t dm_place(uint32_t r)             /* .. its place in its TIME SIG's list */
{
    uint32_t i, k = 0;
    for (i = 0; i < r && i < DM_N; i++)
        k += DM_RHY[i].beats == DM_RHY[r % DM_N].beats;
    return k;
}
/* a bar's drum steps in a TIME SIG of so many beats (as cf_seqd.c sq_dsteps for the one now) */
static uint32_t dm_steps(uint32_t beats) { return beats * 4u <= 16u ? beats * 4u : beats * 2u; }
/* drum l's hits in bar b of rhythm r: a pattern's DRUMS bits (bit i, step i), in the rhythm's own TIME SIG */
static uint32_t dm_mask(uint32_t r, uint32_t l, uint32_t b)
{
    const dm_rhythm_t *y = &DM_RHY[r % DM_N];
    const char *s = y->lane[l % SQ_NLANE];
    uint32_t sd = dm_steps(y->beats), i, m = 0, lb;
    if (!s || b >= y->bars)
        return 0;
    lb = str_len(s) / sd;                        /* (TrapRoot: a lane shorter than the rhythm plays round) */
    if (!lb)
        return 0;
    s += (b % lb) * sd;
    for (i = 0; i < sd && s[i]; i++)
        if (s[i] != '.')
            m |= 1u << i;
    return m;
}

/* ------------------------------------------------------------ the player --- */
/* PLAY in the drum machine (or on HOME, the rhythm last chosen): its bars round, on the sequencer's clock (96 ticks a
 * beat from the tempo; MIDI clock in: its pulses), the metronome's beat 1 at its start. Another rhythm picked while it
 * plays comes in at the next bar line, from its first bar. REC in the SEQUENCER while it plays: it ends its bar (the
 * next too, with less than a beat of it left: the player gets ready), then the pattern on screen plays and the take
 * goes into it (rin) */
static struct {
    uint8_t on;                  /* a rhythm plays */
    uint8_t r, kit, swing;       /* .. which, its kit and SWING as played (its own or the user's) */
    uint8_t next, nkit, nswing;  /* .. the one coming in at the next bar line (0xFF none) */
    uint8_t bar;                 /* .. its bar, 0 .. its bars - 1 */
    uint32_t nbar;               /* .. bars since it began */
    uint32_t t, acc;             /* .. ticks into the bar; the clock's samples x bpm */
    uint8_t nh;                  /* .. the next step not struck yet */
    uint8_t rin, rtrk, rbars;    /* REC in the SEQUENCER: pattern + 1 (0 none), its track, bar lines to wait */
} dmp = {.next = 0xFF};

static uint32_t dmp_tick_of(uint32_t i)          /* where step i sounds: every second one late by SWING */
{
    uint32_t bt = sqp_bar_ticks(), sd = sq_dsteps(), t = i * bt / sd;
    if (i & 1u)
        t += dmp.swing * (bt / sd) / 200u;
    return t;
}
/* TrapRoot: a step's character (above: x X o, a roll's count), the velocity it strikes with */
static char dm_char(uint32_t r, uint32_t l, uint32_t b, uint32_t i)
{
    const dm_rhythm_t *y = &DM_RHY[r % DM_N];
    const char *s = y->lane[l % SQ_NLANE];
    uint32_t sd = dm_steps(y->beats), lb;
    if (!s || b >= y->bars || i >= sd)
        return '.';
    lb = str_len(s) / sd;                        /* (a lane shorter than the rhythm plays round, as dm_mask) */
    return lb ? s[(b % lb) * sd + i] : '.';
}
static uint32_t dm_vel(char c) { return c == 'X' ? 124u : c == 'o' ? 56u : 100u; }
static uint32_t dm_roll(char c) { return c >= '2' && c <= '8' ? (uint32_t)(c - '0') : 0u; }

/* the rolls under way: lane l's hits k .. n - 1 of the step that began at t0 (ticks) and lasts len */
static struct { uint8_t n, k; uint16_t t0, len; } dmroll[SQ_NLANE];

static void dm_strike(uint32_t l, uint32_t vel)
{
    trk_note_on(&trk[CF_PART_DRUM], SQ_LANE_NOTE[l], vel);
    cfp_midi(0x90u, cfp_ch(CF_PART_DRUM), SQ_LANE_NOTE[l], vel);
    sqp.doff |= (uint16_t)(1u << l);
    sqp.lane_t[l] = cf_now_ms;
}

/* ------------------------------------------------- TrapRoot: the 808 line --- */
/* the bass part plays the rhythm's 808 pattern, tick by tick (dm8_tick): each note the bass the keys would play
 * (dm8.root: the last chord's, kept while no key is down; none yet: the key's tonic), put in the 808's own octave (C1 ..
 * B1), plus the note's interval; at its velocity; one-shot, until the next note (dm8.left); a slide glides into its note
 * from the one sounding (the 808 engine's GLIDE, legato), any other note strikes afresh */
static struct { uint8_t root, have, cur; uint16_t ix, left, last; } dm8;
static const dm8_line_t *dm8_line_of(uint32_t r)
{
    uint32_t k = DM_RHY[r % DM_N].line;
    return k && k <= DM8_N && DM_RHY[r % DM_N].beats == 4u ? &DM8_LINES[k - 1u] : 0;
}
static int dm8_active(void)
{
    return dmp.on && dm8_line_of(dmp.r) && cfp_cfg.bass_on && !cfp_cp && !cfp_bo;
}
static void dm8_target(const uint8_t *nb, int mb)
{
    if (mb > 0) {
        dm8.root = nb[0];
        dm8.have = 1;
    }
}
static void dm8_off(void)
{
    if (!dm8.cur)
        return;
    trk_note_off(&trk[1], dm8.cur);
    cfp_midi(0x80u, cfp_ch(1), dm8.cur, 0);
    dm8.cur = 0;
}
static void dm8_strike(uint32_t note, uint32_t vel, int slide)
{
    if (slide && dm8.cur) {                      /* (a slide: the new note under the old one's gate, legato) */
        uint8_t old = dm8.cur;
        trk[1].slide_glide = 1;
        trk_note_on(&trk[1], note, vel);
        cfp_midi(0x90u, cfp_ch(1), note, vel);
        if (old != note) {
            trk_note_off(&trk[1], old);
            cfp_midi(0x80u, cfp_ch(1), old, 0);
        }
        trk[1].slide_glide = 0;
    } else {
        dm8_off();
        trk[1].slide_glide = 0;
        trk_note_on(&trk[1], note, vel);
        cfp_midi(0x90u, cfp_ch(1), note, vel);
    }
    dm8.cur = (uint8_t)note;
}
/* every tick of a rhythm (dmp_tick1, dmp_start): the note sounding ends when its time is up, then the pattern's notes
 * at this tick sound (its own bars, round from the rhythm's start) */
static void dm8_tick(void)
{
    const dm8_line_t *L = dm8_line_of(dmp.r);
    uint32_t bt = sqp_bar_ticks(), T;
    if (dm8.cur && dm8.left && !--dm8.left)
        dm8_off();
    if (!L || !dm8_active())
        return;
    T = (dmp.nbar % L->bars) * bt + dmp.t;
    if (T == 0u || T < dm8.last)                 /* (its top again, or another rhythm) */
        dm8.ix = 0;
    dm8.last = (uint16_t)T;
    while (dm8.ix < L->n && L->ev[dm8.ix].t <= T) {
        const dm8_ev_t *e = &L->ev[dm8.ix++];
        int note = dm8.have ? (int)dm8.root : 24 + (int)cfc.cfg.tonic;
        while (note > 35)
            note -= 12;
        while (note < 24)
            note += 12;
        note += e->iv;
        while (note < 18)
            note += 12;
        while (note > 71)
            note -= 12;
        dm8_strike((uint32_t)note, (e->vel & 127u) ? (e->vel & 127u) : 1u, (e->vel & 128u) != 0u);
        dm8.left = e->len;
    }
}
/* every block: the line stopped (the rhythm stopped, another without one, the bass switched off): its note goes and
 * the keys' bass comes back */
static void dm8_poll(void)
{
    if (dm8.cur && !dm8_active()) {
        dm8_off();
        cfp_sync(1);
    }
}

static void dmp_fire(uint32_t h)
{
    const dm_rhythm_t *y = &DM_RHY[dmp.r % DM_N];
    uint32_t l;
    for (l = 0; l < SQ_NLANE; l++)
        if ((dm_mask(dmp.r, l, dmp.bar) >> h) & 1u) {
            char c = dm_char(dmp.r, l, dmp.bar, h);
            uint32_t n = dm_roll(c);
            sqp_lane_kit(l, dmp.kit, y->lev[l]);
            dm_strike(l, dm_vel(c));
            dmroll[l].n = (uint8_t)n;               /* (a roll: its other hits come in dmr_now) */
            dmroll[l].k = 1;
            dmroll[l].t0 = (uint16_t)dmp_tick_of(h);
            dmroll[l].len = (uint16_t)(sqp_bar_ticks() / sq_dsteps());
        }
}
static void dmr_now(void)
{
    uint32_t l;
    for (l = 0; l < SQ_NLANE; l++)
        while (dmroll[l].k < dmroll[l].n && dmp.t >= dmroll[l].t0 + (uint32_t)dmroll[l].k * dmroll[l].len / dmroll[l].n) {
            dm_strike(l, 72u + 24u * (uint32_t)dmroll[l].k / dmroll[l].n);   /* (a roll swells into its last hit) */
            dmroll[l].k++;
        }
}
static void dmp_now(void)
{
    while (dmp.nh < sq_dsteps() && dmp.t >= dmp_tick_of(dmp.nh))
        dmp_fire(dmp.nh++);
    dmr_now();
}
static void dmp_stop(void)
{
    if (!dmp.on)
        return;
    dmp.on = 0;
    dmp.next = 0xFF;
    dmp.rin = 0;
    sqp_drum_offs();
    if (!sqp.on)
        cfm.running = 0;
}
static void dmp_start(uint32_t r, uint32_t kit, uint32_t swing)
{
    sqp_stop();
    dmp.r = (uint8_t)(r % DM_N);
    dmp.kit = (uint8_t)kit;
    dmp.swing = (uint8_t)swing;
    dmp.next = 0xFF;
    dmp.rin = 0;
    dmp.bar = 0;
    dmp.nbar = 0;
    dmp.t = 0;
    dmp.acc = 0;
    dmp.nh = 0;
    dmp.on = 1;
    {
        uint32_t l;
        for (l = 0; l < SQ_NLANE; l++)
            dmroll[l].n = 0;
    }
    if (dm8_active())                            /* (TrapRoot: the 808 line takes the bass from the keys) */
        cfp_sync(1);
    cfm_restart();
    cfm.running = 1;
    dmp_now();
    dm8.ix = 0;                                  /* (TrapRoot: the 808 pattern from its top) */
    dm8.last = 0;
    dm8_tick();
}
/* REC in the SEQUENCER while a rhythm plays: its bar ends first, then pattern pat plays from its top, the take into
 * track trk */
static void dmp_rec_in(uint32_t pat, uint32_t trk)
{
    uint32_t bt = sqp_bar_ticks();
    if (!dmp.on) {
        sqp_rec_start(pat, trk);
        return;
    }
    dmp.rin = (uint8_t)(pat + 1u);
    dmp.rtrk = (uint8_t)trk;
    dmp.rbars = (uint8_t)(bt - dmp.t < SQ_PPQN ? 2u : 1u);
}
static void dmp_next_bar(void)
{
    const dm_rhythm_t *y;
    uint32_t l;
    dmp.t = 0;
    dmp.nh = 0;
    for (l = 0; l < SQ_NLANE; l++)               /* (a roll ends with its bar) */
        dmroll[l].n = 0;
    if (dmp.rin && !--dmp.rbars) {               /* (REC in: the pattern takes over at this bar line) */
        uint32_t pat = dmp.rin - 1u, trk = dmp.rtrk;
        dmp_stop();
        sqp_begin(pat, 0, 0);
        sqp_rec_set(trk);
        return;
    }
    if (dmp.next != 0xFFu) {
        dmp.r = dmp.next;
        dmp.kit = dmp.nkit;
        dmp.swing = dmp.nswing;
        dmp.next = 0xFF;
        dmp.bar = 0;
        dmp.nbar = 0;
        if (dm8_active() && cfp.nb)              /* (TrapRoot: one with an 808 line takes the bass from the keys) */
            cfp_sync(1);
        if (DM_RHY[dmp.r % DM_N].beats != cfm.beats) {   /* (a rhythm of another TIME SIG: it comes with it, here at
                                                  * the bar line; the UI takes it into cs.beats, cf_dmui.c dm_poll) */
            cfm.beats = DM_RHY[dmp.r % DM_N].beats;
            sq_beats = cfm.beats;
        }
        return;
    }
    y = &DM_RHY[dmp.r % DM_N];
    dmp.nbar++;
    dmp.bar = (uint8_t)(dmp.nbar % (y->bars ? y->bars : 1u));
}
static void dmp_tick1(void)
{
    sqp_drum_offs();
    if (++dmp.t >= sqp_bar_ticks())
        dmp_next_bar();
    if (dmp.on) {
        dmp_now();
        dm8_tick();
    }
}
static void dmp_tick(uint32_t n, uint32_t pulses)   /* every audio block, as sqp_tick */
{
    uint32_t k;
    dm8_poll();
    if (!dmp.on)
        return;
    if (cfx_in.active) {
        for (k = 0; k < pulses * (SQ_PPQN / 24u) && dmp.on; k++)
            dmp_tick1();
        return;
    }
    dmp.acc += n * cfm.bpm * SQ_PPQN;
    if (dmp.acc < FS * 60u)
        return;
    dmp.acc -= FS * 60u;
    dmp_tick1();
}
/* PLAY on a rhythm, or SELECT in the drum machine while one plays: the next comes in at the bar line */
static void dmp_queue(uint32_t r, uint32_t kit, uint32_t swing)
{
    if (!dmp.on) {
        dmp_start(r, kit, swing);
        return;
    }
    if (r == dmp.r) {                            /* (back to the one playing: it plays on) */
        dmp.next = 0xFF;
        dmp.kit = (uint8_t)kit;
        dmp.swing = (uint8_t)swing;
        return;
    }
    dmp.next = (uint8_t)r;
    dmp.nkit = (uint8_t)kit;
    dmp.nswing = (uint8_t)swing;
}
/* REC under a rhythm playing (on HOME, the drum machine): the new pattern, its DRUMS this rhythm from the bar playing,
 * takes over at once where the bar is, its take into CHORDS (8 bars at most, as from HOME) */
static void dmp_take_over(uint32_t pat)
{
    uint32_t t = dmp.on ? dmp.t : 0u, acc = dmp.acc, bt = sqp_bar_ticks();
    int run = dmp.on;
    dmp.on = 0;
    dmp.next = 0xFF;
    dmp.rin = 0;
    if (!run) {                                  /* (stopped meanwhile: a count-in, as from silence) */
        sqp_rec_start(pat, SQ_CHORDS);
        sqp.rlim = SQ_BARS;
        return;
    }
    sqp_setup(pat, 0);                           /* (the beat goes on where it is) */
    sqp_rec_set(SQ_CHORDS);
    sqp.rlim = SQ_BARS;
    sqp.t = t < bt ? t : 0u;
    sqp.acc = acc;
    while (sqp.nu < SQ_U && sqp.nu * bt / SQ_U < sqp.t)   /* (what lay before is not struck now) */
        sqp.nu++;
    while (sqp.nh < sq_dsteps() && sqp_drum_tick(&sq_st.pat[pat % SQ_NPAT], sqp.nh) < sqp.t)
        sqp.nh++;
    cfm.running = 1;
    sqp_now();
}
