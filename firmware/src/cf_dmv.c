/* SPDX-License-Identifier: GPL-3.0-only
 * CoralFlower: the drum machine drawn (0.6): its page, the METRONOME's carousel of where SEQ goes, the plaque (what
 * plays) on the top lines. Included by cf_screens.c after cf_seqv.c (its flowers draw the hits). */

static void draw_view(void (*view)(int));
static int cv_head_x;                            /* (the rhythm's head line: where its details begin) */

/* the plaque at xr (its right edge) on the top line, as HOME's "SEQ A" always looked; returns its left edge. What plays
 * leads it as its section's icon at 1x (rc6: the notes for a pattern or the song, the drum for a rhythm, the same
 * drawings as the heads'); a take keeps its square, the record dot */
static int dm_plaque_chip(int xr, const char *b, uint32_t kind)
{
    int icon = kind != DMP_REC && kind != DMP_RECIN;
    int w = fb_text_w(&CF_FONT_UI, b, 1) + 12 + (icon ? 2 : 0), x = xr - w;
    uint8_t box = kind == DMP_REC ? FL_C(FL_R_FIRE, 2) : kind == DMP_RECIN || kind == DMP_PALE ? FL_BG2 : cf_panel(2);
    uint8_t dot = kind == DMP_REC ? FL_INK : kind == DMP_RECIN ? FL_C(FL_R_FIRE, 1) : kind == DMP_PALE ? FL_BG3 :
                  C_ACC(0);
    fb_rbox(x, Y_TOP + 2, w, 11, 2, box);
    if (icon)
        draw_icon1(x + 2, Y_TOP + 4, dm_plaque_rhythm ? SCR_DRUM : SCR_SQ, dot);
    else
        fb_rect(x + 3, Y_TOP + 5, 4, 4, dot);
    fb_text(&CF_FONT_UI, x + (icon ? 11 : 9), Y_TOP + 1, b, kind == DMP_PALE ? FL_INK3 : FL_INK, 1);
    return x;
}
/* the other pages' top line (cf_ui_draw): the plaque where the hint stands (it says what PLAY stops); 1 drawn */
static int dm_plaque_top(void)
{
    char b[24];
    uint32_t kind = dm_plaque(b, sizeof b);
    if (!dm_plaque_here(kind) || (kind == DMP_PALE && cu.scr != SCR_SEQ))
        return 0;
    dm_plaque_chip(212, b, kind);
    return 1;
}

/* the METRONOME's choice of where its SEQ goes: two cards side by side, each its section's icon and name, the chosen one
 * raised in SELECT's frame, as the FX chain's tiles; what to press over them (rc6, the designer's M1, the owner's pick:
 * the carousel showed one place at a time) */
static void dm_draw_carousel(void)
{
    static const char *const NM[2] = {"Sequencer", "Rhythm machine"};
    int k;
    draw_chip_text_c(Y_VIEW + 54, "[SELECT] pick  [SEQ] open", FL_INK2);
    for (k = 0; k < 2; k++) {
        int x = 8 + k * 116, y = Y_VIEW + 72, on = (int)dm_seq_to == k;
        uint32_t scr = k ? SCR_DRUM : SCR_SQ;
        fb_rbox(x, y, 108, 46, 2, cf_panel(on ? 2 : 1));
        if (on)
            fb_rframe(x, y, 108, 46, C_ACC(1));
        draw_icon(x + 54 + CF_ICON[scr].w, y + 8 + CF_ICON[scr].h, scr, on ? C_ACC(1) : FL_INK3);
        fb_text_halo(&CF_FONT_UI, x + 54 - (fb_text_w(&CF_FONT_UI, NM[k], 1) + 1) / 2, y + 28, NM[k],
                     on ? FL_INK : FL_INK3, FL_BG, 1);
    }
}

/* GLO's help on the rhythm machine, as the SEQUENCER's (cf_seqv.c draw_sq_help): buttons and gestures only, the knob
 * cells name the knobs */
static void dm_draw_help(void)
{
    /* (rc5, the owner on rc4: "keys: chords, as on HOME" and "[SEQ] home" "вообще не нужно", SEL "просто resets ритм";
     * [SELECT] stays: the page's caption names EDIT now) */
    static const char *const L[] = {"[SELECT] a rhythm", "[SEL] reset the rhythm", "[EDIT] make it a pattern",
                                    "[REC] record a pattern with it", "hold [REC]: undo last change", "[GLO] close", 0};
    int n = 0, i;
    while (L[n])
        n++;
    draw_panel(10, Y_VIEW + 4, 220, 10 + n * 14);
    for (i = 0; i < n; i++)
        draw_chip_text(18, Y_VIEW + 9 + i * 14, L[i], FL_INK);
}

/* Leviathan: an 808 pattern alone (no drums), as a piano roll: two bars at a time (the pair playing), a note a bar from
 * its tick to the next note's (one-shot, as it sounds), higher for a higher note, brighter for a louder one; a slide
 * joined to the note it glides from; where it plays, a line */
static void dm8_roll(const dm8_line_t *L, int x0, int x1, int y0, int y1, int play)
{
    uint32_t bt = 4u * 96u, w = 2u * bt, i, base = 0;
    int lo = 127, hi = -128, span, prevx = -1, prevy = 0;
    if (L->bars > 2u && play)
        base = ((cf_sn.dm_nbar % L->bars) & ~1u) * bt;
    for (i = 0; i < L->n; i++) {
        lo = L->ev[i].iv < lo ? L->ev[i].iv : lo;
        hi = L->ev[i].iv > hi ? L->ev[i].iv : hi;
    }
    span = hi - lo < 12 ? 12 : hi - lo;
    lo -= (span - (hi - lo)) / 2;                /* (a narrow pattern in the middle) */
    for (i = 0; i <= 8u; i++)                    /* the beats, the bar line between the two bars brighter */
        fb_rect(x0 + (int)(i * (uint32_t)(x1 - x0) / 8u), y0, 1, y1 - y0, i == 4u ? FL_C(FL_R_ORCHID, 2) : FL_BG2);
    fb_text(&CF_FONT_UI, x0 + 2, y0 - 1, "808", FL_INK3, 1);
    for (i = 0; i < L->n; i++) {
        const dm8_ev_t *e = &L->ev[i];
        uint32_t len = e->len;
        int xa, xb, yy;
        uint8_t c;
        if (e->t < base || e->t >= base + w)
            continue;
        if (e->t + len > base + w)
            len = base + w - e->t;
        xa = x0 + (int)((e->t - base) * (uint32_t)(x1 - x0) / w);
        xb = x0 + (int)((e->t - base + len) * (uint32_t)(x1 - x0) / w);
        yy = y1 - 4 - (e->iv - lo) * (y1 - y0 - 8) / span;
        c = FL_C(FL_R_ROSE, (e->vel & 127u) > 96u ? 0 : (e->vel & 127u) > 56u ? 1 : 2);
        if ((e->vel & 128u) && prevx >= 0)       /* (a slide: from the note before) */
            fb_rect(xa, yy < prevy ? yy : prevy, 1, (yy < prevy ? prevy - yy : yy - prevy) + 1, FL_C(FL_R_ROSE, 1));
        fb_rect(xa, yy - 1, xb - xa > 1 ? xb - xa - 1 : 1, 3, FL_C(FL_R_ROSE, 2));
        fb_rect(xa, yy - 2, 2, 5, c);            /* (its strike) */
        prevx = xb;
        prevy = yy;
    }
    if (play && cf_sn.sq_bt) {
        uint32_t pos = (cf_sn.dm_nbar % L->bars) * bt + cf_sn.dm_t * bt / cf_sn.sq_bt;
        if (pos >= base && pos < base + w)
            fb_rect(x0 + (int)((pos - base) * (uint32_t)(x1 - x0) / w), y0 - 2, 1, y1 - y0 + 2, FL_C(FL_R_FIRE, 1));
    }
}

/* the drum machine: its drums' rows of hits, each led by its drum's picture
 * (both bars of a 2-bar one, a dashed line between), a cursor running over them while it plays; under them its 808
 * (a drop a note). An 808 pattern alone: its piano roll (dm8_roll). Its name blinks while it waits for the bar line; a
 * dot after it: the user's KIT or SWING on it (SEL takes them back) */
static void view_drum(int x0)
{
    uint32_t r = dm_cur(), sd, nb, steps, l, b, i, nl = 0;
    const dm_rhythm_t *y = &DM_RHY[r % DM_N];
    int gx = x0 + 34, gw = 216 - 34, row, yt = VIEW_Y - 2, k = 0, play = dm_plays(r);
    const dm8_line_t *L = dm8_line_of(r);
    uint32_t wb, wn;
    sd = dm_steps(y->beats);                     /* (its own TIME SIG: one picked from another shows its bars) */
    nb = y->bars ? y->bars : 1u;
    for (l = 0; l < SQ_NLANE; l++)
        nl += y->lane[l] != 0;
    if (!nl && L) {
        dm8_roll(L, x0 - 4, 232, yt + 4, Y_VIEW + H_VIEW - 20, play);
        return;
    }
    /* (two bars at a time, the pair playing) */
    wn = nb < 2u ? nb : 2u;
    wb = play ? (cf_sn.dm_bar & ~1u) % nb : 0u;
    steps = wn * sd;
    nl += L != 0;                                /* (Leviathan: the 808, a row of its own under the drums) */
    row = nl <= 4u ? 16 : nl <= 6u ? 13 : 9;
    for (l = 0; l < SQ_NLANE; l++) {
        int yr = yt + k * row;
        if (!y->lane[l])
            continue;
        k++;
        fb_rect(x0 - 6, yr + 6, 26, 7, FL_BG);  /* (rc6, the designer's R1: its drum's picture on a clear cell, as on
                                                  * the DRUMS editor's keys; the hand kit's congas and clave) */
        draw_drum_icon(x0 + 3, yr + 8, sq_lane_pic(l, dm_kit(r)), FL_INK2);
        fb_hline(gx, yr + SQG_GND + 1, gw, FL_BG3);
        for (b = 0; b < wn; b++) {
            uint32_t bb = (wb + b) % nb, m = dm_mask(r, l, bb);
            for (i = 0; i < sd; i++) {
                int s = (int)(b * sd + i), cw = gw / (int)steps, cx = gx + s * gw / (int)steps + cw / 2;
                if (i & 1u)
                    cx += (int)dm_swing(r) * cw / 200;
                if ((m >> i) & 1u) {
                    uint32_t rn = dm_roll(dm_char(r, l, bb, i));
                    sq_flower(cx, yr, cw, 2, sq_lane_ink(l), 0);
                    if (rn)                      /* (a roll: a bar over the flame, as long as its hits) */
                        fb_rect(cx - (int)rn / 2, yr + 1, (int)rn, 1, FL_C(FL_R_FIRE, 0));
                } else {
                    fb_rect(cx, yr + SQG_GND, 1, 1, FL_BG3);
                }
            }
        }
    }
    if (L && sd == 16u) {                        /* the 808: a drop a note, at its tick */
        int yr = yt + k * row;
        uint32_t bt = 4u * 96u, base = L->bars > 2u && play ? ((cf_sn.dm_nbar % L->bars) & ~1u) * bt : 0u;
        sq_ink_t ink;
        ink.ramp = FL_R_ROSE;
        ink.eye = FL_R_FIRE;
        k++;
        fb_rect(x0 - 6, yr + 6, 26, 7, FL_BG);
        fb_text(&CF_FONT_UI, x0 - 4, yr + 3, "808", FL_INK2, 1);
        fb_hline(gx, yr + SQG_GND + 1, gw, FL_BG3);
        for (i = 0; i < L->n; i++) {
            uint32_t t = L->ev[i].t;
            if (t >= base && t < base + wn * bt)
                sq_flower(gx + (int)((t - base) * (uint32_t)gw / (wn * bt)) + gw / (int)steps / 2, yr,
                          gw / (int)steps, 1, ink, 0);
        }
    }
    if (wn > 1u) {                               /* (the second bar begins: a dashed line) */
        int xb = gx + (int)sd * gw / (int)steps - 1;
        int yy;
        for (yy = yt; yy < yt + k * row; yy += 4)
            fb_rect(xb, yy, 1, 2, FL_C(FL_R_ORCHID, 2));
    }
    if (play && cf_sn.sq_bt) {                   /* (playing: where it is) */
        uint32_t pos = (cf_sn.dm_bar - wb) * sd * 256u + cf_sn.dm_t * sd * 256u / cf_sn.sq_bt;
        int xc = gx + (int)(pos * (uint32_t)gw / (steps * 256u));
        fb_rect(xc, yt - 2, 1, k * row + 2, FL_C(FL_R_FIRE, 1));
    }
}
static void draw_drum(void)
{
    uint32_t r = dm_cur(), n = DM_N, p = r;      /* (one list: its place among all the rhythms) */
    const dm_rhythm_t *y = &DM_RHY[r % DM_N];
    uint32_t beats = y->beats;                   /* (its own TIME SIG) */
    int wait = cf_sn.dm_on && cf_sn.dm_next == r && cf_sn.dm_r != r, lit = cu.now - cv_turn_ms < 220u && cv_turn_ms;
    char t[40], a[8];
    flora_render(&flo, &fl_cv);
    draw_plant(Y_VIEW);
    fb_veil(0, Y_VIEW, FB_W, H_VIEW, FL_BG);    /* (the plant faint: the picture leads) */
    /* the head as every section's (rc6, the designer's R1, the owner's pick): the rhythm's name white at the left, the
     * drum at the right; under it its place as a counter "‹ 1/27 ›" with the pager's arrows, then its TIME SIG, bars and
     * tempo. The name blinks while it waits for the bar line; a dot after it: the user's KIT or SWING */
    if (!wait || (cu.now / 250u) & 1u)
        draw_head(y->name);
    else
        draw_icon(234, HEAD_Y + 14, SCR_DRUM, C_ACC(1));
    if (dm_tweaked(r))
        fb_rect(6 + fb_text_w(&CF_FONT_UI, y->name, 2) + 3, HEAD_Y + 14, 4, 4, C_ACC(0));
    {
        int x = 17;                              /* (the designer's R1: 4 px clear of each arrow) */
        kint(t, (int)p + 1, "/");
        kint(a, (int)n, 0);
        str_cpy(t + str_len(t), a, sizeof t - str_len(t));
        draw_chevron(8, HEAD_LINE + 2, 0, p ? (lit && cv_turn_dir < 0 ? C_ACC(0) : FL_INK2) : FL_BG3);
        x += fb_text_halo(&CF_FONT_UI, x, HEAD_LINE, t, FL_INK2, FL_BG, 1) + 4;
        draw_chevron(x, HEAD_LINE + 2, 1, p + 1u < n ? (lit && cv_turn_dir > 0 ? C_ACC(0) : FL_INK2) : FL_BG3);
        cv_head_x = x + 12;
    }
    t[0] = 0;
    kint(a, (int)beats, 0);
    str_cpy(t + str_len(t), a, sizeof t - str_len(t));
    str_cpy(t + str_len(t), beats == 6u ? "/8  " : "/4  ", sizeof t - str_len(t));
    kint(a, y->bars, 0);
    str_cpy(t + str_len(t), a, sizeof t - str_len(t));
    str_cpy(t + str_len(t), y->bars > 1u ? " bars  ~" : " bar  ~", sizeof t - str_len(t));
    kint(a, y->bpm, 0);                          /* (its tempo: Leviathan sets TEMPO to it when it is picked, dm_select) */
    str_cpy(t + str_len(t), a, sizeof t - str_len(t));
    str_cpy(t + str_len(t), " BPM", sizeof t - str_len(t));
    fb_text_halo(&CF_FONT_UI, cv_head_x, HEAD_LINE, t, C_ACC(1), FL_BG, 1);
    draw_view(view_drum);
    draw_chip_text_c(Y_VIEW + H_VIEW - 14, dm_has_drums(r) ? "[EDIT] make it a pattern" : "808: it follows your chords",
                     FL_INK3);                   /* (rc5: the page's main use, on
                                                  * it; the owner on rc4: "потерялась функция чтобы вставить барабаны из
                                                  * драм-машины в паттерны") */
    if (squ.help || squ.help_hold)
        dm_draw_help();
    {
        static const char *const H[2] = {"GLO", "help"};   /* (as on the SEQUENCER) */
        draw_hints(H, 1);
    }
}
