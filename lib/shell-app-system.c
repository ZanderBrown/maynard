/* Original code taken from gnome-shell */
#include "config.h"

#include "shell-app-system.h"
#include <string.h>

#include <gio/gio.h>

enum {
  INSTALLED_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

typedef struct _ShellAppSystemPrivate ShellAppSystemPrivate;
struct _ShellAppSystemPrivate {
  GAppInfoMonitor *apps_tree;

  GHashTable *id_to_info;
};

static void shell_app_system_finalize (GObject *object);
static void on_apps_tree_changed_cb (GAppInfoMonitor *tree, gpointer user_data);

G_DEFINE_TYPE_WITH_PRIVATE(ShellAppSystem, shell_app_system, G_TYPE_OBJECT);

static void shell_app_system_class_init(ShellAppSystemClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = shell_app_system_finalize;

  signals[INSTALLED_CHANGED] =
    g_signal_new ("installed-changed",
        SHELL_TYPE_APP_SYSTEM,
        G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET (ShellAppSystemClass, installed_changed),
        NULL, NULL, NULL,
        G_TYPE_NONE, 0);
}

static void
shell_app_system_init (ShellAppSystem *self)
{
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);

  priv->id_to_info = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           (GDestroyNotify)g_free,
                                           (GDestroyNotify)g_object_unref);

  priv->apps_tree = g_app_info_monitor_get ();
  g_signal_connect (priv->apps_tree, "changed", G_CALLBACK (on_apps_tree_changed_cb), self);

  on_apps_tree_changed_cb (priv->apps_tree, self);
}

static void
shell_app_system_finalize (GObject *object)
{
  ShellAppSystem *self = SHELL_APP_SYSTEM (object);
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);

  g_object_unref (priv->apps_tree);

  g_hash_table_destroy (priv->id_to_info);

  G_OBJECT_CLASS (shell_app_system_parent_class)->finalize (object);
}

static void
on_apps_tree_changed_cb (GAppInfoMonitor *tree,
                         gpointer         user_data)
{
  ShellAppSystem *self = SHELL_APP_SYSTEM (user_data);
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);
  GList *new_apps;

  new_apps = g_app_info_get_all ();
  g_hash_table_remove_all (priv->id_to_info);
  while ((new_apps = g_list_next (new_apps))) {
    GDesktopAppInfo *info = G_DESKTOP_APP_INFO (new_apps->data);
    const gchar *id = g_app_info_get_id (G_APP_INFO (info));

    g_hash_table_insert (priv->id_to_info, g_strdup (id), g_object_ref (info));
  }

  g_list_free_full (new_apps, g_object_unref);

  g_signal_emit (self, signals[INSTALLED_CHANGED], 0);
}

/**
 * shell_app_system_get_default:
 *
 * Return Value: (transfer none): The global #ShellAppSystem singleton
 */
ShellAppSystem *
shell_app_system_get_default ()
{
  static ShellAppSystem *instance = NULL;

  if (instance == NULL)
    instance = g_object_new (SHELL_TYPE_APP_SYSTEM, NULL);

  return instance;
}

GHashTable *
shell_app_system_get_entries (ShellAppSystem *self)
{
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);
  
  return priv->id_to_info;
}
