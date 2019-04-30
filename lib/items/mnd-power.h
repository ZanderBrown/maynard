#include <gtk/gtk.h>

#include "items/mnd-panel-button.h"

#pragma once

G_BEGIN_DECLS

struct _MndPower {
  MndPanelButton parent;
};

#define MND_TYPE_POWER mnd_power_get_type()
G_DECLARE_FINAL_TYPE (MndPower, mnd_power, MND, POWER, MndPanelButton)

GtkWidget *mnd_power_new ();

G_END_DECLS
