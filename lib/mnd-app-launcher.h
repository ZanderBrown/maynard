#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>

#pragma once

G_BEGIN_DECLS

#define MND_TYPE_APP_LAUNCHER mnd_app_launcher_get_type()
G_DECLARE_DERIVABLE_TYPE (MndAppLauncher, mnd_app_launcher, MND, APP_LAUNCHER, GtkEventBox)

struct _MndAppLauncherClass
{
  GtkEventBoxClass parent_class;
};

GtkWidget *mnd_app_launcher_new (GAppInfo *info);

G_END_DECLS
