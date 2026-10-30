# Leviathan

Leviathan is CoralFlower for trap, rage and plugg production (Ken Carson, Playboi Carti, Destroy Lonely, the Opium
sound, Atlanta trap). It keeps CoralFlower's instrument and its look: scale keys and chord buttons, the bass, the
arpeggiator, the sequencer and its song, the rhythm machine, the effects, the sound editor, the pixel screens. It adds:

- **A melody engine** on the ARP button (MODE **Melody**): your chord keeps sounding while a melody plays over it on
  the LAYER, in time. It is built like a synth: a rhythm layer and a pitch layer, each an oscillator with its own
  algorithms, two LFOs and a four-slot modulation matrix.
- **Six dual-layer leads** for it, LEVIATHAN first: two oscillator pairs with their own envelopes and LFO and
  envelope routings in the VA engine's matrix.
- **An 808 engine** on the bass: a tuned kick-bass with pitch punch, a long tail, slides, and six waveshapers from a
  clean sub to a blown-out rage 808. Eleven presets, HEAVY first, first in the bass list.
- **A rhythm machine of 808 patterns**: **Rage** (drums and an 808), then 33 of your own 808 MIDI patterns, the 808
  alone, each note at its own tick and velocity, with its slides and rolls, over the root of the chord you play.
- **Your two snares**, Metro 2 and Mafia, played as the samples they are on the X Rage kit's snare and clap.
- **Eight rage leads** for the chords and the layer: RAGE SAW, KEN LEAD, ICE BELL, DARK PLUCK, TRAP FLUTE, HYPER PAD,
  SIREN, PLUGG KEYS.
- **A fallen angel** in the flower's place, who reacts to your chords and turns demonic as the song plays.
- **The DRUMS editor in a step-grid layout**: the drum picked big, bars 1 and 2 at once, four drums by 16 steps each.

> **Status: untested on hardware.** Leviathan is built and checked on a computer: CoralFlower's host tests, its stuck-note
> fuzz test and its browser-emulator test all pass. The browser build plays bit for bit like the native one. The device
> package is built by GitHub Actions with JieLi's toolchain (below). Keep a backup and know CoralFlower's recovery path
> before you install.

## Getting the firmware

1. Fork CoralFlower on GitHub and push this tree to it.
2. Open **Actions → release → Run workflow** and enter a version such as `0.1`. The workflow fetches JieLi's toolchain,
   builds and keeps the package as the run's artifact (`coralflower-0.1.fwsc`; its version string is `Leviathan 0.1`).
3. Install it as CoralFlower installs a candidate: the web installer's "Choose a .fwsc file", or
   `python3 tools/fm1_install.py coralflower-0.1.fwsc`.

Leviathan keeps CoralFlower's identity (`FM-1_930`), so CoralFlower's installer, backups and settings work with it, and
installing CoralFlower again takes it back.

## Making a beat

1. **PLAY**. On a new unit, PLAY on HOME starts the first rhythm, **Rage**, at 180 BPM. Its 808 takes over the bass.
   (BASS on the LFO button must be on, as it is by default.)
2. **Tap a chord button** (Top layout: the upper row; **m** for minor). With Lock it stays on.
3. **Play roots** on the lower row. The 808 plays its pattern on each new root, and the chords use the chord sound
   (RAGE SAW by default).
4. **SEQ twice** opens the rhythm machine. **SELECT** picks a rhythm: Rage, or one of your 808 patterns. Picking one
   sets **TEMPO** to the tempo its MIDI was made at (shown under its name); K3 turns it after. A rhythm picked while
   another plays comes in at the next bar line.
5. **EDIT** on Rage makes it a pattern and opens its drums to edit (below); an 808 pattern has no drums to edit. REC
   records a take over any rhythm, as in CoralFlower.
6. **SELECT on HOME** changes the 808 (the bass sound); **PRESETS** changes the chord sound.

## The melody engine

**ARP** opens its page; **K1** (ON/OFF) turns it on. On a new unit MODE is already **Melody**. Hold a chord on HOME
(a chord button and a root): the chord sounds as it always does, and a melody plays over it on the LAYER part with
its own sound. It runs on 16ths, on the beat while a rhythm, a pattern or the metronome plays, else from your first
chord. When the chord changes, the melody keeps its shape and moves to the new chord. LATCH (the ARP's Feel page, in
another mode) keeps it playing after you let go.

It is built the way Serum, Electra X and Surge XT build a sound, but for notes:

- **The rhythm layer** decides when a note starts. Its algorithms: **Euclid** (hits spread evenly), **Bounce** (3-3-2
  and its pickups), **Drive** (on the beat, then 8ths, then 16ths), **Sync** (off the beat), **Rage** (the rage leads'
  stabs) and **Random**. **DENSITY** sets how many of a bar's 16 steps play.
- **The pitch layer** decides which note. A contour oscillator (**Sine**, **Tri**, **Rise**, **Fall**, **Square**,
  **Steps**, **Walk** or **Arp**) runs at a **RATE** of cycles per phrase over a **RANGE** above the chord's root.
  Its value lands on the nearest chord tone on the beats, and on **TONES** % of the other steps; the rest take the
  nearest scale tone (the key's scale, or the chord's own minor or major with Quantize off).
- **Two LFOs** (Sine, Tri, Saw, Square, S&H), a quarter bar to eight bars a cycle, on the phrase's clock.
- **A four-slot modulation matrix.** Each slot takes a source (LFO 1, LFO 2, **Ramp** the phrase's position,
  **Chord** a fall over a bar from each new chord, **Random** each step's own, **Beat** the beats, **Height** the last
  note's place in the range) times an amount (-100 to +100) onto a destination (**Density**, **Pitch**, **Range**,
  **Octave**, **Gate**, **Velo**, **Slide**, **Tones**, **Rotate** the rhythm, **Rate** the contour's).

A phrase is **BARS** long (1, 2 or 4) and plays again; **VARY** % of its steps change on each repeat. **SEED** picks
another phrase: the same settings, seed and chord always give the same notes, so the screen shows what will play, as a
piano roll of the two bars playing. **GATE** is each note's length as a share of the gap to the next, **SLIDE** the
share of notes that glide in legato, **ACCENT** the velocity added on the beats, **OCTAVE** moves the range.

| Page | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| Pattern | ON/OFF | MODE | RHYTHM | DENSITY |
| Pitch | CONTOUR | RATE | RANGE | TONES |
| Phrase | BARS | VARY | GATE | SEED |
| Voice | OCTAVE | SLIDE | ACCENT | TEMPO |
| LFOs | LFO 1 | RATE 1 | LFO 2 | RATE 2 |
| Matrix | SLOT | SOURCE | DEST | AMOUNT |

SELECT turns the pages. **PRESETS** on this page changes the melody's sound (the LAYER's). The default matrix: LFO 1
(a triangle over two bars) bends the pitch contour, the beats get louder, and a random octave jump now and then.
The settings are kept with the rhythm machine's, over a power-off.

### The melody's sounds

Six VA leads made for it, each two layers. Layer A is oscillators 1 and 2 on the main envelope; layer B is
oscillators 3 and 4 with an envelope of their own (ENV 3 on their levels through the VA matrix):

| Sound | Layer A | Layer B | Matrix |
| --- | --- | --- | --- |
| **LEVIATHAN** (the default) | two MORPH saws | a ring-modulated bell two octaves up, struck | an LFO pulls the saws' shapes apart (a wavetable position), velocity opens the filter, S&H pans |
| **ABYSS BELL** | a sine and a triangle an octave up | a ring-modulated bell that fades first | a late vibrato, velocity on the bell |
| **SYNC SERPENT** | a hard-synced saw | a square an octave down | ENV 3 sweeps the synced saw down (the sync sweep), an LFO moves it |
| **TIDE PLUCK** | two saws plucked through a resonant filter | a sine sub and a breath of white noise on the attack | each note's cutoff a little different (Random) |
| **KRAKEN KEYS** | a pulse whose width an LFO sweeps, a saw an octave up | a ring-modulated metal partial, struck | velocity on the cutoff |
| **HYDRA SAW** | two saws wide | two MORPH saws an octave up, their shapes moving apart | a pitch blip on the attack, a late vibrato |

They are in every VA sound list, so they work as chord sounds too. The sound editor (EDIT) shows their oscillators,
envelopes, LFOs and matrix slots on its pages.

### Rage

Rage is an FL Studio pattern you picked, read from its channel rack at 180 BPM. It has no kick: the 808 is the low end
and hits once every two bars. The snare plays on steps 1, 7 and 13 of bar 1 and 5 and 11 of bar 2 (the rage bounce),
the clap on beat 3 of each bar, one open hat at step 13, and one shaker at bar 2's step 3. The shaker is on the hat
lane, since the kits have no shaker. The snare is your **Metro 2** and the clap your **Mafia** (below).

### The 808 patterns

After Rage, the rhythm machine holds your 808 MIDI files, one rhythm each, with the 808 alone and no drums. They are
listed in the order of their numbers (246 to 263, then 282 to 298); 292 and 293 were copies of 283 and 282 and are
left out.

| # | Name | Bars | BPM | # | Name | Bars | BPM |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 246 | Short | 8 | 108 | 282 | Spinz 1 | 2 | 160 |
| 247 | LastSpinz | 8 | 146 | 283 | Spinz 2 | 8 | 160 |
| 248 | UseALot 1 | 8 | 160 | 284 | SF RT | 4 | 120 |
| 249 | UseALot 2 | 8 | 140 | 285 | Forsaken | 4 | 160 |
| 250 | UseALot 3 | 8 | 140 | 286 | King | 8 | 160 |
| 251 | Zay 1 | 4 | 130 | 287 | Bitcrush | 8 | 160 |
| 252 | MingoJugg | 4 | 150 | 288 | OhMyGawd | 4 | 158 |
| 253 | Zay 2 | 8 | 130 | 289 | LuhTwizz | 4 | 105 |
| 254 | Louder 1 | 4 | 140 | 290 | Chase | 8 | 157 |
| 255 | Louder 2 | 8 | 140 | 291 | Chichi | 8 | 146 |
| 256 | Jugg | 8 | 147 | 294 | Quis | 4 | 120 |
| 257 | Spinz 5 | 8 | 130 | 295 | Spinz 3 | 8 | 160 |
| 258 | Spinz 6 | 8 | 130 | 296 | Spinz 4 | 4 | 160 |
| 259 | Spinz 7 | 8 | 130 | 297 | Bounce | 8 | 182 |
| 260 | Spinz 8 | 8 | 155 | 298 | Secret | 2 | 130 |
| 261 | Mingo | 2 | 110 | | | | |
| 262 | Goon | 8 | 100 | | | | |
| 263 | New Rack | 8 | 150 | | | | |

Each note plays at its own tick (96 to a beat, as in the MIDI file), so off-grid notes, triplets and fast rolls keep
their timing. Each keeps its velocity, so the fading and swelling rolls fade and swell. A note that starts while the one
before still sounds (in the file) at another pitch slides into it with the 808's GLIDE. Notes play one-shot, as an 808
sample rings in FL Studio: each sounds until the next starts, whatever its length in the file, and a long one fades by
the 808's own decay. A file whose second half repeats its first is kept as the half (the Bars column). Goon's two tracks play as one 808.

The patterns are intervals over the most frequent note in each file. The 808 plays them over the root of the chord you
play (the last chord's, kept while no key is down; before any, the key's tonic), in the 808's own octave, C1 to B1. The
chords no longer strike the bass themselves while a pattern plays. When the rhythm stops, the keys' bass comes back.
The view shows a pattern as a piano roll, two bars at a time: a note a bar from its tick to the next note, higher for a
higher note, brighter for a louder one, a slide joined to the note before.

`tools/trap/mid2lines.py` turns the MIDI files into `firmware/src/dm8_lines.h`. Send more and they become rhythms the
same way.

### Your snares

Metro 2 (37 ms) and Mafia (60 ms) play as the samples they are: 16-bit, 44.1 kHz, mono (their two channels were the
same), no resampling. The **X Rage** kit plays Metro 2 on its snare and Mafia on its clap, so Rage and any pattern on X
Rage use them. They take the drum's LEVEL; the kit's other knobs do nothing to a sample. `tools/trap/wav2smp.py` turns
WAV files into `firmware/src/drum_smp.h`, 8.5 KB for the two.

### HEAVY, the default 808

HEAVY is matched by measurement to a reference 808 you chose (a heavy, saturated C1 808). A seventh MODEL, **HEAVY**,
is a shaping curve built from the reference's first 16 harmonics. At full DRIVE a sine through it gives the reference's
tone within half a dB on the first eight harmonics: the third harmonic 10 dB over the fundamental, the second and
fourth near it. Lower DRIVE uses less of the curve, for a rounder sub. During the punch the curve swells in as the
pitch falls two octaves, as the reference does in its first 70 ms. With DECAY at 127 the note holds for 1.8 s, then
falls about 14 dB a second like the reference one-shot, whether or not the key is still down. The bass part starts at
0 dB (CoralFlower's is -8 dB), so the 808 sits up front, just under the output limiter.

With an 808 on the bass part, the part is one voice (legato, so it slides). The octaver is off under it.

## The 808 engine

| Macro | What it does |
| --- | --- |
| MODEL | The waveshaper: CLEAN (a pure sine), WARM (tanh), HARD (a hard clip), GROWL (asymmetric), FOLD (a wavefolder, talking), RAGE (tanh into a clip with a rectified octave of fuzz). Each model is levelled so a change keeps the loudness. |
| DRIVE | How hard the model is driven. |
| PUNCH | How far above the note the pitch starts. |
| SNAP | How fast it drops to the note. |
| DECAY | The tail, from a punch to a long boom. |
| TONE | A two-pole low-pass after the shaper. |
| CLICK | A click on the hit, so the 808 reads on small speakers. |
| GLIDE | The slide time. |

The sound editor (EDIT on HOME, the bass part) shows them on two pages, **808** and **Shape**. The presets:
HEAVY (the default),
CARSON, SLIDE, CLEAN, HARD, BLOWN, GROWL, TALK, SHORT, ROUND SUB and DIRTY.

## The DRUMS editor

The DRUMS track of a pattern (EDIT on Rage in the rhythm machine, or the sequencer's DRUMS) is drawn as a step editor.
The drum picked sits big at the top with its neighbours either side, and a dot for each drum shows where it is among
the 11. Below that are two bars at once, one over the other: the pair the frame's bar is in (bars 1 and 2, then 3 and
4, ...), each four drums by the bar's steps, its number at its left. The picked drum is bright, the others dim, each in
its own colour, the beats' columns a shade lighter; the picked drum's row in the frame's bar is framed, and a mark over
a bar shows the step playing. Under them are the drum and the bar, then the bars as tabs. The controls are
CoralFlower's: the upper keys pick a drum, the lower keys toggle its steps in the frame's bar, SELECT moves through the
bars, and K1-K4 set the drum's kit, its level, the bars and the position. The bar-copy and erase gestures switch back
to the bars' view while they run.

## The fallen angel

She takes the plant's place on HOME and behind every screen that showed the plant (faintly, as it was). At rest she
kneels with her hands together, wrapped in her wings.

- **She reacts like the flower.** Each chord tone unfurls and lights a feather (its pitch class picks the feather:
  C to F on one wing, F# to B on the other). More tones open the wings wider. Major lifts her wings and head, minor
  lowers them, diminished keeps them tight, augmented spreads them, sus tilts them, and single notes open one wing.
  Notes send light along their feather and shed it: a feather drifting down, or an ember rising once she has fallen.
  Loudness opens her, and the 808 and the kick jolt her and the fire under her.
- **She falls as the song plays.** While a rhythm, a pattern or the song plays, her fall grows, and every new chord
  adds a little. After about two and a half minutes she is all the way down. Chords alone, with nothing playing, pull
  her down slowly. Stopped and quiet for 8 seconds, she heals back in about 25 seconds.
- **Calm:** white wings, fair skin, golden hair, a halo, closed eyes. A cool night sky with stars and a crescent moon
  over clouds.
- **Falling:** the halo cracks, flickers and fades. Horns grow out of her hair, her eyes open, her colours turn in
  patches. The sky burns from the ground up.
- **Fallen:** red skin, glowing eyes, fangs, black hair and robe, black wings torn to red membranes. A blood moon over
  a burning ground with embers, and the panels under the screen turn red.

The Backdrop setting (GLO > Display) still works: Scene, Ripples (a ring of sound at every chord) or None.

Under the hood: `firmware/src/flora/angel.c` implements the same `flora_*` calls as CoralFlower's `flora.c`, on the same
120 x 70 canvas and palette, plus `flora_set_playing` and `flora_thump`. The change between calm and fallen is dithered
pixel by pixel through fixed palette entries, with no palette change and so no full-screen redraw. Each part turns in
its own window of the fall: the sky first, her skin last. Her figure is a 26 x 34 pixel sprite; the wings, horns, halo,
sky, moon, clouds and fire are drawn in code. It is about 3 KB smaller than the plant's code and uses far less RAM.

## What changed from CoralFlower

- The 808 engine in engine slot 11 (NOISE's retired slot): `FELUCCA_808`.
- The bass list starts with the 808s; a new unit starts on RAGE SAW chords over the CARSON 808, the LAYER on KEN LEAD.
- The rhythm machine holds Rage and the owner's 808 patterns only (CoralFlower's rhythms are out). Picking one sets the
  tempo. PLAY on HOME plays a rhythm on a new unit, and SEQ twice opens the rhythm machine.
- The X Rage kit plays the owner's snare samples on its snare and clap (eng_drum.c can play a sample on a lane).
- The tempo starts at 180 BPM (Rage's). The interface colour starts on **Blood** (CoralFlower's Coral ramp, renamed).
- The sequencer's marks are trap marks instead of flowers: a chord an ice diamond, a bass note a drop, a drum a flame,
  the layer a star, on wine-red stems.
- The ARP button's MODE has **Melody** (the default), the melody engine (`cf_mel.c`); its settings are kept in the
  rhythm machine's record.
- Six dual-layer leads added to the VA engine; the LAYER's sound starts on LEVIATHAN. KEN LEAD and SIREN were too hot
  for six-note chords (the VA test caught it) and are levelled.
- The splash says Leviathan; the version string is `Leviathan X.Y`. (It was TrapRoot before 0.1's release.)

## Building and testing

```sh
python3 tools/build.py --gen-only                       # the generated headers
./build.sh                                              # the device package (JieLi's toolchain: tools/get_toolchain.sh)
sh tools/emu/cf_headless.sh SCRIPT OUTDIR               # the firmware headless: screenshots, dumps, a WAV
sh tools/emu/web/build_web.sh                           # the browser emulator (Emscripten)
sh tools/emu/web/build_web_clang.sh                     # .. or with clang's wasm32-wasi target, no Emscripten
node tools/emu/web/test_web_emu.mjs --no-page           # the browser build's checks, bit-exact against the native one
python3 tests/stuck_notes.py                            # no stuck notes through a thousand random changes
```

The host tests of `.github/workflows/check.yml` (cf_backup_test, cf_chord_test, cf_theory_test, st_log_test) pass.

## Credits

Leviathan is built on [CoralFlower](https://github.com/FicusFicusov/CoralFlower) (its instrument, screens, rhythm machine
and pixel style), [ChoralRoot FM-1](https://github.com/Quixotic7/ChoralRootFM1) and
[Felucca](https://github.com/hugelton/Felucca). The 808's models follow [Surge XT](https://surge-synthesizer.github.io/)'s
waveshaper set, rewritten in fixed point (no Surge code is copied). Leviathan is GPL-3.0-only, like everything it is built
on. It is independent of and unaffiliated with M-VAVE, the Surge Synth Team and the artists named here, whose names
only describe a style.
