#ifndef LAYOUT_H
#define LAYOUT_H

#include "monitor.h"

typedef struct layout {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

typedef void (*layout_arrange_function_t)(
    void *layout_cfg,
    const struct Monitor *mon);

#endif
