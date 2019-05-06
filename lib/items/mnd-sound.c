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

#include <glib/gi18n-lib.h>

#include "items/mnd-sound.h"
#include "items/mnd-sound-popover.h"

typedef struct _MndSoundPrivate MndSoundPrivate;
struct _MndSoundPrivate {
  GtkWidget *image;
  GtkWidget *popover;
};

G_DEFINE_TYPE_WITH_PRIVATE(MndSound, mnd_sound, MND_TYPE_PANEL_BUTTON)

static void
icon_changed (GObject    *object,
              GParamSpec *pspec,
              MndSound   *self)
{
  MndSoundPrivate *priv = mnd_sound_get_instance_private (self);
  gchar *icon_name;

  g_object_get (priv->popover,
                "icon-name", &icon_name,
                NULL);

  gtk_image_set_from_icon_name (GTK_IMAGE (priv->image),
                                icon_name,
                                GTK_ICON_SIZE_BUTTON);
}

static void
mnd_sound_init (MndSound *self)
{
  MndSoundPrivate *priv = mnd_sound_get_instance_private (self);
  GtkWidget *widget;

  priv->image = gtk_image_new_from_icon_name ("audio-volume-muted-symbolic",
                                              GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (priv->image);
  gtk_container_add (GTK_CONTAINER (self), priv->image);

  priv->popover = mnd_sound_popover_new (GTK_WIDGET (self));
  g_signal_connect (priv->popover,
                    "notify::icon-name",
                    G_CALLBACK (icon_changed),
                    self);
  gtk_popover_set_constrain_to (GTK_POPOVER (priv->popover),
                                GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (self), priv->popover);

  icon_changed (priv->popover, NULL, self);
}

static void
mnd_sound_class_init (MndSoundClass *klass)
{
}

GtkWidget *
mnd_sound_new (void)
{
  return g_object_new (MND_TYPE_SOUND, NULL);
}
