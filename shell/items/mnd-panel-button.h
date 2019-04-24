#include <gtk/gtk.h>

#pragma once

G_BEGIN_DECLS

#define MND_TYPE_PANEL_BUTTON mnd_panel_button_get_type()
G_DECLARE_DERIVABLE_TYPE (MndPanelButton, mnd_panel_button, MND, PANEL_BUTTON, GtkMenuButton)

struct _MndPanelButtonClass {
  GtkMenuButtonClass parent;
};

GtkWidget *mnd_panel_button_new ();

G_END_DECLS
