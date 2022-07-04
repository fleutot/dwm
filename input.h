#ifndef INPUT_H
#define INPUT_H

#include <X11/Xlib.h>

#include "ui.h"

#define BUTTONMASK              (ButtonPressMask | ButtonReleaseMask)
#define MOUSEMASK               (BUTTONMASK | PointerMotionMask)

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg    arg;
} Button;

enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast };                     /* clicks */

typedef struct {
	unsigned int mod;
	KeySym       keysym;
	void (*func)(const Arg *);
	const Arg    arg;
} Key;

#endif
