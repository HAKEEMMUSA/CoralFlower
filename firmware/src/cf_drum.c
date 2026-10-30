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
    uint8_t line;                /* TrapRoot: its own 808 line, DL_* (DM8_LINES + 1; 0 none): while it plays the bass part
                                  * plays the line (cf_play.c cfp_sync: the keys only set its note). ALGORITHM in the
                                  * rhythm machine picks another for every rhythm, or none (dm8_sel) */
} dm_rhythm_t;

/* TrapRoot: the 808 lines, after the owner's (their MIDI files, 2026-10-30; Rage's from their FL Studio pattern): 16
 * steps a bar, a line as long as its own phrase (2, 4 or 8 bars, round under any rhythm). hit a char a step: 'x' a
 * hit, 's' a slide into it from the note sounding, '2' .. '8' a roll of so many hits inside the step (the "spinz",
 * fading). note a char a step: 'm' the bass the keys play (the line's first note in its MIDI), 'n' .. 'y' +1 .. +12,
 * 'l' .. 'a' -1 .. -12, 'A' .. 'Z' +13 .. +38. Notes off the 16ths (Secret's triplets) on the nearest one. bpm: the
 * tempo the MIDI was written at (shown, never set) */
typedef struct { const char *name; uint8_t bars, bpm; const char *hit, *note; } dm8_line_t;
static const dm8_line_t DM8_LINES[] = {
    {"Rage", 2, 180, "x..............." "................", "m..............." "................"},
    {"Spinz 1", 2, 160,
     "x.....x.....x..." "..x.....x.x...x.",
     "m.....n.....m..." "..m.....H.m...y."},
    {"Spinz 2", 8, 160,
     "x...........2.x." "....x.....x.532." "x...........2.x." "....x...x.x.576." "x...........2.x." "....x.....x.532." "x...........2.x." "....x...x.x.576.",
     "m...........m.m." "....n.....n.nnn." "m...........m.m." "....n...n.i.iii." "m...........m.m." "....n.....n.nnn." "m...........m.m." "....A...A.u.uuu."},
    {"Spinz 3", 8, 160,
     "x.....x.....xx.x" "2...x.....574..." "x.....x.....x..." "2...x.....578787" "5.....x.....xx.x" "2...x.....574..." "x.....x.....x..." "2...x.....578788",
     "m.....m.....LL.L" "m...m.....mmm..." "m.....m.....m..." "m...m.....mmmyyy" "y.....m.....LL.L" "m...m.....mmm..." "m.....m.....m..." "m...m.....mmmyyy"},
    {"Spinz 4", 4, 160,
     "x.............33" "3...........x..." "..574..........." "332.x.....x.x...",
     "m.............LX" "L...........n..." "..nnn..........." "XZX.m.....n.n..."},
    {"Forsaken", 4, 160,
     "x.............x." "..x.x.....x.x.x." "x.............x." "....x.....x.x...",
     "m.............m." "..h.m.....m.m.m." "m.............m." "....m.....n.A..."},
    {"King", 8, 160,
     "x..............." "....x..........." "x.....x........." "....x.....x....." "x..............." "....x..........." "x.....x.......x." "....x.....x...x.",
     "m..............." "....r..........." "t.....h........." "....m.....i....." "m..............." "....r..........." "t.....h.......h." "....m.....m...m."},
    {"Bitcrush", 8, 160,
     "x.............x." "..x.x.....x....." "x.............x." "..x.x.....x....." "x.............x." "..x.x.....x...x." "x.............x." "..x.x...x.x...x.",
     "m.............m." "..m.m.....h....." "m.............m." "..m.m.....h....." "m.............m." "..m.m.....h...h." "m.............m." "..m.m...t.t...t."},
    {"OhMyGawd", 4, 158,
     "x..............." "....x.....x....." "x..............." "....x...........",
     "m..............." "....u.....u....." "m..............." "....u..........."},
    {"LuhTwizz", 4, 105,
     "x.....x.....x..." "....x.....x.x..." "x.....x.....x..." "..x.x.....x.x...",
     "m.....m.....m..." "....m.....h.i..." "m.....m.....m..." "..m.m.....h.i..."},
    {"Chase", 8, 157,
     "x.x...x.....x..." "..xs....x......." "x.x...x.....x.x." "..x.s...x......." "x.x...x.....x..." "..xs....x......." "x.x...x.....x..." "..x.s...x.......",
     "m.m...m.....m..." "..to....k......." "m.m...m.....m.p." "..p.c...k......." "m.m...m.....m..." "..ho....k......." "m.m...m.....m..." "..p.o...k......."},
    {"Chichi", 8, 146,
     "x.............x." "..x.x.....xx..x." "x.............x." "..x.x.....x....." "x.............x." "..x.x.....xx..x." "x.............x." "..x.x.....x.....",
     "m.............m." "..m.m.....tt..t." "m.............m." "..m.m.....t....." "m.............m." "..m.m.....hh..h." "m.............m." "..m.m.....t....."},
    {"Quis", 4, 120,
     "x...........x..." "....x.x.x.....x." "x...........x..." "....x.x.x.2...x.",
     "m...........n..." "....i.n.n.....m." "m...........n..." "....i.n.n.n...m."},
    {"Bounce", 8, 182,
     "x.....x...x.x..." "....x.....x....." "x.........x.x..." "....x.....x....." "x.....x...x.x..." "....x.....x....." "x.....x...x.x..." "....x.....xxx.x.",
     "y.....y...m.m..." "....m.....m....." "m.........m.m..." "....m.....A....." "y.....y...m.m..." "....m.....m....." "m.....m...m.m..." "....m.....nnn.n."},
    {"Secret", 2, 130,
     "x....x...x.x.x.." "...x...xxx.x.x.x",
     "m....m...m.m.m.." "...m...mmm.m.y.m"},
    {"SF RT", 4, 120,
     "x.....x.....x..." "....x.....x....." "x.....x.....x..." "....x...........",
     "m.....m.....m..." "....m.....m....." "m.....m.....m..." "....m..........."},
};
#define DM8_N ((uint32_t)(sizeof DM8_LINES / sizeof DM8_LINES[0]))
enum { DL_NONE, DL_RAGE, DL_SPINZ1, DL_SPINZ2, DL_SPINZ3, DL_SPINZ4, DL_FORSAKEN, DL_KING, DL_BITCRUSH, DL_OHMYGAWD,
       DL_LUHTWIZZ, DL_CHASE, DL_CHICHI, DL_QUIS, DL_BOUNCE, DL_SECRET, DL_SFRT, DL_END };
typedef char dm8_lines_named[DL_END - 1 == (int)(sizeof DM8_LINES / sizeof DM8_LINES[0]) ? 1 : -1];
/* TrapRoot: a hit's character in a lane: 'x' a hit, 'X' an accent, 'o' a ghost note, '2' '3' '4' '6' '8' a roll of
 * that many hits inside the step (the trap hat rolls; the pattern's DRUMS keep its first hit) */

/* the kits by name (SQ_KITS' order) */
enum { DMK_STD, DMK_808, DMK_909, DMK_TIGHT, DMK_BOOM, DMK_LOFI, DMK_HAND, DMK_CRUSH, DMK_XCLUB, DMK_XDEEP, DMK_XLOFI,
       DMK_XLIVE, DMK_XTECH, DMK_XBREAK, DMK_XRAGE };

/* 4/4: the owner's list for rc4 ("моя прошивка больше ориентирована на более современные жанры"; rc3's Rock, Bossa,
 * Reggae, Swing, Motown, Dub, Afro, Ballad, Punk, Disco, Trap and Samba gone; "Все, решу на слух": every one proposed
 * is in, the owner cuts by ear before the release). Pop and HipHop on X Lo-Fi, Funk on X Club with SWING 40 %, DnB on
 * X Tech with more in it than two steps (the owner's). rc7, the owner by ear on rc6: Footwork ("убрать"), Phonk
 * ("кринж"), TrapWaltz and AfroHouse out; their ids 33, 37, 43, 44 are never reused (a kept KIT, SWING or place of
 * theirs is dropped, cf_dmui.c) */
static const dm_rhythm_t DM_RHY[] = {
    /* 4/4: 16 steps a bar */
    /* TrapRoot: trap, rage and plugg first (ids 48 on). The hats roll ('3' '4' '6' '8'), the 808 line follows the chord
     * (DM8_LINES); the kit is X Rage (drum_kits.c), the kick short: the 808 is the low end */
    /* Rage: the owner's pick (an FL Studio pattern at 180 BPM, read from its channel rack): no kick, the 808 is the
     * low end and hits once every two bars; the snare on 1, 7, 13 and 21, 27 (the rage bounce), the clap on 3 of each
     * bar, one open hat (step 13), one shaker (bar 2's step 3, on the hat lane: the kits have no shaker) */
    {"Rage", 48, 4, 2, DMK_XRAGE, 0, 180,
     {[DM_S] = "x.....x.....x..." "....x.....x.....", [DM_C] = "........x......." "........x.......",
      [DM_O] = "............x..." "................", [DM_H] = "................" "..x............."},
     {[DM_S] = 100, [DM_C] = 85, [DM_O] = 55, [DM_H] = 50},
     DL_RAGE},
    {"Atlanta", 49, 4, 2, DMK_XRAGE, 0, 140,
     {[DM_K] = "x......x..x....." "x......x.x......", [DM_S] = "........X......." "........X.......",
      [DM_C] = "........X......." "........X.......", [DM_H] = "xxxxxxxxxxxx3.xx" "xxxxxxxxxx4.xxx6",
      [DM_R] = "..............o." "..............o."},
     {[DM_K] = 100, [DM_S] = 90, [DM_C] = 85, [DM_H] = 45, [DM_R] = 40},
     DL_CHASE},
    {"Carti", 50, 4, 2, DMK_XRAGE, 52, 150,
     {[DM_K] = "x.........x....." "x......x..x.....", [DM_S] = "........X......." "........X.......",
      [DM_C] = "........X......." "........X......o", [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.3.x.x.",
      [DM_O] = "..x.......x....." "..x.......x.....", [DM_R] = "...x.......x...x" ".......x....x..."},
     {[DM_K] = 100, [DM_S] = 90, [DM_C] = 80, [DM_H] = 50, [DM_O] = 45, [DM_R] = 40},
     DL_BOUNCE},
    {"Opium", 51, 4, 2, DMK_XRAGE, 0, 165,
     {[DM_K] = "x..x..x...x.x..." "x..x..x.x.x.x...", [DM_S] = "....X.......X..." "....X.......X...",
      [DM_C] = "....X.......X..." "....X.......X...", [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.x.x.6.",
      [DM_X] = "X..............." "X..............."},
     {[DM_K] = 100, [DM_S] = 85, [DM_C] = 80, [DM_H] = 50, [DM_X] = 40},
     DL_SPINZ2},
    {"Plugg", 52, 4, 2, DMK_XRAGE, 54, 145,
     {[DM_K] = "x.....x........." "x.....x...x.....", [DM_C] = "........X......." "........X.......",
      [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.x.x.x.", [DM_R] = "...x.......x...x" "...x.......x.o.."},
     {[DM_K] = 95, [DM_C] = 85, [DM_H] = 45, [DM_R] = 40},
     DL_BITCRUSH},
    {"Memphis", 53, 4, 2, DMK_XRAGE, 56, 135,
     {[DM_K] = "x..x..x...x.x..." "x..x..x...x.x...", [DM_S] = "........X......." "........X.......",
      [DM_H] = "xxxxxxxxxxxxxxxx" "xxxxxxxxxxxx4.xx", [DM_O] = "......x.......x." "......x.......x.",
      [DM_B] = "x..x..x.x..x..x." "x..x..x.x..x.x.."},
     {[DM_K] = 100, [DM_S] = 90, [DM_H] = 40, [DM_O] = 45, [DM_B] = 45},
     DL_QUIS},
    {"TrapDrill", 54, 4, 2, DMK_XRAGE, 0, 142,
     {[DM_K] = "x.........x....." "....x.....x.....", [DM_S] = "........X......." "....X...X.......",
      [DM_H] = "x..x..x.x..x..x." "x..x..x.x.3.4.x.", [DM_R] = "......x.......x." "......x...x.....",
      [DM_X] = "X..............." "................"},
     {[DM_K] = 100, [DM_S] = 95, [DM_H] = 50, [DM_R] = 40, [DM_X] = 40},
     DL_SPINZ4},
    {"Rolls", 55, 4, 2, DMK_XRAGE, 0, 150,
     {[DM_K] = "x.........x....." "x.........x.x...", [DM_S] = "........X......." "........X.......",
      [DM_C] = "........X......." "........X.......", [DM_H] = "x.x.3.x.4.x.x6x." "x3x.x.4.x8x.6.88",
      [DM_O] = "..............x." "..............x."},
     {[DM_K] = 100, [DM_S] = 90, [DM_C] = 80, [DM_H] = 50, [DM_O] = 45},
     DL_SPINZ3},
    /* GTR and Moet: the owner's drum loops (Cymatics, 4 bars), transcribed by their bands' onsets: kick under 110 Hz,
     * snare and clap 0.2 .. 4 kHz, hats over 7 kHz, on the 32nds (two 32nds in a 16th: a roll of 2) */
    {"GTR", 56, 4, 4, DMK_XRAGE, 0, 140,
     {[DM_K] = "x..............." "....x.....x....." "x..............." "....x.....x.....",
      [DM_S] = "........x......." "........x......." "........x......." "........x.......",
      [DM_C] = "........x......." "........x......." "........x......." "........x.......",
      [DM_H] = "2.x.x...x.x.x.x." "x.x.2.x.x.2xx.x." "2.x.x...x.x.x.x." "x.x.2xx.x.2xx.x."},
     {[DM_K] = 100, [DM_S] = 90, [DM_C] = 80, [DM_H] = 50},
     DL_CHICHI},
    {"Moet", 57, 4, 4, DMK_XRAGE, 0, 98,
     {[DM_K] = "x..............." "...x..x........." "x..............." "...x..x....x....",
      [DM_S] = "........x......." "........x.....x." "........x......." "........x...o.x.",
      [DM_C] = "........x......." "........x......." "........x......." "........x.......",
      [DM_H] = "..x.x.xxX.x.x.xx" "x.x...xxX.2x..xx" "x.x.x.xxX.xxx.xx" "x.x...xxX.2...xx"},
     {[DM_K] = 100, [DM_S] = 90, [DM_C] = 70, [DM_H] = 50},
     DL_LUHTWIZZ},
    {"Pop", 1, 4, 1, DMK_XLOFI, 0, 110,
     {[DM_K] = "x.....x.x.......", [DM_S] = "....x.......x...", [DM_C] = "....x.......x...",
      [DM_H] = "x.x.x.x.x.x.x.x."},
     {[DM_K] = 100, [DM_S] = 80, [DM_C] = 60, [DM_H] = 50}},
    {"Funk", 2, 4, 2, DMK_XCLUB, 40, 105,
     {[DM_K] = "x..x..x...x....." "x.x...x..x..x...", [DM_S] = "....x.......x..." "....x.......x...",
      [DM_R] = ".......x.x....x." "..x....x.....x..", [DM_H] = "xxxxxxxxxxxxxxxx" "xxxxxxxxxxxxxx..",
      [DM_O] = "................" "..............x."},
     {[DM_K] = 100, [DM_S] = 90, [DM_R] = 35, [DM_H] = 40, [DM_O] = 50}},
    {"HipHop", 7, 4, 2, DMK_XLOFI, 30, 90,
     {[DM_K] = "x......x.x......" "x.........x.....", [DM_S] = "....x.......x..." "....x.......x...",
      [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.x.x.x."},
     {[DM_K] = 100, [DM_S] = 90, [DM_H] = 50}},
    {"Lo-fi", 30, 4, 2, DMK_XLOFI, 45, 80,
     {[DM_K] = "x.......x.x....." "x......x..x.....", [DM_S] = "....x.......x..." "....x.......x...",
      [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.x.xx.."},
     {[DM_K] = 90, [DM_S] = 75, [DM_H] = 35}},
    {"House", 4, 4, 1, DMK_909, 0, 124,
     {[DM_K] = "x...x...x...x...", [DM_C] = "....x.......x...", [DM_H] = ".x.x.x.x.x.x.x.x",
      [DM_O] = "..x...x...x...x."},
     {[DM_K] = 100, [DM_C] = 80, [DM_H] = 35, [DM_O] = 60}},
    {"Garage", 18, 4, 1, DMK_XCLUB, 55, 132,
     {[DM_K] = "x......x..x.....", [DM_S] = "....x.......x...", [DM_H] = "x.xxx.xxx.xxx.xx"},
     {[DM_K] = 100, [DM_S] = 85, [DM_H] = 40}},
    {"Techno", 5, 4, 1, DMK_XTECH, 0, 132,
     {[DM_K] = "x...x...x...x...", [DM_C] = "....x.......x...", [DM_H] = "xx..xx..xx..xx..",
      [DM_O] = "..x...x...x...x.", [DM_R] = "...x.....x....x."},
     {[DM_K] = 100, [DM_C] = 70, [DM_H] = 40, [DM_O] = 55, [DM_R] = 45}},
    {"Break", 13, 4, 2, DMK_XBREAK, 10, 130,
     {[DM_K] = "x.x.......xx...." "x.x.......x.....", [DM_S] = "....x..x.x..x..x" "....x..x.x....x.",
      [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.x.x.x."},
     {[DM_K] = 100, [DM_S] = 90, [DM_H] = 50}},
    {"Jungle", 31, 4, 2, DMK_XBREAK, 0, 165,
     {[DM_K] = "x.........x....." "..x.......x.....", [DM_S] = "....x.x....x.x.." "....x..x.x..x.x.",
      [DM_R] = ".x....x.x......x" "x....x......x...", [DM_H] = "xxxxxxxxxxxxxxxx" "xxxxxxxxxxxxxxxx",
      [DM_X] = "x..............." "................"},
     {[DM_K] = 100, [DM_S] = 90, [DM_R] = 35, [DM_H] = 30, [DM_X] = 40}},
    {"DnB", 14, 4, 2, DMK_XTECH, 0, 174,
     {[DM_K] = "x.........x....." "x.x.......x..x..", [DM_S] = "....x.......x..." "....x.......x...",
      [DM_R] = ".......x.x....x." "..x....x.......x", [DM_H] = "x.xxx.x.x.xxx.x." "x.x.x.xxx.x.x.x.",
      [DM_X] = "x..............." "................"},
     {[DM_K] = 100, [DM_S] = 95, [DM_R] = 35, [DM_H] = 45, [DM_X] = 40}},
    {"Breakcore", 39, 4, 2, DMK_XBREAK, 0, 180,
     {[DM_K] = "x.x...x...x.x..." "x...x.x..x...x..", [DM_S] = "....x..xx..x.x.x" "..x.x.x..x.xx.xx",
      [DM_H] = "xxxxxxxxxxxxxxxx" "xxxxxxxxxxxxxxxx", [DM_X] = "x..............." "................"},
     {[DM_K] = 100, [DM_S] = 90, [DM_H] = 35, [DM_X] = 40}},
    /* Jersey club (the owner on rc6: "джерси не похож на джерси клаб бит"): its five-kick figure, two quarters, two
     * dotted eighths and an eighth (steps 1 5 9 12 15), a kick roll into the next bar; claps on 2 and 4; the bed squeak
     * as a rim answering on the off-beats; open hats on the "and"s */
    {"Jersey", 32, 4, 2, DMK_XCLUB, 0, 140,
     {[DM_K] = "x...x...x..x..x." "x...x...x..x.xxx", [DM_C] = "....x.......x..." "....x.......x...",
      [DM_R] = "......x.......x." "......x...x...x.", [DM_O] = "..x...x...x...x." "..x...x...x....."},
     {[DM_K] = 100, [DM_C] = 85, [DM_R] = 55, [DM_O] = 35}},
    {"Hyperpop", 34, 4, 2, DMK_CRUSH, 0, 150,
     {[DM_K] = "x.....x...x....." "x.....x.x.x.....", [DM_C] = "....x.......x..." "....x.......x...",
      [DM_H] = "x.x.x.x.x.x.xxxx" "x.x.x.x.xxxxxxxx", [DM_X] = "x..............." "................"},
     {[DM_K] = 100, [DM_C] = 90, [DM_H] = 40, [DM_X] = 35}},
    {"Drill", 35, 4, 2, DMK_XTECH, 15, 142,             /* (X Tech: the owner on rc6) */
     {[DM_K] = "x......x...x...." "..x....x..x.x...", [DM_S] = "........x......." "........x.....x.",
      [DM_H] = "x..x..x.x..x..x." "x..x..x.x.xx..x."},
     {[DM_K] = 100, [DM_S] = 90, [DM_H] = 45}},
    {"Grime", 36, 4, 2, DMK_TIGHT, 0, 140,
     {[DM_K] = "x.....x...x..x.." "x..x......x.....", [DM_S] = "........x......." "........x.......",
      [DM_H] = "x...x...x...x..." "x...x...x...x.x."},
     {[DM_K] = 100, [DM_S] = 90, [DM_H] = 40}},
    {"Halftime", 38, 4, 2, DMK_XDEEP, 0, 140,
     {[DM_K] = "x.........x....." "x......x.x......", [DM_S] = "........x......." "........x.......",
      [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.x.x.x."},
     {[DM_K] = 100, [DM_S] = 95, [DM_H] = 35}},
    {"Amapiano", 40, 4, 2, DMK_XTECH, 0, 112,           /* (X Tech: the owner on rc6) */
     {[DM_K] = "x...x...x...x..." "x...x...x...x...", [DM_LT] = "...x..x...x..x.." "..x..x....x...x.",
      [DM_R] = "....x.......x..." "....x.......x...", [DM_H] = "xxxxxxxxxxxxxxxx" "xxxxxxxxxxxxxxxx"},
     {[DM_K] = 85, [DM_LT] = 90, [DM_R] = 60, [DM_H] = 30}},
    {"Baile", 41, 4, 2, DMK_808, 0, 130,
     {[DM_K] = "x.....x...x....." "x.....x...x.x...", [DM_MT] = "...x.....x...x.." "...x.....x...x..",
      [DM_LT] = "..x.....x.....x." "..x.....x.....x.", [DM_C] = "....x.......x..." "....x.......x..."},
     {[DM_K] = 100, [DM_MT] = 75, [DM_LT] = 80, [DM_C] = 80}},
    {"Dembow", 42, 4, 1, DMK_XCLUB, 0, 95,
     {[DM_K] = "x...x...x...x...", [DM_S] = "...x..x....x..x.", [DM_H] = "x.x.x.x.x.x.x.x."},
     {[DM_K] = 100, [DM_S] = 85, [DM_H] = 35}},
    /* (the owner on rc6: "поп рок в конец, не оч он как-то": the last of the 4/4 ones) */
    {"PopRock", 29, 4, 2, DMK_XLIVE, 0, 120,
     {[DM_K] = "x.......x.x....." "x.....x.x.......", [DM_S] = "....x.......x..." "....x.......x...",
      [DM_H] = "x.x.x.x.x.x.x.x." "x.x.x.x.x.x.x.x.", [DM_X] = "x..............." "................"},
     {[DM_K] = 100, [DM_S] = 90, [DM_H] = 55, [DM_X] = 45}},
    /* the other TIME SIGs: modern takes (the owner's pick B for rc4: "3/4 trap-waltz, 6/8 afro-house, 5/4 and 7/4
     * math-rock, 2/4 baile"; rc3's Waltz, Jazz3, March, Polka, Five, Odd5, Blues, Bembe and Seven gone). A 6/8 bar is
     * 6 beats of 8ths: its tempo counts 8ths */
    /* (3/4 and 6/8: none since rc7, TrapWaltz and AfroHouse cut by the owner: "полная ебанина", "кринж") */
    /* 2/4: 8 steps */
    {"Tamborzao", 47, 2, 2, DMK_808, 0, 130,
     {[DM_K] = "x..x..x." "x..x..x.", [DM_MT] = ".x...x.." ".x...x.x", [DM_LT] = "..x....x" "..x.....",
      [DM_C] = "....x..." "....x..."},
     {[DM_K] = 100, [DM_MT] = 75, [DM_LT] = 80, [DM_C] = 80}},
    /* 5/4: 10 steps (8ths) */
    {"MathRock", 45, 5, 1, DMK_XLIVE, 0, 140,
     {[DM_K] = "x..x..x...", [DM_S] = "....x...x.", [DM_H] = "x.x.x.x.x.", [DM_X] = "x........."},
     {[DM_K] = 95, [DM_S] = 90, [DM_H] = 45, [DM_X] = 35}},
    /* 7/4: 14 steps (8ths) */
    {"MathRock", 46, 7, 1, DMK_XLIVE, 0, 130,
     {[DM_K] = "x...x...x..x..", [DM_S] = "..x...x...x..x", [DM_H] = "xxxxxxxxxxxxxx", [DM_X] = "x............."},
     {[DM_K] = 95, [DM_S] = 90, [DM_H] = 35, [DM_X] = 35}},
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
/* the bass part plays the rhythm's 808 line: each hit the note the keys' bass would play (dm8.root: the last chord's,
 * kept while no key is down; none yet: the key's tonic in the second octave) plus the step's interval; a slide step
 * glides into it from the note sounding (the 808 engine's GLIDE, legato), a hit strikes it afresh */
static struct { uint8_t root, have, cur; } dm8;
static struct { uint8_t n, k, note; uint16_t t0, len; } dm8r;   /* an 808 roll under way (as dmroll) */
/* ALGORITHM in the rhythm machine (cf_dmui.c dm_algo, kept in dm_rec_t's rsv[0]): 0 each rhythm's own line, k a line
 * (DL_*) for every rhythm, 0xFF none. A byte the UI writes and the audio reads */
static volatile uint8_t dm8_sel;
static const dm8_line_t *dm8_line_of(uint32_t r)
{
    uint32_t k = dm8_sel == 0u ? DM_RHY[r % DM_N].line : dm8_sel == 0xFFu ? 0u : dm8_sel;
    return k && k <= DM8_N && dm_steps(DM_RHY[r % DM_N].beats) == 16u ? &DM8_LINES[k - 1u] : 0;
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
static int dm8_iv(char c)                        /* a note char: its interval over the root */
{
    return c >= 'a' && c <= 'y' ? c - 'm' : c >= 'A' && c <= 'Z' ? 13 + (c - 'A') : 0;
}
static void dm8_step(uint32_t h)
{
    const dm8_line_t *L = dm8_line_of(dmp.r);
    uint32_t k, n;
    char c;
    int note;
    if (!dm8_active() || !L || h >= 16u)
        return;
    k = (dmp.nbar % (L->bars ? L->bars : 1u)) * 16u + h;   /* (the line's own bars, round from the rhythm's start) */
    if (k >= str_len(L->hit))
        return;
    c = L->hit[k];
    n = c >= '2' && c <= '8' ? (uint32_t)(c - '0') : 0u;
    if (c != 'x' && c != 's' && !n)
        return;
    note = dm8.have ? (int)dm8.root : 24 + (int)cfc.cfg.tonic;
    while (note > 35)                            /* (the root in the 808's own octave, C1 .. B1: deep, whatever the keys'
                                                  * octave; the line's intervals over it) */
        note -= 12;
    while (note < 24)
        note += 12;
    note += dm8_iv(k < str_len(L->note) ? L->note[k] : 'm');
    while (note < 18)
        note += 12;
    while (note > 71)
        note -= 12;
    dm8_strike((uint32_t)note, 110, c == 's');
    dm8r.n = (uint8_t)n;                         /* (a roll: the "spinz", its other hits fading, in dmr_now) */
    dm8r.k = 1;
    dm8r.note = (uint8_t)note;
    dm8r.t0 = (uint16_t)dmp_tick_of(h);
    dm8r.len = (uint16_t)(sqp_bar_ticks() / sq_dsteps());
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
    dm8_step(h);
}
static void dmr_now(void)
{
    uint32_t l;
    for (l = 0; l < SQ_NLANE; l++)
        while (dmroll[l].k < dmroll[l].n && dmp.t >= dmroll[l].t0 + (uint32_t)dmroll[l].k * dmroll[l].len / dmroll[l].n) {
            dm_strike(l, 72u + 24u * (uint32_t)dmroll[l].k / dmroll[l].n);   /* (a roll swells into its last hit) */
            dmroll[l].k++;
        }
    while (dm8r.k < dm8r.n && dmp.t >= dm8r.t0 + (uint32_t)dm8r.k * dm8r.len / dm8r.n) {
        if (dm8_active())
            dm8_strike(dm8r.note, 30u + 80u * (uint32_t)(dm8r.n - dm8r.k) / dm8r.n, 0);
        dm8r.k++;
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
    dm8r.n = 0;
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
    if (dmp.on)
        dmp_now();
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
