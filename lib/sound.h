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

#ifndef __MAYNARD_SOUND_H__
#define __MAYNARD_SOUND_H__

#include <gtk/gtk.h>

#include "panel.h"

#define MAYNARD_SOUND_TYPE                 (maynard_sound_get_type ())
#define MAYNARD_SOUND(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MAYNARD_SOUND_TYPE, MaynardSound))
#define MAYNARD_SOUND_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MAYNARD_SOUND_TYPE, MaynardSoundClass))
#define MAYNARD_IS_SOUND(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAYNARD_SOUND_TYPE))
#define MAYNARD_IS_SOUND_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MAYNARD_SOUND_TYPE))
#define MAYNARD_SOUND_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MAYNARD_SOUND_TYPE, MaynardSoundClass))

typedef struct MaynardSound MaynardSound;
typedef struct MaynardSoundClass MaynardSoundClass;
typedef struct MaynardSoundPrivate MaynardSoundPrivate;

struct MaynardSound
{
  GtkBox parent;

  MaynardSoundPrivate *priv;
};

struct MaynardSoundClass
{
  GtkBoxClass parent_class;
};

GType maynard_sound_get_type (void) G_GNUC_CONST;

GtkWidget * maynard_sound_new (void);

#endif /* __MAYNARD_SOUND_H__ */
