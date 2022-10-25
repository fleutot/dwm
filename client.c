#include <stdbool.h>
#include <X11/Xlib.h>

#include "client.h"
#include "config.h"
#include "dwm.h"
#include "util.h"

//******************************************************************************
// Function prototypes
//******************************************************************************
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);

//******************************************************************************
// Function definitions
//******************************************************************************
void
configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

bool
isvisible(const Client *c)
{
	return c->tags & c->mon->tagset[c->mon->seltags];
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
	if (applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
	XWindowChanges wc;

	c->oldx = c->x; c->x = wc.x = x;
	c->oldy = c->y; c->y = wc.y = y;
	c->oldw = c->w; c->w = wc.width = w;
	c->oldh = c->h; c->h = wc.height = h;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}


// Since all clients were owned by a monitor, this showed or hid the
// clients depending on the visibility, which was determined by tags
// for any client. The new way to do this is with tagviews, so this
// show hide is not necessary, we just need to show all clients in the
// tagview. Hiding is possibly going to be needed as well, or just
// hide everything (or hide all clients in the tagview we're leaving)
// and start showing the clients of the tagview.
void
showhide(Client *c)
{
	if (!c)
		return;
	if (isvisible(c)) {
		/* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if ((!c->mon->tagview->layout->arrange || c->isfloating)
		    && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->snext);
	} else {
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, width(c) * -2, c->y);
	}
}

int
width(const Client *c)
{
	return c->w + (2 * c->bw);
}

int
height(const Client *c)
{
	return c->h + (2 * c->bw);
}

//******************************************************************************
// Internal functions
//******************************************************************************
static int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > screen_w)
			*x = screen_w - width(c);
		if (*y > screen_h)
			*y = screen_h - height(c);
		if (*x + *w + 2 * c->bw < 0)
			*x = 0;
		if (*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if (*x >= m->wx + m->ww)
			*x = m->wx + m->ww - width(c);
		if (*y >= m->wy + m->wh)
			*y = m->wy + m->wh - height(c);
		if (*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if (*h < bar_h)
		*h = bar_h;
	if (*w < bar_h)
		*w = bar_h;
	if (resizehints || c->isfloating || !c->mon->tagview->arrange) {
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;
		if (c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw)
			*w = MIN(*w, c->maxw);
		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}
