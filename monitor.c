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
	if (m)
		showhide(m->stack);
	else for (m = mons; m; m = m->next)
			showhide(m->stack);
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
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
	return m;
}

void
drawbar(Monitor *m)
{
	int x, w, tw = 0;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0;
	Client *c;

	/* draw status first so it can be overdrawn by tags later */
	if (m == selmon) {                                 /* status is only drawn on selected monitor */
		drw_setscheme(drw, scheme[SchemeNorm]);
		tw = drw_fontset_getwidth(drw, stext) + 2; /* 2px right padding */
		drw_text(drw, m->ww - tw, 0, tw, bh, 0, stext, 0);
	}

	for (c = m->clients; c; c = c->next) {
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
}

Client *
nexttiled(Client *c)
{
	for (; c && (c->isfloating || !isvisible(c)); c = c->next);
	return c;
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
	unsigned int i, n, h, mw, my, ty;
	Client *c;

	for (n = 0, c = nexttiled(m->clients);
	     c != NULL;
	     c = nexttiled(c->next), n++)
		;

	if (n == 0)
		return;

	if (n > m->nmaster)
		mw = m->nmaster ? m->ww * m->mfact : 0;
	else
		mw = m->ww;

	for (i = my = ty = 0, c = nexttiled(m->clients);
	     c != NULL;
	     c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i);
			resize(c, m->wx, m->wy + my, mw - (2 * c->bw),
			       h - (2 * c->bw), 0);
			if (my + height(c) < m->wh)
				my += height(c);
		} else {
			h = (m->wh - ty) / (n - i);
			resize(c, m->wx + mw, m->wy + ty, m->ww - mw - (2 * c->bw), h - (2 * c->bw), 0);
			if (ty + height(c) < m->wh)
				ty += height(c);
		}
}

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (isvisible(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
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
arrangemon(Monitor *m)
{
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if (m->lt[m->sellt]->arrange)
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
