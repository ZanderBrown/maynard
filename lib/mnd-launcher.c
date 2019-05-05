/*
 * Copyright (C) 2013-2014 Collabora Ltd.
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
 * Authors: Emilio Pozuelo Monfort <emilio.pozuelo@collabora.co.uk>
 *          Jonny Lamb <jonny.lamb@collabora.co.uk>
 */

#include "config.h"

#include "mnd-launcher.h"

#include "clock.h"
#include "panel.h"
#include "mnd-app-list-model.h"

#include "gtk-list-models/gtksortlistmodel.h"
#include "gtk-list-models/gtkfilterlistmodel.h"

typedef struct _MndLauncherPrivate MndLauncherPrivate;
struct _MndLauncherPrivate {
  GtkSortListModel   *model;
  GtkFilterListModel *filter;

  GtkWidget *scrolled_window;
  GtkWidget *grid;
};

G_DEFINE_TYPE_WITH_PRIVATE(MndLauncher, mnd_launcher, GTK_TYPE_POPOVER)

/* each grid item is 114x114 */
#define GRID_ITEM_WIDTH 114
#define GRID_ITEM_HEIGHT 114

static void
mnd_launcher_init (MndLauncher *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static gint
sort_apps (gconstpointer a,
           gconstpointer b,
           gpointer      data)
{
  GAppInfo *info1 = G_APP_INFO (a);
  GAppInfo *info2 = G_APP_INFO (b);
  gchar *s1, *s2;
  gint ret;

  s1 = g_utf8_casefold (g_app_info_get_display_name (info1), -1);
  s2 = g_utf8_casefold (g_app_info_get_display_name (info2), -1);

  ret = g_strcmp0 (s1, s2);

  g_free (s1);
  g_free (s2);

  return ret;
}

static gboolean
get_child_position_cb (GtkOverlay *overlay,
    GtkWidget *widget,
    GdkRectangle *allocation,
    gpointer user_data)
{
  GtkOverlayClass *klass = GTK_OVERLAY_GET_CLASS (overlay);

  klass->get_child_position (overlay, widget, allocation);

  /* use the same valign and halign properties, but given we have a
   * border of 1px, respect it and don't draw the overlay widget over
   * the border. */
  allocation->x += 1;
  allocation->y -= 1;
  allocation->width -= 2;

  return TRUE;
}

static gboolean
app_launched_idle_cb (gpointer data)
{
  MndLauncher *self = data;
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);
  GtkAdjustment *adjustment;

  /* make the scrolled window go back to the top */

  adjustment = gtk_scrolled_window_get_vadjustment (
      GTK_SCROLLED_WINDOW (priv->scrolled_window));

  gtk_adjustment_set_value (adjustment, 0.0);

  return G_SOURCE_REMOVE;
}

static void
clicked_cb (GtkWidget *widget,
    GDesktopAppInfo *info)
{
  MndLauncher *self;

  g_app_info_launch (G_APP_INFO (info), NULL, NULL, NULL);

  self = g_object_get_data (G_OBJECT (widget), "launcher");
  g_assert (self);

  gtk_popover_popdown (GTK_POPOVER (self));

  /* do this in an idle so it's not done so obviously onscreen */
  g_idle_add (app_launched_idle_cb, self);
}

static gboolean
app_enter_cb (GtkWidget *widget,
    GdkEvent *event,
    GtkWidget *revealer)
{
  gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), TRUE);
  return FALSE;
}

static gboolean
app_leave_cb (GtkWidget *widget,
    GdkEvent *event,
    GtkWidget *revealer)
{
  gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), FALSE);
  return FALSE;
}

static GtkWidget *
app_launcher_new_from_desktop_info (MndLauncher *self,
    GDesktopAppInfo *info)
{
  GIcon *icon;
  GtkWidget *image;
  GtkWidget *button;
  GtkWidget *overlay;
  GtkWidget *revealer;
  GtkWidget *label;
  GtkWidget *ebox;

  /* we need an ebox to catch enter and leave events */
  ebox = gtk_event_box_new ();
  gtk_style_context_add_class (gtk_widget_get_style_context (ebox),
      "maynard-grid-item");

  /* we use an overlay so we can have the app icon showing but use a
   * GtkRevealer to show a label of the app's name. */
  overlay = gtk_overlay_new ();
  gtk_container_add (GTK_CONTAINER (ebox), overlay);

  /* ...but each item has a border of 1px and we don't want the
   * revealer to paint into the border, so overload this function to
   * know where to put it. */
  g_signal_connect (overlay, "get-child-position",
      G_CALLBACK (get_child_position_cb), NULL);

  revealer = gtk_revealer_new ();
  g_object_set (revealer,
      "halign", GTK_ALIGN_FILL, /* all the width */
      "valign", GTK_ALIGN_END, /* only at the bottom */
      NULL);
  gtk_revealer_set_transition_type (GTK_REVEALER (revealer),
      GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
  gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), FALSE);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), revealer);

  /* app name */
  label = gtk_label_new (g_app_info_get_display_name (G_APP_INFO (info)));
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_style_context_add_class (gtk_widget_get_style_context (label), "maynard-grid-label");
  gtk_container_add (GTK_CONTAINER (revealer), label);

  /* icon button to load the app */
  icon = g_app_info_get_icon (G_APP_INFO (info));
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_DIALOG);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 48);
  button = gtk_button_new ();
  gtk_style_context_remove_class (
      gtk_widget_get_style_context (button),
      "button");
  gtk_style_context_remove_class (
      gtk_widget_get_style_context (button),
      "image-button");
  gtk_button_set_image (GTK_BUTTON (button), image);
  g_object_set (image,
      "margin", 30,
      NULL);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (overlay), button);

  /* TODO: a bit ugly */
  g_object_set_data (G_OBJECT (button), "launcher", self);
  g_signal_connect (button, "clicked", G_CALLBACK (clicked_cb), info);

  /* now we have set everything up, we can refernce the ebox and the
   * revealer. enter will show the label and leave will hide the label. */
  g_signal_connect (ebox, "enter-notify-event", G_CALLBACK (app_enter_cb), revealer);
  g_signal_connect (ebox, "leave-notify-event", G_CALLBACK (app_leave_cb), revealer);

  return ebox;
}

static void
installed_changed_cb (GListModel     *model,
                      guint           position,
                      guint           removed,
                      guint           added,
                      MndLauncher    *self)
{
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);
  int i = 0;
  GDesktopAppInfo *info;
  guint left, top;

  /* remove all children first */
  gtk_container_foreach (GTK_CONTAINER (priv->grid),
      (GtkCallback) gtk_widget_destroy, NULL);

  left = top = 0;
  while ((info = g_list_model_get_item (G_LIST_MODEL (model), i)))
    {
      GtkWidget *app = app_launcher_new_from_desktop_info (self, info);

      gtk_grid_attach (GTK_GRID (priv->grid), app, left++, top, 1, 1);

      if (left > 5)
        {
          left = 0;
          top++;
        }
      i++;
    }

  gtk_widget_show_all (priv->grid);
}

static void
mnd_launcher_constructed (GObject *object)
{
  MndLauncher *self = MND_LAUNCHER (object);
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);

  G_OBJECT_CLASS (mnd_launcher_parent_class)->constructed (object);

  /* fill the grid with apps */
  priv->model = gtk_sort_list_model_new (G_LIST_MODEL (mnd_app_list_model_get_default ()),
                                         sort_apps,
                                         NULL,
                                         NULL);
  g_signal_connect (priv->model,
                    "items-changed",
                    G_CALLBACK (installed_changed_cb),
                    self);

  /* now actually fill the grid */
  installed_changed_cb (G_LIST_MODEL (priv->model), 0, 0, 0, self);
}

static void
search_changed (GtkSearchEntry *entry,
                MndLauncher    *self)
{

}

static void
mnd_launcher_class_init (MndLauncherClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = mnd_launcher_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/raspberry-pi/maynard/mnd-launcher.ui");

  gtk_widget_class_bind_template_child_private (widget_class, MndLauncher, grid);
  gtk_widget_class_bind_template_child_private (widget_class, MndLauncher, scrolled_window);

  gtk_widget_class_bind_template_callback (widget_class, search_changed);
}

GtkWidget *
mnd_launcher_new (GtkWidget *background_widget)
{
  return g_object_new (MND_TYPE_LAUNCHER,
                       "relative-to", background_widget,
                       "height-request", 400,
                       NULL);
}
