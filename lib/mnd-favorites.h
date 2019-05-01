/*
 * Copyright (C) 2013 Collabora Ltd.
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
 * Author: Emilio Pozuelo Monfort <emilio.pozuelo@collabora.co.uk>
 */

#include <gtk/gtk.h>

#pragma once

G_BEGIN_DECLS

#define MND_TYPE_FAVORITES mnd_favorites_get_type()
G_DECLARE_DERIVABLE_TYPE (MndFavorites, mnd_favorites, MND, FAVORITES, GtkBox)

struct _MndFavoritesClass {
  GtkBoxClass parent;
};

GtkWidget *mnd_favorites_new ();

G_END_DECLS
