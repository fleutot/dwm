/* See LICENSE file for copyright and license details. */
/* This file defines much data, it should be a .c file rather then .h */
#ifndef CONFIG_H
#define CONFIG_H

#include "client.h"
#include "input.h"
#include "ui.h"
#include "util.h"

#define LAYOUT_DEFAULT layout_two_cols_arrange

/* appearance */
extern const unsigned int borderpx;      /* border pixel of windows */
extern const unsigned int snap;          /* snap pixel */
extern const int showbar;                /* 0 means no bar */
extern const int topbar;                 /* 0 means bottom bar */
extern const char *fonts[];
extern const int num_fonts;
extern const char dmenufont[];

extern const char *colors[][3];
extern const int num_colors;

/* tagging */
#define NUMBER_OF_TAGS 10
extern const char *tags[NUMBER_OF_TAGS];
extern const int num_tags;
extern const unsigned int tag_mask;

extern const Rule rules[];
extern const int num_rules;

/* layout(s) */
extern const float mfact;           /* factor of master area size [0.05..0.95] */
extern const int nmaster;           /* number of clients in master area */
extern const int resizehints;       /* 1 means respect size hints in tiled resizals */

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char *[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
extern char dmenumon[2]; /* component of dmenucmd, manipulated in spawn() */
extern const char *launchercmd[];

extern const char *termcmd[];

extern Key keys[];
extern const int num_keys;

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
extern Button buttons[];
extern const int num_buttons;

#endif
