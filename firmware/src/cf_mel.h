/* SPDX-License-Identifier: GPL-3.0-only
 * Leviathan: the melody engine's settings (cf_mel.c plays them; the ARP page sets them, cf_dmui.c keeps them). */
#ifndef CF_MEL_H
#define CF_MEL_H
enum { ML_R_EUCLID, ML_R_BOUNCE, ML_R_DRIVE, ML_R_SYNC, ML_R_RAGE, ML_R_RANDOM, ML_NR };
enum { ML_C_SINE, ML_C_TRI, ML_C_RISE, ML_C_FALL, ML_C_SQUARE, ML_C_STEPS, ML_C_WALK, ML_C_ARP, ML_NC };
enum { ML_W_SIN, ML_W_TRI, ML_W_SAW, ML_W_SQR, ML_W_SH, ML_NW };
enum { ML_S_OFF, ML_S_LFO1, ML_S_LFO2, ML_S_RAMP, ML_S_CHORD, ML_S_RAND, ML_S_BEAT, ML_S_HEIGHT, ML_NS };
enum { ML_D_OFF, ML_D_DENSITY, ML_D_PITCH, ML_D_RANGE, ML_D_OCTAVE, ML_D_GATE, ML_D_VELO, ML_D_SLIDE, ML_D_TONES,
       ML_D_ROTATE, ML_D_RATE, ML_ND };
#define ML_NMOD 4u
typedef struct {
    uint8_t rhythm, density;                         /* ML_R_*; 0..100 % */
    uint8_t contour, rate, range, tones;             /* ML_C_*; ML_RATE_X2 index; 5..36 semitones; 0..100 % */
    uint8_t bars, vary, gate, seed;                  /* 1, 2, 4; 0..100 %; 10..100 %; 0..255 */
    int8_t oct;                                      /* -2 .. +2 */
    uint8_t slide, accent;                           /* 0..100 %; 0..60 velocity on the beat */
    uint8_t lfo_w[2], lfo_r[2];                      /* ML_W_*; ML_LRATE index */
    uint8_t src[ML_NMOD], dst[ML_NMOD];              /* ML_S_*, ML_D_* */
    int8_t amt[ML_NMOD];                             /* -100 .. +100 */
    uint8_t slot;                                    /* (the page: the matrix slot shown) */
} cf_mel_t;
#define CF_MEL_DEFAULT {ML_R_RAGE, 45, ML_C_SINE, 2, 19, 60, 2, 15, 70, 7, 0, 10, 20, {ML_W_TRI, ML_W_SH}, {3, 1}, \
                        {ML_S_LFO1, ML_S_BEAT, ML_S_RAND, ML_S_OFF}, {ML_D_PITCH, ML_D_VELO, ML_D_OCTAVE, ML_D_OFF}, \
                        {35, 40, 60, 0}, 0}

#endif
