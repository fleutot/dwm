#include <stdio.h>

#include "layouts/layout_two_cols.h"

#include "debug.h"
#include "monitor.h"

struct positioning {
	const int n_masters;
	int current_client_index;

	int current_master_y;
	const int master_x;
	const int master_w;
	const int master_client_h;

	int current_stack_y;
	const int stack_x;
	const int stack_w;
	const int stack_client_h;
};

static int border_width = 3;

static void client_position_apply(void *client, void *storage)
{
	struct Client *c = (struct Client *) client;
	struct positioning *p = (struct positioning *) storage;

	P_DEBUG("client index %d\n", p->current_client_index);
	if (p->current_client_index < p->n_masters) {
		P_DEBUG("in master area\n");
		// Master area
		// TODO: shouldn't Client deal with border width internally?
		// But that would disallow layouts from managing them...
		resize(
			c,
			p->master_x,
			p->current_master_y,
			p->master_w - (2 * border_width),
			p->master_client_h - (2 * border_width),
			border_width,
			0
			);
		P_DEBUG("(x, y, w, h): (%d, %d, %d, %d)\n",
			p->master_x,
			p->current_master_y,
			p->master_w - (2 * border_width),
			p->master_client_h - (2 * border_width));
		p->current_master_y += p->master_client_h;
	} else {
		// Stack area
		P_DEBUG("in stack area\n");
		resize(
			c,
			p->master_x + p->master_w,
			p->current_stack_y,
			p->stack_w - (2 * border_width),
			p->stack_client_h - (2 * border_width),
			border_width,
			0
			);
		P_DEBUG("(x, y, w, h): (%d, %d, %d, %d)\n",
			p->master_x + p->master_w,
			p->current_stack_y,
			p->stack_w - (2 * border_width),
			p->stack_client_h - (2 * border_width));
		p->current_stack_y += p->stack_client_h;
	}
	p->current_client_index++;

	/// TODO: why not do client_show here? Instead of in tagview_show.
#if 0
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

void layout_two_cols_arrange(void *layout_cfg, struct Monitor *mon)
{
	P_DEBUG("m %p mon.xy: (%d, %d)\n", (void *) mon, mon->mx, mon->my);
	struct layout_cfg_two_cols *cfg = layout_cfg;

	int n_clients = mon_n_clients_get(mon);

	if (n_clients == 0) {
		P_DEBUG("no clients\n");
		return;
	}

	unsigned int master_w =
		(cfg->n_master == 0) ? 0
		: (n_clients <= cfg->n_master) ? mon->ww
		: mon->ww * cfg->col_ratio;

	int n_masters = MIN(n_clients, cfg->n_master);
	int n_stacked = n_clients - n_masters;

	struct positioning p = {
		.n_masters = n_masters,
		.current_client_index = 0,

		.current_master_y = mon->wy,
		.master_x = mon->wx,
		.master_w = master_w,
		.master_client_h = n_masters > 0 ? mon->wh / n_masters : 0,

		.current_stack_y = mon->wy,
		.stack_x = mon->wx + master_w,
		.stack_w = mon->ww - master_w,
		.stack_client_h = n_stacked > 0 ? mon->wh / n_stacked : 0,
	};

	list_run_for_all(
		&mon->tagview->clients,
		client_position_apply,
		&p);

#if 0
	// This code is from dwm's `void tile(Monitor *m)`, with some of my
	// modifications.

	unsigned int i, n, h, mw, my, ty;
	Client *c;

	struct ll_node *node;

	// TODO: this counts the number of clients. Do it in a struct list?
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
