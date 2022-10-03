#include "bar.h"

//******************************************************************************
// Function definitions
//******************************************************************************

enum bar_zone
bar_x_to_zone(int x)
{
	enum bar_zone zone;
	unsigned int i, x;
	i = x = 0;
	do
		x += drw_fontset_getwidth(drw, tags[i]);
	while (ev->x >= x && ++i < LENGTH(tags));
	if (i < LENGTH(tags)) {
		zone = BAR_ZONE_TAGS;
		arg.ui = 1 << i;
	} else if (ev->x < x + blw) {
		zone = BAR_ZONE_LAYOUT;
	} else if (ev->x > selmon->ww - ((int) drw_fontset_getwidth(drw, stext) + lrpad)) {
		zone = BAR_ZONE_STATUS;
	} else {
		zone = BAR_ZONE_WIN_TITLE;
	}
}

//******************************************************************************
// Internal functions
//******************************************************************************
