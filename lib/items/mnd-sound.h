/*
 * Copyright (C) 2014 Collabora Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * Author: Jonny Lamb <jonny.lamb@collabora.co.uk>
 */

#include <gtk/gtk.h>

#include "items/mnd-panel-button.h"

#pragma once

G_BEGIN_DECLS

struct _MndSound {
  MndPanelButton parent;
};

#define MND_TYPE_SOUND mnd_sound_get_type()
G_DECLARE_FINAL_TYPE (MndSound, mnd_sound, MND, SOUND, MndPanelButton)

GtkWidget *mnd_sound_new ();

G_END_DECLS
