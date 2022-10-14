#ifndef BAR_H
#define BAR_H

#include "monitor.h"

enum bar_zone {
	BAR_ZONE_NONE,
	BAR_ZONE_TAGS,
	BAR_ZONE_LAYOUT,
	BAR_ZONE_WIN_TITLE,
	BAR_ZONE_STATUS
};

enum bar_zone bar_x_to_zone(int x);

// Not finished. Use like this, probably:
// click = unsigned int[] {
//	[BAR_ZONE_TAGS] = ClkTagBar,
//	[BAR_ZONE_LAYOUT] = ClkLtSymbol,
//	[BAR_ZONE_WIN_TITLE] = ClkWinTitle,
//	[BAR_ZONE_STATUS] = ClkStatusText
// } [bar_x_to_zone(ev->x)];


void bar_draw(struct Monitor *m);

#endif
