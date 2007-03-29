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
  ClutterActor      *duration, *title, *position;
  ClutterTimeline   *foo_effect;
  gboolean           video_playing;
  gboolean           video_controls_visible;
  
  guint              controls_timeout;
  WHVideoModelRow   *video_row;
};

enum
{
  PLAYBACK_STARTED,
  PLAYBACK_FINISHED,
  LAST_SIGNAL
};

static guint _screen_signals[LAST_SIGNAL] = { 0 };


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

      g_signal_emit (screen, _screen_signals[PLAYBACK_STARTED], 0);

      util_actor_fade_in (CLUTTER_ACTOR(screen), NULL, NULL);

      priv->video_playing = TRUE;

      duration_txt = nice_time (duration);
      clutter_label_set_text (CLUTTER_LABEL(priv->duration), duration_txt);
      g_free(duration_txt);
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
      util_actor_fade_out (priv->video_controls, NULL, NULL);
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
      util_actor_fade_in (priv->video_controls, NULL, NULL);
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
      ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

      switch (clutter_key_event_symbol (kev))
	{
	case CLUTTER_p:
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
	  break;
	case CLUTTER_Down:
	  clutter_media_set_volume 
	    (CLUTTER_MEDIA(priv->video),
	     clutter_media_get_volume (CLUTTER_MEDIA(priv->video)) - 0.1);
	  break;
	case CLUTTER_Return:
	  video_show_controls (screen);
	  if (!priv->video_playing)
	    break;
	  break;
	case CLUTTER_e:
	  /*
	  if (!clutter_timeline_is_playing (screen->foo_effect))
	    clutter_timeline_start (screen->foo_effect);
	  */
	  break;
	case CLUTTER_Escape:
	  wh_screen_video_deactivate (screen);
	  break;
	case CLUTTER_q:
	  clutter_main_quit();
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
wh_screen_video_allocate_coords (ClutterActor    *self,
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
  actor_class->allocate_coords = wh_screen_video_allocate_coords;

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
  ClutterColor          bgcol = { 0x4f, 0x4f, 0x68, 0x99 },
                        seekcol = { 0x99, 0x99, 0xa8, 0xff },
                        txtcol = { 0xb4, 0xe2, 0xff, 0xff },
                        fgcol = { 0xff, 0xff, 0xff, 0xff };

  priv->video_controls = clutter_group_new();

  /* And this code here is why some kind of optional simple layout engine 
   * would be a good idea in cluter...
  */
  h = CSH()/6;
  w = CSW() - CSW()/4;
  
  actor = clutter_rectangle_new_with_color (&bgcol);
  clutter_actor_set_size (actor, w, h);
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), actor);

  g_snprintf(font_desc, 32, "Sans %ipx", h/5); 
  priv->duration = clutter_label_new_full (font_desc, "00:00", &fgcol);
  priv->position = clutter_label_new_full (font_desc, "00:00", &fgcol);

  so = clutter_actor_get_width (priv->position)/2 + 10;

  g_snprintf(font_desc, 32, "Sans %ipx", h/3); 

  priv->title = clutter_label_new_with_text (font_desc, "");
  clutter_label_set_color (CLUTTER_LABEL(priv->title), &txtcol);
  clutter_label_set_line_wrap (CLUTTER_LABEL(priv->title), FALSE);
  clutter_label_set_ellipsize  (CLUTTER_LABEL(priv->title), 
				PANGO_ELLIPSIZE_MIDDLE);
  clutter_actor_set_width (priv->title, so * 2);
  clutter_actor_set_position (priv->title, so, 10);
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->title);

  /* Seek bar */
  priv->video_seekbar_bg = clutter_rectangle_new_with_color (&seekcol);
  clutter_actor_set_size (priv->video_seekbar_bg, w - (2*so), 20);
  clutter_actor_set_position (priv->video_seekbar_bg, so, h/2);
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), 
		     priv->video_seekbar_bg);

  priv->video_seekbar = clutter_rectangle_new_with_color (&fgcol);
  clutter_actor_set_size (priv->video_seekbar, 0, 20);
  clutter_actor_set_position (priv->video_seekbar, so, h/2);
  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->video_seekbar);


  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->duration);
  clutter_actor_set_position (priv->duration, 
			      w - clutter_actor_get_width (priv->duration)-10, 
			      h/2 + 20);  

  clutter_group_add (CLUTTER_GROUP(priv->video_controls), priv->position);
  clutter_actor_set_position (priv->position, so, h/2 + 20);  

  clutter_actor_set_position (priv->video_controls, 
			      CSW()/8, CSH() - CSH()/3);

  clutter_actor_set_parent (CLUTTER_ACTOR(priv->video_controls), 
			    CLUTTER_ACTOR(screen)); 
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

#if 0
  /* FIXME: Can cause crashes with current thumbnailer code so  
   * disabled. Need to investigate further.       
   *
  */
  if (priv->video 
      && (wh_video_model_row_get_thumbnail (priv->video_row) == NULL
	  || wh_video_model_row_get_thumbnail (priv->video_row) != 
	                      wh_theme_get_pixbuf("default-thumbnail")))
    {
      GdkPixbuf *shot;
      shot = clutter_texture_get_pixbuf (CLUTTER_TEXTURE(priv->video));
      
      if (shot)
	{
	  GdkPixbuf *thumb, *pic;
	  gint       x, y, nw, nh, w, h, size;

	  size = 100;
	  w = clutter_actor_get_width (priv->video);
	  h = clutter_actor_get_height (priv->video);
	  
	  nh = ( h * size) / w;
      
	  if (nh <= size)
	    {
	      nw = size;
	      x = 0;
	      y = (size - nh) / 2;
	    }
	  else
	    {
	      nw  = ( w * size ) / h;
	      nh = size;
	      x = (size - nw) / 2;
	      y = 0;
	    }

	  thumb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, size, size);
	  gdk_pixbuf_fill (thumb, 0x000000FF);
	  pic = gdk_pixbuf_scale_simple (shot, nw, nh, GDK_INTERP_BILINEAR);
	  gdk_pixbuf_copy_area  (pic, 0, 0, nw, nh, thumb, x, y);
	  
	  wh_video_model_row_set_thumbnail (priv->video_row, thumb);

	  g_object_unref (shot);
	  g_object_unref (thumb);
	  g_object_unref (pic);
	}
    }
#endif

  priv->video_playing = FALSE;

  util_actor_fade_out (CLUTTER_ACTOR(screen), NULL, NULL);

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
  gchar *episode = NULL, *series = NULL, *title = NULL;

  priv->video_row = wh_video_view_get_selected (WH_VIDEO_VIEW(view));

  if (priv->video_row == NULL 
      || wh_video_model_row_get_path(priv->video_row) == NULL)
    return;

  g_signal_connect (clutter_stage_get_default(), 
		    "input-event",
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

  priv->video_controls_visible = FALSE;

  wh_video_model_row_get_extended_info (priv->video_row, &series, &episode);
  
  title = g_strdup_printf("%s%s%s%s%s%s",
			  wh_video_model_row_get_title (priv->video_row),
			  (series != NULL || episode != NULL) ? " (" : "", 
			  series != NULL  ?  series : "",
			  (series != NULL && episode != NULL) ? "/" : "", 
			  episode != NULL ?  episode : "",
			  (series != NULL || episode != NULL) ? ")" : "");
      
  clutter_label_set_text (CLUTTER_LABEL(priv->title), title);

  g_free (title);

  clutter_media_set_filename(CLUTTER_MEDIA(priv->video), 
			     wh_video_model_row_get_path(priv->video_row));
  clutter_media_set_playing (CLUTTER_MEDIA(priv->video), TRUE);
}
