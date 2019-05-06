#include "mnd-app-launcher.h"

typedef struct _MndAppLauncherPrivate MndAppLauncherPrivate;
struct _MndAppLauncherPrivate {
  GtkWidget *image;
  GtkWidget *label;

  GAppInfo  *app;
};

G_DEFINE_TYPE_WITH_PRIVATE (MndAppLauncher, mnd_app_launcher, GTK_TYPE_EVENT_BOX)

enum {
  PROP_0,
  PROP_APP,
  LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = { NULL, };

enum {
  LAUNCHED,
  LAST_SIGNAL
};
static guint signals [LAST_SIGNAL] = { 0, };

static void
mnd_app_launcher_init (MndAppLauncher *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
mnd_app_launcher_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  MndAppLauncher *self = MND_APP_LAUNCHER (object);
  MndAppLauncherPrivate *priv = mnd_app_launcher_get_instance_private (self);
  GIcon *icon;
  const gchar* name;

  switch (property_id) {
    case PROP_APP:
      g_clear_object (&priv->app);
      priv->app = g_value_get_object (value);
      name = g_app_info_get_display_name (G_APP_INFO (priv->app));
      gtk_label_set_label (GTK_LABEL (priv->label), name);
      icon = g_app_info_get_icon (priv->app);
      gtk_image_set_from_gicon (GTK_IMAGE (priv->image), icon, GTK_ICON_SIZE_BUTTON);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
mnd_app_launcher_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  MndAppLauncher *self = MND_APP_LAUNCHER (object);
  MndAppLauncherPrivate *priv = mnd_app_launcher_get_instance_private (self);

  switch (property_id) {
    case PROP_APP:
      g_value_set_object (value, priv->app);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static gboolean
get_child_position_cb (GtkOverlay   *overlay,
                       GtkWidget    *widget,
                       GdkRectangle *allocation,
                       gpointer      user_data)
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

static void
clicked_cb (GtkWidget      *widget,
            MndAppLauncher *self)
{
  MndAppLauncherPrivate *priv = mnd_app_launcher_get_instance_private (self);

  g_app_info_launch (G_APP_INFO (priv->app), NULL, NULL, NULL);

  g_signal_emit (self, signals[LAUNCHED], 0);
}

static gboolean
app_enter_cb (GtkWidget *widget,
              GdkEvent  *event,
              GtkWidget *revealer)
{
  gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), TRUE);
  return FALSE;
}

static gboolean
app_leave_cb (GtkWidget *widget,
              GdkEvent  *event,
              GtkWidget *revealer)
{
  gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), FALSE);
  return FALSE;
}

static void
mnd_app_launcher_class_init (MndAppLauncherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = mnd_app_launcher_set_property;
  object_class->get_property = mnd_app_launcher_get_property;

  pspecs[PROP_APP] =
    g_param_spec_object ("app", "App", "App Info",
                         G_TYPE_APP_INFO,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, pspecs);

  signals [LAUNCHED] =
        g_signal_new ("launched",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/raspberry-pi/maynard/mnd-app-launcher.ui");

  gtk_widget_class_bind_template_child_private (widget_class, MndAppLauncher, image);
  gtk_widget_class_bind_template_child_private (widget_class, MndAppLauncher, label);

  gtk_widget_class_bind_template_callback (widget_class, get_child_position_cb);
  gtk_widget_class_bind_template_callback (widget_class, clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, app_enter_cb);
  gtk_widget_class_bind_template_callback (widget_class, app_leave_cb);
}

GtkWidget *
mnd_app_launcher_new (GAppInfo *info)
{
  return g_object_new (MND_TYPE_APP_LAUNCHER,
                       "app", info,
                       NULL);
}
