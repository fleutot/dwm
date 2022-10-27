/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "dwm.h"

#include "bar.h"
#include "client.h"
#include "drw.h"
#include "linkedlist/linkedlist.h"
#include "monitor.h"
#include "tagview.h"
#include "util.h"

/* macros */
#define CLEANMASK(mask)                                                 \
	(mask & ~(numlockmask | LockMask) &                             \
	 (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | \
	  Mod5Mask))
#define INTERSECT(x, y, w, h, m)                                        \
	(MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) * \
	 MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))
#define TAGMASK ((1 << num_tags) - 1)

/* enums */
enum {
	NetSupported,
	NetWMName,
	NetWMState,
	NetWMCheck,
	NetWMFullscreen,
	NetActiveWindow,
	NetWMWindowType,
	NetWMWindowTypeDialog,
	NetClientList,
	NetLast
}; /* EWMH atoms */

/* function declarations */
static void applyrules(Client *c);
static void attach(Client *c);
static void attachabove(Client *c);
static void buttonpress(const XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(void *mon, void *storage);
static void clientmessage(const XEvent *e);
static void configurenotify(const XEvent *e);
static void configurerequest(const XEvent *e);
static void destroynotify(const XEvent *e);
static void detach(Client *c);
static void drawbars(void);
static void enternotify(const XEvent *e);
static void expose(const XEvent *e);
static void focusin(const XEvent *e);
static Atom getatomprop(Client *c, Atom prop);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void keypress(const XEvent *e);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(const XEvent *e);
static void maprequest(const XEvent *e);
static void motionnotify(const XEvent *e);
static void propertynotify(const XEvent *e);
static void run(void);
static void scan(void);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setup(void);
static void seturgent(Client *c, int urg);
static void sigchld(int unused);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(const XEvent *e);
static void updatebars(void);
static void updateclientlist(void);
static int updategeom(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);

/* variables */
static const char broken[] = "broken";
static int screen;
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent])(const XEvent *) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
static Atom netatom[NetLast];
static int running = 1;
static Window wmcheckwin;

Cur *cursor[CurLast];
Display *dpy;
Drw *drw;
struct list mons;
Monitor *selmon;
Atom wmatom[WMLast];
int lrpad; /* sum of left and right padding for text in the bar*/
Window root;
Clr **scheme;
char stext[256];
int screen_w, screen_h; // X display screen geometry width, height. Of the
                        // default screen only??
int bar_h, bar_w = 0;   /* bar geometry */

/* configuration, allows nested code to access above variables */
#include "config.h"

void pdebug(const char *s)
{
	printf("%s", s);
	fflush(stdout);
}

/* function implementations */
void
applyrules(Client *c)
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

void
attach(Client *c)
{
	tagview_prepend_client(c->mon->tagview, c);
}

void
attachabove(Client *c)
{
	printf("%s(%p)\n", __func__, (void *)c);

	Client *selected_client = tagview_selected_client_get(c->mon->tagview);

	if (selected_client == NULL || selected_client->isfloating) {
		tagview_prepend_client(c->mon->tagview, c);
	} else {
		tagview_add_client(c->mon->tagview, c);
	}
}

void
buttonpress(const XEvent *e)
{
	unsigned int i, click;
	Arg arg = { 0 };
	Client *c;
	Monitor *m;
	const XButtonPressedEvent *ev = &e->xbutton;

	printf("%s(%p)\n", __func__, (void *)e);

	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		unfocus(tagview_selected_client_get(selmon->tagview), 1);
		selmon = m;
		focus(NULL);
	}
	if (ev->window == selmon->barwin) {
#if 0
		// From dwm, obsolete. Interface to the bar with the mouse??
		// TODO: break this out to a function (see local file bar.c)
		unsigned int x;
		i = x = 0;
		do
			x += drw_fontset_getwidth(drw, tags[i]) + lrpad;
		while (ev->x >= x && ++i < num_tags);
		if (i < num_tags) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		} else if (ev->x < x + blw) {
			click = ClkLtSymbol;
		} else if (ev->x > selmon->ww - (int)drw_fontset_getwidth(drw, stext) + lrpad) {
			click = ClkStatusText;
		} else {
			click = ClkWinTitle;
		}
#endif
	} else if ((c = wintoclient(ev->window))) {
		focus(c);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}
	for (i = 0; i < num_buttons; i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		    && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
checkotherwm(void)
{
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void)
{
	Arg a = { .ui = ~0 };
	size_t i;

	view(&a);
	// TODO: free the monitors, clients, layouts, tagviews, ...
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	list_run_for_all(&mons, cleanupmon, NULL);

	for (i = 0; i < CurLast; i++)
		drw_cur_free(drw, cursor[i]);
	for (i = 0; i < num_colors; i++)
		free(scheme[i]);
	XDestroyWindow(dpy, wmcheckwin);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

/* The argument must be of type (Monitor *), but this function is a
 * callback and must have (void *) as argument, to be passed to
 * list_run_for_all(). */
void cleanupmon(void *input, void *storage)
{
	Monitor *mon = input;

	list_rm(&mons, mon);
	monitor_destruct(mon);
}

void
clientmessage(const XEvent *e)
{
	printf("%s(%p)\n", __func__, (void *)e);
	const XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if (!c)
		return;
	if (cme->message_type == netatom[NetWMState]) {
		if (cme->data.l[1] == netatom[NetWMFullscreen]
		    || cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
					  || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (c != tagview_selected_client_get(selmon->tagview) && !c->isurgent)
			seturgent(c, 1);
	}
}



void
configurenotify(const XEvent *e)
{
	// X calls this function whenever it is done reconfiguring a
	// window.
	const XConfigureEvent *ev = &e->xconfigure;
	int dirty;

	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {
		printf("%s on root\n", __func__);
		dirty = (screen_w != ev->width || screen_h != ev->height);
		screen_w = ev->width;
		screen_h = ev->height;
		if (updategeom() || dirty) {
			// Resize drw with... bar_h? What IS drw??
			drw_resize(drw, screen_w, bar_h);
			updatebars();
#if 0
			I dont really know what to do with this yet
				for (Monitor *m = mons; m; m = m->next) {
					for (struct ll_node *node = m->tagview->clients.head;
					     node;
					     node = node->next) {
						Client *c = node->data;
						if (c->isfullscreen)
							resizeclient(c, m->mx, m->my, m->mw, m->mh);
					}
#if BAR
					Bar not implemented.Probably not going to deal with them
					like this anyway.
					XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bar_h);
#endif
				}
#endif
			focus(NULL);
			arrange(NULL);
		}
	}
}

void
configurerequest(const XEvent *e)
{
	Client *c;
	Monitor *m;
	const XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if ((c = wintoclient(ev->window))) {
		if (ev->value_mask & CWBorderWidth) {
			c->bw = ev->border_width;
		} else if (c->isfloating || !selmon->tagview->arrange) {
			m = c->mon;
			if (ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if (ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}
			if (ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if (ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - width(c) / 2);      /* center in x direction */
			if ((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - height(c) / 2);     /* center in y direction */
			if ((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight)))
				configure(c);
			if (isvisible(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else {
			configure(c);
		}
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

void
destroynotify(const XEvent *e)
{
	Client *c;
	const XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window)))
		unmanage(c, 1);
}

void detach(Client *c)
{
	printf("%s(%p)\n", __func__, (void *)c);
	struct Client *new_selected_c = list_next_select(&c->mon->tagview->clients);

	list_rm(&c->mon->tagview->clients, c);

	if (new_selected_c == NULL) {
		// The list head might be NULL
		list_head_select(&c->mon->tagview->clients);
	} else {
		list_select(&c->mon->tagview->clients, new_selected_c);
	}
}

static void drawbar(void *monitor, void *storage)
{
	bar_draw((struct Monitor *)monitor);
}

void
drawbars(void)
{
	list_run_for_all(&mons, drawbar, NULL);
}

void
dwm_quit(void)
{
	running = 0;
}

void
enternotify(const XEvent *e)
{
	Client *c;
	Monitor *m;
	const XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	struct Client *sel_client = mon_selected_client_get(selmon);

	if (m != selmon) {
		unfocus(sel_client, 1);
		selmon = m;
	} else if (!c || c == sel_client) {
		return;
	}
	focus(c);
}

void
expose(const XEvent *e)
{
	Monitor *m;
	const XExposeEvent *ev = &e->xexpose;

	if (ev->count == 0 && (m = wintomon(ev->window)))
		bar_draw(m);
}

// What is the intended behavior when c is NULL? It originally ran on
// the first visible client in the "stack".
// Now running on the selected client
void
focus(Client *c)
{
	//	does c have mon set correctly?
	printf("%s(%p)\n", __func__, (void *)c);
	if (c == NULL) {
		c = mon_selected_client_get(selmon);
		if (c == NULL) {
			printf("%s: nothing to focus\n", __func__);
			return;
		}
	}

	struct Client *sel_client = mon_selected_client_get(selmon);
	if (c != sel_client) {
		unfocus(sel_client, 0);
	}
	if (c) {
		if (c->mon != selmon)
			selmon = c->mon;
		if (c->isurgent)
			seturgent(c, 0);
		grabbuttons(c, 1);
		XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		setfocus(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	mon_selected_client_set(selmon, c);
	drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(const XEvent *e)
{
	const XFocusChangeEvent *ev = &e->xfocus;

	struct Client *sel_client = mon_selected_client_get(selmon);

	if (sel_client && ev->window != sel_client->win)
		setfocus(sel_client);
}

Atom
getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
			       &da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

int
getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
			       &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if (n != 0)
		result = *p;
	XFree(p);
	return result;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;
	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
		return 0;
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else {
		if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

/* Should move to DM module, not using type Client */
void
grabbuttons(Client *c, int focused)
{
	updatenumlockmask();
	{
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
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask | LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for (i = 0; i < num_keys; i++)
			if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
						 True, GrabModeAsync, GrabModeAsync);
	}
}

#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		    && unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void
keypress(const XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	const XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for (i = 0; i < num_keys; i++)
		if (keysym == keys[i].keysym
		    && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		    && keys[i].func)
			keys[i].func(&(keys[i].arg));
}

/*
 * TODO: this function creates new client windows. Should move to client.c?
 */
void
manage(Window w, XWindowAttributes *wa)
{
	printf("%s\n", __func__);
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

	updatetitle(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		applyrules(c);
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
	updatewindowtype(c);
	updatesizehints(c);
	updatewmhints(c);
	XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	grabbuttons(c, 0);
	if (!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if (c->isfloating)
		XRaiseWindow(dpy, c->win);
	attachabove(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
			(unsigned char *)&(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * screen_w, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if (c->mon == selmon)
		unfocus(mon_selected_client_get(selmon), 0);
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	focus(NULL);
}

void
mappingnotify(const XEvent *e)
{
	/* local copy to avoid discarding const qualifier */
	XMappingEvent ev = e->xmapping;

	XRefreshKeyboardMapping(&ev);
	if (ev.request == MappingKeyboard)
		grabkeys();
}

void
maprequest(const XEvent *e)
{
	static XWindowAttributes wa;
	const XMapRequestEvent *ev = &e->xmaprequest;

	if (!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if (wa.override_redirect)
		return;
	if (!wintoclient(ev->window))
		manage(ev->window, &wa);
}

void
motionnotify(const XEvent *e)
{
	/* A MotionNotify event reports that the user moved the pointer or
	 * that a program warped the pointer to a new position within a
	 * single window.
	 */
	static Monitor *mon = NULL;
	Monitor *m;
	const XMotionEvent *ev = &e->xmotion;

	if (ev->window != root)
		return;
	if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(mon_selected_client_get(selmon), 1);
		selmon = m;
		focus(NULL);
	}
	mon = m;
}

void
pop(Client *c)
{
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void
propertynotify(const XEvent *e)
{
	Client *c;
	Window trans;
	const XPropertyEvent *ev = &e->xproperty;

	if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
		updatestatus();
	} else if (ev->state == PropertyDelete) {
		return; /* ignore */
	} else if ((c = wintoclient(ev->window))) {
		switch (ev->atom) {
		default:
			break;
		case XA_WM_TRANSIENT_FOR:
			if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
			    (c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == mon_selected_client_get(c->mon))
				bar_draw(c->mon);
		}
		if (ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

struct Monitor *
recttomon(int x, int y, int w, int h)
{
	Monitor *ret = selmon;
	int a, area = 0;

	for (int i = 0; i < mons.size; i++) {
		// TODO: this could be done with a list_run_for_all
		// with callback and storage.
		struct Monitor *m = list_data_handle_get(&mons, i);
		if ((a = area_in_mon(x, y, w, h, m)) > area) {
			area = a;
			ret = m;
		}
	}
	return ret;
}

void
run(void)
{
	XEvent ev;

	printf("run start\n"); fflush(stdout);
	/* main event loop */
	XSync(dpy, False);
	while (running && !XNextEvent(dpy, &ev)) {
		if (handler[ev.type])
			handler[ev.type](&ev); /* call handler */
	}
}

void
scan(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)
			    || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
			    && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if (wins)
			XFree(wins);
	}
}

void
sendmon(Client *c, Monitor *m)
{
	if (c->mon == m)
		return;
	unfocus(c, 1);
	detach(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	attachabove(c);
	focus(NULL);
	arrange(NULL);
}

void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

int
sendevent(Client *c, Atom proto)
{
	int n;
	Atom *protocols;
	int exists = 0;
	XEvent ev;

	if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		while (!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}
	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->win, False, NoEventMask, &ev);
	}
	return exists;
}

void
setfocus(Client *c)
{
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
				XA_WINDOW, 32, PropModeReplace,
				(unsigned char *)&(c->win), 1);
	}
	sendevent(c, wmatom[WMTakeFocus]);
}

void
setfullscreen(Client *c, int fullscreen)
{
	if (fullscreen && !c->isfullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
				PropModeReplace, (unsigned char *)&netatom[NetWMFullscreen], 1);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (!fullscreen && c->isfullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
				PropModeReplace, (unsigned char *)0, 0);
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
setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;

	printf("%s\n", __func__); fflush(stdout);

	/* clean up any zombies immediately */
	sigchld(0);

	/* init screen */
	screen = DefaultScreen(dpy);
	screen_w = DisplayWidth(dpy, screen);
	screen_h = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	drw = drw_create(dpy, screen, root, screen_w, screen_h);
	if (!drw_fontset_create(drw, fonts, num_fonts))
		die("no fonts could be loaded.");
	lrpad = drw->fonts->h;
	bar_h = drw->fonts->h + 2;
	pdebug("before updategeom\n");
	updategeom();
	pdebug("after updategeom\n");/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init appearance */
	pdebug("before num_colors ecalloc\n");
	scheme = ecalloc(num_colors, sizeof(Clr *));
	for (i = 0; i < num_colors; i++)
		scheme[i] = drw_scm_create(drw, colors[i], 3);
	tagview_init();
	/* init bars */
	pdebug("before updatebars\n");
	updatebars();
	updatestatus();
	pdebug("before createsimple\n");
	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
			PropModeReplace, (unsigned char *)"dwm", 3);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&wmcheckwin, 1);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *)netatom, NetLast);
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	/* select events */
	pdebug("before cursor\n");
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
			| ButtonPressMask | PointerMotionMask | EnterWindowMask
			| LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	pdebug("before grabkeys\n");
	grabkeys();
	pdebug("before focus null\n");
	focus(NULL);
	pdebug("setup done\n");
}


void
seturgent(Client *c, int urg)
{
	XWMHints *wmh;

	c->isurgent = urg;
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
sigchld(int unused)
{
	if (signal(SIGCHLD, sigchld) == SIG_ERR)
		die("can't install SIGCHLD handler:");
	while (0 < waitpid(-1, NULL, WNOHANG));
}

/* Should move to DM module, not using type Client */
void
unfocus(Client *c, int setfocus)
{
	if (!c)
		return;
	grabbuttons(c, 0);
	XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void
unmanage(Client *c, int destroyed)
{
	Monitor *m = c->mon;
	XWindowChanges wc;

	printf("%s(%p, %d)\n", __func__, (void *)c, destroyed);
	detach(c);
	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy);                                       /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);      /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	free(c);
	focus(NULL);
	updateclientlist();
	arrange(m);
}

void
unmapnotify(const XEvent *e)
{
	Client *c;
	const XUnmapEvent *ev = &e->xunmap;

	if ((c = wintoclient(ev->window))) {
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	}
}

void
updatebars(void)
{
#if BAR
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask | ExposureMask
	};
	XClassHint ch = { "dwm", "dwm" };

	for (m = mons; m; m = m->next) {
		if (m->barwin)
			continue;
		m->barwin = XCreateWindow(dpy, root, m->wx, m->by, m->ww, bar_h, 0, DefaultDepth(dpy, screen),
					  CopyFromParent, DefaultVisual(dpy, screen),
					  CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
		XMapRaised(dpy, m->barwin);
		XSetClassHint(dpy, m->barwin, &ch);
	}
#endif
}


static void
client_list_update(void *data, void *storage)
{
	struct Client *c = (struct Client *)data;

	XChangeProperty(dpy, root, netatom[NetClientList],
			XA_WINDOW, 32, PropModeAppend,
			(unsigned char *)&(c->win), 1);
}

void
updateclientlist()
{
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	tagview_run_for_all_tv_all_clients(client_list_update);
}

struct xinerama_screen_info {
	XineramaScreenInfo *unique;
	int size;
};

static void update_mons_with_new(void *data, void *storage)
{
	struct Monitor *m = data;
	struct xinerama_screen_info *screen_info = storage;
	XineramaScreenInfo *unique = screen_info->unique;

	int i;

	for (i = 0; i < screen_info->size; i++) {
		if (unique[i].x_org != m->mx
		    || unique[i].y_org != m->my
		    || unique[i].width != m->mw
		    || unique[i].height != m->mh) {
			m->num = i;
			m->mx = m->wx = unique[i].x_org;
			m->my = m->wy = unique[i].y_org;
			m->mw = m->ww = unique[i].width;
			m->mh = m->wh = unique[i].height;
			updatebarpos(m);
		}
	}
}

static bool
monitor_has_tagview(void *monitor, void *tagview)
{
	struct Monitor *m = (struct Monitor *)monitor;
	struct tagview *tv = (struct tagview *)tagview;

	return m->tagview == tv;
}

static struct tagview *
undisplayed_tagview_get(void)
{
	int tv_index = 0;

	while (list_find(
		       &mons,
		       monitor_has_tagview,
		       tagview_get(tv_index))) {
		tv_index++;
	}
	return tagview_get(tv_index);
}

int
updategeom(void)
{
	bool dirty = false;

	pdebug("updategeom start\n");

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, new_num_mons;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &new_num_mons);
		XineramaScreenInfo *unique = NULL;

		/* only consider unique geometries as separate screens */
		unique = ecalloc(new_num_mons, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < new_num_mons; i++)
			if (isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		new_num_mons = j;

		if (mons.size <= new_num_mons) {
			pdebug("updategeom: new monitors\n");
			/* More monitors available */
			for (i = mons.size; i < new_num_mons; i++) {
				pdebug(".");
				list_add(&mons, createmon(undisplayed_tagview_get()));
			}
			pdebug("\n");
			struct xinerama_screen_info screen_info = {
				.unique = unique,
				.size = new_num_mons
			};

			list_run_for_all(&mons, update_mons_with_new, &screen_info);

			// TODO: make the new monitors show tagviews
			// that are not visible.
			//
			// For example: populate a bit mask with 1 for
			// tagviews that are not visible. Use
			// list_run_for_all to show the first
			// invisible tagview in the next new monitor,
			// and clear the bit in the mask. That should
			// be possible by passing the mask to the
			// callback through storage.
		} else {
			pdebug("updategeom remove mons");
			/* fewer monitors available */
			for (i = new_num_mons; i < mons.size; i++) {
				struct Monitor *m = list_pop(&mons);
				monitor_destruct(m);
				if (m == selmon) {
					selmon = list_data_handle_get(&mons, 0);
				}
			}
		}
		dirty = true;
		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		/* UNTESTED, only run with Xinerama so far */
		struct Monitor *m;
		if (mons.size == 0) {
			m = createmon(undisplayed_tagview_get());
			list_add(&mons, m);
		} else {
			m = selmon;
		}
		if (m->mw != screen_w || m->mh != screen_h) {
			dirty = true;
			m->mw = m->ww = screen_w;
			m->mh = m->wh = screen_h;
			updatebarpos(m);
		}
	}
	if (dirty) {
		selmon = mons.head->data;
	}
	return dirty;
}

void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
			    == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
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
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else {
		c->maxa = c->mina = 0.0;
	}
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatestatus(void)
{
	if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)))
		strcpy(stext, "dwm-"VERSION);
	bar_draw(selmon);
}

void
updatetitle(Client *c)
{
	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void
updatewindowtype(Client *c)
{
	Atom state = getatomprop(c, netatom[NetWMState]);
	Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

	if (state == netatom[NetWMFullscreen])
		setfullscreen(c, 1);
	if (wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

void
updatewmhints(Client *c)
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

Client *
wintoclient(Window w)
{
	return tagview_find_window_client(&w);
}

static bool
window_is_barwin(void *monitor, void *window)
{
	struct Monitor *m = monitor;
	Window *w = window;

	return *w == m->barwin;
}

Monitor *
wintomon(Window w)
{
	int x, y;
	Client *c;
	Monitor *m;

	if (w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);

	m = list_find(&mons, window_is_barwin, &w);
	if (m)
		return m;

	if ((c = wintoclient(w)))
		return c->mon;

	return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow
	    || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	    || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	    || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("dwm: another window manager is already running");
	return -1;
}

void
xeventhandler(const XEvent *ev)
{
	assert(ev && ev->type < LASTEvent);
	handler[ev->type](ev);
}

int
main(int argc, char *argv[])
{
	pdebug("main\n");
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION);
	else if (argc != 1)
		die("usage: dwm [-v]");
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display");
	checkotherwm();
	setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
	run();
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
