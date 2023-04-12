#include <stdio.h>
#include <X11/Xatom.h>
#include <unistd.h>

#include "bar.h"
#include "debug.h"
#include "dwm.h"
#include "config.h"
#include "input.h"
#include "ui.h"
#include "util.h"

//******************************************************************************
// Prototypes
//******************************************************************************

//******************************************************************************
// Definitions
//******************************************************************************
void monitor_focus(const Arg *arg)
{
	int index = arg->i;
	struct Monitor *target_mon = list_data_handle_get(&mons, index);

	if (selmon == target_mon || target_mon == NULL) {
		return;
	}
	client_unfocus(mon_selected_client_get(selmon), false);
	selmon = target_mon;
	mon_arrange(target_mon);
	client_focus(NULL);
}

void
focusstack(const Arg *arg)
{
	Client *c = mon_selected_client_get(selmon);

	if (c == NULL)
		return;

	client_unfocus(c, false);

	if (arg->i > 0) {
		c = list_next_wrap_select(&selmon->tagview->clients);
		if (c == NULL) {
			c = list_head_select(&selmon->tagview->clients);
		}
	} else {
		c = list_prev_wrap_select(&selmon->tagview->clients);
		if (c == NULL) {
			c = list_tail_select(&selmon->tagview->clients);
		}
	}
	if (c) {
		client_focus(c);
		restack(selmon);
	}
}

void
incnmaster(const Arg *arg)
{
	selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
	mon_arrange(selmon);
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
		client_focus(NULL);
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
			/// Is this only  for snapping at monitor edges?
			//if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			//    && c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh) {
			//	if (!c->isfloating && selmon->tagview->arrange
			//	    && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
			//		togglefloating(NULL);
			//}
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
		client_focus(NULL);
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
		selmon->lt[selmon->sellt] = (Layout *) arg->v;
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
	/// TODO: this must go to the layout config of the current
	/// tagview instead. Maybe requires a common config format for
	/// all layouts?
	selmon->mfact = f;
	mon_arrange(selmon);
}

void
spawn(const Arg *arg)
{
	if (fork() == 0) {
		/* In the child process */
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **) arg->v)[0], (char **) arg->v);
		fprintf(stderr, "dwm: execvp %s", ((char **) arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}

void
tag_send(const Arg *arg)
{
	struct Client *c = tagview_selected_client_get(selmon->tagview);

	if (c == NULL) {
		return;
	}

	struct tagview *dst_tagview = tagview_get(arg->ui);

	tagview_add_client(dst_tagview, c);
	tagview_rm_client(selmon->tagview, c);
	client_unfocus(c, true);
	client_focus(NULL);
	mon_arrange(selmon);

	////// Somehow, this makes room for the client in the destination
	////// tagview/monitor, but it does not draw it there.
	struct Monitor *other_mon = list_find(
		&mons,
		mon_shows_tagview,
		dst_tagview);

	if (other_mon != NULL) {
		printf("Tagview is visible, arrange\n");
		mon_arrange(other_mon);
	}
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
		client_focus(NULL);
		mon_arrange(selmon);
	}
}


static bool monitor_shows_tagview_index(void *mon, void *index)
{
	const struct Monitor *m = (struct Monitor *) mon;
	int i = *(int *) index;

	return m->tagview->index == i;
}

void
tag_view(const Arg *arg)
{
	if (arg->ui == selmon->tagview->index) {
		return;
	}

	int tv_index = arg->ui;

	P_DEBUG("### %s: finding monitor to swap with\n", __func__);
	struct Monitor *other_m = list_find(
		&mons,
		monitor_shows_tagview_index,
		&tv_index);

	if (config.focus.tagview_change_ignores_mouse_over) {
		skip_mouse_over_focus_once = true;
	}

	if (other_m != NULL) {
		P_DEBUG("### %s: swapping tagview on other, tvother:%d, tvselmon:%d\n",
			__func__,
			other_m->tagview->index,
			selmon->tagview->index);
		// Must hide both, to avoid hiding a tagview that has
		// already moved to another monitor.
		tagview_hide(other_m->tagview);
		tagview_hide(selmon->tagview);

		struct tagview *selmon_new_tagview = other_m->tagview;
		tagview_show(selmon->tagview, other_m);
		tagview_show(selmon_new_tagview, selmon);
		client_focus(NULL);
	} else {
		P_DEBUG("### %s: getting tagview %d on selmon\n", __func__, tv_index);
		mon_tag_switch(selmon, tagview_get(tv_index));
	}
}

void to_master_send(const Arg *arg)
{
	struct Client *c = mon_selected_client_get(selmon);

	if (c == NULL) {
		return;
	}

	if ((selmon->tagview->arrange == NULL)
	    || c->isfloating) {
		return;
	}

	list_data_swap(
		&selmon->tagview->clients,
		c,
		selmon->tagview->clients.head->data);
	client_focus(c);
	mon_arrange(selmon);
}

//******************************************************************************
// Internal functions
//******************************************************************************
