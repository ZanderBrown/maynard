#include "items/mnd-panel-button.h"

G_DEFINE_TYPE (MndPanelButton, mnd_panel_button, GTK_TYPE_MENU_BUTTON)

static void
mnd_panel_button_init (MndPanelButton *self)
{
}

static void
mnd_panel_button_class_init (MndPanelButtonClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_css_name (widget_class, "panel-button");
}

GtkWidget *
mnd_panel_button_new ()
{
  return g_object_new (MND_TYPE_PANEL_BUTTON, NULL);
}
