
#include <clutter-gst/clutter-gst.h>
#include "clutter-video-player.h"
#include "play_png.h"
#include "pause_png.h"

#define CTRL_SIZE 10

#ifndef CLUTTER_PARAM_READWRITE
#define CLUTTER_PARAM_READWRITE \
        G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |G_PARAM_STATIC_BLURB
#endif

G_DEFINE_TYPE (ClutterVideoPlayer, clutter_video_player, CLUTTER_TYPE_GROUP);

#define CLUTTER_VIDEO_PLAYER_GET_PRIVATE(obj) \
(G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_VIDEO_PLAYER, ClutterVideoPlayerPrivate))

struct _ClutterVideoPlayerPrivate
{
  ClutterActor    * vtexture;

  ClutterActor    * control;
  ClutterActor    * control_play;
  ClutterActor    * control_pause;
  
  gboolean          paused;
  
  gchar           * uri;
};

static void
toggle_pause_state (ClutterVideoPlayer *player);

static void 
input_cb (ClutterStage * stage, ClutterEvent * event, gpointer data);

static gboolean
autostop_playback (gpointer data)
{
  ClutterVideoPlayer        * player = data;
  ClutterVideoPlayerPrivate * priv = player->priv;

  toggle_pause_state (player);
  clutter_media_set_position (CLUTTER_MEDIA (priv->vtexture), 0);
  return FALSE;
}

static void
eos_cb (ClutterMedia * media, gpointer data)
{
  ClutterVideoPlayer * player = data;

  if (!player->priv->paused)
    toggle_pause_state (player);

  clutter_media_set_position (media, 0);
}

static GdkPixbuf *
pixbuf_from_data (const guchar * data, gint length)
{
  GdkPixbuf *pixbuf;
  GdkPixbufLoader * ldr = gdk_pixbuf_loader_new_with_type ("png", NULL);

  if (!ldr)
    {
      g_warning ("Could not create loader");
      return NULL;
    }
  
  if (!gdk_pixbuf_loader_write (ldr, data, length, NULL))
    {
      g_warning ("Failed to write to loader.");
      return NULL;
    }
  
  pixbuf = gdk_pixbuf_loader_get_pixbuf (ldr);

  return pixbuf;
}

static void
construct_controls (ClutterVideoPlayer *player)
{
  ClutterVideoPlayerPrivate *priv = player->priv;
  GdkPixbuf           *pixb;

  priv->vtexture = clutter_gst_video_texture_new ();

  if (priv->vtexture == NULL)
    g_error("failed to create vtexture");
  
  /* Dont let the underlying pixbuf dictate size */
  g_object_set (G_OBJECT(priv->vtexture), "sync-size", FALSE, NULL);

  clutter_media_set_filename(CLUTTER_MEDIA(priv->vtexture), priv->uri);
  clutter_media_set_playing (CLUTTER_MEDIA(priv->vtexture), TRUE);
  priv->paused = FALSE;
  g_signal_connect (priv->vtexture, "eos", eos_cb, player);
  g_timeout_add (100, autostop_playback, player);
  
  clutter_actor_show (priv->vtexture);
  
  priv->control = clutter_group_new ();
  
  pixb = pixbuf_from_data (&play_png[0], sizeof (play_png));

  if (pixb == NULL)
    g_error("Unable to load play button image");

  priv->control_play = clutter_texture_new_from_pixbuf (pixb);
  clutter_actor_set_size (priv->control_play, CTRL_SIZE, CTRL_SIZE);
  clutter_actor_show (priv->control_play);
  
  pixb = pixbuf_from_data (&pause_png[0], sizeof (pause_png));

  if (pixb == NULL)
    g_error("Unable to load pause button image");

  priv->control_pause = clutter_texture_new_from_pixbuf (pixb);
  clutter_actor_set_size (priv->control_pause, CTRL_SIZE, CTRL_SIZE);
  
  clutter_group_add_many (CLUTTER_GROUP (priv->control), 
			  priv->control_play, 
			  priv->control_pause,
			  NULL);

  clutter_actor_set_opacity (priv->control, 0xee);

  clutter_actor_set_position (priv->control_play, 0, 0);
  clutter_actor_set_position (priv->control_pause, 0, 0);

  clutter_group_add_many (CLUTTER_GROUP (player), 
			  priv->vtexture, priv->control, NULL);

  g_signal_connect (clutter_stage_get_default(), "event",
		    G_CALLBACK (input_cb), 
		    player);
}

enum
{
  PROP_0,
  PROP_URI,
};

static void 
clutter_video_player_set_property (GObject      *object, 
				  guint         prop_id,
				  const GValue *value, 
				  GParamSpec   *pspec)
{
  ClutterVideoPlayer        *player;
  ClutterVideoPlayerPrivate *priv;

  player = CLUTTER_VIDEO_PLAYER(object);
  priv = player->priv;

  switch (prop_id) 
    {
    case PROP_URI:
      g_free (priv->uri);
      priv->uri = g_strdup (g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void 
clutter_video_player_get_property (GObject    *object, 
				   guint       prop_id,
				   GValue     *value, 
				   GParamSpec *pspec)
{
  ClutterVideoPlayer        *player;
  ClutterVideoPlayerPrivate *priv;

  player = CLUTTER_VIDEO_PLAYER(object);
  priv = player->priv;

  switch (prop_id) 
    {
    case PROP_URI:
      g_value_set_string (value, priv->uri);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static GObject *
clutter_video_player_constructor (GType                  gtype,
				  guint                  n_params,
				  GObjectConstructParam *params)
{
  GObjectClass       * parent_class;
  GObject            * retval;

  parent_class = G_OBJECT_CLASS (clutter_video_player_parent_class);
  retval = parent_class->constructor (gtype, n_params, params);

  construct_controls (CLUTTER_VIDEO_PLAYER (retval));
  
  return retval;
}

static void 
clutter_video_player_finalize (GObject *object)
{
  ClutterVideoPlayer *player = CLUTTER_VIDEO_PLAYER (object);

  g_free (player->priv->uri);
  
  G_OBJECT_CLASS (clutter_video_player_parent_class)->finalize (object);
}

static void
clutter_video_player_request_coords (ClutterActor        *self,
				     ClutterActorBox     *box)
{
  ClutterVideoPlayer * player = CLUTTER_VIDEO_PLAYER (self);
  ClutterVideoPlayerPrivate *priv = player->priv;
  ClutterActorBox cbox;

  cbox.x1 = 0;
  cbox.y1 = 0;
  cbox.x2 = box->x2 - box->x1;
  cbox.y2 = box->y2 - box->y1;

  clutter_actor_request_coords (priv->vtexture, &cbox);
  
  CLUTTER_ACTOR_CLASS (clutter_video_player_parent_class)->request_coords (self, box);

  clutter_actor_set_position (priv->control,
			      (clutter_actor_get_width (priv->vtexture) -
			       CTRL_SIZE) / 2,
			      clutter_actor_get_height (priv->vtexture) -
			      (CTRL_SIZE + CTRL_SIZE/2));
  
  clutter_actor_show (priv->control);
  clutter_actor_queue_redraw (priv->vtexture);
}

static void
clutter_video_player_class_init (ClutterVideoPlayerClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->constructor  = clutter_video_player_constructor;
  gobject_class->set_property = clutter_video_player_set_property;
  gobject_class->get_property = clutter_video_player_get_property;
  gobject_class->finalize     = clutter_video_player_finalize;

  actor_class->request_coords  = clutter_video_player_request_coords;

  g_object_class_install_property (gobject_class,
                                   PROP_URI,
                                   g_param_spec_string ("uri",
							"uri",
							"uri",
							"test.avi",
							G_PARAM_CONSTRUCT |
							CLUTTER_PARAM_READWRITE));
   
  g_type_class_add_private (gobject_class, sizeof (ClutterVideoPlayerPrivate));
}

static void
clutter_video_player_init (ClutterVideoPlayer *self)
{
  ClutterVideoPlayerPrivate *priv;

  self->priv = priv = CLUTTER_VIDEO_PLAYER_GET_PRIVATE (self);

  priv->paused = TRUE;
}

ClutterActor *
clutter_video_player_new (const gchar * uri)
{
  return g_object_new (CLUTTER_TYPE_VIDEO_PLAYER, "uri", uri, NULL);
}

static void
toggle_pause_state (ClutterVideoPlayer *player)
{
  if (player->priv->paused)
    {
      clutter_media_set_playing (CLUTTER_MEDIA(player->priv->vtexture), 
				 TRUE);
      player->priv->paused = FALSE;
      clutter_actor_hide (player->priv->control_play);
      clutter_actor_show (player->priv->control_pause);
    }
  else
    {
      clutter_media_set_playing (CLUTTER_MEDIA(player->priv->vtexture), 
				     FALSE);
      player->priv->paused = TRUE;
      clutter_actor_hide (player->priv->control_pause);
      clutter_actor_show (player->priv->control_play);
    }
}

static void 
input_cb (ClutterStage *stage, 
	  ClutterEvent *event,
	  gpointer      user_data)
{
  ClutterVideoPlayer *player = (ClutterVideoPlayer*)user_data;
  ClutterVideoPlayerPrivate *priv = player->priv;
  
  switch (event->type)
    {
    case CLUTTER_BUTTON_PRESS:
	{
	  ClutterActor       *actor;
	  ClutterButtonEvent *bev = (ClutterButtonEvent *) event;

	  actor 
	    = clutter_stage_get_actor_at_pos 
	                         (CLUTTER_STAGE(clutter_stage_get_default()),
				  bev->x, bev->y);

	  printf("got actor %p at pos %ix%i\n", actor, bev->x, bev->y);

	  if (actor == priv->control_pause || actor == priv->control_play)
	    {
	      toggle_pause_state (player);
	      return;
	    }

	}
      break;
    default:
      break;
    }
}

