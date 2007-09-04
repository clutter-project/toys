#include <clutter/clutter.h>
#include <clutter-gst/clutter-gst.h>
#include <gst/gst.h>

#include "wh-screen-video.h"
#include "util.h"

G_DEFINE_TYPE (WHScreenVideo, wh_screen_video, CLUTTER_TYPE_ACTOR);

#define SCREEN_VIDEO_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                WH_TYPE_SCREEN_VIDEO, WHScreenVideoPrivate))

typedef struct _WHScreenVideoPrivate WHScreenVideoPrivate;

struct _WHScreenVideoPrivate
{
  ClutterActor      *video;
  ClutterActor      *bg;
  ClutterActor      *video_controls;
  ClutterActor      *video_seekbar;
  ClutterActor      *video_seekbar_bg;
  ClutterActor      *duration, *title, *position, *vol_label;
  ClutterTimeline   *foo_effect;
  gboolean           video_playing;
  gboolean           video_controls_visible;
  
  guint              controls_timeout;
  WHVideoModelRow   *video_row;

  /* Effects */
  ClutterTimeline   *cheese_timeline;
  ClutterEffectTemplate *controls_effect_tmpl, *fadein_effect_tmpl;
};

enum
{
  PLAYBACK_STARTED,
  PLAYBACK_FINISHED,
  LAST_SIGNAL
};

static guint _screen_signals[LAST_SIGNAL] = { 0 };

void
cheese_cb (ClutterTimeline *timeline, 
	   gint             frame_num, 
	   WHScreenVideo   *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  clutter_actor_rotate_y (priv->video,
			  frame_num * 12,
			  CLUTTER_STAGE_WIDTH()/2,
			  0);
}

static void
video_size_change (ClutterTexture *texture, 
		   gint            width,
		   gint            height,
		   gpointer        user_data)
{
  gint           new_x, new_y, new_width, new_height;
  
  new_height = ( height * CLUTTER_STAGE_WIDTH() ) / width;

  if (new_height <= CLUTTER_STAGE_HEIGHT())
    {
      new_width = CLUTTER_STAGE_WIDTH();
      new_x = 0;
      new_y = (CLUTTER_STAGE_HEIGHT() - new_height) / 2;
    }
  else
    {
      new_width  = ( width * CLUTTER_STAGE_HEIGHT() ) / height;
      new_height = CLUTTER_STAGE_HEIGHT();
      
      new_x = (CLUTTER_STAGE_WIDTH() - new_width) / 2;
      new_y = 0;
    }
  
  clutter_actor_set_position (CLUTTER_ACTOR (texture), new_x, new_y);
  clutter_actor_set_size (CLUTTER_ACTOR (texture), new_width, new_height);
}

void
video_pixbuf_change (ClutterTexture *texture, WHScreenVideo  *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  g_signal_emit (screen, _screen_signals[PLAYBACK_STARTED], 0);

  // util_actor_fade_in (CLUTTER_ACTOR(screen), NULL, NULL);

  clutter_effect_fade (priv->fadein_effect_tmpl,
		       CLUTTER_ACTOR(screen),
		       0,
		       0xff,
		       NULL,
		       NULL);

  clutter_actor_show (CLUTTER_ACTOR(screen));
  clutter_actor_set_opacity (CLUTTER_ACTOR(screen), 0);

  g_signal_handlers_disconnect_by_func (priv->video, 
					G_CALLBACK (video_pixbuf_change),
					screen);
} 

static gchar*
nice_time (int time)
{
  int    hours, minutes, seconds;

  hours = time / 3600;
  seconds = time % 3600;
  minutes = seconds / 60;
  seconds = seconds % 60;

  if (hours > 0) 
    return g_strdup_printf("%d:%.2d:%.2d", hours, minutes, seconds);
  else
    return g_strdup_printf("%.2d:%.2d", minutes, seconds);
}


static void 
video_tick (GObject        *object,
	    GParamSpec     *pspec,
	    WHScreenVideo  *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);
  ClutterGstVideoTexture *vtex;
  gint                    position, duration, seek_width;

  vtex = CLUTTER_GST_VIDEO_TEXTURE(object);

  position = clutter_media_get_position (CLUTTER_MEDIA(vtex));
  duration = clutter_media_get_duration (CLUTTER_MEDIA(vtex));

  if (duration == 0 || position == 0)
    return;

  if (!priv->video_playing && position > 0)
    {
      char *duration_txt;
      duration_txt = nice_time (duration);
      clutter_label_set_text (CLUTTER_LABEL(priv->duration), duration_txt);
      g_free(duration_txt);

      priv->video_playing = TRUE;
#if 0
      char *duration_txt;

      g_signal_emit (screen, _screen_signals[PLAYBACK_STARTED], 0);

      util_actor_fade_in (CLUTTER_ACTOR(screen), NULL, NULL);

      priv->video_playing = TRUE;

      duration_txt = nice_time (duration);
      clutter_label_set_text (CLUTTER_LABEL(priv->duration), duration_txt);
      g_free(duration_txt);
#endif
    }

  seek_width = clutter_actor_get_width(priv->video_seekbar_bg);

  clutter_actor_set_size (priv->video_seekbar, 
			  (position * seek_width) / duration, 
			  20);  

  if (priv->video_controls_visible)
    {
      char *position_txt;      

      position_txt = nice_time (position);
      clutter_label_set_text (CLUTTER_LABEL(priv->position), position_txt);
      g_object_set (priv->position, "x",
		    ((position * seek_width) / duration) + 10,
		    NULL);
      g_free(position_txt);
    }
}

static void
video_hide_controls (WHScreenVideo *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  if (priv->video_controls_visible)
    {
      clutter_effect_fade (priv->controls_effect_tmpl,
			   priv->video_controls,
			   0xff,
			   0,
			   (ClutterEffectCompleteFunc)clutter_actor_hide,
			   NULL);

      priv->video_controls_visible = FALSE;
    }
}

static gboolean
video_controls_timeout_cb (WHScreenVideo *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);
 
  priv->controls_timeout = 0; 
  video_hide_controls (screen);
  return FALSE;
}

static void
video_show_controls (WHScreenVideo *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  if (!priv->video_controls_visible)
    {
      clutter_actor_show_all (CLUTTER_ACTOR(priv->video_controls));
      clutter_actor_set_opacity (CLUTTER_ACTOR(priv->video_controls), 0);

      printf("showing video controls\n");

      clutter_effect_fade (priv->controls_effect_tmpl,
			   priv->video_controls,
			   0,
			   0xff,
			   NULL,
			   NULL);
      priv->video_controls_visible = TRUE;

      priv->controls_timeout  
	= g_timeout_add (5 * 1000,  
			 (GSourceFunc)video_controls_timeout_cb,
			 screen);
    }
  else if (priv->controls_timeout)
    {
      g_source_remove (priv->controls_timeout);
      priv->controls_timeout  
	= g_timeout_add (5 * 1000,  
			 (GSourceFunc)video_controls_timeout_cb,
			 screen);
    }
    
}

static void 
video_input_cb (ClutterStage *stage,
		ClutterEvent *event,
		gpointer      user_data)
{
  WHScreenVideo        *screen = (WHScreenVideo*)user_data;
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  if (event->type == CLUTTER_KEY_RELEASE)
    {
      gchar buf[16];
      ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

      switch (clutter_key_event_symbol (kev))
	{
	case CLUTTER_Return:
	case CLUTTER_p:
	  if (clutter_media_get_playing (CLUTTER_MEDIA(priv->video)))
	    video_show_controls (screen);	    
	  clutter_media_set_playing 
	    (CLUTTER_MEDIA(priv->video),
		  !clutter_media_get_playing (CLUTTER_MEDIA(priv->video)));
	  break;
	case CLUTTER_Left:
	  video_show_controls (screen);
	  clutter_media_set_position 
	    (CLUTTER_MEDIA(priv->video),
	      clutter_media_get_position (CLUTTER_MEDIA(priv->video)) - 25);
	  break;
	case CLUTTER_Right:
	  video_show_controls (screen);
	  clutter_media_set_position 
	    (CLUTTER_MEDIA(priv->video),
	      clutter_media_get_position (CLUTTER_MEDIA(priv->video)) + 25);
	  break;
	case CLUTTER_Up:
	  clutter_media_set_volume 
	    (CLUTTER_MEDIA(priv->video),
	     clutter_media_get_volume (CLUTTER_MEDIA(priv->video)) + 0.1);
	  g_snprintf (buf, sizeof(buf), "Vol:%.2i", 
		      (gint)(clutter_media_get_volume (CLUTTER_MEDIA(priv->video))/0.1));
	  clutter_label_set_text (CLUTTER_LABEL(priv->vol_label), buf);
	  video_show_controls (screen);
	  break;
	case CLUTTER_Down:
	  clutter_media_set_volume 
	    (CLUTTER_MEDIA(priv->video),
	     clutter_media_get_volume (CLUTTER_MEDIA(priv->video)) - 0.1);
	  g_snprintf (buf, sizeof(buf), "Vol:%.2i", 
		      (gint)(clutter_media_get_volume (CLUTTER_MEDIA(priv->video))/0.1));
	  clutter_label_set_text (CLUTTER_LABEL(priv->vol_label), buf);
	  video_show_controls (screen);
	  break;
	case CLUTTER_e:
	  if (!clutter_timeline_is_playing (priv->cheese_timeline))
	    clutter_timeline_start (priv->cheese_timeline);
	  break;
	case CLUTTER_Escape:
	case CLUTTER_q:
	  wh_screen_video_deactivate (screen);
	  break;
	default:
	  break;
	}
    }
}


static void
wh_screen_video_paint (ClutterActor *actor)
{
  WHScreenVideo *screen = WH_SCREEN_VIDEO(actor);
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  clutter_actor_paint (priv->bg); 
  clutter_actor_paint (priv->video); 

  clutter_actor_paint (priv->video_controls); 
}

static void
wh_screen_video_query_coords (ClutterActor    *self,
			      ClutterActorBox *box)
{
  box->x1 = 0;
  box->y1 = 0;
  box->x2 = CSW();
  box->y2 = CSH();
}

static void
wh_screen_video_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_screen_video_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_screen_video_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (wh_screen_video_parent_class)->dispose)
    G_OBJECT_CLASS (wh_screen_video_parent_class)->dispose (object);
}

static void
wh_screen_video_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_screen_video_parent_class)->finalize (object);
}

static void
wh_screen_video_class_init (WHScreenVideoClass *klass)
{
  GObjectClass      *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class  = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (WHScreenVideoPrivate));

  object_class->get_property = wh_screen_video_get_property;
  object_class->set_property = wh_screen_video_set_property;
  object_class->dispose = wh_screen_video_dispose;
  object_class->finalize = wh_screen_video_finalize;

  actor_class->paint           = wh_screen_video_paint;
  actor_class->query_coords    = wh_screen_video_query_coords;

  _screen_signals[PLAYBACK_STARTED] =
    g_signal_new ("playback-started",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (WHScreenVideoClass, started),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  _screen_signals[PLAYBACK_FINISHED] =
    g_signal_new ("playback-finished",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (WHScreenVideoClass, finished),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

static void
video_make_controls (WHScreenVideo *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);
  gchar                 font_desc[32];
  gint                  h, w, so;
  ClutterActor         *actor;
  ClutterColor          seekcol = { 0xbb, 0xbb, 0xbb, 0xff },
                        txtcol = { 0x72, 0x9f, 0xcf, 0xff },
                        fgcol = { 0x72, 0x9f, 0xcf, 0xff };

  priv->video_controls = clutter_group_new();

  /* And this code here is why some kind of optional simple layout engine 
   * would be a good idea in cluter...
  */
  h = CSH()/6;
  w = CSW() - CSW()/4;

  actor = util_actor_from_file (PKGDATADIR "/header.svg", w, h);
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), actor);

  g_snprintf(font_desc, 32, "Sans Bold %ipx", h/8); 
  priv->duration = clutter_label_new_full (font_desc, "00:00", &fgcol);
  priv->position = clutter_label_new_full (font_desc, "00:00", &fgcol);

  so = clutter_actor_get_width (priv->position)/2 + 10;

  g_snprintf(font_desc, 32, "Sans Bold %ipx", h/6); 

  priv->title = clutter_label_new_with_text (font_desc, "");
  clutter_label_set_color (CLUTTER_LABEL(priv->title), &txtcol);
  clutter_label_set_line_wrap (CLUTTER_LABEL(priv->title), FALSE);
  clutter_label_set_ellipsize  (CLUTTER_LABEL(priv->title), 
				PANGO_ELLIPSIZE_MIDDLE);
  clutter_actor_set_width (priv->title, w/2);
  clutter_actor_set_position (priv->title, so, 10);
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->title);

  priv->vol_label = clutter_label_new_with_text (font_desc, "");
  clutter_label_set_color (CLUTTER_LABEL(priv->vol_label), &seekcol);
  clutter_label_set_line_wrap (CLUTTER_LABEL(priv->vol_label), FALSE);
  clutter_actor_set_width (priv->vol_label, w/8);
  clutter_actor_set_position (priv->vol_label, w-(w/8)-10, 10);
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->vol_label);

  /* Seek bar */
  priv->video_seekbar_bg = clutter_rectangle_new_with_color (&seekcol);
  clutter_actor_set_size (priv->video_seekbar_bg, w - (2*so), 20);
  clutter_actor_set_position (priv->video_seekbar_bg, so, 
			      15 + clutter_actor_get_height (priv->title));
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), 
		     priv->video_seekbar_bg);

  priv->video_seekbar = clutter_rectangle_new_with_color (&fgcol);
  clutter_actor_set_size (priv->video_seekbar, 0, 20);
  clutter_actor_set_position (priv->video_seekbar, so, 
			      15 + clutter_actor_get_height (priv->title));
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->video_seekbar);


  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->duration);
  clutter_actor_set_position (priv->duration, 
			      w - clutter_actor_get_width (priv->duration)-10, 
			      15 + clutter_actor_get_height (priv->title) + 20);  

  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->position);
  clutter_actor_set_position (priv->position, so, 15 + clutter_actor_get_height (priv->title) + 20);  

  clutter_actor_set_position (priv->video_controls, 
			      CSW()/8, h/3);

  clutter_actor_set_parent (CLUTTER_ACTOR(priv->video_controls), 
			    CLUTTER_ACTOR(screen)); 

  priv->controls_effect_tmpl 
    = clutter_effect_template_new (clutter_timeline_new (30, 60),
				   CLUTTER_ALPHA_SINE_INC);

  priv->fadein_effect_tmpl 
    = clutter_effect_template_new (clutter_timeline_new (30, 60),
				   CLUTTER_ALPHA_SINE_INC);

}

static void
wh_screen_video_init (WHScreenVideo *self)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(self);
  ClutterColor          black = { 0,0,0,255 };

  /* Create child video texture */
  priv->video = clutter_gst_video_texture_new ();

  /* Dont let the underlying pixbuf dictate size */
  g_object_set (G_OBJECT(priv->video), "sync-size", FALSE, NULL);

  /* Handle it ourselves so can scale up for fullscreen better */
  g_signal_connect (CLUTTER_TEXTURE(priv->video), 
		    "size-change",
		    G_CALLBACK (video_size_change), NULL);

  priv->bg = clutter_rectangle_new_with_color (&black);
  clutter_actor_set_size (priv->bg, 
			  CLUTTER_STAGE_WIDTH(), CLUTTER_STAGE_HEIGHT());

  clutter_actor_set_parent (priv->bg, CLUTTER_ACTOR(self)); 
  clutter_actor_set_parent (priv->video, CLUTTER_ACTOR(self)); 

  clutter_actor_show (priv->video);

  /* Make  */
  video_make_controls (self);

  /* Cheap effect */
  priv->cheese_timeline = clutter_timeline_new (30, 90);
  g_signal_connect (priv->cheese_timeline, "new-frame", 
		    G_CALLBACK (cheese_cb), self);

}

ClutterActor*
wh_screen_video_new (void)
{
  return CLUTTER_ACTOR(g_object_new (WH_TYPE_SCREEN_VIDEO, NULL));
}

static void
on_wh_screen_video_error (WHScreenVideo *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  /* Hack to stop looping on an unplayable file. 
   * FIXME: Need much better error handling..
  */

  g_signal_emit (screen, _screen_signals[PLAYBACK_STARTED], 0);

  g_signal_handlers_disconnect_by_func (priv->video, 
					G_CALLBACK (video_tick),
					screen);

  g_signal_handlers_disconnect_by_func(clutter_stage_get_default(),
				       video_input_cb,
				       screen);

  g_signal_emit (screen, _screen_signals[PLAYBACK_FINISHED], 0);
}


void
wh_screen_video_deactivate (WHScreenVideo *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  g_signal_handlers_disconnect_by_func (priv->video, 
					G_CALLBACK (video_tick),
					screen);

  clutter_media_set_playing (CLUTTER_MEDIA(priv->video), FALSE);

  priv->video_playing = FALSE;

  //util_actor_fade_out (CLUTTER_ACTOR(screen), NULL, NULL);

  video_hide_controls (screen);

  g_signal_handlers_disconnect_by_func(clutter_stage_get_default(),
				       video_input_cb,
				       screen);

  g_signal_emit (screen, _screen_signals[PLAYBACK_FINISHED], 0);
}

gboolean
wh_screen_video_get_playing (WHScreenVideo *screen)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);

  return priv->video_playing;
}

void
wh_screen_video_activate (WHScreenVideo *screen, WHVideoView *view)
{
  WHScreenVideoPrivate *priv  = SCREEN_VIDEO_PRIVATE(screen);
  gchar *episode = NULL, *series = NULL, *title = NULL, buf[16];

  priv->video_row = wh_video_view_get_selected (WH_VIDEO_VIEW(view));

  if (priv->video_row == NULL 
      || wh_video_model_row_get_path(priv->video_row) == NULL)
    return;

  g_signal_connect (clutter_stage_get_default(), 
		    "event",
		    G_CALLBACK (video_input_cb),
		    screen);

  g_signal_connect (priv->video,
		    "notify::position",
		    G_CALLBACK (video_tick),
		    screen);

  g_signal_connect_swapped (priv->video,
		    "eos",
		    G_CALLBACK (wh_screen_video_deactivate),
		    screen);

  g_signal_connect_swapped (priv->video,
		    "error",
		    G_CALLBACK (on_wh_screen_video_error),
		    screen);

  g_signal_connect (priv->video,
		    "pixbuf-change",
		    G_CALLBACK(video_pixbuf_change),
		    screen);

  priv->video_controls_visible = FALSE;

  g_snprintf (buf, sizeof(buf), "Vol:%.2i", 
	      (gint)(clutter_media_get_volume (CLUTTER_MEDIA(priv->video))/0.1));
  clutter_label_set_text (CLUTTER_LABEL(priv->vol_label), buf);

  wh_video_model_row_get_extended_info (priv->video_row, &series, &episode);
  
  title = g_strdup_printf("%s%s%s%s%s%s",
			  wh_video_model_row_get_title (priv->video_row),
			  (series != NULL || episode != NULL) ? " (" : "", 
			  series != NULL  ?  series : "",
			  (series != NULL && episode != NULL) ? "/" : "", 
			  episode != NULL ?  episode : "",
			  (series != NULL || episode != NULL) ? ")" : "");
      
  clutter_label_set_text (CLUTTER_LABEL(priv->title), title);
  clutter_actor_set_width (priv->title, CSW()/2);

  g_free (title);

  clutter_media_set_uri(CLUTTER_MEDIA(priv->video), 
			wh_video_model_row_get_path(priv->video_row));
  clutter_media_set_playing (CLUTTER_MEDIA(priv->video), TRUE);
}
