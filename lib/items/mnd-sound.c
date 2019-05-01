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

#include <alsa/asoundlib.h>
#include <pulse/pulseaudio.h>
#include <gvc-mixer-control.h>
#include <glib/gi18n-lib.h>

#include "items/mnd-sound.h"

enum {
  VOLUME_CHANGED,
  N_SIGNALS
};
static guint signals[N_SIGNALS] = { 0 };

typedef struct _MndSoundPrivate MndSoundPrivate;
struct _MndSoundPrivate {
  GtkWidget *image;
};

G_DEFINE_TYPE_WITH_PRIVATE(MndSound, mnd_sound, MND_TYPE_PANEL_BUTTON)

static void
mnd_sound_init (MndSound *self)
{
  MndSoundPrivate *priv = mnd_sound_get_instance_private (self);
  GtkWidget *widget;

  priv->image = gtk_image_new_from_icon_name ("audio-volume-muted-symbolic",
                                              GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (priv->image);
  gtk_container_add (GTK_CONTAINER (self), priv->image);

  widget = mnd_sound_popover_new (self);
  gtk_popover_set_constrain_to (GTK_POPOVER (widget), GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (self), widget);
}

static void
mnd_sound_class_init (MndSoundClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
}

GtkWidget *
mnd_sound_new (void)
{
  return g_object_new (MND_TYPE_SOUND, NULL);
}
