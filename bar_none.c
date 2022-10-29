#include "bar.h"
#include "dm.h"
#include "dwm.h"
#include "monitor.h"
#include <X11/Xatom.h>

//******************************************************************************
// Variables
//******************************************************************************
static char status_text[256];

//******************************************************************************
// Prototypes
//******************************************************************************

static void callback_bar_draw(void *monitor, void *storage);

//******************************************************************************
// Function definitions
//******************************************************************************

enum bar_zone
bar_x_to_zone(int x)
{
	return BAR_ZONE_NONE;
}

void bar_draw(struct Monitor *m)
{
}

void bar_draw_all_mons(struct list *mons)
{
	list_run_for_all(mons, callback_bar_draw, NULL);
}

void bar_status_update(void)
{
	if (!dm_gettextprop(root, XA_WM_NAME, status_text, sizeof(status_text)))
		strcpy(status_text, "dwm-"VERSION);
	bar_draw(selmon);
}

//******************************************************************************
// Internal functions
//******************************************************************************
static void callback_bar_draw(void *monitor, void *storage)
{
	bar_draw((struct Monitor *) monitor);
}
