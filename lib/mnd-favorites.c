/*
 * Copyright (C) 2013 Collabora Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * Author: Emilio Pozuelo Monfort <emilio.pozuelo@collabora.co.uk>
 */

#include "config.h"

#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>

#include "mnd-favorites.h"
#include "mnd-favorites-button.h"

typedef struct _MndFavoritesPrivate MndFavoritesPrivate;
struct _MndFavoritesPrivate {
  GSettings *settings;
};

G_DEFINE_TYPE_WITH_PRIVATE (MndFavorites, mnd_favorites, GTK_TYPE_BOX)

static void
add_favorite (MndFavorites *self,
    const gchar *favorite)
{
  GDesktopAppInfo *info;
  GtkWidget *button;

  info = g_desktop_app_info_new (favorite);

  if (!info)
    return;

  button = mnd_favorites_button_new (G_APP_INFO (info));

  gtk_box_pack_end (GTK_BOX (self), button, FALSE, FALSE, 0);
}

static void
remove_favorite (GtkWidget *favorite,
    gpointer user_data)
{
  gtk_widget_destroy (favorite);
}

static void
favorites_changed (GSettings *settings,
    const gchar *key,
    MndFavorites *self)
{
  gchar **favorites = g_settings_get_strv (settings, key);
  gint i;

  /* Remove all favorites first */
  gtk_container_foreach (GTK_CONTAINER (self), remove_favorite, NULL);

  for (i = 0; i < g_strv_length (favorites); i++)
    {
      gchar *fav = favorites[i];

      add_favorite (self, fav);
    }

  g_strfreev (favorites);
}

static void
mnd_favorites_dispose (GObject *object)
{
  MndFavorites *self = MND_FAVORITES (object);
  MndFavoritesPrivate *priv = mnd_favorites_get_instance_private (self);

  g_clear_object (&priv->settings);

  G_OBJECT_CLASS (mnd_favorites_parent_class)->dispose (object);
}

static void
mnd_favorites_init (MndFavorites *self)
{
  MndFavoritesPrivate *priv = mnd_favorites_get_instance_private (self);

  priv->settings = g_settings_new ("org.raspberrypi.maynard");
  g_signal_connect (priv->settings, "changed::favorites",
                    G_CALLBACK (favorites_changed), self);
  favorites_changed (priv->settings, "favorites", self);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);
}

static void
mnd_favorites_class_init (MndFavoritesClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->dispose = mnd_favorites_dispose;
}

GtkWidget *
mnd_favorites_new (void)
{
  return g_object_new (MND_TYPE_FAVORITES,
                       "orientation", GTK_ORIENTATION_HORIZONTAL,
                       "halign", GTK_ALIGN_START,
                       NULL);
}
