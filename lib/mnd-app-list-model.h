#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#pragma once

G_BEGIN_DECLS

#define MND_TYPE_APP_LIST_MODEL mnd_app_list_model_get_type()
G_DECLARE_DERIVABLE_TYPE (MndAppListModel, mnd_app_list_model, MND, APP_LIST_MODEL, GObject)

struct _MndAppListModelClass
{
  GObjectClass parent_class;
};

MndAppListModel *mnd_app_list_model_get_default (void);

G_END_DECLS
