/* mcinfo: lists the saves on a PlayStation memory card image, and exports one as .mcs.
 *   mcinfo card.mcd                 list
 *   mcinfo card.mcd <slot> out.mcs  export the save that starts in block <slot>
 * Copyright (C) 2026 f3bandit. GPL-3.0-or-later (see LICENSE); Retro Remake: see LICENSE-RETRO-REMAKE.md. */
#include "mcformat.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc < 2) { fprintf(stderr, "usage: mcinfo card.mcd [slot out.mcs]\n"); return 2; }
	static unsigned char img[MC_SIZE];
	FILE *f = fopen(argv[1], "rb");
	if (!f || fread(img, 1, MC_SIZE, f) != MC_SIZE) { fprintf(stderr, "%s: not a 128 KB card image\n", argv[1]); return 1; }
	fclose(f);
	mc_card_t c;
	if (mc_parse(img, &c) != 0 || !c.formatted) { printf("not formatted\n"); return 1; }
	printf("%d saves, %d blocks free\n", c.nsaves, c.free);
	for (int i = 0; i < c.nsaves; i++)
		printf("  block %2d  %d block(s)  %-10s %-6s %s%s\n", c.saves[i].slot, c.saves[i].nblocks, c.saves[i].product,
		       c.saves[i].region, c.saves[i].title, c.saves[i].broken ? "  (broken)" : "");
	if (argc == 4) {
		static unsigned char out[128 + 15 * MC_BLOCK];
		size_t n = 0;
		const char *err = "";
		if (mc_export(img, atoi(argv[2]), out, &n, &err) != 0) { fprintf(stderr, "export: %s\n", err); return 1; }
		FILE *o = fopen(argv[3], "wb");
		if (!o || fwrite(out, 1, n, o) != n) { fprintf(stderr, "%s: can't write\n", argv[3]); return 1; }
		fclose(o);
		printf("exported %zu bytes to %s\n", n, argv[3]);
	}
	return 0;
}
