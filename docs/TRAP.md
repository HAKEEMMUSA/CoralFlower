# TrapRoot

TrapRoot is CoralFlower for trap, rage and plugg production (Ken Carson, Playboi Carti, Destroy Lonely, the Opium
sound, Atlanta trap). It keeps CoralFlower's instrument and its look: scale keys and chord buttons, the bass, the
arpeggiator, the sequencer and its song, the rhythm machine, the effects, the sound editor, the pixel screens. It adds:

- **An 808 engine** on the bass: a tuned kick-bass with pitch punch, a long tail, slides, and six waveshapers from a
  clean sub to a blown-out rage 808. Ten presets, first in the bass list.
- **Trap rhythms** in the rhythm machine, first in its list: Rage, Atlanta, Carti, Opium, Plugg, Memphis, TrapDrill and
  Rolls. They have **hat rolls** (x2 to x8 hits in a step), accents and ghost notes, and an **808 line** that plays the
  bass at the root of your chord, with slides.
- **X Rage**, a drum kit for them: a short punchy kick (the 808 is the low end), a high cracking snare, a wide clap,
  tight bright hats.
- **Eight rage leads** for the chords and the layer: RAGE SAW, KEN LEAD, ICE BELL, DARK PLUCK, TRAP FLUTE, HYPER PAD,
  SIREN, PLUGG KEYS.
- **A fallen angel** in the flower's place, who reacts to your chords and turns demonic as the song plays.
- **The DRUMS editor in a step-grid layout**: the drum picked big, four drums by 16 steps.

> **Status: untested on hardware.** TrapRoot is built and checked on a computer: CoralFlower's host tests, its stuck-note
> fuzz test and its browser-emulator test all pass. The browser build plays bit for bit like the native one. The device
> package is built by GitHub Actions with JieLi's toolchain (below). Keep a backup and know CoralFlower's recovery path
> before you install.

## Getting the firmware

1. Fork CoralFlower on GitHub and push this tree to it.
2. Open **Actions → release → Run workflow** and enter a version such as `0.1`. The workflow fetches JieLi's toolchain,
   builds and keeps the package as the run's artifact (`coralflower-0.1.fwsc`; its version string is `TrapRoot 0.1`).
3. Install it as CoralFlower installs a candidate: the web installer's "Choose a .fwsc file", or
   `python3 tools/fm1_install.py coralflower-0.1.fwsc`.

TrapRoot keeps CoralFlower's identity (`FM-1_930`), so CoralFlower's installer, backups and settings work with it, and
installing CoralFlower again takes it back.

## Making a beat

1. **PLAY**. On a new unit, PLAY on HOME starts the first rhythm, **Rage**, at 180 BPM. The 808 line takes over the
   bass. (BASS on the LFO button must be on, as it is by default.)
2. **Tap a chord button** (Top layout: the upper row; **m** for minor). With Lock it stays on.
3. **Play roots** on the lower row. The 808 hits land on each new root, slides glide into it, and the chords use
   the chord sound (RAGE SAW by default).
4. **SEQ twice** opens the rhythm machine (TrapRoot opens it rather than the sequencer). **SELECT** picks a rhythm;
   it comes in at the next bar line. KIT, VOLUME, TEMPO and SWING are on the knobs.
5. **EDIT** in the rhythm machine makes the rhythm a pattern and opens its drums to edit (below). REC records a take
   over it, as in CoralFlower.
6. **SELECT on HOME** changes the 808 (the bass sound); **PRESETS** changes the chord sound.

The tempo is the user's, as in CoralFlower. Each rhythm shows the tempo its style lives at (Rage ~180, Atlanta ~140,
Opium ~165, Plugg ~145, Memphis ~135).

### Rage

Rage is an FL Studio pattern you picked, read from its channel rack at 180 BPM. It has no kick: the 808 is the low end
and hits once every two bars. The snare plays on steps 1, 7 and 13 of bar 1 and 5 and 11 of bar 2 (the rage bounce),
the clap on beat 3 of each bar, one open hat at step 13, and one shaker at bar 2's step 3. The shaker is on the hat
lane, since the kits have no shaker.

### HEAVY, the default 808

HEAVY is matched by measurement to a reference 808 you chose (a heavy, saturated C1 808). A seventh MODEL, **HEAVY**,
is a shaping curve built from the reference's first 16 harmonics. At full DRIVE a sine through it gives the reference's
tone within half a dB on the first eight harmonics: the third harmonic 10 dB over the fundamental, the second and
fourth near it. Lower DRIVE uses less of the curve, for a rounder sub. During the punch the curve swells in as the
pitch falls two octaves, as the reference does in its first 70 ms. With DECAY at 127 the note holds for 1.8 s, then
falls about 14 dB a second like the reference one-shot, whether or not the key is still down. In a rhythm's 808 line
the root sits in the 808's own octave, C1 to B1 (33 to 62 Hz), whatever octave the keys are in.

### The 808 lines

The 808 lines come from your MIDI files: Spinz 1-4, Forsaken, King, Bitcrush, OhMyGawd, LuhTwizz, Chase, Chichi,
Quis, Bounce, Secret and SF RT, plus Rage's. Each keeps its own phrase length (2, 4 or 8 bars), its intervals, its
slides and its rolls (the "spinz": up to 8 fading hits inside a 16th). Secret's triplets sit on the nearest 16ths.
Each rhythm has its own line. **ALGORITHM** in the rhythm machine picks the line for every rhythm instead: Own (each
rhythm's), any line, or None. The choice is kept with the rhythm machine's settings and named under the rhythm
(`[ALGO] Spinz 2`). The rhythm view shows two bars at a time, the pair playing, with the line's own pair under them.

| Rhythm | Its 808 line |
| --- | --- |
| Rage | Rage (one hit every two bars) |
| Atlanta | Chase |
| Carti | Bounce |
| Opium | Spinz 2 |
| Plugg | Bitcrush |
| Memphis | Quis |
| TrapDrill | Spinz 4 |
| Rolls | Spinz 3 |
| GTR | Chichi |
| Moet | LuhTwizz |

**GTR** (~140 BPM) and **Moet** (~98 BPM) are your two drum loops, transcribed to 4 bars. GTR has a half-time snare
and clap and 8th hats with 32nd-note rolls. Moet has its kick on syncopated 16ths, the snare on 3 with pickups, and
16th hats.

### How the 808 line plays

A rhythm with an 808 line plays the bass part while it runs. Each hit strikes the note the keys' bass would play (the
last chord's; kept while no key is down), plus the step's interval (octaves, fifths and sevenths in the factory lines).
A slide step glides into its note from the one sounding, without a new attack. The chords no longer strike the bass
themselves. When the rhythm stops, or another one without an 808 line comes in, the keys' bass comes back. The line
plays whatever the bass sound is; the 808s are made for it. A rhythm made into a pattern keeps its drums. The 808 line
and the rolls stay in the rhythm machine: a pattern's BASS track is written as in CoralFlower.

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

The DRUMS track of a pattern (EDIT in the rhythm machine, or the sequencer's DRUMS) is drawn as a step editor. The drum
picked sits big in the middle with its neighbours either side, and a dot for each drum shows where it is among the 11.
Below that is a grid of four drums by the bar's steps: the picked drum bright and framed, the others dim, each in its
own colour, the beats' columns a shade lighter, and a mark over the step playing. Under the grid are the drum and the
bar, then the bars as tabs. The controls are CoralFlower's: the upper keys pick a drum, the lower keys toggle its steps,
SELECT moves through the bars, and K1-K4 set the drum's kit, its level, the bars and the position. The bar-copy and erase
gestures switch back to the bars' view while they run.

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
- The rhythm machine lists the trap rhythms first. PLAY on HOME plays a rhythm on a new unit, and SEQ twice opens the
  rhythm machine.
- The tempo starts at 180 BPM (Rage's). The interface colour starts on **Blood** (CoralFlower's Coral ramp, renamed).
- The sequencer's marks are trap marks instead of flowers: a chord an ice diamond, a bass note a drop, a drum a flame,
  the layer a star, on wine-red stems.
- The splash says TrapRoot; the version string is `TrapRoot X.Y`.

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

TrapRoot is built on [CoralFlower](https://github.com/FicusFicusov/CoralFlower) (its instrument, screens, rhythm machine
and pixel style), [ChoralRoot FM-1](https://github.com/Quixotic7/ChoralRootFM1) and
[Felucca](https://github.com/hugelton/Felucca). The 808's models follow [Surge XT](https://surge-synthesizer.github.io/)'s
waveshaper set, rewritten in fixed point (no Surge code is copied). TrapRoot is GPL-3.0-only, like everything it is built
on. It is independent of and unaffiliated with M-VAVE, the Surge Synth Team and the artists named here, whose names
only describe a style.
