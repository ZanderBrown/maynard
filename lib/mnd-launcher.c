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
#include "mnd-app-launcher.h"

#include "gtk-list-models/gtksortlistmodel.h"
#include "gtk-list-models/gtkfilterlistmodel.h"

typedef struct _MndLauncherPrivate MndLauncherPrivate;
struct _MndLauncherPrivate {
  GtkSortListModel   *model;
  GtkFilterListModel *filter;

  GtkWidget *entry;
  GtkWidget *scrolled_window;
  GtkWidget *grid;
};

G_DEFINE_TYPE_WITH_PRIVATE(MndLauncher, mnd_launcher, GTK_TYPE_POPOVER)

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
search_apps (gpointer item, gpointer data)
{
  MndLauncher *self = data;
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);
  GAppInfo *info = item;
  const char *search = gtk_entry_get_text (GTK_ENTRY (priv->entry));
  const char *str;

  if ((str = g_app_info_get_display_name (info)) && strstr (str, search))
    return TRUE;

  if ((str = g_app_info_get_name (info)) && strstr (str, search))
    return TRUE;

  if ((str = g_app_info_get_description (info)) && strstr (str, search))
    return TRUE;

  if ((str = g_app_info_get_executable (info)) && strstr (str, search))
    return TRUE;

  if (G_IS_DESKTOP_APP_INFO (info)) {
    const char * const *kwds;
    int i = 0;

    if ((str = g_desktop_app_info_get_generic_name (G_DESKTOP_APP_INFO (info))) &&
         strstr (str, search))
      return TRUE;

    if ((str = g_desktop_app_info_get_categories (G_DESKTOP_APP_INFO (info))) &&
         strstr (str, search))
      return TRUE;

    kwds = g_desktop_app_info_get_keywords (G_DESKTOP_APP_INFO (info));

    if (kwds) {
      while ((str = kwds[i])) {
        if (strstr (str, search))
          return TRUE;
        i++;
      }
    }
  }

  return FALSE;
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
app_launched (MndAppLauncher *launcher,
              MndLauncher    *self)
{
  gtk_popover_popdown (GTK_POPOVER (self));

  /* do this in an idle so it's not done so obviously onscreen */
  g_idle_add (app_launched_idle_cb, self);
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
  GAppInfo *info;
  guint left, top;

  /* remove all children first */
  gtk_container_foreach (GTK_CONTAINER (priv->grid),
      (GtkCallback) gtk_widget_destroy, NULL);

  left = top = 0;
  while ((info = g_list_model_get_item (G_LIST_MODEL (model), i))) {
    GtkWidget *app = mnd_app_launcher_new (info);

    g_signal_connect (app, "launched", G_CALLBACK (app_launched), self);

    gtk_grid_attach (GTK_GRID (priv->grid), app, left++, top, 1, 1);

    if (left > 5) {
      left = 0;
      top++;
    }
    i++;
  }

  gtk_widget_show_all (priv->grid);
}

static void
mnd_launcher_init (MndLauncher *self)
{
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  /* fill the grid with apps */
  priv->model = gtk_sort_list_model_new (G_LIST_MODEL (mnd_app_list_model_get_default ()),
                                         sort_apps,
                                         NULL,
                                         NULL);
  priv->filter = gtk_filter_list_model_new (G_LIST_MODEL (priv->model),
                                            search_apps,
                                            self,
                                            NULL);
  g_signal_connect (priv->filter,
                    "items-changed",
                    G_CALLBACK (installed_changed_cb),
                    self);

  /* now actually fill the grid */
  installed_changed_cb (G_LIST_MODEL (priv->model), 0, 0, 0, self);
}

static void
mnd_launcher_finalize (GObject *object)
{
  MndLauncher *self = MND_LAUNCHER (object);
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);

  g_clear_object (&priv->model);
  g_clear_object (&priv->filter);

  G_OBJECT_CLASS (mnd_launcher_parent_class)->finalize (object);
}

static gboolean
mnd_launcher_key_press_event (GtkWidget   *widget,
                              GdkEventKey *event)
{
  MndLauncher *self = MND_LAUNCHER (widget);
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);

  return gtk_search_entry_handle_event (GTK_SEARCH_ENTRY (priv->entry), 
                                        (GdkEvent *) event);
}

static void
search_changed (GtkSearchEntry *entry,
                MndLauncher    *self)
{
  MndLauncherPrivate *priv = mnd_launcher_get_instance_private (self);

  gtk_filter_list_model_refilter (priv->filter);
}

static void
mnd_launcher_class_init (MndLauncherClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = mnd_launcher_finalize;

  widget_class->key_press_event = mnd_launcher_key_press_event;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/raspberry-pi/maynard/mnd-launcher.ui");

  gtk_widget_class_bind_template_child_private (widget_class, MndLauncher, entry);
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
