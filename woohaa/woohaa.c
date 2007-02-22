#include <clutter/clutter.h>
#include <clutter-gst/clutter-gst.h>
#include <gst/gst.h>
#include <sqlite3.h>

#include "clutter-slider-menu.h"
#include "clutter-simple-layout.h"
#include "wh-video-model.h"
#include "wh-video-model-row.h"
#include "wh-video-view.h"
#include "wh-db.h"
#include "util.h"

#define FONT "Sans Bold 56"

#define CSW() CLUTTER_STAGE_WIDTH()
#define CSH() CLUTTER_STAGE_HEIGHT()

typedef struct MovieApp
{
  ClutterActor      *screen_browse, *screen_video;
  WHVideoModel      *model;
  
  ClutterActor      *view;
  ClutterActor      *menu;
  ClutterTimeline   *fade_timeline;
  ClutterAlpha      *fade_alpha;

  ClutterActor      *video;
  ClutterActor      *video_busy;
  ClutterActor      *video_seekbar;
  ClutterActor      *video_controls;
  ClutterTimeline   *video_busy_timeline, *foo_effect;
  gboolean           video_playing;
  gboolean           video_controls_visible;

  sqlite3           *db;
}
MovieApp;

void 
browse_input_cb (ClutterStage *stage,
		 ClutterEvent *event,
		 gpointer      user_data);

void 
video_input_cb (ClutterStage *stage,
		ClutterEvent *event,
		gpointer      user_data);

void
foo_effect_cb (ClutterTimeline *timeline, 
	       gint             frame_num, 
	       MovieApp        *app)
{
  clutter_actor_rotate_y (app->video,
			  frame_num * 12,
			  CLUTTER_STAGE_WIDTH()/2,
			  0);
}

void
video_busy_timeline_cb (ClutterTimeline *timeline, 
			gint             frame_num, 
			MovieApp        *app)
{
  clutter_actor_rotate_z(app->video_busy, (float)frame_num * 2.0, 
			 clutter_actor_get_width (app->video_busy)/2, 
			 clutter_actor_get_height (app->video_busy)/2);
}


void
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
  
  clutter_actor_set_size (CLUTTER_ACTOR (texture),
			  new_width,
			  new_height);
}


void 
video_tick (GObject      *object,
	    GParamSpec   *pspec,
	    MovieApp     *app)
{
  ClutterGstVideoTexture *vtex;
  gint                    position, duration;

  vtex = CLUTTER_GST_VIDEO_TEXTURE(object);

  position = clutter_media_get_position (CLUTTER_MEDIA(vtex));
  duration = clutter_media_get_duration (CLUTTER_MEDIA(vtex));

  if (duration == 0 || position == 0)
    return;

  if (!app->video_playing && position > 0)
    {
      WHVideoModelRow *row = NULL;

      util_actor_fade_in (app->screen_video);

      clutter_timeline_stop (app->video_busy_timeline);
#if 0
      /* FIXME need a call back on the fade in complete call 
       *       to simple hide..
       * util_actor_fade_in () needs a 'complete' callback
      */
      util_actor_fade_out (app->video_busy);
      util_actor_fade_out (app->screen_browse);
#endif
      /* Update underlying db */
      row = wh_video_view_get_selected (WH_VIDEO_VIEW(app->view));

      wh_video_view_enable_animation (WH_VIDEO_VIEW(app->view), FALSE);
      wh_video_model_row_set_n_views (row, 
				      wh_video_model_row_get_n_views(row)+1);
      wh_video_model_row_set_vtime (row, time(NULL));
      wh_db_sync_row (app->db, row);

      app->video_playing = TRUE;
    }

  clutter_actor_set_size (app->video_seekbar, 
			  (position * (CSW() - CSW()/4 - 20)) / duration, 
			  20);  
}

void
video_stop (MovieApp *app)
{
  clutter_media_set_playing (CLUTTER_MEDIA(app->video), FALSE);

  g_signal_handlers_disconnect_by_func (app->video, 
					G_CALLBACK (video_tick),
					app);

  app->video_playing = FALSE;
  wh_video_view_enable_animation (WH_VIDEO_VIEW(app->view), TRUE);

  util_actor_fade_out (app->screen_video);

  clutter_simple_layout_fade_in
    (CLUTTER_SIMPLE_LAYOUT(app->screen_browse), app->fade_alpha); 

  clutter_actor_hide (app->video_busy);
  clutter_actor_hide_all (app->video_controls);

  g_signal_handlers_disconnect_by_func(clutter_stage_get_default(),
				       video_input_cb,
				       app);

  g_signal_connect (clutter_stage_get_default(), 
		    "input-event",
		    G_CALLBACK (browse_input_cb),
		    app);
}

void 
video_input_cb (ClutterStage *stage,
		ClutterEvent *event,
		gpointer      user_data)
{
  MovieApp *app = (MovieApp*)user_data;

  if (event->type == CLUTTER_KEY_RELEASE)
    {
      ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

      switch (clutter_key_event_symbol (kev))
	{
	case CLUTTER_p:
	  clutter_media_set_playing 
	    (CLUTTER_MEDIA(app->video),
		  !clutter_media_get_playing (CLUTTER_MEDIA(app->video)));
	  break;
	case CLUTTER_Left:
	  clutter_media_set_position 
	    (CLUTTER_MEDIA(app->video),
		  clutter_media_get_position (CLUTTER_MEDIA(app->video)) - 25);
	  break;
	case CLUTTER_Right:
	  clutter_media_set_position 
	    (CLUTTER_MEDIA(app->video),
		  clutter_media_get_position (CLUTTER_MEDIA(app->video)) + 25);
	  break;
	case CLUTTER_Return:
	  if (!app->video_playing)
	    break;

	  if (app->video_controls_visible)
	    clutter_actor_hide_all (app->video_controls);
	  else
	    clutter_actor_show_all (app->video_controls);

	  app->video_controls_visible = ~app->video_controls_visible;
	  break;
	case CLUTTER_e:
	  if (!clutter_timeline_is_playing (app->foo_effect))
	    clutter_timeline_start (app->foo_effect);
	  break;
	case CLUTTER_Escape:
	  video_stop (app);
	  break;
	case CLUTTER_q:
	  clutter_main_quit();
	  break;
	default:
	  break;
	}
    }
}

void
video_start (MovieApp *app)
{
  WHVideoModelRow *row = NULL;

  row = wh_video_view_get_selected (WH_VIDEO_VIEW(app->view));

  if (row == NULL || wh_video_model_row_get_path(row) == NULL)
    return;

  g_signal_handlers_disconnect_by_func(clutter_stage_get_default(),
				       browse_input_cb,
				       app);

  g_signal_connect (clutter_stage_get_default(), 
		    "input-event",
		    G_CALLBACK (video_input_cb),
		    app);

  clutter_simple_layout_fade_out 
    (CLUTTER_SIMPLE_LAYOUT(app->screen_browse), app->fade_alpha); 

  util_actor_fade_in (app->video_busy);
  clutter_timeline_start (app->video_busy_timeline);

  g_signal_connect (app->video,
		    "notify::position",
		    G_CALLBACK (video_tick),
		    app);

  app->video_controls_visible = FALSE;

  clutter_media_set_filename(CLUTTER_MEDIA(app->video), 
			     wh_video_model_row_get_path(row));
  clutter_media_set_playing (CLUTTER_MEDIA(app->video), TRUE);
}

void
video_make_controls (MovieApp *app)
{
  ClutterActor *actor;
  ClutterColor  bgcol = { 0xff, 0xff, 0xff, 0x99 },
                 seekcol = { 0xcc, 0xcc, 0xcc, 0x99 },
                fgcol = { 0, 0, 0, 0xff };

  /* video controls */
  app->video_controls = clutter_group_new();
  
  actor = clutter_rectangle_new_with_color (&bgcol);
  clutter_actor_set_size (actor, CSW() - CSW()/4, CSH()/8);
  clutter_group_add (CLUTTER_GROUP(app->video_controls), actor);

  actor = clutter_rectangle_new_with_color (&seekcol);
  clutter_actor_set_size (actor, CSW() - CSW()/4 - 20, 20);
  clutter_actor_set_position (actor, 10, CSH()/8 - 40);
  clutter_group_add (CLUTTER_GROUP(app->video_controls), actor);

  app->video_seekbar = clutter_rectangle_new_with_color (&fgcol);
  clutter_actor_set_size (app->video_seekbar, 0, 20);
  clutter_actor_set_position (app->video_seekbar, 10, CSH()/8 - 40);
  clutter_group_add (CLUTTER_GROUP(app->video_controls), app->video_seekbar);

  clutter_group_add (CLUTTER_GROUP(clutter_stage_get_default ()), 
		     app->video_controls);

  clutter_actor_set_position (app->video_controls, CSW()/8, CSH() - CSH()/6);

  app->foo_effect = clutter_timeline_new (30, 90);
  g_signal_connect (app->foo_effect, "new-frame", 
		    G_CALLBACK (foo_effect_cb), app);

}

void 
browse_input_cb (ClutterStage *stage,
		 ClutterEvent *event,
		 gpointer      user_data)
{
  MovieApp *app = (MovieApp*)user_data;

  if (event->type == CLUTTER_KEY_RELEASE)
    {
      ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

      switch (clutter_key_event_symbol (kev))
	{
	case CLUTTER_Left:
	  clutter_slider_menu_advance (CLUTTER_SLIDER_MENU(app->menu), -1);
	  break;
	case CLUTTER_Right:
	  clutter_slider_menu_advance (CLUTTER_SLIDER_MENU(app->menu), 1);
	  break;
	case CLUTTER_Up:
	  wh_video_view_advance (WH_VIDEO_VIEW(app->view), -1);
	  break;
	case CLUTTER_Down:
	  wh_video_view_advance (WH_VIDEO_VIEW(app->view), 1);
	  break;
	case CLUTTER_Return:
	  video_start (app);
	  break;
	case CLUTTER_q:
	  clutter_main_quit();
	  break;
	default:
	  break;
;	}
    }
}


static gint
model_sort_mtime (WHVideoModelRow *rowa, WHVideoModelRow *rowb, gpointer data)
{
  time_t a, b;

  a = wh_video_model_row_get_age(rowa);
  b = wh_video_model_row_get_age(rowb);

  if (a > b) return -1;
  else if (a < b) return 1;
  else  return 0;
}

static gboolean  
model_recently_added_filter (WHVideoModel    *model,
			     WHVideoModelRow *row,
			     gpointer         data)
{
  return TRUE;
}

static void
view_recently_added_selected (ClutterSliderMenu *menu,
			      ClutterActor      *actor,
			      gpointer           userdata)
{
  MovieApp *app = (MovieApp*)userdata;

  wh_video_model_set_filter (app->model, model_recently_added_filter, app);
  wh_video_model_set_sort_func (app->model, model_sort_mtime, NULL);
}

static gint
model_sort_vtime (WHVideoModelRow *rowa, WHVideoModelRow *rowb, gpointer data)
{
  time_t a, b;

  a = wh_video_model_row_get_vtime(rowa);
  b = wh_video_model_row_get_vtime(rowb);

  if (a > b) return -1;
  else if (a < b) return 1;
  else  return 0;
}

static gboolean  
model_recently_viewed_filter (WHVideoModel    *model,
			      WHVideoModelRow *row,
			      gpointer         data)
{
  if (wh_video_model_row_get_n_views(row) == 0) 
    return FALSE;

  return TRUE;
}

void
view_recently_viewed_selected (ClutterSliderMenu *menu,
			      ClutterActor      *actor,
			      gpointer           userdata)
{
  MovieApp *app = (MovieApp*)userdata;

  wh_video_model_set_filter (app->model, model_recently_viewed_filter, app);
  wh_video_model_set_sort_func (app->model, model_sort_vtime, NULL);
}

static gboolean  
model_not_viewed_filter (WHVideoModel    *model,
			 WHVideoModelRow *row,
			 gpointer         data)
{
  if (wh_video_model_row_get_n_views(row) > 0) 
    return FALSE;

  return TRUE;
}

void
view_not_viewed_selected (ClutterSliderMenu *menu,
			  ClutterActor      *actor,
			  gpointer           userdata)
{
  MovieApp *app = (MovieApp*)userdata;

  wh_video_model_set_filter (app->model, model_not_viewed_filter, app);
  wh_video_model_set_sort_func (app->model, model_sort_mtime, NULL);
}

static gint
model_sort_popular (WHVideoModelRow *rowa, 
		    WHVideoModelRow *rowb, 
		    gpointer         data)
{
  gint a, b;

  a = wh_video_model_row_get_n_views(rowa);
  b = wh_video_model_row_get_n_views(rowb);

  if (a > b) return -1;
  else if (a < b) return 1;
  else  return 0;
}


void
view_popular_selected (ClutterSliderMenu *menu,
		       ClutterActor      *actor,
		       gpointer           userdata)
{
  MovieApp *app = (MovieApp*)userdata;

  wh_video_model_set_filter (app->model, model_recently_viewed_filter, app);
  wh_video_model_set_sort_func (app->model, model_sort_popular, NULL);
}

int
main (int argc, char *argv[])
{
  MovieApp      *app;
  ClutterActor  *stage, *label, *box;
  GdkPixbuf     *pixbuf;
  ClutterColor   stage_color = { 0x4a, 0x52, 0x5a, 0xff },
                 text_color   = { 0xff, 0xff, 0xff, 0xff },
		 black       = { 0,0,0,255 };

  gst_init (&argc, &argv);

  clutter_init (&argc, &argv);

  if (argc < 2)
      g_error ("Usage: %s <path to movie files>\n", argv[0]);

  app = g_new0(MovieApp, 1);

  app->db = wh_db_init (argv[1]);
  app->model = wh_video_model_new ();

  wh_db_populate_model (app->db, app->model);

  wh_video_model_set_filter (app->model, model_recently_added_filter, app);
  wh_video_model_set_sort_func (app->model, model_sort_mtime, NULL);

  stage = clutter_stage_get_default ();

  g_object_set (stage, "fullscreen", TRUE, NULL);

  clutter_stage_set_color (CLUTTER_STAGE (stage),
		           &stage_color);

  app->screen_browse = clutter_simple_layout_new();
  clutter_actor_set_size (app->screen_browse, CSW(), CSH());

  app->menu = clutter_slider_menu_new ();

  label = clutter_label_new_with_text (FONT, "Recently Added");
  clutter_label_set_color (CLUTTER_LABEL(label), &text_color);
  clutter_slider_menu_append_actor (CLUTTER_SLIDER_MENU(app->menu), 
				    label, view_recently_added_selected, app);

  label = clutter_label_new_with_text (FONT, "Recently Viewed");
  clutter_label_set_color (CLUTTER_LABEL(label), &text_color);
  clutter_slider_menu_append_actor (CLUTTER_SLIDER_MENU(app->menu), 
				    label, view_recently_viewed_selected, app);

  label = clutter_label_new_with_text (FONT, "Not Viewed");
  clutter_label_set_color (CLUTTER_LABEL(label), &text_color);
  clutter_slider_menu_append_actor (CLUTTER_SLIDER_MENU(app->menu), 
				    label, view_not_viewed_selected, app);

  label = clutter_label_new_with_text (FONT, "Popular");
  clutter_label_set_color (CLUTTER_LABEL(label), &text_color);
  clutter_slider_menu_append_actor (CLUTTER_SLIDER_MENU(app->menu), 
				    label, view_popular_selected, app);

  clutter_group_add (CLUTTER_GROUP(app->screen_browse), app->menu);
  g_object_set(app->menu, "y", 25, NULL); 

  app->view = wh_video_view_new (app->model, 6);
  clutter_actor_set_size (app->view, CSW() - (CSW()/8), 600);
  clutter_group_add (CLUTTER_GROUP(app->screen_browse), app->view);

  g_object_set(app->view, "y", 
	       clutter_actor_get_y(app->menu) 
	         + clutter_actor_get_height(app->menu) + 25, NULL); 

  clutter_group_add (CLUTTER_GROUP (stage), app->screen_browse);

  /* Busy */
  pixbuf = gdk_pixbuf_new_from_file ("busy.png", NULL); 

  app->video_busy = clutter_texture_new_from_pixbuf (pixbuf);

  clutter_actor_set_position 
    (app->video_busy, 
     (CSW() - clutter_actor_get_width (app->video_busy))/2,
     (CSH() - clutter_actor_get_height (app->video_busy))/2);

  clutter_group_add (CLUTTER_GROUP(stage), app->video_busy);

  app->video_busy_timeline = clutter_timeline_new (180, 120);
  clutter_timeline_set_loop (app->video_busy_timeline, TRUE);
  g_signal_connect (app->video_busy_timeline, 
		    "new-frame", 
		    G_CALLBACK (video_busy_timeline_cb), 
		    app);

  /* Video */
  app->screen_video = clutter_group_new();
  app->video        = clutter_gst_video_texture_new ();

  /* Dont let the underlying pixbuf dictate size */
  g_object_set (G_OBJECT(app->video), "sync-size", FALSE, NULL);

  /* Handle it ourselves so can scale up for fullscreen better */
  g_signal_connect (CLUTTER_TEXTURE(app->video), 
		    "size-change",
		    G_CALLBACK (video_size_change), NULL);

  box = clutter_rectangle_new_with_color (&black);
  clutter_actor_set_size (box, CLUTTER_STAGE_WIDTH(), CLUTTER_STAGE_HEIGHT());

  clutter_group_add_many (CLUTTER_GROUP(app->screen_video), 
			  box, app->video, NULL);
  clutter_group_add (CLUTTER_GROUP(stage), app->screen_video);

  video_make_controls (app);

  /* Show our screen and stage */
  clutter_actor_show (stage);
  clutter_group_show_all (CLUTTER_GROUP (app->screen_browse));

  /* General timeline */
  app->fade_timeline = clutter_timeline_new (20, 120);
  app->fade_alpha = clutter_alpha_new_full (app->fade_timeline,
					    alpha_sine_inc_func,
					    NULL, NULL);
  g_signal_connect (stage, 
		    "input-event",
		    G_CALLBACK (browse_input_cb),
		    app);
  clutter_main();

  return 0;
}
