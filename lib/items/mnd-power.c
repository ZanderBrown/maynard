#include "config.h"
#include "mnd-power.h"

#include <glib/gi18n-lib.h>
#include <upower.h>

struct _MndPowerPrivate {
  /* button contents */
  GtkWidget *image;
  GtkWidget *label;

  /* popover */
  GtkWidget *level;
  GtkWidget *desc;

  UpDevice  *device;
};
typedef struct _MndPowerPrivate MndPowerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MndPower, mnd_power, MND_TYPE_PANEL_BUTTON)

/* get_timestring and get_details_string are taken from the power panel of gnome-control-center */
static gchar *
get_timestring (guint64 time_secs)
{
  gchar* timestring = NULL;
  gint  hours;
  gint  minutes;

  /* Add 0.5 to do rounding */
  minutes = (int) ( ( time_secs / 60.0 ) + 0.5 );

  if (minutes == 0)
    {
      timestring = g_strdup (_("Unknown time"));
      return timestring;
    }

  if (minutes < 60)
    {
      timestring = g_strdup_printf (ngettext ("%i minute",
                                    "%i minutes",
                                    minutes), minutes);
      return timestring;
    }

  hours = minutes / 60;
  minutes = minutes % 60;

  if (minutes == 0)
    {
      timestring = g_strdup_printf (ngettext (
                                    "%i hour",
                                    "%i hours",
                                    hours), hours);
      return timestring;
    }

  /* TRANSLATOR: "%i %s %i %s" are "%i hours %i minutes"
   * Swap order with "%2$s %2$i %1$s %1$i if needed */
  timestring = g_strdup_printf (_("%i %s %i %s"),
                                hours, ngettext ("hour", "hours", hours),
                                minutes, ngettext ("minute", "minutes", minutes));
  return timestring;
}

static gchar *
get_details_string (gdouble percentage, UpDeviceState state, guint64 time)
{
  gchar *details;

  if (time > 0)
    {
      g_autofree gchar *time_string = NULL;

      time_string = get_timestring (time);
      switch (state)
        {
          case UP_DEVICE_STATE_CHARGING:
            /* TRANSLATORS: %1 is a time string, e.g. "1 hour 5 minutes" */
            details = g_strdup_printf (_("%s until fully charged"), time_string);
            break;
          case UP_DEVICE_STATE_DISCHARGING:
          case UP_DEVICE_STATE_PENDING_DISCHARGE:
            if (percentage < 20)
              {
                /* TRANSLATORS: %1 is a time string, e.g. "1 hour 5 minutes" */
                details = g_strdup_printf (_("Caution: %s remaining"), time_string);
              }
            else
              {
                /* TRANSLATORS: %1 is a time string, e.g. "1 hour 5 minutes" */
                details = g_strdup_printf (_("%s remaining"), time_string);
              }
            break;
          case UP_DEVICE_STATE_FULLY_CHARGED:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Fully charged"));
            break;
          case UP_DEVICE_STATE_PENDING_CHARGE:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Not charging"));
            break;
          case UP_DEVICE_STATE_EMPTY:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Empty"));
            break;
          default:
            details = g_strdup_printf ("error: %s", up_device_state_to_string (state));
            break;
        }
    }
  else
    {
      switch (state)
        {
          case UP_DEVICE_STATE_CHARGING:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Charging"));
            break;
          case UP_DEVICE_STATE_DISCHARGING:
          case UP_DEVICE_STATE_PENDING_DISCHARGE:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Discharging"));
            break;
          case UP_DEVICE_STATE_FULLY_CHARGED:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Fully charged"));
            break;
          case UP_DEVICE_STATE_PENDING_CHARGE:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Not charging"));
            break;
          case UP_DEVICE_STATE_EMPTY:
            /* TRANSLATORS: primary battery */
            details = g_strdup (_("Empty"));
            break;
          default:
            details = g_strdup_printf ("error: %s",
                                       up_device_state_to_string (state));
            break;
        }
    }

  return details;
}

static void
mnd_power_sync (MndPower *self)
{
  MndPowerPrivate *priv = mnd_power_get_instance_private (self);
  gdouble percent = 0.0;
  char *icon_name;
  UpDeviceState state;
  gint64 time_to_empty;
  gint64 time_to_full;
  gint64 time_to;

  g_object_get (priv->device,
                "percentage", &percent,
                "icon-name", &icon_name,
                "state", &state,
                "time-to-empty", &time_to_empty,
                "time-to-full", &time_to_full,
                NULL);

  if (state == UP_DEVICE_STATE_DISCHARGING)
    time_to = time_to_empty;
  else
    time_to = time_to_full;


  gtk_label_set_label (GTK_LABEL (priv->label),
                       g_strdup_printf ("%.0lf\u2009%%", percent));

  gtk_level_bar_set_value (GTK_LEVEL_BAR (priv->level), percent);

  gtk_image_set_from_icon_name (GTK_IMAGE (priv->image),
                                icon_name,
                                GTK_ICON_SIZE_BUTTON);

  gtk_label_set_label (GTK_LABEL (priv->desc),
                       get_details_string (percent, state, time_to));
}

static void
mnd_power_init (MndPower *self)
{
  GtkWidget *box, *popover;
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

  popover = gtk_popover_new (GTK_WIDGET (self));
  gtk_popover_set_constrain_to (GTK_POPOVER (popover), GTK_POPOVER_CONSTRAINT_NONE);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (self), popover);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (popover), box);
  gtk_widget_show (box);

  priv->desc = gtk_label_new ("Power");
  gtk_label_set_xalign (GTK_LABEL (priv->desc), 0.0);
  gtk_container_add (GTK_CONTAINER (box), priv->desc);
  gtk_widget_show (priv->desc);

  priv->level = gtk_level_bar_new ();
  gtk_level_bar_set_min_value (GTK_LEVEL_BAR (priv->level), 0.0);
  gtk_level_bar_set_max_value (GTK_LEVEL_BAR (priv->level), 100.0);
  gtk_widget_set_size_request (priv->level, 250, -1);
  gtk_container_add (GTK_CONTAINER (box), priv->level);
  gtk_widget_show (priv->level);

  client = up_client_new ();
  priv->device = up_client_get_display_device (client);
  g_signal_connect_swapped (priv->device, "notify::percentage",
                            G_CALLBACK (mnd_power_sync), self);
  g_signal_connect_swapped (priv->device, "notify::icon-name",
                            G_CALLBACK (mnd_power_sync), self);

  mnd_power_sync (self);
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
