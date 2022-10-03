#include <stdio.h>

#include "layouts/layout_two_cols.h"

#include "monitor.h"

void layout_two_cols_arrange(void *layout_cfg, const struct Monitor *mon)
{
    struct layout_cfg_two_cols *cfg = layout_cfg;
    printf("%s\n", __func__);

#if 0
    This code is from dwm's `void tile(Monitor *m)`, with some of my
        modifications.

        	unsigned int i, n, h, mw, my, ty;
	Client *c;

	struct ll_node *node;

	// TODO: this counts the number of clients. Do it in a struct linkedlist?
	for (n = 0, node = nexttiled(m->clients.head);
	     node != NULL;
	     node = nexttiled(node->next), n++)
		;

	if (n == 0)
		return;

	if (n > m->nmaster)
		mw = m->nmaster ? m->ww * m->mfact : 0;
	else
		mw = m->ww;

	for (i = my = ty = 0, node = nexttiled(m->clients.head);
	     node != NULL;
	     node = nexttiled(node->next), i++)
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i);
			resize(c, m->wx, m->wy + my, mw - (2 * c->bw),
			       h - (2 * c->bw), 0);
			if (my + height(c) < m->wh)
				my += height(c);
		} else {
			h = (m->wh - ty) / (n - i);
			resize(c, m->wx + mw, m->wy + ty, m->ww - mw - (2 * c->bw), h - (2 * c->bw), 0);
			if (ty + height(c) < m->wh)
				ty += height(c);
		}

#endif

}
