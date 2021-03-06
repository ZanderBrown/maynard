/*
 * Copyright (c) 2013 Tiago Vignatti
 * Copyright (c) 2013-2014 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkwayland.h>

#include "weston-desktop-shell-client-protocol.h"

#include "panel.h"
#include "mnd-wallpaper.h"

struct element {
  GtkWidget *window;
  struct wl_surface *surface;
};

struct desktop {
  struct wl_display *display;
  struct wl_registry *registry;
  struct weston_desktop_shell *wshell;
  struct wl_output *output;

  struct wl_seat *seat;

  GdkDisplay *gdk_display;

  struct element *background;
  struct element *panel;
  struct element *curtain;
};

static void
shell_configure (struct desktop *desktop,
    uint32_t edges,
    struct wl_surface *surface,
    int32_t width, int32_t height)
{
  gtk_widget_set_size_request (desktop->background->window,
      width, height);

  gtk_window_resize (GTK_WINDOW (desktop->panel->window),
      width, 32);

  weston_desktop_shell_desktop_ready (desktop->wshell);
}

static void
weston_desktop_shell_configure (void *data,
    struct weston_desktop_shell *weston_desktop_shell,
    uint32_t edges,
    struct wl_surface *surface,
    int32_t width, int32_t height)
{
  shell_configure(data, edges, surface, width, height);
}

static void
weston_desktop_shell_prepare_lock_surface (void *data,
    struct weston_desktop_shell *weston_desktop_shell)
{
  weston_desktop_shell_unlock (weston_desktop_shell);
}

static void
weston_desktop_shell_grab_cursor (void *data,
    struct weston_desktop_shell *weston_desktop_shell,
    uint32_t cursor)
{
}

static const struct weston_desktop_shell_listener wshell_listener = {
  weston_desktop_shell_configure,
  weston_desktop_shell_prepare_lock_surface,
  weston_desktop_shell_grab_cursor
};

static void
panel_create (struct desktop *desktop)
{
  struct element *panel;
  GdkWindow *gdk_window;

  panel = malloc (sizeof *panel);
  memset (panel, 0, sizeof *panel);

  panel->window = maynard_panel_new ();
  gtk_window_set_interactive_debugging (TRUE);

  /* set it up as the panel */
  gdk_window = gtk_widget_get_window (panel->window);
  gdk_wayland_window_set_use_custom_surface (gdk_window);

  panel->surface = gdk_wayland_window_get_wl_surface (gdk_window);

  weston_desktop_shell_set_user_data (desktop->wshell, desktop);
  weston_desktop_shell_set_panel (desktop->wshell, desktop->output,
      panel->surface);
  weston_desktop_shell_set_panel_position (desktop->wshell,
     WESTON_DESKTOP_SHELL_PANEL_POSITION_TOP);

  gtk_widget_show_all (panel->window);

  desktop->panel = panel;
}

static void
background_create (struct desktop *desktop)
{
  GdkWindow *gdk_window;
  struct element *background;

  background = malloc (sizeof *background);
  memset (background, 0, sizeof *background);

  background->window = mnd_wallpaper_new ();

  gdk_window = gtk_widget_get_window (background->window);
  gdk_wayland_window_set_use_custom_surface (gdk_window);

  background->surface = gdk_wayland_window_get_wl_surface (gdk_window);

  weston_desktop_shell_set_user_data (desktop->wshell, desktop);
  weston_desktop_shell_set_background (desktop->wshell, desktop->output,
      background->surface);

  desktop->background = background;

  gtk_widget_show_all (background->window);
}

static void
curtain_create (struct desktop *desktop)
{
  GdkWindow *gdk_window;
  struct element *curtain;

  curtain = malloc (sizeof *curtain);
  memset (curtain, 0, sizeof *curtain);

  curtain->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (curtain->window), "maynard");
  gtk_window_set_decorated (GTK_WINDOW (curtain->window), FALSE);
  gtk_widget_set_size_request (curtain->window, 8192, 8192);
  gtk_widget_realize (curtain->window);

  gdk_window = gtk_widget_get_window (curtain->window);
  gdk_wayland_window_set_use_custom_surface (gdk_window);

  curtain->surface = gdk_wayland_window_get_wl_surface (gdk_window);

  desktop->curtain = curtain;

  gtk_widget_show_all (curtain->window);
}

static void
css_setup (struct desktop *desktop)
{
  GtkCssProvider *provider;
  GFile *file;
  GError *error = NULL;

  provider = gtk_css_provider_new ();

  file = g_file_new_for_uri ("resource:///org/raspberry-pi/maynard/style.css");

  if (!gtk_css_provider_load_from_file (provider, file, &error))
    {
      g_warning ("Failed to load CSS file: %s", error->message);
      g_clear_error (&error);
      g_object_unref (file);
      return;
    }

  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
      GTK_STYLE_PROVIDER (provider), 600);

  g_object_unref (file);
}

static void
seat_handle_capabilities (void *data,
    struct wl_seat *seat,
    enum wl_seat_capability caps)
{
}

static void
seat_handle_name (void *data,
    struct wl_seat *seat,
    const char *name)
{
}

static const struct wl_seat_listener seat_listener = {
  seat_handle_capabilities,
  seat_handle_name
};

static void
registry_handle_global (void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{
  struct desktop *d = data;

  if (!strcmp (interface, "weston_desktop_shell"))
    {
      d->wshell = wl_registry_bind (registry, name,
          &weston_desktop_shell_interface, MIN(version, 1));
      weston_desktop_shell_add_listener (d->wshell, &wshell_listener, d);
      weston_desktop_shell_set_user_data (d->wshell, d);
    }
  else if (!strcmp (interface, "wl_output"))
    {
      /* TODO: create multiple outputs */
      d->output = wl_registry_bind (registry, name,
          &wl_output_interface, 1);
    }
  else if (!strcmp (interface, "wl_seat"))
    {
      d->seat = wl_registry_bind (registry, name,
          &wl_seat_interface, 1);
      wl_seat_add_listener (d->seat, &seat_listener, d);
    }
}

static void
registry_handle_global_remove (void *data,
    struct wl_registry *registry,
    uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
  registry_handle_global,
  registry_handle_global_remove
};

static void grab_surface_create(struct desktop *desktop)
{
  GdkWindow *gdk_window;
  struct element *curtain;

  curtain = malloc (sizeof *curtain);
  memset (curtain, 0, sizeof *curtain);

  curtain->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (curtain->window), "maynard2");
  gtk_window_set_decorated (GTK_WINDOW (curtain->window), FALSE);
  gtk_widget_set_size_request (curtain->window, 8192, 8192);
  gtk_widget_realize (curtain->window);

  gdk_window = gtk_widget_get_window (curtain->window);
  gdk_wayland_window_set_use_custom_surface (gdk_window);

  curtain->surface = gdk_wayland_window_get_wl_surface (gdk_window);

  desktop->curtain = curtain;

  gtk_widget_show_all (curtain->window);
  weston_desktop_shell_set_grab_surface(desktop->wshell, curtain->surface);
}

int
main (int argc,
    char *argv[])
{
  struct desktop *desktop;

  gdk_set_allowed_backends ("wayland");

  gtk_init (&argc, &argv);

  g_object_set (gtk_settings_get_default (),
                "gtk-application-prefer-dark-theme", TRUE,
                NULL);

  desktop = malloc (sizeof *desktop);
  desktop->output = NULL;
  desktop->wshell = NULL;
  desktop->seat = NULL;
  
  desktop->gdk_display = gdk_display_get_default ();
  desktop->display =
    gdk_wayland_display_get_wl_display (desktop->gdk_display);
  if (desktop->display == NULL)
    {
      fprintf (stderr, "failed to get display: %m\n");
      return -1;
    }

  desktop->registry = wl_display_get_registry (desktop->display);
  wl_registry_add_listener (desktop->registry,
      &registry_listener, desktop);

  /* Wait until we have been notified about the compositor,
   * and shell objects */
  if (!desktop->output || !desktop->wshell)
    wl_display_roundtrip (desktop->display);
  if (!desktop->output || !desktop->wshell)
    {
      fprintf (stderr, "could not find output or shell modules\n");
      fprintf (stderr, "output: %p, wshell: %p\n",
               desktop->output, desktop->wshell);
      return -1;
    }

  css_setup (desktop);
  background_create (desktop);
  curtain_create (desktop);

  /* panel needs to be first so the clock and launcher grid can
   * be added to its layer */
  panel_create (desktop);
  grab_surface_create (desktop);

  gtk_main ();

  /* TODO cleanup */
  return EXIT_SUCCESS;
}
