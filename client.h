#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <X11/Xlib.h>

/* This must come before including header files using the type. */
typedef struct Client Client;

#include "monitor.h"

enum { SchemeNorm, SchemeSel };                         /* color schemes */

struct Client {
	char         name[256];
	float        mina, maxa;
	int          x, y, w, h;
	int          oldx, oldy, oldw, oldh;
	int          basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int          bw, oldbw;
	unsigned int tags;
	int          isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	Client       *next;
	Client       *snext;
	Monitor      *mon;
	Window       win;
};

typedef struct {
	const char   *class;
	const char   *instance;
	const char   *title;
	unsigned int tags;
	int          isfloating;
	int          monitor;
} Rule;

bool isvisible(const Client *c);
void showhide(Client *c);
int width(const Client *c);
int height(const Client *c);
void configure(Client *c);
void resize(Client *c, int x, int y, int w, int h, int interact);
void resizeclient(Client *c, int x, int y, int w, int h);

#endif
