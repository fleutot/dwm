#include <X11/Xlib.h>

#include "bar.h"
#include "debug.h"
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

//******************************************************************************
// Function definitions
//******************************************************************************
int mon_n_clients_get(const struct Monitor *m)
{
	return m->tagview->clients.size;
}

struct Client *mon_selected_client_get(const struct Monitor *m)
{
	return tagview_selected_client_get(m->tagview);
}

void mon_selected_client_set(struct Monitor *m, struct Client *c)
{
	list_select(&m->tagview->clients, c);
}

void mon_tag_switch(struct Monitor *m, struct tagview *tagview)
{
	if (m->tagview == tagview) {
		return;
	}
	tagview_hide(m->tagview);
	m->tagview = tagview;
	tagview_show(m->tagview, m);
}

int area_in_mon(int x, int y, int w, int h, const Monitor *m)
{
	return MAX(0,
		   MIN(x + w, m->wx + m->ww) - MAX(x, m->wx))
	       * MAX(0,
		     MIN(y + h, m->wy + m->wh) - MAX(y, m->wy));
}

void
mon_arrange(Monitor *m)
{
	P_DEBUG("%s: %p\n", __func__, (void *) m);
	if (m) {
		tagview_arrange(m);
		restack(m);
	} else {
		P_DEBUG("%s with argument NULL\n", __func__);
	}
}

void mon_arrange_cb(void *monitor, void *storage)
{
	mon_arrange((struct Monitor *) monitor);
}

bool mon_has_client(void *monitor, void *client)
{
	struct Monitor *m = (struct Monitor *) monitor;

	return tagview_has_client(m->tagview, (struct Client *) client);
}

bool mon_has_window(void *monitor, void *window)
{
	struct Monitor *m = (struct Monitor *) monitor;
	Window *w = (Window *) window;

	return tagview_find_window_client(m->tagview, w) != NULL;
}

bool mon_shows_tagview(void *monitor, void *tagview)
{
	struct Monitor *m = (struct Monitor *) monitor;

	return m->tagview == (struct tagview *) tagview;
}

static void
configure_client_w_changes(void *client, void *storage)
{
	struct Client *c = (struct Client *) client;
	XWindowChanges *win_changes = (XWindowChanges *) storage;

	if (c->isfloating) {
		return;
	}

	XConfigureWindow(
		dpy,
		c->win,
		CWSibling | CWStackMode,
		win_changes);

	win_changes->sibling = c->win;
}


/// What does this do? Is this stack for selection history, go back to
/// previous selection? Or is stack something else in this context? Z
/// order?
void
restack(struct Monitor *m)
{
	Client *c;
	XEvent ev;

	P_DEBUG("%s(%p)\n", __func__, (void *) m);

	bar_draw(m);
	c = tagview_selected_client_get(m->tagview);
	if (c == NULL)
		return;
	if (c->isfloating || (m->tagview->arrange == NULL)) {
		/// Should raise tiled windows as well, for the sake
		/// of painting shadows?
		XRaiseWindow(dpy, c->win);
	}
	if (m->tagview->arrange != NULL) {
		XWindowChanges win_changes;
		win_changes.stack_mode = Below;
		win_changes.sibling = m->barwin;
		list_run_for_all(
			&m->tagview->clients,
			configure_client_w_changes,
			&win_changes);
	}
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

Monitor *
createmon(struct tagview *with_tagview, int x, int y, int w, int h)
{
	Monitor *m;

	m = ecalloc(1, sizeof(Monitor));

	// Not supporting bars yet, so window area is the same as
	// monitor area
	m->wx = m->mx = x;
	m->wy = m->my = y;
	m->ww = m->mw = w;
	m->wh = m->mh = h;

	m->showbar = showbar;
	m->topbar = topbar;
	m->tagview = with_tagview;
	return m;
}

void monitor_destruct(struct Monitor *m)
{
#if BAR
	XUnmapWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->barwin);
#endif
	free(m);
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
	/// This must move (and be fixed) to layout/layout_fullscreen.c, when it exists.
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
		m->wh -= bar_h;
		m->by = m->topbar ? m->wy : m->wy + m->wh;
		m->wy = m->topbar ? m->wy + bar_h : m->wy;
	} else {
		m->by = -bar_h;
	}
}

//******************************************************************************
// Internal functions
//******************************************************************************
