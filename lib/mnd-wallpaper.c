#include "mnd-wallpaper.h"
#include <math.h>

typedef struct _MndWallpaperPrivate MndWallpaperPrivate;
struct _MndWallpaperPrivate {
  GdkPixbuf *pixbuf;

  GSettings *settings;
};

G_DEFINE_TYPE_WITH_PRIVATE (MndWallpaper, mnd_wallpaper, GTK_TYPE_WINDOW)

static gboolean
mnd_wallpaper_draw (GtkWidget *widget,
                    cairo_t   *ctx)
{
  MndWallpaper *self = MND_WALLPAPER (widget);
  MndWallpaperPrivate *priv = mnd_wallpaper_get_instance_private (self);

  GTK_WIDGET_CLASS (mnd_wallpaper_parent_class)->draw (widget, ctx);

  if (priv->pixbuf) {
    gdk_cairo_set_source_pixbuf (ctx, priv->pixbuf, 0, 0);
    cairo_paint (ctx);
  }

  return TRUE;
}

static void
mnd_wallpaper_class_init (MndWallpaperClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  widget_class->draw = mnd_wallpaper_draw;
}

static GdkPixbuf *
scale_background (GdkPixbuf *original_pixbuf)
{
  /* Scale original_pixbuf so it mostly fits on the screen.
   * If the aspect ratio is different than a bit on the right or on the
   * bottom could be cropped out. */
  GdkDisplay *display = gdk_display_get_default ();
  /* There's no primary monitor on nested wayland so just use the
     first one for now */
  GdkMonitor *monitor = gdk_display_get_monitor (display, 0);
  GdkRectangle geom;
  gint original_width, original_height;
  gint final_width, final_height;
  gdouble ratio_horizontal, ratio_vertical, ratio;

  g_return_val_if_fail(monitor, NULL);

  gdk_monitor_get_geometry (monitor, &geom);

  original_width = gdk_pixbuf_get_width (original_pixbuf);
  original_height = gdk_pixbuf_get_height (original_pixbuf);

  ratio_horizontal = (double) geom.width / original_width;
  ratio_vertical = (double) geom.height / original_height;
  ratio = MAX (ratio_horizontal, ratio_vertical);

  final_width = ceil (ratio * original_width);
  final_height = ceil (ratio * original_height);

  return gdk_pixbuf_scale_simple (original_pixbuf,
      final_width, final_height, GDK_INTERP_BILINEAR);
}

static void
prepare_pixbuf (MndWallpaper *self)
{
  MndWallpaperPrivate *priv = mnd_wallpaper_get_instance_private (self);
  g_autoptr (GdkPixbuf) unscaled_background = NULL;
  g_autofree char* filename = NULL;

  filename = g_settings_get_string (priv->settings, "wallpaper-path");

  if (filename && filename[0] != '\0')
    unscaled_background = gdk_pixbuf_new_from_file (filename, NULL);
  else
    unscaled_background = gdk_pixbuf_new_from_resource ("/org/raspberry-pi/maynard/wallpaper.jpg", NULL);

  priv->pixbuf = scale_background (unscaled_background);
}

static void
mnd_wallpaper_init (MndWallpaper *self)
{
  MndWallpaperPrivate *priv = mnd_wallpaper_get_instance_private (self);

  priv->settings = g_settings_new ("org.raspberrypi.maynard");

  g_object_set (self,
                "title", "Maynard Wallpaper",
                "decorated", FALSE,
                NULL);

  prepare_pixbuf (self);

  gtk_widget_realize (GTK_WIDGET (self));
}

GtkWidget *
mnd_wallpaper_new ()
{
  return g_object_new (MND_TYPE_WALLPAPER, NULL);
}