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

#define GNOME_DESKTOP_USE_UNSTABLE_API
#include <libgnome-desktop/gnome-wall-clock.h>

#include "clock.h"

struct MaynardClockPrivate {
  GtkWidget *revealer_clock;
  GtkWidget *label;
  GnomeWallClock *wall_clock;
};

G_DEFINE_TYPE_WITH_PRIVATE(MaynardClock, maynard_clock, GTK_TYPE_BOX)

static void
maynard_clock_init (MaynardClock *self)
{
  self->priv = maynard_clock_get_instance_private (self);
}

static void
wall_clock_notify_cb (GnomeWallClock *wall_clock,
    GParamSpec *pspec,
    MaynardClock *self)
{
  GDateTime *datetime;
  gchar *str;

  datetime = g_date_time_new_now_local ();

  str = g_date_time_format (datetime,
      "<span font=\"Droid Sans 32\">%H:%M</span>\n"
      "<span font=\"Droid Sans 12\">%d/%m/%Y</span>");
  gtk_label_set_markup (GTK_LABEL (self->priv->label), str);

  g_free (str);
  g_date_time_unref (datetime);
}

static void
maynard_clock_constructed (GObject *object)
{
  MaynardClock *self = MAYNARD_CLOCK (object);
  GtkWidget *box;

  G_OBJECT_CLASS (maynard_clock_parent_class)->constructed (object);

  self->priv->wall_clock = g_object_new (GNOME_TYPE_WALL_CLOCK, NULL);
  g_signal_connect (self->priv->wall_clock, "notify::clock",
      G_CALLBACK (wall_clock_notify_cb), self);

  gtk_style_context_add_class (
      gtk_widget_get_style_context (GTK_WIDGET (self)),
      "maynard-clock");

  /* the box for the revealers */
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (self), box);

  /* clock */
  self->priv->revealer_clock = gtk_revealer_new ();
  gtk_revealer_set_transition_type (
      GTK_REVEALER (self->priv->revealer_clock),
      GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
  gtk_revealer_set_reveal_child (
      GTK_REVEALER (self->priv->revealer_clock), TRUE);
  gtk_box_pack_start (GTK_BOX (box), self->priv->revealer_clock,
      TRUE, TRUE, 0);

  self->priv->label = gtk_label_new ("");
  gtk_label_set_justify (GTK_LABEL (self->priv->label), GTK_JUSTIFY_CENTER);
  gtk_container_add (GTK_CONTAINER (self->priv->revealer_clock),
      self->priv->label);

  wall_clock_notify_cb (self->priv->wall_clock, NULL, self);
}

static void
maynard_clock_dispose (GObject *object)
{
  MaynardClock *self = MAYNARD_CLOCK (object);

  g_clear_object (&self->priv->wall_clock);

  G_OBJECT_CLASS (maynard_clock_parent_class)->dispose (object);
}

static void
maynard_clock_class_init (MaynardClockClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->constructed = maynard_clock_constructed;
  object_class->dispose = maynard_clock_dispose;
}

GtkWidget *
maynard_clock_new (void)
{
  return g_object_new (MAYNARD_CLOCK_TYPE, NULL);
}
