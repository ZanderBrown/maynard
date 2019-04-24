#include "power.h"

#include <upower.h>

struct _MndPowerPrivate {
  GtkWidget *image;
  GtkWidget *label;

  UpDevice  *device;
};
typedef struct _MndPowerPrivate MndPowerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MndPower, mnd_power, MND_TYPE_PANEL_BUTTON)

static void
mnd_power_update_percent (MndPower *self)
{
  MndPowerPrivate *priv = mnd_power_get_instance_private (self);
  gdouble percent = 0.0;

  g_object_get (priv->device, "percentage", &percent, NULL);

  gtk_label_set_label (GTK_LABEL (priv->label),
                       g_strdup_printf ("%.0lf\u2009%%", percent));
}

static void
mnd_power_update_icon_name (MndPower *self)
{
  MndPowerPrivate *priv = mnd_power_get_instance_private (self);
  char *icon_name;

  g_object_get (priv->device, "icon-name", &icon_name, NULL);

  gtk_image_set_from_icon_name (GTK_IMAGE (priv->image),
                                icon_name,
                                GTK_ICON_SIZE_BUTTON);
}

static void
mnd_power_init (MndPower *self)
{
  GtkWidget *box;
  g_autoptr (UpClient) client;
  MndPowerPrivate *priv = mnd_power_get_instance_private (self);

  gtk_button_set_relief (GTK_BUTTON (self), GTK_RELIEF_NONE);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add (GTK_CONTAINER (self), box);

  priv->label = gtk_label_new ("...");
  gtk_container_add (GTK_CONTAINER (box), priv->label);
  gtk_widget_show (priv->label);

  priv->image = gtk_image_new_from_icon_name ("view-grid-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE (priv->image), 16);
  gtk_container_add (GTK_CONTAINER (box), priv->image);
  gtk_widget_show (priv->image);

  client = up_client_new ();
  priv->device = up_client_get_display_device (client);
  g_signal_connect_swapped (priv->device, "notify::percentage",
                            G_CALLBACK (mnd_power_update_percent), self);
  g_signal_connect_swapped (priv->device, "notify::icon-name",
                            G_CALLBACK (mnd_power_update_icon_name), self);

  mnd_power_update_percent (self);
  mnd_power_update_icon_name (self);
}

static void
mnd_power_dispose (GObject *object)
{
  MndPowerPrivate *priv = mnd_power_get_instance_private (MND_POWER (object));

  g_clear_object (&priv->device);

  G_OBJECT_CLASS (mnd_power_parent_class)->dispose (object);
}

static void
mnd_power_class_init (MndPowerClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = mnd_power_dispose;
}

GtkWidget *
mnd_power_new ()
{
  return g_object_new (MND_TYPE_POWER, NULL);
}
