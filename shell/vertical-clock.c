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

#include "vertical-clock.h"

#define GNOME_DESKTOP_USE_UNSTABLE_API
#include <libgnome-desktop/gnome-wall-clock.h>

#include "panel.h"

struct MaynardVerticalClockPrivate {
  GtkWidget *label;

  GnomeWallClock *wall_clock;
};

G_DEFINE_TYPE_WITH_PRIVATE(MaynardVerticalClock, maynard_vertical_clock, GTK_TYPE_BOX)

/* this widget takes up the entire width of the panel and displays
 * padding for the first (panel width - vertical clock width) pixels,
 * then shows the vertical clock itself. the idea is to put this into
 * a GtkRevealer and only show it when appropriate. */

static void
maynard_vertical_clock_init (MaynardVerticalClock *self)
{
  self->priv = maynard_vertical_clock_get_instance_private (self);
}

static void
wall_clock_notify_cb (GnomeWallClock *wall_clock,
    GParamSpec *pspec,
    MaynardVerticalClock *self)
{
  GDateTime *datetime;
  gchar *str;

  datetime = g_date_time_new_now_local ();

  str = g_date_time_format (datetime, "<span font=\"Droid Sans 12\">%H:%M</span>");
  gtk_label_set_markup (GTK_LABEL (self->priv->label), str);

  g_free (str);
  g_date_time_unref (datetime);
}

static void
maynard_vertical_clock_constructed (GObject *object)
{
  MaynardVerticalClock *self = MAYNARD_VERTICAL_CLOCK (object);

  G_OBJECT_CLASS (maynard_vertical_clock_parent_class)->constructed (object);

  self->priv->wall_clock = g_object_new (GNOME_TYPE_WALL_CLOCK, NULL);
  g_signal_connect (self->priv->wall_clock, "notify::clock",
      G_CALLBACK (wall_clock_notify_cb), self);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);

  /* the actual clock label */
  self->priv->label = gtk_label_new ("");
  gtk_label_set_justify (GTK_LABEL (self->priv->label), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start (GTK_BOX (self), self->priv->label, FALSE, FALSE, 0);

  wall_clock_notify_cb (self->priv->wall_clock, NULL, self);
}

static void
maynard_vertical_clock_dispose (GObject *object)
{
  MaynardVerticalClock *self = MAYNARD_VERTICAL_CLOCK (object);

  G_OBJECT_CLASS (maynard_vertical_clock_parent_class)->dispose (object);
}

static void
maynard_vertical_clock_class_init (MaynardVerticalClockClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->constructed = maynard_vertical_clock_constructed;
  object_class->dispose = maynard_vertical_clock_dispose;
}

GtkWidget *
maynard_vertical_clock_new (void)
{
  return g_object_new (MAYNARD_VERTICAL_CLOCK_TYPE,
      NULL);
}
