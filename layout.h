#ifndef LAYOUT_H
#define LAYOUT_H

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

#endif
