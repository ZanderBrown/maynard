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

#include "panel.h"

#include "app-icon.h"
#include "favorites.h"
#include "launcher.h"
#include "clock.h"
#include "sound.h"
#include "vertical-clock.h"

enum {
  APP_MENU_TOGGLED,
  SYSTEM_TOGGLED,
  VOLUME_TOGGLED,
  FAVORITE_LAUNCHED,
  N_SIGNALS
};
static guint signals[N_SIGNALS] = { 0 };

struct MaynardPanelPrivate {
  gboolean hidden;

  GtkWidget *system_button;

  gboolean volume_showing;
  GtkWidget *volume_button;
  gchar *volume_icon_name;
};

G_DEFINE_TYPE_WITH_PRIVATE(MaynardPanel, maynard_panel, GTK_TYPE_WINDOW)

static void
maynard_panel_init (MaynardPanel *self)
{
  self->priv = maynard_panel_get_instance_private (self);

  self->priv->volume_icon_name = g_strdup ("audio-volume-high-symbolic");
}

static gboolean
widget_enter_notify_event_cb (GtkWidget *widget,
    GdkEventCrossing *event,
    MaynardPanel *self)
{
  gboolean handled;
  g_signal_emit_by_name (self, "enter-notify-event", event, &handled);
  return handled;
}

static void
app_menu_button_clicked_cb (GtkButton *button,
    MaynardPanel *self)
{
  g_signal_emit (self, signals[APP_MENU_TOGGLED], 0);
}

static void
favorite_launched_cb (MaynardFavorites *favorites,
    MaynardPanel *self)
{
  g_signal_emit (self, signals[FAVORITE_LAUNCHED], 0);
}

static void
maynard_panel_constructed (GObject *object)
{
  MaynardPanel *self = MAYNARD_PANEL (object);
  GtkWidget *main_box, *menu_box, *buttons_box;
  GtkWidget *image;
  GtkWidget *button;
  GtkWidget *favorites;
  GtkWidget *widget;
  GtkWidget *popover;

  G_OBJECT_CLASS (maynard_panel_parent_class)->constructed (object);

  /* window properties */
  gtk_window_set_title (GTK_WINDOW (self), "maynard");
  gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
  gtk_widget_realize(GTK_WIDGET (self));

  /* make it black and slightly alpha */
  gtk_style_context_add_class (
      gtk_widget_get_style_context (GTK_WIDGET (self)),
      "maynard-panel");

  /* main vbox */
  main_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (self), main_box);

  /* for the top buttons and vertical clock we have a few more
   * boxes. the hbox has two cells. in each cell there is a
   * GtkRevealer for hiding and showing the content. only one revealer
   * is ever visibile at one point and transitions happen at the same
   * time so the width stays constant (the animation duration is the
   * same). the first revealer contains another box which has the two
   * wifi and sound buttons. the second revealer has the vertical
   * clock widget.
   */

  /* bottom app menu button */
  button = gtk_menu_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  widget = gtk_image_new_from_icon_name ("view-grid-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_image_set_pixel_size (GTK_IMAGE (widget), 24);
  gtk_button_set_image (GTK_BUTTON (button), widget);
  gtk_style_context_add_class (gtk_widget_get_style_context (button),
      "maynard-apps");
  gtk_style_context_remove_class (gtk_widget_get_style_context (button),
      "button");
  gtk_style_context_remove_class (gtk_widget_get_style_context (button),
      "image-button");
  popover = maynard_launcher_new (button);
  gtk_popover_set_constrain_to (GTK_POPOVER (popover), GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), popover);
  //g_signal_connect (button, "clicked",
  //    G_CALLBACK (app_menu_button_clicked_cb), self);
  gtk_container_add (GTK_CONTAINER (main_box), button);

  /* favorites */
  favorites = maynard_favorites_new ();
  gtk_container_add (GTK_CONTAINER (main_box), favorites);
  gtk_widget_set_hexpand (favorites, TRUE);

  g_signal_connect (favorites, "app-launched",
      G_CALLBACK (favorite_launched_cb), self);

  menu_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (main_box), menu_box);

  /* the box for the top buttons */
  buttons_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (main_box), buttons_box);

  self->priv->volume_button = gtk_menu_button_new ();
  gtk_style_context_add_class (gtk_widget_get_style_context (self->priv->volume_button),
      "maynard-audio");
  gtk_style_context_remove_class (gtk_widget_get_style_context (self->priv->volume_button),
      "button");
  gtk_style_context_remove_class (gtk_widget_get_style_context (self->priv->volume_button),
      "image-button");
  gtk_container_add (GTK_CONTAINER (buttons_box), self->priv->volume_button);
  gtk_widget_show (self->priv->volume_button);
  widget = gtk_popover_new (self->priv->volume_button);
  gtk_popover_set_constrain_to (GTK_POPOVER (widget), GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (self->priv->volume_button), widget);
  button = maynard_sound_new ();
  gtk_widget_show_all (button);
  gtk_container_add (GTK_CONTAINER (widget), button);

  /* system button */
  button = gtk_menu_button_new ();
  gtk_style_context_add_class (gtk_widget_get_style_context (button),
      "maynard-system");
  gtk_style_context_remove_class (gtk_widget_get_style_context (button),
      "button");
  gtk_style_context_remove_class (gtk_widget_get_style_context (button),
      "image-button");
  gtk_container_add (GTK_CONTAINER (buttons_box), button);
  self->priv->system_button = button;
  widget = gtk_popover_new (button);
  gtk_popover_set_constrain_to (GTK_POPOVER (widget), GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), widget);
  button = maynard_clock_new ();
  gtk_widget_show_all (button);
  gtk_container_add (GTK_CONTAINER (widget), button);
  button = maynard_vertical_clock_new ();
  gtk_widget_show_all (button);
  gtk_container_add (GTK_CONTAINER (self->priv->system_button), button);

  /* end of the menu buttons and vertical clock */

  /* done */
  self->priv->hidden = FALSE;
  self->priv->volume_showing = FALSE;
}

static void
maynard_panel_dispose (GObject *object)
{
  MaynardPanel *self = MAYNARD_PANEL (object);

  g_free (self->priv->volume_icon_name);
  self->priv->volume_icon_name = NULL;

  G_OBJECT_CLASS (maynard_panel_parent_class)->dispose (object);
}

static void
maynard_panel_class_init (MaynardPanelClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
  GParamSpec *param_spec;

  object_class->constructed = maynard_panel_constructed;
  object_class->dispose = maynard_panel_dispose;

  signals[APP_MENU_TOGGLED] = g_signal_new ("app-menu-toggled",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);

  signals[SYSTEM_TOGGLED] = g_signal_new ("system-toggled",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);

  signals[VOLUME_TOGGLED] = g_signal_new ("volume-toggled",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);

  signals[FAVORITE_LAUNCHED] = g_signal_new ("favorite-launched",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);
}

GtkWidget *
maynard_panel_new (void)
{
  return g_object_new (MAYNARD_PANEL_TYPE,
      NULL);
}

static void
set_icon (GtkWidget *button,
    const gchar *icon_name)
{
  GtkWidget *image;

  image = gtk_image_new_from_icon_name (icon_name,
      GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_button_set_image (GTK_BUTTON (button),
      image);
}

void
maynard_panel_set_volume_icon_name (MaynardPanel *self,
    const gchar *icon_name)
{
  g_free (self->priv->volume_icon_name);
  self->priv->volume_icon_name = g_strdup (icon_name);

  set_icon (self->priv->volume_button, icon_name);
}
