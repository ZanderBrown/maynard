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

#include "clock.h"

#include "items/mnd-panel-button.h"
#include "items/mnd-power.h"
#include "items/mnd-clock.h"
#include "items/mnd-sound.h"

#include "mnd-launcher.h"
#include "mnd-favorites.h"

G_DEFINE_TYPE (MaynardPanel, maynard_panel, GTK_TYPE_WINDOW)

static void
maynard_panel_init (MaynardPanel *self)
{
}

static void
maynard_panel_constructed (GObject *object)
{
  MaynardPanel *self = MAYNARD_PANEL (object);
  GtkWidget *main_box, *menu_box, *buttons_box;
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
  button = mnd_panel_button_new ();
  widget = gtk_image_new_from_icon_name ("view-grid-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE (widget), 16);
  gtk_container_add (GTK_CONTAINER (button), widget);
  popover = mnd_launcher_new (button);
  gtk_popover_set_constrain_to (GTK_POPOVER (popover), GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), popover);
  gtk_container_add (GTK_CONTAINER (main_box), button);

  /* favorites */
  favorites = mnd_favorites_new ();
  gtk_container_add (GTK_CONTAINER (main_box), favorites);
  gtk_widget_set_hexpand (favorites, TRUE);

  menu_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (main_box), menu_box);

  /* the box for the top buttons */
  buttons_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (main_box), buttons_box);

  /* Power status */
  button = mnd_power_new ();
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (buttons_box), button);

  button = mnd_sound_new ();
  gtk_widget_show_all (button);
  gtk_container_add (GTK_CONTAINER (buttons_box), button);

  /* system button */
  button = mnd_clock_new ();
  gtk_container_add (GTK_CONTAINER (buttons_box), button);
  widget = gtk_popover_new (button);
  gtk_popover_set_constrain_to (GTK_POPOVER (widget), GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), widget);
  button = maynard_clock_new ();
  gtk_widget_show_all (button);
  gtk_container_add (GTK_CONTAINER (widget), button);

  /* end of the menu buttons and vertical clock */
}

static void
maynard_panel_class_init (MaynardPanelClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->constructed = maynard_panel_constructed;
}

GtkWidget *
maynard_panel_new (void)
{
  return g_object_new (MAYNARD_PANEL_TYPE,
      NULL);
}
