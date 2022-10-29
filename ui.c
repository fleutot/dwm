#include <X11/Xatom.h>
#include <unistd.h>

#include "bar.h"
#include "dwm.h"
#include "config.h"
#include "input.h"
#include "ui.h"
#include "util.h"

//******************************************************************************
// Prototypes
//******************************************************************************
static struct Monitor *dirtomon(int dir);

//******************************************************************************
// Definitions
//******************************************************************************
void
focusmon(const Arg *arg)
{
	struct Monitor *m;

#if 0
	// Unnecessary, dirtomon takes care of size 1. and 0 To be verified
	if (mons.size <= 1)
		return;
#endif
	m = dirtomon(arg->i);
	if (m == selmon)
		return;
	unfocus(mon_selected_client_get(selmon), 0);
	selmon = m;
	focus(NULL);
}

void
focusstack(const Arg *arg)
{
	Client *c = NULL;

	if (mon_selected_client_get(selmon) == NULL)
		return;

	if (arg->i > 0) {
		c = list_next_select(&selmon->tagview->clients);
		if (c == NULL) {
			c = list_head_select(&selmon->tagview->clients);
		}
	} else {
		c = list_prev_select(&selmon->tagview->clients);
		if (c == NULL) {
			c = list_tail_select(&selmon->tagview->clients);
		}
	}
	if (c) {
		focus(c);
		restack(selmon);
	}
}

void
incnmaster(const Arg *arg)
{
	selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

void
killclient(const Arg *arg)
{
	struct Client *c = mon_selected_client_get(selmon);
	if (c == NULL) {
		return;
	}

	if (!sendevent(mon_selected_client_get(selmon), wmatom[WMDelete])) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, c->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void
movemouse(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	struct Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	c = mon_selected_client_get(selmon);

	if (c == NULL)
		return;
	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			 None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			xeventhandler(&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < snap)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + width(c))) < snap)
				nx = selmon->wx + selmon->ww - width(c);
			if (abs(selmon->wy - ny) < snap)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + height(c))) < snap)
				ny = selmon->wy + selmon->wh - height(c);
			if (!c->isfloating && selmon->tagview->arrange
			    && (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
				togglefloating(NULL);
			if (!selmon->tagview->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void
quit(const Arg *arg)
{
	dwm_quit();
}

void
resizemouse(const Arg *arg)
{
	int ocx, ocy, nw, nh;
	struct Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	c = mon_selected_client_get(selmon);

	if (c == NULL)
		return;
	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			 None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			xeventhandler(&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			    && c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh) {
				if (!c->isfloating && selmon->tagview->arrange
				    && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!selmon->tagview->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void
setlayout(const Arg *arg)
{
	// Not implemented. Below is the origin dwm, kind of:
#if 0
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->tagview->active_client)
		arrange(selmon);
	else
		bar_draw(selmon);
#endif
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
	float f;

	if (!arg || !selmon->tagview->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;
	selmon->mfact = f;
	arrange(selmon);
}

void
spawn(const Arg *arg)
{
	if (fork() == 0) {
		/* In the child process */
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}

void
tag(const Arg *arg)
{
#if 0
// The whole tag thing is from dwm, now obsolete. Implement with
// tagview instead! If there really is a need for a client to have
// more than one tag, just add a pointer in the client list of that
// tag, pointing to the same Client. It might make sense to lose the
// owneship of clients from tagviews, and have a list of clients at
// the top-level. Tagview would then reference Clients from that
// list. Having a top-level reference to the active_client could maybe
// be useful, although tagviews still need to remember which client of
// theirs is the active one, so they can focus it when they themselves
// come in focus.
	if (selmon->tagview->active_client && arg->ui & tag_mask) {
		selmon->tagview->active_client->tags = arg->ui & tag_mask;
		focus(NULL);
		arrange(selmon);
	}
#endif
}

// Purpose: move the selected client to next monitor. From dwm, now
// obsolete. Functionality covered by sending client to tagview. Possible future
// implementation TODO: send active client to other monitor, effectively putting
// it in the current tagview of that monitor.
void
tagmon(const Arg *arg)
{
#if 0
	if (!selmon->tagview->active_client || !mons->next)
		return;
	sendmon(selmon->tagview->active_client, dirtomon(arg->i));
#endif
}

void
togglebar(const Arg *arg)
{
#if BAR
	// Legacy from dwm
	selmon->showbar = !selmon->showbar;
	updatebarpos(selmon);
	XMoveResizeWindow(dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, bh);
	arrange(selmon);
#endif
}

void
togglefloating(const Arg *arg)
{
#if 0
	// From dwm , now obsolete. There isn't going to be a floating layout,
	// floating windows will be in an overlay layer, atop other non-floating
	// windows.
	if (!selmon->tagview->active_client)
		return;
	if (selmon->tagview->active_client->isfullscreen) {
		/* no support for fullscreen windows */
		return;
	}
	selmon->tagview->active_client->isfloating =
		!selmon->tagview->active_client->isfloating
		|| selmon->tagview->active_client->isfixed;
	if (selmon->tagview->active_client->isfloating) {
		resize(
			selmon->tagview->active_client,
			selmon->tagview->active_client->x,
			selmon->tagview->active_client->y,
			selmon->tagview->active_client->w,
			selmon->tagview->active_client->h,
			0);
	}
	arrange(selmon);
#endif
}

void
toggletag(const Arg *arg)
{
#if 0
	// From dwm, now obsolete. Toggling tags with... the mouse??
	unsigned int newtags;

	if (!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & tag_mask);
	if (newtags) {
		selmon->sel->tags = newtags;
		focus(NULL);
		arrange(selmon);
	}
#endif
}

void
toggleview(const Arg *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & tag_mask);

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focus(NULL);
		arrange(selmon);
	}
}

void
view(const Arg *arg)
{
	if ((arg->ui & tag_mask) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & tag_mask)
		selmon->tagset[selmon->seltags] = arg->ui & tag_mask;
	focus(NULL);
	arrange(selmon);
}

/// Zoom is "send to master"?
void
zoom(const Arg *arg)
{
	struct Client *c = mon_selected_client_get(selmon);

	if (c == NULL) {
		return;
	}

	if ((selmon->tagview->arrange == NULL)
	    || c->isfloating) {
		return;
	}

	pop(c);
}

//******************************************************************************
// Internal functions
//******************************************************************************
static struct Monitor *
dirtomon(int dir)
{
	if (dir > 0) {
		return list_next_select(&mons);
	} else {
		return list_prev_select(&mons);
	}
}
