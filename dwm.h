#ifndef DWM_H
#define DWM_H

#include <stdbool.h>
#include <X11/Xlib.h>

#include "drw.h"
#include "monitor.h"

enum atom { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast };
enum cursor { CurNormal, CurResize, CurMove, CurLast };

extern Cur *cursor[CurLast];
extern Drw *drw;
extern Monitor *selmon;
extern int lrpad;               /* sum of left and right padding for text */
extern Monitor *mons;
extern Display *dpy;
extern Atom wmatom[WMLast];
extern Window root;
extern Clr **scheme;
extern char stext[];
extern int sw, sh;              /* X display screen geometry width, height */
extern int bh, blw;             /* bar geometry */

void focus(Client *c);
int getrootptr(int *x, int *y);
void dwm_quit(void);
bool isvisible(const Client *c);
void pop(Client *c);
void restack(Monitor *m);
int sendevent(Client *c, Atom proto);
void sendmon(Client *c, Monitor *m);
void unfocus(Client *c, int setfocus);
int xerror(Display *dpy, XErrorEvent *ee);
int xerrordummy(Display *dpy, XErrorEvent *ee);
int xerrorstart(Display *dpy, XErrorEvent *ee);
void xeventhandler(const XEvent *ev);

#endif
