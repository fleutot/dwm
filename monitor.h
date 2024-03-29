#ifndef MONITOR_H
#define MONITOR_H

/* This must come before including header files using the type. */
typedef struct Monitor Monitor;

#include "client.h"
#include "config.h"
#include "linkedlist/linkedlist.h"
#include "tagview.h"
#include "util.h"

struct Monitor {
	char ltsymbol[16];  // TODO: move to tagview
	float mfact;        // TODO: move to tagview
	int nmaster;        // TODO: move to tagview
	int num;            // This seems to be the "name" or "ID" of
	                    // that monitor
	int by;             /* bar geometry */
	int mx, my, mw, mh; /* screen size */
	int wx, wy, ww, wh; /* window area  */
	unsigned int seltags;
	unsigned int sellt; // TODO: move to tagview
	unsigned int tagset[2];
	int showbar;
	int topbar;
	struct tagview *tagview;
	// struct list clients;
	// Client       *sel;
	//Client       *stack;
	//Monitor      *next;
	Window barwin;
};

int mon_n_clients_get(const struct Monitor *m);

struct Client *mon_selected_client_get(const struct Monitor *m);
void mon_selected_client_set(struct Monitor *m, struct Client *c);

void mon_tag_switch(struct Monitor *m, struct tagview *tagview);
void mon_arrange(Monitor *m);

/* Callbacks for lists */
bool mon_has_client(void *monitor, void *client);
bool mon_has_window(void *monitor, void *window);
bool mon_shows_tagview(void *monitor, void *tagview);
void mon_arrange_cb(void *monitor, void *storage);


int area_in_mon(int x, int y, int w, int h, const Monitor *m);

void restack(Monitor *m);

Monitor *createmon(struct tagview *with_tagview, int x, int y, int w, int h);

void monitor_destruct(struct Monitor *m);

void tile(Monitor *m);

void monocle(Monitor *m);

void updatebarpos(Monitor *m);

#endif
