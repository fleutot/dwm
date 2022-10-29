/* See LICENSE file for copyright and license details. */
/* This file defines much data, it should be a .c file rather then .h */

#include <X11/keysym.h>

#include "config.h"

#include "input.h"
#include "ui.h"
#include "util.h"

/* appearance */
const unsigned int borderpx = 3;         /* border pixel of windows */
const unsigned int snap = 32;            /* snap pixel */
const int showbar = 0;                   /* 0 means no bar */
const int topbar = 1;                    /* 0 means bottom bar */
const char *fonts[] = { "monospace:size=10" };
const int num_fonts = LENGTH(fonts);
const char dmenufont[] = "monospace:size=10";

static const char col_gray1[] = "#222222";
static const char col_gray2[] = "#444444";
static const char col_gray3[] = "#bbbbbb";
static const char col_gray4[] = "#eeeeee";
static const char col_cyan[] = "#00aaee";//"#005577";
const char *colors[][3] = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel] = { col_gray4, col_cyan, col_cyan },
};
const int num_colors = LENGTH(colors);

/* tagging */
const char *tags[NUMBER_OF_TAGS] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };
const int num_tags = LENGTH(tags);
const unsigned int tag_mask = (1u << LENGTH(tags)) - 1;

/* *INDENT-OFF* */
const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",    NULL, NULL, 0,      1, -1 },
	{ "Firefox", NULL, NULL, 1 << 8, 0, -1 },
};
/* *INDENT-ON* */

const int num_rules = LENGTH(rules);

/* layout(s) */
const int resizehints = 0;       /* 1 means respect size hints in tiled resizals */

/// How to make the layout configurables? They need to be members of
/// tagviews, so not so easy to make from the config file(s). Some
/// pre-processing maybe?
///const Layout layouts[] = {
///	/* symbol     arrange function */
///	{ "[]=", tile    },             /* first entry is default */
///	{ "><>", NULL    },             /* no layout function means floating behavior */
///	{ "[M]", monocle },
///};
///const int num_layouts = LENGTH(layouts);

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY, TAG) \
	{ MODKEY, KEY, view, { .ui = 1 << TAG } }, \
	{ MODKEY | ControlMask, KEY, toggleview, { .ui = 1 << TAG } }, \
	{ MODKEY | ShiftMask, KEY, tag, { .ui = 1 << TAG } }, \
	{ MODKEY | ControlMask | ShiftMask, KEY, toggletag, { .ui = 1 << TAG } }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char *[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
const char *launchercmd[] =
{ "rofi", "-show", "combi", "-combi-modi", "drun,run", "-modi", "drun,run,ssh", "-config", "/home/gauthier/.xmonad/rofi.conf", NULL };

const char *termcmd[] = { "xterm", NULL };

/* *INDENT-OFF* */
Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                 XK_p,      spawn,          { .v  = launchercmd } },
	{ MODKEY | ShiftMask,     XK_Return, spawn,          { .v  = termcmd     } },
	{ MODKEY,                 XK_b,      togglebar,      { 0 } },
	{ MODKEY,                 XK_j,      focusstack,     { .i  = +1          } },
	{ MODKEY,                 XK_k,      focusstack,     { .i  = -1          } },
	{ MODKEY,                 XK_i,      incnmaster,     { .i  = +1          } },
	{ MODKEY,                 XK_d,      incnmaster,     { .i  = -1          } },
	{ MODKEY,                 XK_h,      setmfact,       { .f  = -0.05       } },
	{ MODKEY,                 XK_l,      setmfact,       { .f  = +0.05       } },
	{ MODKEY,                 XK_Return, to_master_send, { 0 } },
	{ MODKEY,                 XK_Tab,    view,           { 0 } },
	{ MODKEY | ShiftMask,     XK_c,      killclient,     { 0 } },
///	{ MODKEY,                 XK_t,      setlayout,      { .v  = &layouts[0] } },
///	{ MODKEY,                 XK_f,      setlayout,      { .v  = &layouts[1] } },
///	{ MODKEY,                 XK_m,      setlayout,      { .v  = &layouts[2] } },
	{ MODKEY,                 XK_space,  setlayout,      { 0 } },
	{ MODKEY | ShiftMask,     XK_space,  togglefloating, { 0 } },
	{ MODKEY,                 XK_0,      view,           { .ui = ~0          } },
	{ MODKEY | ShiftMask,     XK_0,      tag,            { .ui = ~0          } },
       // TODO: keeping this focusmon for now, but I want absolute mon
       // selection in the future. Mod+w = select monitor 1
	{ MODKEY,                 XK_comma,  focusmon,       { .i  = -1          } },
	{ MODKEY,                 XK_period, focusmon,       { .i  = +1          } },
	{ MODKEY | ShiftMask,     XK_comma,  tagmon,         { .i  = -1          } },
	{ MODKEY | ShiftMask,     XK_period, tagmon,         { .i  = +1          } },
	TAGKEYS(XK_1, 0),
	TAGKEYS(XK_2, 1),
	TAGKEYS(XK_3, 2),
	TAGKEYS(XK_4, 3),
	TAGKEYS(XK_5, 4),
	TAGKEYS(XK_6, 5),
	TAGKEYS(XK_7, 6),
	TAGKEYS(XK_8, 7),
	TAGKEYS(XK_9, 8),
	{ MODKEY | ShiftMask,     XK_q,      quit,           { 0 } },

	/* My personal keys */
	{ ControlMask | Mod1Mask, XK_equal,  killclient,     { 0 } },
};
/* *INDENT-ON* */

const int num_keys = LENGTH(keys);

#define MOUSE_MODKEY Mod1Mask
/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
/* *INDENT-OFF* */
Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,   0,            Button1, setlayout,      { 0           } },
///	{ ClkLtSymbol,   0,            Button3, setlayout,      { .v = &layouts[2]} },
	{ ClkWinTitle,   0,            Button2, to_master_send, { 0           } },
	{ ClkStatusText, 0,            Button2, spawn,          { .v = termcmd} },
	{ ClkClientWin,  MOUSE_MODKEY, Button1, movemouse,      { 0           } },
	{ ClkClientWin,  MOUSE_MODKEY, Button2, togglefloating, { 0           } },
	{ ClkClientWin,  MOUSE_MODKEY, Button3, resizemouse,    { 0           } },
	{ ClkTagBar,     0,            Button1, view,           { 0           } },
	{ ClkTagBar,     0,            Button3, toggleview,     { 0           } },
	{ ClkTagBar,     MOUSE_MODKEY, Button1, tag,            { 0           } },
	//{ ClkTagBar,     MOUSE_MODKEY, Button3, toggletag,      { 0           } },
};
/* *INDENT-ON* */
const int num_buttons = LENGTH(buttons);
