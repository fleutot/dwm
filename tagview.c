#include <assert.h>

#include "tagview.h"

#include "config.h"
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
                  .arrange = layout_two_cols_arrange,
                  .layout_cfg = &tagviews[i].layout_cfg_two_cols,
                  .clients = LINKEDLIST_EMPTY,
                  .active_client = NULL,
                  .layout_cfg_two_cols = (struct layout_cfg_two_cols) {
                      .n_master = 1,
                      .col_ratio = 0.5,
                      //.selected_window = 12345687 // TODO
                  },
		};
	}
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
	linkedlist_add_before(&t->clients, &t->active_client, c);
	t->clients.selected->data = c;
}

void tagview_prepend_client(struct tagview *t, Client *c)
{
	linkedlist_prepend(&t->clients, c);
	t->clients.selected->data = c;
}

Client *tagview_selected_client_get(struct tagview *t)
{
	return (Client *) t->clients.selected->data;
}

void tagview_run_for_all_tv_all_clients(void (*callback)(void *))
{
	for (int i = 0; i < LENGTH(tags); i++) {
           linkedlist_run_for_all(&tagviews[i].clients, callback);
       }
}

static Window window_to_find;
static bool window_to_find_is_client(void *c)
{
	return ((struct Client *)c)->win == window_to_find;
}
struct Client *tagview_find_window_client(Window w)
{
	struct Client *c = NULL;
	window_to_find = w;

	for (int i = 0; i < LENGTH(tagviews); i++) {
		c = linkedlist_find(
			&tagviews[i].clients,
			window_to_find_is_client);
		if (c != NULL) {
			return c;
		}
	}
	return NULL;
}

//******************************************************************************
// Internal functions
//******************************************************************************
