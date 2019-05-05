#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#pragma once

G_BEGIN_DECLS

#define SHELL_TYPE_APP_SYSTEM shell_app_system_get_type()
G_DECLARE_DERIVABLE_TYPE (ShellAppSystem, shell_app_system, SHELL, APP_SYSTEM, GObject)

struct _ShellAppSystemClass
{
  GObjectClass parent_class;
};

ShellAppSystem *shell_app_system_get_default (void);

G_END_DECLS
