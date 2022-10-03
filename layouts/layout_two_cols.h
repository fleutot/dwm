#ifndef LAYOUT_TWO_COLS
#define LAYOUT_TWO_COLS

struct layout_cfg_two_cols {
	int n_master; // Number of windows in the master area
	float col_ratio; // relative size of the master area
	//Win selected_window; // TODO: should be a client instead? An index?
};


#include "monitor.h"

// This prototype must match for all layouts, see struct tagview, and the
// function pointer type layout_arrange_function_t.
void layout_two_cols_arrange(void *layout_cfg, const struct Monitor *mon);

#endif
