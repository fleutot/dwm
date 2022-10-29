#ifndef DM_H
#define DM_H

#include "client.h"

void dm_focus(struct Client *c);
int dm_gettextprop(Window w, Atom atom, char *text, unsigned int size);

#endif
