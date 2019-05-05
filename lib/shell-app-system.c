/* Original code taken from gnome-shell */
#include "config.h"

#include "shell-app-system.h"
#include <string.h>

#include <gio/gio.h>


typedef struct _ShellAppSystemPrivate ShellAppSystemPrivate;
struct _ShellAppSystemPrivate {
  GAppInfoMonitor *apps_tree;

  GSequence *items;

  /* cache */
  struct {
    gboolean       is_valid;
    guint          position;
    GSequenceIter *iter;
  } last;
};

static void list_iface_init (GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE (ShellAppSystem, shell_app_system, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (ShellAppSystem)
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, list_iface_init))

static void
shell_app_system_finalize (GObject *object)
{
  ShellAppSystem *self = SHELL_APP_SYSTEM (object);
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);

  g_object_unref (priv->apps_tree);

  g_sequence_free (priv->items);

  G_OBJECT_CLASS (shell_app_system_parent_class)->finalize (object);
}

static void
shell_app_system_class_init (ShellAppSystemClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = shell_app_system_finalize;
}

static GType
list_get_item_type (GListModel *list)
{
  return G_TYPE_APP_INFO;
}

static gpointer
list_get_item (GListModel *list, guint position)
{
  ShellAppSystem *self = SHELL_APP_SYSTEM (list);
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);
  GSequenceIter *it = NULL;

  if (priv->last.is_valid)
    {
      if (position < G_MAXUINT && priv->last.position == position + 1)
        it = g_sequence_iter_prev (priv->last.iter);
      else if (position > 0 && priv->last.position == position - 1)
        it = g_sequence_iter_next (priv->last.iter);
      else if (priv->last.position == position)
        it = priv->last.iter;
    }

  if (it == NULL)
    it = g_sequence_get_iter_at_pos (priv->items, position);

  priv->last.iter = it;
  priv->last.position = position;
  priv->last.is_valid = TRUE;

  if (g_sequence_iter_is_end (it))
    return NULL;
  else
    return g_object_ref (g_sequence_get (it));

}

static unsigned int
list_get_n_items (GListModel *list)
{
  ShellAppSystem *self = SHELL_APP_SYSTEM (list);
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);

  return g_sequence_get_length (priv->items);
}

static void
list_iface_init (GListModelInterface *iface)
{
  iface->get_item_type = list_get_item_type;
  iface->get_item = list_get_item;
  iface->get_n_items = list_get_n_items;
}

static void
on_apps_tree_changed_cb (GAppInfoMonitor *tree,
                         gpointer         user_data)
{
  ShellAppSystem *self = SHELL_APP_SYSTEM (user_data);
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);
  GList *new_apps;
  int removed;
  int added = 0;

  new_apps = g_app_info_get_all ();

  removed = g_sequence_get_length (priv->items);

  g_sequence_remove_range (g_sequence_get_begin_iter (priv->items),
                           g_sequence_get_end_iter (priv->items));

  while ((new_apps = g_list_next (new_apps))) {
    g_sequence_append (priv->items, g_object_ref (new_apps->data));
    added++;
  }

  g_list_free_full (new_apps, g_object_unref);

  // Tad terrible but no worse than the old system
  g_list_model_items_changed (G_LIST_MODEL (self), 0, removed, added);
}

static void
shell_app_system_init (ShellAppSystem *self)
{
  ShellAppSystemPrivate *priv = shell_app_system_get_instance_private (self);

  priv->items = g_sequence_new ((GDestroyNotify) g_object_unref);
  priv->apps_tree = g_app_info_monitor_get ();
  g_signal_connect (priv->apps_tree, "changed", G_CALLBACK (on_apps_tree_changed_cb), self);

  on_apps_tree_changed_cb (priv->apps_tree, self);
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
