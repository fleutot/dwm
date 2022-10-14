#ifndef TAGVIEW_H
#define TAGVIEW_H

#include "client.h"
#include "layout_two_cols.h"
#include "linkedlist/linkedlist.h"

enum layout_index {
    TAGVIEW_LAYOUT_TWO_COLS,
    NUMBER_OF_LAYOUTS // Must be last
};

struct tagview {
    // Tagview still own the clients, would be an improvement to
    // flatten the ownersip hierarchy further?
    struct list clients;

    struct layout *layout;
    Client *active_client; // Is this needed, or use _selected_client_get()?

    // Each tagview must have separate layout configs for each
    // available layout.
    struct layout_cfg_two_cols layout_cfg_two_cols;
    //struct layout_cfg_fullscreen fullscreen;
    //struct layout_cfg_three_cols three_cols;
    // --- ^ --- Add more config struct for more layouts

    // Points to the config for current layout, one of the above struct.
    void *layout_cfg;

    // This points to the arrange function of the selected layout
    void (*arrange)(void *layout_config, const struct Monitor *mon);
};

void tagview_init(void);
void tagview_layout_set(struct tagview *t, enum layout_index layout);
void tagview_arrange(struct Monitor *m);
void tagview_add_client(struct tagview *t, Client *c);
void tagview_prepend_client(struct tagview *t, Client *c);
Client *tagview_next_client_select(struct tagview *t);
Client *tagview_selected_client_get(struct tagview *t);
void tagview_run_for_all_tv_all_clients(
    void (*callback)(void *data, void *storage));
struct Client *tagview_find_window_client(Window *w);

struct tagview *tagview_get(int index);

// TODO
// void tagview_client_select(int direction);

#endif
