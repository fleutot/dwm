#ifndef MONITOR_H
#define MONITOR_H

/* This must come before including header files using the type. */
typedef struct Monitor Monitor;

#include "client.h"
#include "layout.h"

struct Monitor {
	char         ltsymbol[16];
	float        mfact;
	int          nmaster;
	int          num;
	int          by;             /* bar geometry */
	int          mx, my, mw, mh; /* screen size */
	int          wx, wy, ww, wh; /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int          showbar;
	int          topbar;
	Client       *clients;
	Client       *sel;
	Client       *stack;
	Monitor      *next;
	Window       barwin;
	const Layout *lt[2];
};

void arrange(Monitor *m);

Monitor *createmon(void);

void drawbar(Monitor *m);

Client *nexttiled(Client *c);

Monitor *recttomon(int x, int y, int w, int h);

void tile(Monitor *m);

void monocle(Monitor *m);

void updatebarpos(Monitor *m);

#endif
