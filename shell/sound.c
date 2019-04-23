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

#include "sound.h"

enum {
  VOLUME_CHANGED,
  N_SIGNALS
};
static guint signals[N_SIGNALS] = { 0 };

struct MaynardSoundPrivate {
  GtkWidget *volume_scale;
  GtkWidget *volume_image;

  snd_mixer_t *mixer_handle;
  snd_mixer_elem_t *mixer;
  glong min_volume, max_volume;
};

G_DEFINE_TYPE_WITH_PRIVATE(MaynardSound, maynard_sound, GTK_TYPE_BOX)

static void
maynard_sound_init (MaynardSound *self)
{
  self->priv = maynard_sound_get_instance_private (self);
}

static gdouble
alsa_volume_to_percentage (MaynardSound *self,
    glong value)
{
  glong range;

  /* min volume isn't always zero unfortunately */
  range = self->priv->max_volume - self->priv->min_volume;

  value -= self->priv->min_volume;

  return (value / (gdouble) range) * 100;
}

static glong
percentage_to_alsa_volume (MaynardSound *self,
    gdouble value)
{
  glong range;

  /* min volume isn't always zero unfortunately */
  range = self->priv->max_volume - self->priv->min_volume;

  return (range * value / 100) + self->priv->min_volume;
}

static void
volume_changed_cb (GtkRange *range,
    MaynardSound *self)
{
  gdouble value;
  const gchar *icon_name;
  GtkWidget *box;

  value = gtk_range_get_value (range);

  if (self->priv->mixer != NULL)
    {
      snd_mixer_selem_set_playback_volume_all (self->priv->mixer,
          percentage_to_alsa_volume (self, value));
    }

  /* update the icon */
  if (value > 75)
    icon_name = "audio-volume-high-symbolic";
  else if (value > 30)
    icon_name = "audio-volume-medium-symbolic";
  else if (value > 0)
    icon_name = "audio-volume-low-symbolic";
  else
    icon_name = "audio-volume-muted-symbolic";

  box = gtk_widget_get_parent (self->priv->volume_image);
  gtk_widget_destroy (self->priv->volume_image);
  self->priv->volume_image = gtk_image_new_from_icon_name (
      icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (box), self->priv->volume_image,
      FALSE, FALSE, 0);
  gtk_widget_show (self->priv->volume_image);

  g_signal_emit (self, signals[VOLUME_CHANGED], 0, value, icon_name);
}

static gboolean
volume_idle_cb (gpointer data)
{
  MaynardSound *self = MAYNARD_SOUND (data);
  glong volume;

  if (self->priv->mixer != NULL)
    {
      snd_mixer_selem_get_playback_volume (self->priv->mixer,
          SND_MIXER_SCHN_MONO, &volume);

      gtk_range_set_value (GTK_RANGE (self->priv->volume_scale),
          alsa_volume_to_percentage (self, volume));
    }

  return G_SOURCE_REMOVE;
}

static GtkWidget *
create_volume_box (MaynardSound *self)
{
  GtkWidget *box;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  self->priv->volume_image = gtk_image_new_from_icon_name (
      "audio-volume-muted-symbolic",
      GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (box), self->priv->volume_image,
      FALSE, FALSE, 0);

  self->priv->volume_scale = gtk_scale_new_with_range (
      GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_draw_value (GTK_SCALE (self->priv->volume_scale), FALSE);
  gtk_widget_set_size_request (self->priv->volume_scale, 100, -1);
  gtk_box_pack_end (GTK_BOX (box), self->priv->volume_scale, TRUE, TRUE, 0);

  g_signal_connect (self->priv->volume_scale, "value-changed",
      G_CALLBACK (volume_changed_cb), self);

  /* set the initial value in an idle so ::volume-changed is emitted
   * when other widgets are connected to the signal and can react
   * accordingly. */
  g_idle_add (volume_idle_cb, self);

  return box;
}

static void
setup_mixer (MaynardSound *self)
{
  snd_mixer_selem_id_t *sid;
  gint ret;

  /* this is all pretty specific to the rpi */

  if ((ret = snd_mixer_open (&self->priv->mixer_handle, 0)) < 0)
    goto error;

  if ((ret = snd_mixer_attach (self->priv->mixer_handle, "default")) < 0)
    goto error;

  if ((ret = snd_mixer_selem_register (self->priv->mixer_handle, NULL, NULL)) < 0)
    goto error;

  if ((ret = snd_mixer_load (self->priv->mixer_handle)) < 0)
    goto error;

  snd_mixer_selem_id_alloca (&sid);
  snd_mixer_selem_id_set_index (sid, 0);
  snd_mixer_selem_id_set_name (sid, "PCM");
  self->priv->mixer = snd_mixer_find_selem (self->priv->mixer_handle, sid);

  /* fallback to mixer "Master" */
  if (self->priv->mixer == NULL)
    {
      snd_mixer_selem_id_set_name (sid, "Master");
      self->priv->mixer = snd_mixer_find_selem (self->priv->mixer_handle, sid);
      if (self->priv->mixer == NULL)
        goto error;
    }

  if ((ret = snd_mixer_selem_get_playback_volume_range (self->priv->mixer,
              &self->priv->min_volume, &self->priv->max_volume)) < 0)
    goto error;

  return;

error:
  g_debug ("failed to setup mixer: %s", snd_strerror (ret));

  if (self->priv->mixer_handle != NULL)
    snd_mixer_close (self->priv->mixer_handle);
  self->priv->mixer_handle = NULL;
  self->priv->mixer = NULL;
}

static void
maynard_sound_constructed (GObject *object)
{
  MaynardSound *self = MAYNARD_SOUND (object);
  GtkWidget *box, *volume_box;

  G_OBJECT_CLASS (maynard_sound_parent_class)->constructed (object);

  /* the box for the revealers */
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (self), box);

  /* volume */
  volume_box = create_volume_box (self);
  gtk_container_add (GTK_CONTAINER (box), volume_box);

  setup_mixer (self);
}

static void
maynard_sound_dispose (GObject *object)
{
  MaynardSound *self = MAYNARD_SOUND (object);

  if (self->priv->mixer_handle != NULL)
    snd_mixer_close (self->priv->mixer_handle);
  self->priv->mixer_handle = NULL;
  self->priv->mixer = NULL;

  G_OBJECT_CLASS (maynard_sound_parent_class)->dispose (object);
}

static void
maynard_sound_class_init (MaynardSoundClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->constructed = maynard_sound_constructed;
  object_class->dispose = maynard_sound_dispose;

  signals[VOLUME_CHANGED] = g_signal_new ("volume-changed",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_STRING);
}

GtkWidget *
maynard_sound_new (void)
{
  return g_object_new (MAYNARD_SOUND_TYPE,
      NULL);
}

