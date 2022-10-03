#ifndef MONITOR_H
#define MONITOR_H

/* This must come before including header files using the type. */
typedef struct Monitor Monitor;

#include "client.h"
#include "config.h"
#include "layout.h"
#include "linkedlist/linkedlist.h"
#include "tagview.h"
#include "util.h"

struct layout;

struct Monitor {
	char         ltsymbol[16]; // TODO: move to tagview
	float        mfact; // TODO: move to tagview
	int          nmaster; // TODO: move to tagview
	int          num; // What is this?
	int          by;             /* bar geometry */
	int          mx, my, mw, mh; /* screen size */
	int          wx, wy, ww, wh; /* window area  */
	unsigned int seltags;
	unsigned int sellt; // TODO: move to tagview
	unsigned int tagset[2];
	int          showbar;
	int          topbar;
	struct tagview *tagview;
	// struct linkedlist clients;
	// Client       *sel;
	//Client       *stack;
	Monitor      *next;
	Window       barwin;
	const struct layout *lt[2]; // TODO: move to tagview
};

void arrange(Monitor *m);

Monitor *createmon(void);

void drawbar(Monitor *m);

struct ll_node *nexttiled(struct ll_node *n);

Monitor *recttomon(int x, int y, int w, int h);

void tile(Monitor *m);

void monocle(Monitor *m);

void updatebarpos(Monitor *m);

#endif
