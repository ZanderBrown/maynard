#include <gtk/gtk.h>

#include "items/mnd-panel-button.h"

#pragma once

G_BEGIN_DECLS

struct _MndSoundPopover {
  GtkPopover parent;
};

#define MND_TYPE_SOUND_POPOVER mnd_sound_popover_get_type()
G_DECLARE_FINAL_TYPE (MndSoundPopover, mnd_sound_popover, MND, SOUND_POPOVER, GtkPopover)

GtkWidget *mnd_sound_popover_new (GtkWidget *parent);

G_END_DECLS
