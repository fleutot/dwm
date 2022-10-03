#include <X11/Xlib.h>

#include "drw.h"
#include "dwm.h"
#include "client.h"
#include "config.h"
#include "monitor.h"
#include "util.h"

//******************************************************************************
// Module constants
//******************************************************************************

//******************************************************************************
// Module variables
//******************************************************************************

//******************************************************************************
// Function prototypes
//******************************************************************************
static void arrangemon(Monitor *m);
static int area_in_mon(int x, int y, int w, int h, const Monitor *m);

//******************************************************************************
// Function definitions
//******************************************************************************
void
arrange(Monitor *m)
{
	if (m) {
		arrangemon(m);
		restack(m);
	} else {
		for (m = mons; m; m = m->next)
			arrangemon(m);
	}
}

Monitor *
createmon(void)
{
	Monitor *m;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->showbar = showbar;
	m->topbar = topbar;
	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % num_layouts];
	m->tagview = NULL;
	//m->clients = LINKEDLIST_EMPTY;
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
	return m;
}

void
drawbar(Monitor *m)
{
#if 0
	I don't care for bars, so this doesn't get reimplemented. At least not yet.
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
	// For now, just hack into the new linkedlist.
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

struct ll_node *
nexttiled(struct ll_node *n)
{
	Client *c = n->data;

	while (n && (c->isfloating || !isvisible(c))) {
		n = n->next;
		c = n->data;
	}
	return n;
}

Monitor *
recttomon(int x, int y, int w, int h)
{
	Monitor *m, *ret = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = area_in_mon(x, y, w, h, m)) > area) {
			area = a;
			ret = m;
		}
	return ret;
}

void
tile(Monitor *m)
{
	// Arranging windows on a monitor moved to the layouts. This
	// one to layout_two_cols.c.
}

void
monocle(Monitor *m)
{
#if 0
	This must move (and be fixed) to layout/layout_fullscreen.c, when it exists.
	unsigned int n = 0;
	struct ll_node *node;

	for (node = m->clients.head; node; node = node->next)
		if (isvisible(node->data))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (node = nexttiled(m->clients.head); node; node = nexttiled(node->next)) {
		Client *c = node->data;
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
	}
#endif
}

void
updatebarpos(Monitor *m)
{
	m->wy = m->my;
	m->wh = m->mh;
	if (m->showbar) {
		m->wh -= bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh;
		m->wy = m->topbar ? m->wy + bh : m->wy;
	} else {
		m->by = -bh;
	}
}

//******************************************************************************
// Internal functions
//******************************************************************************
static void
arrangemon(struct Monitor *m)
{
    tagview_arrange(m);
    if (m->tagview->arrange)
		m->lt[m->sellt]->arrange(m);
}

static int area_in_mon(int x, int y, int w, int h, const Monitor *m)
{
	return
		MAX(
		0,
		MIN(x + w, m->wx + m->ww) - MAX(x, m->wx)
		)
		* MAX(
		0,
		MIN(y + h, m->wy + m->wh) - MAX(y, m->wy)
		);
}
