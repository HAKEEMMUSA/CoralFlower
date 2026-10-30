/* TrapRoot: the fallen angel at the centre of the screen (in place of CoralFlower's plant and coral, behind the same
 * flora_* calls, so every screen that showed the plant shows her).
 *
 * A kneeling angel, her hands together, wrapped in her own wings while nothing plays. Chords open her: each chord tone
 * unfurls and lights a feather (the tone's pitch class picks it: C .. F on her right wing, F# .. B on her left), more
 * tones open the wings wider; the chord's quality sets her pose (major lifts the wings and her head, minor lowers them,
 * diminished keeps them tight, augmented spreads them, sus tilts them, single notes open one wing). Notes send light
 * along their feather and shed it: a feather drifting down, an ember rising once she has fallen. Loudness opens her,
 * an 808 hit jolts her and the fire under her.
 *
 * And she falls as the song goes on: while something plays (a rhythm, a pattern, the song) her corruption grows, the
 * chords add to it; stopped and quiet, she heals back. Calm: white wings, fair skin, a halo, closed eyes, a cool night
 * sky with a crescent moon over clouds. Fallen: the halo gone, horns, red skin, glowing eyes and fangs, black wings
 * torn to red membrane, a blood moon over a burning ground, embers. The change is dithered pixel by pixel between the
 * calm and the fallen colours, so it creeps in.
 *
 * Integer maths only, no allocation, deterministic for a given seed. Same canvas and palette as the plant
 * (flora_canvas.h, flora_pal.h): the backdrop tints are the calm night (FL_L*) and the burning sky (FL_S*). */
#ifndef FLORA_H
#define FLORA_H

#include <stdint.h>
#include "flora_canvas.h"

enum {
    FL_Q_NONE = 0, /* nothing held: she folds her wings around her and rests */
    FL_Q_MAJOR,
    FL_Q_MINOR,
    FL_Q_DIM,
    FL_Q_AUG,
    FL_Q_SUS2,
    FL_Q_SUS4,
    FL_Q_SINGLE    /* single notes (chord type Off / none) */
};

enum { FL_SP_PLANT = 0, FL_SP_CORAL = 1 };   /* (kept for the screens: she is the same in both layouts) */

/* what is behind her: SCENE the sky, the moon, the clouds or the fire; RIPPLES the sky and a ring of sound out of
 * her at every chord; NONE plain dark */
enum { FL_BD_SCENE = 0, FL_BD_RIPPLES, FL_BD_NONE, FL_NBD };

typedef struct {
    uint8_t n;       /* number of chord tones, 0 = nothing held */
    uint8_t note[8]; /* MIDI notes, low to high as voiced */
    uint8_t root;    /* root pitch class 0..11 */
    uint8_t quality; /* FL_Q_* */
    int8_t bass;     /* MIDI note of the bass part, or -1 */
} fl_chord_t;

#define AN_NFEATH 12   /* feathers a wing */
#define AN_NPART 28

typedef struct {
    int32_t x, y;   /* canvas Q8 */
    int16_t vx, vy; /* Q8 px per 64 ms */
    uint16_t life;  /* ms left */
    uint8_t kind;   /* 0 a feather falling, 1 an ember rising, 2 a spark */
    uint8_t shade;
} an_part_t;

typedef struct {
    fl_chord_t chord;    /* what is held now (n = 0: nothing) */
    uint8_t species;     /* FL_SP_* (unused by her) */
    uint8_t backdrop;    /* FL_BD_* (set it directly) */
    uint8_t bloom;       /* (the plant's intro colour; unused) */
    uint8_t home;        /* the key's tonic */
    uint16_t nseg;       /* not 0 once flora_init ran (the UI's test) */
    int16_t cx, cy;      /* canvas position of her base (the ground), px */
    uint32_t t;          /* ms */
    uint32_t rng;
    /* the pose, Q12 (4096 = 1): how far the wings are open, their lift (- down .. + up), the head's lift */
    int32_t open[2], open_t[2];
    int32_t lift, lift_t;
    int32_t head, head_t;
    int32_t tight;       /* diminished: the feathers held close */
    uint16_t glow[2][AN_NFEATH];   /* each feather's light, Q12, from the chord's tones and the notes' pulses */
    uint8_t lit[2][AN_NFEATH];     /* a chord tone's feather */
    int32_t level, level_in;       /* smoothed loudness 0..65535 */
    /* the fall: Q16 (65536 = fallen); playing / quiet as the UI says (flora_set_playing) */
    int32_t corr;
    uint8_t playing;
    uint32_t quiet_ms;
    uint32_t thump_t;    /* when the last 808 hit came (ms), 0 none */
    int32_t thump;       /* its force, Q12, decaying */
    uint32_t ring_t[4];
    uint8_t ring_i;
    uint32_t chords;     /* chords struck since power-on */
    an_part_t part[AN_NPART];
    uint16_t part_acc;
} flora_t;

void flora_init(flora_t *f, uint32_t seed, int cx, int cy);
void flora_set_species(flora_t *f, int species);
void flora_set_key(flora_t *f, int tonic_pc);
void flora_set_chord(flora_t *f, const fl_chord_t *c);
void flora_note_on(flora_t *f, uint8_t note, uint8_t vel);
void flora_set_level(flora_t *f, uint16_t level); /* master loudness, 0..65535 */
void flora_tick(flora_t *f, uint32_t dt_ms);
void flora_render(flora_t *f, flc_t *c);
/* TrapRoot's own: something plays (a rhythm, a pattern, the song): she falls as it goes on; an 808 (a bass) hit */
void flora_set_playing(flora_t *f, int on);
void flora_thump(flora_t *f, uint8_t vel);
/* her fall, 0 .. 65536 (the screens may show it; a test may set it) */
static inline int32_t flora_fall(const flora_t *f) { return f->corr; }

#endif
