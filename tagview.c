#include <assert.h>
#include <stdio.h>

#include "tagview.h"

#include "client.h"
#include "config.h"
#include "debug.h"
#include "dwm.h"
#include "layout.h"
#include "util.h"

//******************************************************************************
// Module variables
//******************************************************************************
static struct tagview tagviews[LENGTH(tags)];

//******************************************************************************
// Function prototypes
//******************************************************************************

//******************************************************************************
// Function definitions
//******************************************************************************
void tagview_init(void)
{
	for (int i = 0; i < LENGTH(tags); i++) {
		tagviews[i] = (struct tagview) {
			.index = i,
			.arrange = LAYOUT_DEFAULT,
			.layout_cfg = &tagviews[i].layout_cfg_two_cols,
			.clients = LIST_EMPTY,
			.layout_cfg_two_cols = (struct layout_cfg_two_cols) {
				.n_master = 1,
				.col_ratio = 0.5,
				//.selected_window = 12345687 // TODO
			},
		};
	}
}

void tagview_hide(struct tagview *tv)
{
	if (tv == NULL) {
		return;
	}
	P_DEBUG("%s, clients size: %d\n", __func__, tv->clients.size);
	list_run_for_all(&tv->clients, client_hide, NULL);
}

void tagview_show(struct tagview *tv, struct Monitor *m)
{
	if (tv == NULL) {
		return;
	}
	m->tagview = tv;
	list_run_for_all(&tv->clients, client_mon_set, m);
	tagview_arrange(m);
	list_run_for_all(&tv->clients, client_show, NULL);
}

void tagview_arrange(struct Monitor *m)
{
	assert(m != NULL);
	assert(m->tagview != NULL);
	assert(m->tagview->arrange != NULL);
	assert(m->tagview->layout_cfg != NULL);

	m->tagview->arrange(m->tagview->layout_cfg, m);
}

void tagview_layout_set(struct tagview *t, enum layout_index layout)
{
	static layout_arrange_function_t arrange_funcs[] = {
		[TAGVIEW_LAYOUT_TWO_COLS] = layout_two_cols_arrange,
	};

	t->arrange = arrange_funcs[layout];
	// Also run it?
}

void tagview_add_client(struct tagview *t, Client *c)
{
	P_DEBUG("%s(%p, %p)\n", __func__, (void *) t, (void *) c);

	///	ERRROR HERE: selected may be NULL. Modify `list_add_before` to allow
	///		adding to empty list if second parameter is null.

	list_add_before(&t->clients, t->clients.selected->data, c);
}

void tagview_rm_client(struct tagview *t, Client *c)
{
	P_DEBUG("%s(%p, %p)\n", __func__, (void *) t, (void *) c);
	if (c == list_selected_data_get(&t->clients)) {
		(void) list_prev_select(&t->clients);
		if (c == list_selected_data_get(&t->clients)) {
			(void) list_next_select(&t->clients);
		}
	}
	list_rm(&t->clients, (void *) c);
}

void tagview_prepend_client(struct tagview *t, Client *c)
{
	P_DEBUG("%s(%p, %p)\n", __func__, (void *) t, (void *) c);
	list_prepend(&t->clients, c);
}

Client *tagview_next_client_select(struct tagview *t)
{
	return list_next_wrap_select(&t->clients);
}

Client *tagview_prev_client_select(struct tagview *t)
{
	return list_prev_wrap_select(&t->clients);
}

Client *tagview_selected_client_get(struct tagview *t)
{
	P_DEBUG("input %p, tv index %d\n", (void *) t, t->index);
	return (Client *) list_selected_data_get(&t->clients);
}

void tagview_selected_client_set(struct tagview *t, const struct Client *c)
{
	list_select(&t->clients, c);
}

void tagview_run_for_all_tv_all_clients(
	void (*callback)(void *data, void *storage))
{
	for (int i = 0; i < LENGTH(tags); i++) {
		list_run_for_all(&tagviews[i].clients, callback, NULL);
	}
}

static bool window_to_find_is_client(void *c, void *w)
{
	Window *window_to_find = (Window *) w;

	return ((struct Client *) c)->win == *window_to_find;
}

struct Client *tagviews_find_window_client(Window *w)
{
	struct Client *c = NULL;
	Window *window_to_find = w;

	for (int i = 0; i < LENGTH(tagviews); i++) {
		c = list_find(
			&tagviews[i].clients,
			window_to_find_is_client,
			window_to_find);
		if (c != NULL) {
			return c;
		}
	}
	return NULL;
}

struct Client *tagview_find_window_client(struct tagview *tv, Window *w)
{
	return list_find(
		&tv->clients,
		client_has_win,
		w);
}

struct tagview *tagview_get(unsigned int index)
{
	return &tagviews[index];
}
//******************************************************************************
// Internal functions
//******************************************************************************
