/*
 * Copyright (C) 2014 Collabora Ltd.
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
 * Author: Jonny Lamb <jonny.lamb@collabora.co.uk>
 */

#include "config.h"

#include "mnd-favorites-button.h"

typedef struct _MndFavoritesButtonPrivate MndFavoritesButtonPrivate;
struct _MndFavoritesButtonPrivate {
  GAppInfo  *app;
  GtkWidget *icon;
};

G_DEFINE_TYPE_WITH_PRIVATE (MndFavoritesButton, mnd_favorites_button, MND_TYPE_PANEL_BUTTON)

enum {
  PROP_0,
  PROP_APP,
  LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = { NULL, };

static void
mnd_favorites_button_init (MndFavoritesButton *self)
{
  MndFavoritesButtonPrivate *priv = mnd_favorites_button_get_instance_private (self);

  priv->app = NULL;

  priv->icon = gtk_image_new ();
  gtk_image_set_pixel_size (GTK_IMAGE (priv->icon), 16);
  gtk_container_add (GTK_CONTAINER (self), priv->icon);
  gtk_widget_show (priv->icon);
}

static void
mnd_favorites_button_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  MndFavoritesButton *self = MND_FAVORITES_BUTTON (object);
  MndFavoritesButtonPrivate *priv = mnd_favorites_button_get_instance_private (self);
  GIcon *icon;

  switch (property_id) {
    case PROP_APP:
      g_clear_object (&priv->app);
      priv->app = g_value_get_object (value);

      icon = g_app_info_get_icon (priv->app);
      gtk_image_set_from_gicon (GTK_IMAGE (priv->icon), icon, GTK_ICON_SIZE_BUTTON);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
mnd_favorites_button_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  MndFavoritesButton *self = MND_FAVORITES_BUTTON (object);
  MndFavoritesButtonPrivate *priv = mnd_favorites_button_get_instance_private (self);

  switch (property_id) {
    case PROP_APP:
      g_value_set_object (value, priv->app);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
mnd_favorites_button_clicked (GtkButton *button)
{
  MndFavoritesButton *self = MND_FAVORITES_BUTTON (button);
  MndFavoritesButtonPrivate *priv = mnd_favorites_button_get_instance_private (self);
  GError *error = NULL;

  g_app_info_launch (priv->app, NULL, NULL, &error);
  if (error) {
    g_warning ("Could not launch app %s: %s",
          g_app_info_get_name (priv->app),
          error->message);
    g_clear_error (&error);
  }
}

static void
mnd_favorites_button_class_init (MndFavoritesButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);

  object_class->set_property = mnd_favorites_button_set_property;
  object_class->get_property = mnd_favorites_button_get_property;

  pspecs[PROP_APP] =
    g_param_spec_object ("app", "App", "App Info",
                         G_TYPE_APP_INFO,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, pspecs);

  button_class->clicked = mnd_favorites_button_clicked;
}

GtkWidget *
mnd_favorites_button_new (GAppInfo *info)
{
  return g_object_new (MND_TYPE_FAVORITES_BUTTON,
                       "app", info,
                       "sensitive", TRUE,
                       NULL);
}
