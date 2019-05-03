#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#pragma once

G_BEGIN_DECLS

#define SHELL_TYPE_APP_SYSTEM shell_app_system_get_type()
G_DECLARE_DERIVABLE_TYPE (ShellAppSystem, shell_app_system, SHELL, APP_SYSTEM, GObject)

struct _ShellAppSystemClass
{
  GObjectClass parent_class;

  void (*installed_changed)(ShellAppSystem *appsys, gpointer user_data);
  void (*favorites_changed)(ShellAppSystem *appsys, gpointer user_data);
};

ShellAppSystem *shell_app_system_get_default (void);
GHashTable     *shell_app_system_get_entries (ShellAppSystem *self);

G_END_DECLS
