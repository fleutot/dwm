#include <X11/Xlib.h>

#include "bar.h"
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

int area_in_mon(int x, int y, int w, int h, const Monitor *m)
{
	return MAX(0,
		   MIN(x + w, m->wx + m->ww) - MAX(x, m->wx))
	       * MAX(0,
		     MIN(y + h, m->wy + m->wh) - MAX(y, m->wy));
}

void
arrange(Monitor *m)
{
	if (m) {
		tagview_arrange(m);
		restack(m);
	} else {
		printf("%s with argument NULL\n", __func__);
	}
}

static void
configure_client_w_changes(void *client, void *storage)
{
	struct Client *c = (struct Client *)client;
	XWindowChanges *win_changes = (XWindowChanges *)storage;

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

void
restack(struct Monitor *m)
{
	Client *c;
	XEvent ev;
	XWindowChanges win_changes;

	printf("%s(%p)\n", __func__, (void *)m);

	bar_draw(m);
	c = tagview_selected_client_get(m->tagview);
	if (c == NULL)
		return;
	if (c->isfloating || (m->tagview->arrange == NULL))
		XRaiseWindow(dpy, c->win);
	if (m->tagview->arrange != NULL) {
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
createmon(struct tagview *with_tagview)
{
	Monitor *m;

	m = ecalloc(1, sizeof(Monitor));
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
