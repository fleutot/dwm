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

void bar_draw(struct Monitor *m)
{
#if 0
	I don 't care for bars, so this doesn't get reimplemented.At least not yet.
	int x, w, tw = 0;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0;

	/* draw status first so it can be overdrawn by tags later */
	if (m == selmon) {                                 /* status is only drawn on selected monitor */
		drw_setscheme(drw, scheme[SchemeNorm]);
		tw = drw_fontset_getwidth(drw, stext) + 2; /* 2px right padding */
		drw_text(drw, m->ww - tw, 0, tw, bh, 0, stext, 0);
	}

	// TODO: what is occ? occurence? Occupancy? Like a bit set in occ means
	// there is at least one client with that tag?
	// This wil change, when moving to tagviews as a separate struct. A
	// tabview holds a list of  clients, and so knows the number of clients in
	// that tag.
	// For now, just hack into the new list.
	for (struct ll_node *n = m->clients.head; n; n = n->next) {
		Client *c = n->data;
		occ |= c->tags;
		if (c->isurgent)
			urg |= c->tags;
	}
	x = 0;
	for (i = 0; i < num_tags; i++) {
		w = drw_fontset_getwidth(drw, tags[i]) + lrpad;
		drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
		drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], urg & 1 << i);
		if (occ & 1 << i)
			drw_rect(drw, x + boxs, boxs, boxw, boxw,
				 m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
				 urg & 1 << i);
		x += w;
	}
	w = blw = drw_fontset_getwidth(drw, m->ltsymbol) + lrpad;
	drw_setscheme(drw, scheme[SchemeNorm]);
	x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);

	if ((w = m->ww - tw - x) > bh) {
		if (m->sel) {
			drw_setscheme(drw, scheme[m == selmon ? SchemeSel : SchemeNorm]);
			drw_text(drw, x, 0, w, bh, lrpad / 2, m->sel->name, 0);
			if (m->sel->isfloating)
				drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
		} else {
			drw_setscheme(drw, scheme[SchemeNorm]);
			drw_rect(drw, x, 0, w, bh, 1, 1);
		}
	}
	drw_map(drw, m->barwin, 0, 0, m->ww, bh);
#endif
}


//******************************************************************************
// Internal functions
//******************************************************************************
