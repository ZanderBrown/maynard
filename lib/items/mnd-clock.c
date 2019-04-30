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

#include "mnd-clock.h"

#define GNOME_DESKTOP_USE_UNSTABLE_API
#include <libgnome-desktop/gnome-wall-clock.h>

#include "panel.h"

typedef struct _MndClockPrivate MndClockPrivate;
struct _MndClockPrivate {
  GtkWidget *label;

  GnomeWallClock *wall_clock;
};

G_DEFINE_TYPE_WITH_PRIVATE (MndClock, mnd_clock, MND_TYPE_PANEL_BUTTON)

/* this widget takes up the entire width of the panel and displays
 * padding for the first (panel width - vertical clock width) pixels,
 * then shows the vertical clock itself. the idea is to put this into
 * a GtkRevealer and only show it when appropriate. */

static void
mnd_clock_init (MndClock *self)
{
}

static void
wall_clock_notify_cb (GnomeWallClock *wall_clock,
    GParamSpec *pspec,
    MndClock *self)
{
  MndClockPrivate *priv = mnd_clock_get_instance_private (self);
  GDateTime *datetime;
  gchar *str;

  datetime = g_date_time_new_now_local ();

  str = g_date_time_format (datetime, "%H:%M");
  gtk_label_set_markup (GTK_LABEL (priv->label), str);

  g_free (str);
  g_date_time_unref (datetime);
}

static void
mnd_clock_constructed (GObject *object)
{
  MndClock *self = MND_CLOCK (object);
  MndClockPrivate *priv = mnd_clock_get_instance_private (self);

  G_OBJECT_CLASS (mnd_clock_parent_class)->constructed (object);

  priv->wall_clock = g_object_new (GNOME_TYPE_WALL_CLOCK, NULL);
  g_signal_connect (priv->wall_clock, "notify::clock",
      G_CALLBACK (wall_clock_notify_cb), self);

  /* the actual clock label */
  priv->label = gtk_label_new ("");
  gtk_label_set_justify (GTK_LABEL (priv->label), GTK_JUSTIFY_CENTER);
  gtk_container_add (GTK_CONTAINER (self), priv->label);

  wall_clock_notify_cb (priv->wall_clock, NULL, self);
}

static void
mnd_clock_dispose (GObject *object)
{
  MndClock *self = MND_CLOCK (object);
  MndClockPrivate *priv = mnd_clock_get_instance_private (self);

  g_clear_object (&priv->wall_clock);

  G_OBJECT_CLASS (mnd_clock_parent_class)->dispose (object);
}

static void
mnd_clock_class_init (MndClockClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->constructed = mnd_clock_constructed;
  object_class->dispose = mnd_clock_dispose;
}

GtkWidget *
mnd_clock_new (void)
{
  return g_object_new (MND_TYPE_CLOCK, NULL);
}
