/* Fire & Ice (Graftgold for Renegade, 1993; Archimedes conversion by Paul
 * Chapman, 1995): what libroger needs to know about the game.
 *
 * !Run sets Fire_Ice$Dir, runs FreeMem (a utility that sets the screen
 * memory to 160K and the other dynamic areas to nothing: as screen_size
 * below, so it isn't run here), then, with a 560K WimpSlot, PromptDisc
 * (Gordian Lock's disc prompt) with the command "*Run <Fire_Ice$Dir>.Intro",
 * which needs nothing from it either. The program is Intro (an absolute at
 * &88000, genintro/), which shows Graftgold's logo, loads Fire at &8000 and
 * jumps to &8008.
 *
 * Fire is an AIF image encrypted in layers (XOR, NOT and ADD chains, each
 * decrypting the next and the program after it). ../tools/unpack runs it
 * under libroger's interpreter to its start-up at &81C0 and saves the
 * image; that is what's translated (gen/). When Intro jumps into Fire, the
 * decrypted image is put in its place and entered at &81BC (its OS_GetEnv),
 * as the layers leave it.
 *
 * The game reads its files as <Fire_Ice$Dir>.data.<dir>.<name>; the two
 * discs' !Fire_Ice are merged into one (game/), so it never asks for disc 2. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roger.h>

extern const struct roger_image recomp_image, recompI_image;   /* gen{,intro}/entries.c */

#define FIRE "../unpacked/fire"
static const char *const vars[] = {"Fire_Ice$Dir", "!Fire_Ice", NULL};
static const char *const prefixes[] = {NULL};

/* Fire's disc check (Gordian Lock), at &32120-&35DF8, is still encrypted
 * after the layers: it decrypts itself in place as it runs, so it is
 * interpreted. When the game has loaded data.M.INFO, &97EC calls it
 * through an obfuscated address; it makes its SWIs by jumping straight to
 * the kernel's SWI handler (where RISC OS 3.1 has it, &1F033C0), and reads
 * the last track of the disc (ADFS_DiscOp 3, read track, at &C5800). Its
 * result is the address it returns to: on the original disc, its caller's
 * (&97F4); otherwise &12D80, which sets bit 14 of the game's flags
 * (workspace +&144, tested in many places) and returns there. A disc
 * image can't hold that track, so a return to &12D80 is taken as the
 * original disc's: to the caller's address. Nothing else goes to &12D80.
 * The check doesn't stay where it is: its stub (&3206C) copies it to
 * &32124 plus a random offset (0-&7C: OS_Mouse's time stamp, AND &7C)
 * and enters it there, so the copy's own variables move with it; the
 * caller's address is the one the stub keeps, at &3211C. */
#define PROTECTION_FAILED 0x12D80
#define PROTECTION_RETURN 0x3211C
static const uint32_t interpreted[] = {0x32120, 0x35DF8, 0, 0};

/* Intro's jump to &8008 (Fire's AIF header: its decryption from there),
 * and the disc check's failure */
static int game_call(uint32_t a)
{
    if (a == PROTECTION_FAILED && roger_current_image() == &recomp_image) {
        npc = RD32(PROTECTION_RETURN) & 0x3FFFFFC;
        return 1;
    }
    if (a != 0x8008 || roger_current_image() != &recompI_image) return 0;
    if (roger_load_file(FIRE, ROGER_LOAD) <= 0) { perror(FIRE); roger_finish("no " FIRE ": make regen unpacks it"); }
    roger_set_image(&recomp_image);
    R[14] = 0x800C + 4;                 /* the header's bl to the decryption: the program exits there */
    npc = 0x81BC;
    return 1;
}

/* The headless scenario (--limit runs; instruction counts, 2000 a ms):
 * Return (fire) on the title, again on the map, then Cool Coyote walks
 * right (X) through the first level, jumping (') and firing (Space) */
#define K(n, ms, len) {n, (ms) * 2000ull, ((ms) + (len)) * 2000ull}
static const struct roger_key keys[] = {
    K(74, 12000, 300),                  /* Return: start */
    K(74, 36000, 300),                  /* Return: the map */
    K(67, 40000, 3000), K(99, 44000, 300), K(80, 45000, 500), K(67, 46000, 4000),
    K(99, 50000, 300), K(80, 52000, 500), K(67, 53000, 5000), K(99, 58000, 300),
};

static struct roger_config config = {
    .title = "Fire & Ice (recompiled)",
    /* the keys (read at &8F30): Z or comma left, X or full stop right, '
     * or Q up, / or A down, Return, Space (or Shift, Ctrl or Alt alone)
     * fire; F1-F4 are read too. The game also asks for Acorn's joystick
     * module (Joystick_Read, &43F40), which a controller stands in for:
     * the stick or D-pad moves, A fires; with --joystick=off, the keys */
    .pad_map = "left=Z,right=X,up=',down=/,a=Return,b=Space,start=Return",
    .joystick = ROGER_JOY_ACORN,
    .image_path = "../game/!Fire_Ice/Intro,00088000-00088000",
    .image = &recompI_image,
    .appdir = "../game",
    .dir_prefixes = prefixes,
    .variables = vars,
    .directory = "<Fire_Ice$Dir>",
    .command_line = "<Fire_Ice$Dir>.Intro",
    .initial_mode = 12,                 /* the desktop's: Intro sets its own */
    .wimpslot = 560 * 1024,             /* !Run's *WimpSlot 560k */
    .screen_size = 160 * 1024,          /* as FreeMem sets it: the game uses 160K from ScreenStart */
    .interpreted = interpreted,
    .call_addr = game_call,
    .keys = keys,
    .nkeys = sizeof keys / sizeof keys[0],
};

int main(int argc, char **argv) { roger_main(argc, argv, &config); }
