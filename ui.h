#ifndef UI_H
#define UI_H

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

void monitor_focus(const Arg *arg);
void focusstack(const Arg *arg);
void incnmaster(const Arg *arg);
void killclient(const Arg *arg);
void movemouse(const Arg *arg);
void quit(const Arg *arg);
void resizemouse(const Arg *arg);
void setlayout(const Arg *arg);
void setmfact(const Arg *arg);
void spawn(const Arg *arg);
void tag_send(const Arg *arg);
void tagmon(const Arg *arg);
void togglebar(const Arg *arg);
void togglefloating(const Arg *arg);
void toggletag(const Arg *arg);
void toggleview(const Arg *arg);
void tag_view(const Arg *arg);
void to_master_send(const Arg *arg);

#endif
