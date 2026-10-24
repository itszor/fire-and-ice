Fire & Ice (Graftgold for Renegade, 1993; Archimedes conversion, 1995) - static recompilation
==========================================================================================

A recipe: this repository holds no game data. It rebuilds the game as a
native executable from your own copy, recognised by content (recipe.json),
so it can be shared without distributing the game.

    ./build.sh "Fire & Ice (1995)(Renegade)(Disk 1 of 2).adf" "Fire & Ice (1995)(Renegade)(Disk 2 of 2).adf"
    cd recomp && ./fireice

build.sh runs recipe.json's steps with SuperSpider's tools/weave.py
(~/stuff/superspider, or $SUPERSPIDER; $SS below): it extracts the two
discs and merges them into one !Fire_Ice, builds the unpacker
(tools/unpack), which undoes the game's encryption, analyses and
translates the game and its loader, and links them with libroger
(~/stuff/libroger, or make ROGER=...). Needs python3, OCaml/dune, gcc,
SDL3 and libjpeg.

The input: TOSEC's two 800K ADFS discs, "Fire & Ice (1995)(Renegade)(Disk
1 of 2).adf" and "Disk 2 of 2" (their sha256s, and their files', are in
recipe.json).

What it makes:

    extracted/disk1, disk2  the discs, via weave.py
    game/!Fire_Ice          libroger's appdir (../game): both discs' !Fire_Ice
    tools/unpack            libroger's interpreter as an unpacker (unpack.c)
    unpacked/fire           Fire, decrypted, as it is at &81C0
    analysis/*.json         SuperSpider's analyses (fire, intro); *.roots:
                            entry points from --trace-gaps runs
    recomp/gen*/            their C translations
    recomp/fireice          the game

Committed: recipe.json, build.sh, tools/unpack.c, tools/Makefile,
analysis/*.roots, recomp/Makefile, recomp/game.c, recomp/fireice.ini,
this file.

Playing
-------

    ./fireice               as !Run: Graftgold's logo, Cool Coyote, the
                            title and its attract mode (demos, high scores)
    ./fireice --crop        only the rows the game draws on (fireice.ini)

The keys (the game reads them at &8F30): Z or comma left, X or full stop
right, ' or Q up, / or A down; Return or Space fires (so does Shift, Ctrl
or Alt on its own). Fire on the title starts a game, fire on the map
starts the level. F1-F4 are read as well (&E210); what they do wasn't
established. The game also asks for Acorn's joystick module
(Joystick_Read): a game controller stands in for it (the stick or D-pad
moves, A fires), or for the keys with --joystick=off (game.c's pad_map).

The two discs are merged, so the game never asks for disc 2.

How the game is put together
----------------------------

!Run sets Fire_Ice$Dir, runs FreeMem (a utility that sets screen memory
to 160K and the other dynamic areas to nothing), and with a 560K WimpSlot
runs PromptDisc, Gordian Lock's disc prompt, with the command "*Run
<Fire_Ice$Dir>.Intro". Intro (an absolute at &88000) shows Graftgold's
logo, loads Fire at &8000 (OS_File 12, with an empty path) and jumps to
&8008, into Fire's AIF header.

Fire (208K) is encrypted in layers: the header's bl leads to a loop that
XORs each word with the next, which decrypts the next loop (NOTs), and so
on (XOR, NOT, ADD chains), each over the rest of the program; the last
one ends at &81BC, the program's start-up (OS_GetEnv, mov sp,r1). The
program is Graftgold's 68000 game system in ARM: routines are called with
stmdb sp!,{pc} ; b routine ; nop and return with ldmia sp!,{pc}, objects
run small bytecode programs through a 243-entry handler table (&AA94),
and data files are packed ("CP" header).

At &32120-&35DF8 the decrypted program still holds an encrypted part:
Gordian Lock's disc check, compiled C that decrypts itself in place as it
runs. After loading data.M.INFO the game calls it through an address
computed from its workspace (&97EC: mov pc,r11,lsr #1). It makes its SWIs
by jumping straight to the kernel's SWI handler, where RISC OS 2 and 3.1
have it (&1F033C0), with the SWI number stored before the return address,
so that a SWI vector claimant never sees them; it measures time, turns off
Escape and the keyboard (OS_Byte 200, 201) and reads the disc's last track
(ADFS_DiscOp 3, read track, at &C5800). Its result is the address it
returns to: the caller's (&97F4) on the original disc, otherwise &12D80,
which sets bit 14 of the game's flags (workspace +&144) first. A disc image
can't hold that track: game.c takes a return to &12D80 as the original
disc's, to the caller's address. The check is also moved before it
runs: a stub at &3206C copies it to &32124 plus 0-&7C bytes, the offset
taken from OS_Mouse's time stamp, so where its variables end up depends
on timing. game.c takes the caller's address from where the stub keeps it
(&3211C), which doesn't move.

The game sets MODE 9, keeps two 40K screen banks and an 80K buffer from
ScreenStart (so it needs FreeMem's 160K), mixes its music and effects in
its own sound channel handler (Sound_Configure, &1D124) and takes its
timing from the vsync and IOC timer 1 interrupts (OS_ClaimDeviceVector 3
and 6).

What's translated (recomp/Makefile, make regen): Fire as tools/unpack
leaves it at &81C0 (gen/, entered at &81BC; the check's range runs
interpreted) and Intro (genintro/, at &88000). game.c loads the decrypted
image in place of the encrypted one when Intro jumps to &8008.

Tested (headless, --trace-gaps): Intro, the Cool Coyote and title
pictures, the title, the attract mode (the demos of the first two worlds,
the high score table), the map, the first level (walking, jumping,
firing, losing lives), game over. analysis/fire.roots holds the gap
entries those runs logged. Later worlds, the end sequence and the high
score entry weren't reached: untranslated code there runs interpreted with
--trace-gaps (logged; make roots, make regen), and otherwise stops the run.

What SuperSpider and libroger needed
------------------------------------

SuperSpider:

- OS_GetEnv in an entry point gives R1 as the entry SP: the start-up's
  mov sp,r1 then sets up a stack the analysis tracks, and everything the
  program pushes there (Graftgold's stmdb sp!,{pc} calls) is followed.
  Before, SP became unknown at &81C0 and the analysis found 69 words.
- A jump table's entry is a target only if the word there isn't below
  &10000 (as already for ranges): the table at &BFE0 runs on into the
  literal pool, whose next word, &BD48, is another table's base.

libroger:

- OS_ReadPalette (Intro's fades).
- OS_File's path lookups (12-15): an empty path is the name itself, and a
  full name (<Var>., a directory prefix, a filing system, $ & % @) isn't
  looked up along the path. Fire passes R4 pointing into its workspace.
- The kernel's SWI handler, where the branch at &8 leads, is at
  &1F033C0, where RISC OS 2 and 3.1 keep their copy of it, rather than an
  address of libroger's own: Gordian Lock jumps there to make its SWIs.

Not supported: the original disc's check is passed by game.c, not run
against a disc. PromptDisc and FreeMem aren't run (nothing in them is
needed).
