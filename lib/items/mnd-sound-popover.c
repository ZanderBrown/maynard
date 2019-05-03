#include "config.h"

#include <alsa/asoundlib.h>
#include <pulse/pulseaudio.h>
#include <gvc-mixer-control.h>
#include <glib/gi18n-lib.h>
#include <math.h>

#include "mnd-sound-popover.h"

struct _MndSoundPopoverPrivate {
  GtkWidget *volume_scale;
  GtkWidget *volume_image;

  GvcMixerControl *mix;
  GvcMixerStream *stream;

  gdouble volume;
};
typedef struct _MndSoundPopoverPrivate MndSoundPopoverPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MndSoundPopover, mnd_sound_popover, GTK_TYPE_POPOVER)

static void
volume_changed_cb (GtkRange *range, MndSoundPopover *self);

static void
update_volume (MndSoundPopover *self)
{
  MndSoundPopoverPrivate *priv = mnd_sound_popover_get_instance_private (self);
  gdouble vol_norm = gvc_mixer_control_get_vol_max_norm (priv->mix);
  gdouble vol = gvc_mixer_stream_get_volume (priv->stream);
  GtkAdjustment *adj;
  gchar *image_name;
  gdouble vol_max;
  gdouble step_size;
  int n;

  /* Same maths as computed by volume.js in gnome-shell, carried over
    * from C->Vala port of budgie-panel and now into maynard */
  n = (int) floor(3*vol/vol_norm)+1;

  // Work out an icon
  if (gvc_mixer_stream_get_is_muted (priv->stream) || vol <= 0) {
    image_name = "audio-volume-muted-symbolic";
  } else {
    switch (n) {
      case 1:
        image_name = "audio-volume-low-symbolic";
        break;
      case 2:
        image_name = "audio-volume-medium-symbolic";
        break;
      default:
        image_name = "audio-volume-high-symbolic";
        break;
    }
  }
  gtk_image_set_from_icon_name (GTK_IMAGE (priv->volume_image),
                                image_name,
                                GTK_ICON_SIZE_BUTTON);

  vol_max = gvc_mixer_control_get_vol_max_amplified (priv->mix);

  // Each scroll increments by 5%, much better than units..
  step_size = vol_max / 20;

  g_signal_handlers_block_by_func (priv->volume_scale, volume_changed_cb, self);

  gtk_range_set_range (GTK_RANGE (priv->volume_scale), 0, vol_norm);
  gtk_range_set_value (GTK_RANGE (priv->volume_scale), vol);

  adj = gtk_range_get_adjustment (GTK_RANGE (priv->volume_scale));
  gtk_adjustment_set_step_increment (adj, step_size);

  g_signal_handlers_unblock_by_func (priv->volume_scale, volume_changed_cb, self);

  if (vol != 0) { // If we haven't muted
      priv->volume = vol; // Get our new volume
  }
}

static void
volume_changed_cb (GtkRange *range,
    MndSoundPopover *self)
{
  MndSoundPopoverPrivate *priv = mnd_sound_popover_get_instance_private (self);
  gdouble scale_value;
  
  if (priv->stream == NULL || priv->mix == NULL) {
    return;
  }
  scale_value = gtk_range_get_value (GTK_RANGE (priv->volume_scale));

  if (gvc_mixer_stream_get_is_muted (priv->stream) && scale_value > 0) {
    gvc_mixer_stream_set_is_muted (priv->stream, TRUE);
    gvc_mixer_stream_change_is_muted (priv->stream, TRUE);
  }

  /* Avoid recursion ! */
  g_signal_handlers_block_by_func (priv->volume_scale, volume_changed_cb, self);
  if (gvc_mixer_stream_set_volume (priv->stream, scale_value)) {
      gvc_mixer_stream_push_volume (priv->stream);
  }
  g_signal_handlers_unblock_by_func (priv->volume_scale, volume_changed_cb, self);
}

static void
stream_changed (GvcMixerStream  *stream,
                GParamSpec      *param,
                MndSoundPopover *self)
{
  update_volume (self);
}

static void
set_default_mixer (MndSoundPopover *self)
{
  MndSoundPopoverPrivate *priv = mnd_sound_popover_get_instance_private (MND_SOUND_POPOVER (self));
  
  if (priv->stream != NULL) {
    g_signal_handlers_disconnect_by_func (priv->stream, stream_changed, self);
  }

  priv->stream = gvc_mixer_control_get_default_sink (priv->mix);
  g_signal_connect (priv->stream, "notify::volume", G_CALLBACK (stream_changed), self);
  g_signal_connect (priv->stream, "notify::is-muted", G_CALLBACK (stream_changed), self);

  update_volume(self);
}

static void
mixer_sink_change (GvcMixerControl *mix, guint id, MndSoundPopover *self)
{
  set_default_mixer (self);
}

static void
mixer_state_change (GvcMixerControl *mix, guint new_state, MndSoundPopover *self)
{
  if (new_state == GVC_STATE_READY) {
    set_default_mixer (self);
  }
}

static void
mnd_sound_popover_init (MndSoundPopover *self)
{
  MndSoundPopoverPrivate *priv = mnd_sound_popover_get_instance_private (MND_SOUND_POPOVER (self));
  GtkWidget *box;

  priv->mix = gvc_mixer_control_new (_("Maynard"));
  g_signal_connect (priv->mix, "state-changed", G_CALLBACK (mixer_state_change), self);
  g_signal_connect (priv->mix, "default-sink-changed", G_CALLBACK (mixer_sink_change), self);
  gvc_mixer_control_open (priv->mix);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_show (box);

  priv->volume_image = gtk_image_new_from_icon_name ("audio-volume-muted-symbolic",
                                                     GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (box), priv->volume_image, FALSE, FALSE, 0);
  gtk_widget_show (priv->volume_image);

  priv->volume_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_draw_value (GTK_SCALE (priv->volume_scale), FALSE);
  gtk_widget_set_size_request (priv->volume_scale, 200, -1);
  gtk_box_pack_end (GTK_BOX (box), priv->volume_scale, TRUE, TRUE, 0);
  gtk_widget_show (priv->volume_scale);

  gtk_container_add (GTK_CONTAINER (self), box);

  g_signal_connect (priv->volume_scale, "value-changed",
      G_CALLBACK (volume_changed_cb), self);

}

static void
mnd_sound_popover_dispose (GObject *object)
{
  MndSoundPopoverPrivate *priv = mnd_sound_popover_get_instance_private (MND_SOUND_POPOVER (object));

  g_clear_object (&priv->mix);

  G_OBJECT_CLASS (mnd_sound_popover_parent_class)->dispose (object);
}

static void
mnd_sound_popover_class_init (MndSoundPopoverClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = mnd_sound_popover_dispose;
}

GtkWidget *
mnd_sound_popover_new (GtkWidget *parent)
{
  return g_object_new (MND_TYPE_SOUND_POPOVER,
                       "relative-to", parent,
                       NULL);
}
