#include <gtk/gtk.h>

#include "items/mnd-panel-button.h"

#pragma once

G_BEGIN_DECLS

struct _MndClock {
  MndPanelButton parent;
};

#define MND_TYPE_CLOCK mnd_clock_get_type()
G_DECLARE_FINAL_TYPE (MndClock, mnd_clock, MND, CLOCK, MndPanelButton)

GtkWidget *mnd_clock_new ();

G_END_DECLS
