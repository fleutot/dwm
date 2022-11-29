#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <X11/Xlib.h>

/* This must come before including header files using the type. */
typedef struct Client Client;

struct Monitor;

enum { SchemeNorm, SchemeSel };                         /* color schemes */

struct Client {
	char name[256];
	float mina, maxa;

	/// TODO: clients should not save their geometry, why would
	/// they? The layouts manage geometries, and update the
	/// display manager window.
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
	bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	Client *next;
	Client *snext;

	/// TODO: obsolete? only tagview know about their current monitor
	struct Monitor *mon;

	Window win;
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int monitor;
} Rule;

void client_create(Window w, XWindowAttributes *wa);

void client_focus(struct Client *c);
void client_unfocus(struct Client *c, bool focus_root);

// These signatures must match that of the callback in list_run_for_all
void client_hide(void *client, void *storage);
void client_show(void *client, void *storage);
void client_mon_set(void *client, void *monitor);
// These signatures must match that of the callback in list_find
bool client_has_win(void *client, void *window);

void client_urgent_set(struct Client *c, bool is_urgent);

void client_name_update(Client *c);
void client_update_size_hints(Client *c);
void client_update_wm_hints(Client *c);
void client_update_window_type(Client *c);
void client_state_set(Client *c, long state);
void client_fullscreen_set(Client *c, bool fullscreen);

bool isvisible(const Client *c);
void showhide(Client *c);
int width(const Client *c);
int height(const Client *c);
void configure(Client *c);
void resize(Client *c, int x, int y, int w, int h, int interact);
void resizeclient(Client *c, int x, int y, int w, int h);

#endif
