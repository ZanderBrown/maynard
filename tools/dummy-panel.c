#include <gtk/gtk.h>

#include "panel.h"
#include "mnd-wallpaper.h"

int
main (int argc, char** argv)
{
  GtkWidget *panel;
  GtkCssProvider *provider;
  GFile *file;
  GError *error = NULL;

  gtk_init (&argc, &argv);

  provider = gtk_css_provider_new ();

  file = g_file_new_for_uri ("resource:///org/raspberry-pi/maynard/style.css");

  if (!gtk_css_provider_load_from_file (provider, file, &error)) {
    g_warning ("Failed to load CSS file: %s", error->message);
    g_clear_error (&error);
    g_object_unref (file);
  }

  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
      GTK_STYLE_PROVIDER (provider), 600);

  g_object_unref (file);

  g_object_set (gtk_settings_get_default (),
                "gtk-application-prefer-dark-theme", TRUE,
                NULL);

  panel = maynard_panel_new ();
  gtk_window_set_default_size (GTK_WINDOW (panel), 700, 32);
  gtk_widget_show_all (panel);

  panel = mnd_wallpaper_new ();
  gtk_window_set_default_size (GTK_WINDOW (panel), 700, 400);
  gtk_widget_show (panel);

  gtk_main ();

  return 0;
}