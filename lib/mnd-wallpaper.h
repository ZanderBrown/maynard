#include <gtk/gtk.h>

#pragma once

G_BEGIN_DECLS

#define MND_TYPE_WALLPAPER mnd_wallpaper_get_type()
G_DECLARE_DERIVABLE_TYPE (MndWallpaper, mnd_wallpaper, MND, WALLPAPER, GtkWindow)

struct _MndWallpaperClass
{
  GtkWindowClass parent_class;
};

GtkWidget *mnd_wallpaper_new ();

G_END_DECLS
