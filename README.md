# ps1_memcard_lib

PlayStation memory card handling in plain C: read a card image, list its saves (names, product codes,
regions, titles, 16x16 icons), copy saves between cards as .mcs, delete them, make blank cards, and keep a
library of card backups with versions. It's the engine of the memory card manager in
[SS1 Tool](https://github.com/f3bandit/ss1_tool) and [Phosphor](https://github.com/f3bandit/phosphor_os),
taken out so other frontends can use it.

The memory card manager is f3bandit's idea and design.

**Licence:** PolyForm Noncommercial 1.0.0 (LICENSE.md): no commercial use. **Retro Remake and Taki Udon**
get their own permission on top (LICENSE-RETRO-REMAKE.md): they can use it in Console Mode or any product of
theirs, closed source and commercial included; the only condition is crediting f3bandit.

- C11, nothing but the C library and POSIX (`dirent.h`, `sys/stat.h`); headers work from C++.
- Builds for the MiSTer's ARM (armhf) as is.
- Cards are raw 128 KB images: MiSTer's `.mcd` / `.sav`, ePSXe's `.mcr`. Saves are `.mcs` (one 128-byte
  directory frame, then the save's 8 KB blocks).

## Files

| File | What it does |
|---|---|
| `src/mcformat.c/.h` | The card format: check, blank, parse, icon, export, import, remove. Works on a 128 KB buffer in memory; no files, no allocation. |
| `src/mclib.c/.h` | The backup library: one folder per card, one `.mcd` per version (`<dir>/<card>/<YYYY-MM-DD_HHMMSS>.mcd`), cards recognised by the saves on them. |
| `examples/mcinfo.c` | Lists a card's saves and exports one. |
| `tests/selftest.c` | Import, list, export, remove, and the library round trip. |

```
make test        # builds and runs the self-test
make mcinfo      # ./mcinfo card.mcd   or   ./mcinfo card.mcd <block> out.mcs
```

## Using it

```c
#include "mcformat.h"

static unsigned char img[MC_SIZE];          /* the card, 128 KB */
mc_card_t c;
mc_parse(img, &c);                           /* c.formatted, c.free, c.saves[0..c.nsaves-1] */

/* copy a save from card a to card b */
static unsigned char mcs[128 + 15 * MC_BLOCK];
size_t n; const char *err;
if (mc_export(a, c.saves[i].slot, mcs, &n, &err) == 0 && mc_import(b, mcs, n, &err) == 0) { /* b changed */ }

mc_remove(img, slot, &err);                  /* delete a save */
mc_icon(img, slot, rgba);                    /* its icon, 16x16 RGBA */
mc_blank(img);                               /* a freshly formatted card */
```

Every function that changes a card works on the buffer only; writing it back (to a file, or to the real
card) is up to you. Errors come back as short English sentences in `err`.

```c
#include "mclib.h"

mclib_set_dir("/media/fat/memcards");        /* once, at start-up */
char card[128]; double m = mclib_recognise(img, card, sizeof card);   /* >= 0.5: it's that card */
char rel[256]; int same;
mclib_store(m >= 0.5 ? card : "Memory card 1", img, rel, sizeof rel, &same);   /* a new version */
```

## Reading and writing a real card on the SuperStation One

The card in the SS1's front ports is read and written by the
[SS1 Memory Card Tool core](https://github.com/f3bandit/SS1_MemCard_MiSTer) (GPL-3.0; a separate
program, launched, not linked). Load it with an MGL that mounts a 128 KB card file (drive 0) and a
1536-byte job file (drive 1):

```xml
<mistergamedescription>
	<rbf>_Utility/SS1_MemCard</rbf>
	<file delay="2" type="s" index="1" path="/media/fat/games/SS1_MemCard/job.bin"/>
	<file delay="3" type="s" index="0" path="/media/fat/games/SS1_MemCard/card.mcd"/>
</mistergamedescription>
```

then `echo "load_core <that .mgl>" > /dev/MiSTer_cmd`. The job file's first bytes ask for the job:
`SMCQ`, then at offset 5 the job (1 back up the card to the file, 2 check the card against the file,
3 write the file to the card), a flags byte and a 4-byte id at 8. The core answers in the same file:
`SMCP` every 0.25 s while it works, `SMCR` when it's done, with the result at offset 6 (1 OK, 2 OK
with retries, 3 sectors failed, 4 no card, 9 card written, ...) and a status byte per sector from 512.
The full layout is in the core's README ("How SS1 Tool drives it"), and SS1 Tool's `memcard.go` is a
working client: back up first, change the image with this library, then write; the core only writes
the sectors that differ and reads each one back.
