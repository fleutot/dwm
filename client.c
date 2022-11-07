#include <assert.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "bar.h"
#include "client.h"
#include "config.h"
#include "dm.h"
#include "dwm.h"
#include "util.h"

//******************************************************************************
// Global variables
//******************************************************************************
static const char broken[] = "broken";

//******************************************************************************
// Function prototypes
//******************************************************************************
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void grab_buttons(struct Client *c, bool focused);
static void apply_rules(Client *c);
static void attach_above(Client *c);
static Atom atom_prop_get(Client *c, Atom prop);

//******************************************************************************
// Function definitions
//******************************************************************************
void client_create(Window w, XWindowAttributes *wa)
{
	printf("%s======================================\n", __func__);
	Client *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	c = ecalloc(1, sizeof(Client));
	c->win = w;
	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	client_name_update(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		apply_rules(c);
	}

	if (c->x + width(c) > c->mon->mx + c->mon->mw)
		c->x = c->mon->mx + c->mon->mw - width(c);
	if (c->y + height(c) > c->mon->my + c->mon->mh)
		c->y = c->mon->my + c->mon->mh - height(c);
	c->x = MAX(c->x, c->mon->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
			  && (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bar_h : c->mon->my);
	c->bw = borderpx;

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	client_update_window_type(c);
	client_update_size_hints(c);
	client_update_wm_hints(c);
	XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	grab_buttons(c, 0);
	if (!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if (c->isfloating)
		XRaiseWindow(dpy, c->win);
	attach_above(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
			(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * screen_w, c->y, c->w, c->h); /* some windows require this */
	client_state_set(c, NormalState);
	if (c->mon == selmon)
		client_unfocus(mon_selected_client_get(selmon), 0);
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	client_focus(c);
}

void client_hide(void *client, void *storage)
{
	struct Client *c = (struct Client *) client;

	XUnmapWindow(dpy, c->win);
}

void client_show(void *client, void *storage)
{
	struct Client *c = (struct Client *) client;

	XMapWindow(dpy, c->win);
}

void client_name_update(Client *c)
{
	if (!dm_gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		dm_gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void client_state_set(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *) data, 2);
}

void client_update_window_type(Client *c)
{
	Atom state = atom_prop_get(c, netatom[NetWMState]);
	Atom wtype = atom_prop_get(c, netatom[NetWMWindowType]);

	if (state == netatom[NetWMFullscreen])
		client_fullscreen_set(c, 1);
	if (wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

void client_focus(struct Client *c)
{
	if (c == NULL) {
		c = tagview_selected_client_get(selmon->tagview);
		if (c == NULL) {
			return;
		}
	}
	selmon = c->mon;
	if (c->isurgent) {
		/// In original dwm, the logic was the opposite. Are
		/// we supposed to *unset* urgency here?
		client_urgent_set(c, true);
	}

	grab_buttons(c, true);
	XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
	dm_focus(c);
	tagview_selected_client_set(selmon->tagview, c);
	bar_draw_all_mons(&mons);
}

void client_unfocus(struct Client *c, bool focus_root)
{
	if (c == NULL) {
		return;
	}

	grab_buttons(c, false);
	XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
	if (focus_root) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void client_urgent_set(struct Client *c, bool is_urgent)
{
	XWMHints *wmh;

	c->isurgent = is_urgent;
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags = is_urgent
		? (wmh->flags | XUrgencyHint)
		: (wmh->flags & ~XUrgencyHint);

	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void client_update_size_hints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else {
		c->basew = c->baseh = 0;
	}
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else {
		c->incw = c->inch = 0;
	}
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else {
		c->maxw = c->maxh = 0;
	}
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else {
		c->minw = c->minh = 0;
	}
	if (size.flags & PAspect) {
		c->mina = (float) size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float) size.max_aspect.x / size.max_aspect.y;
	} else {
		c->maxa = c->mina = 0.0;
	}
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
}

void client_update_wm_hints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == mon_selected_client_get(selmon) && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else {
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
		}
		if (wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = 0;
		XFree(wmh);
	}
}

void client_fullscreen_set(Client *c, bool fullscreen)
{
	if (fullscreen && !c->isfullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
				PropModeReplace, (unsigned char *) &netatom[NetWMFullscreen], 1);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (!fullscreen && c->isfullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
				PropModeReplace, (unsigned char *) 0, 0);
		c->isfullscreen = 0;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}

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
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *) &ce);
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
		if ((!c->mon->tagview->arrange || c->isfloating)
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
			if (c->maxa < (float) *w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float) *h / *w)
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

static void grab_buttons(struct Client *c, bool focused)
{
	updatenumlockmask();

	unsigned int i, j;
	unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask | LockMask };
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	if (!focused)
		XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
			    BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
	for (i = 0; i < num_buttons; i++)
		if (buttons[i].click == ClkClientWin) {
			for (j = 0; j < LENGTH(modifiers); j++)
				XGrabButton(dpy, buttons[i].button,
					    buttons[i].mask | modifiers[j],
					    c->win, False, BUTTONMASK,
					    GrabModeAsync, GrabModeSync, None, None);
		}
}

static void apply_rules(Client *c)
{
#if 0
	Implement rules later
	const char *class, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = 0;
	c->tags = 0;
	XGetClassHint(dpy, c->win, &ch);
	class = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;

	for (i = 0; i < num_rules; i++) {
		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title))
		    && (!r->class || strstr(class, r->class))
		    && (!r->instance || strstr(instance, r->instance))) {
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			// Find the monitor in the rule and put the client there.
			for (m = mons; m && m->num != r->monitor; m = m->next);
			if (m)
				c->mon = m;
		}
	}
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
#endif
}

static void attach_above(Client *c)
{
	printf("%s(%p)\n", __func__, (void *) c);

	Client *selected_client = tagview_selected_client_get(c->mon->tagview);

	if (selected_client == NULL || selected_client->isfloating) {
		tagview_prepend_client(c->mon->tagview, c);
	} else {
		tagview_add_client(c->mon->tagview, c);
	}
}

/// TODO: this should move to dm_*.c, and use Win instead of Client as input parameter
static Atom atom_prop_get(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
			       &da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *) p;
		XFree(p);
	}
	return atom;
}
