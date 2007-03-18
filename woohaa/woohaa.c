#include <clutter/clutter.h>
#include <clutter-gst/clutter-gst.h>
#include <gst/gst.h>
#include <sqlite3.h>
#include <string.h>

#include "wh-slider-menu.h"
#include "clutter-simple-layout.h"
#include "wh-video-model.h"
#include "wh-video-model-row.h"
#include "wh-video-view.h"
#include "wh-db.h"
#include "wh-screen-video.h"
#include "wh-busy.h"
#include "util.h"

#define FONT "VistaSansBook 75px"

typedef struct WooHaa
{
  ClutterActor      *screen_browse, *screen_video;
  WHVideoModel      *model;
  ClutterActor      *view;
  ClutterActor      *menu;
  ClutterActor      *busy;
  WHDB              *db;
}
WooHaa;

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
	       WooHaa          *wh)
{
  /*
  clutter_actor_rotate_y (wh->video,
			  frame_num * 12,
			  CLUTTER_STAGE_WIDTH()/2,
			  0);
  */
}

void
playback_finish_complete (ClutterActor *actor, WooHaa *wh)
{
  wh_video_view_enable_animation (WH_VIDEO_VIEW(wh->view), TRUE);
}

void
on_video_playback_start (WHScreenVideo *video,
			 WooHaa      *wh)
{
  clutter_actor_raise_top (wh->screen_browse);

  util_actor_fade (wh->screen_browse, NULL, 0x99, 0, NULL);
  util_actor_zoom (wh->screen_browse, NULL, 1.0, 0.5, NULL);
  util_actor_fade (wh->screen_video, NULL, 0, 0xff, NULL);

  wh_video_view_enable_animation (WH_VIDEO_VIEW(wh->view), FALSE);

  /* Stop the busy 'cursor' */
  util_actor_fade_out (wh->busy, NULL, NULL);
}

void
on_video_playback_finish (WHScreenVideo *video,
			  WooHaa        *wh)
{
  WHVideoModelRow *row;

  g_signal_connect (clutter_stage_get_default(),
		    "input-event",
		    G_CALLBACK (browse_input_cb),
		    wh);

  /* update db */
  row = wh_video_view_get_selected (WH_VIDEO_VIEW(wh->view));
  wh_video_model_row_set_n_views (row, 
				  wh_video_model_row_get_n_views(row)+1);
  wh_video_model_row_set_vtime (row, time(NULL));
  wh_db_sync_row (row);
  
  clutter_actor_show (wh->screen_browse);

  util_actor_fade (wh->screen_browse, NULL, 0, 0xff, NULL);
  util_actor_zoom (wh->screen_browse, NULL, 0.5, 1.0, NULL);
  util_actor_fade (wh->screen_video, 
		   (UtilAnimCompleteFunc)playback_finish_complete, 
		   0xff, 0, wh);
}

void 
browse_input_cb (ClutterStage *stage,
		 ClutterEvent *event,
		 gpointer      user_data)
{
  WooHaa *wh = (WooHaa*)user_data;

  if (event->type == CLUTTER_KEY_RELEASE)
    {
      ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

      switch (clutter_key_event_symbol (kev))
	{
	case CLUTTER_Left:
	  wh_slider_menu_advance (WH_SLIDER_MENU(wh->menu), -1);
	  break;
	case CLUTTER_Right:
	  wh_slider_menu_advance (WH_SLIDER_MENU(wh->menu), 1);
	  break;
	case CLUTTER_Up:
	  wh_video_view_advance (WH_VIDEO_VIEW(wh->view), -1);
	  break;
	case CLUTTER_Down:
	  wh_video_view_advance (WH_VIDEO_VIEW(wh->view), 1);
	  break;
	case CLUTTER_Return:
	  g_signal_handlers_disconnect_by_func(clutter_stage_get_default(),
					       browse_input_cb,
					       wh);

	  wh_screen_video_activate (WH_SCREEN_VIDEO(wh->screen_video), 
				    WH_VIDEO_VIEW(wh->view));

	  util_actor_fade (wh->screen_browse, NULL, 0xff, 0x99, NULL);
	  util_actor_fade_in (wh->busy, NULL, NULL);
	  break;
	case CLUTTER_q:
	  clutter_main_quit();
	  break;
	default:
	  break;
;	}
    }
}

void
screen_fadeout_complete (ClutterActor *actor, WooHaa *wh)
{
  g_object_ref (wh->busy);
  clutter_group_remove (CLUTTER_GROUP(actor), wh->busy);
  clutter_group_add (CLUTTER_GROUP(clutter_stage_get_default()), wh->busy);
  g_object_unref (wh->busy);
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
view_recently_added_selected (WHSliderMenu *menu,
			      ClutterActor      *actor,
			      gpointer           userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_recently_added_filter, wh);
  wh_video_model_set_sort_func (wh->model, model_sort_mtime, NULL);
}

static gint
model_sort_alpha (WHVideoModelRow *rowa, WHVideoModelRow *rowb, gpointer data)
{
  return strcmp (wh_video_model_row_get_title(rowa),
		 wh_video_model_row_get_title(rowb));
}

#define FILTER_AF 0
#define FILTER_GL 1
#define FILTER_MR 2
#define FILTER_SZ 3

static gboolean  
model_alpha_filter (WHVideoModel    *model,
		    WHVideoModelRow *row,
		    gpointer         data)
{
  gint type = GPOINTER_TO_INT(data);
  gchar lo, hi;
  const gchar *title;

  title = wh_video_model_row_get_title(row);

  switch (type)
    {
    case FILTER_AF:
      lo = 'a'; hi = 'f';
      /* Also numerical titles */
      if (g_ascii_tolower(title[0]) >= '0' && g_ascii_tolower(title[0]) <= '9')
	return TRUE;
      break;
    case FILTER_GL:
      lo = 'g'; hi = 'l';
      break;
    case FILTER_MR:
      lo = 'm'; hi = 'r';
      break;
    case FILTER_SZ:
      lo = 's'; hi = 'z';
      break;
    default:
      lo = 's'; hi = 'z';
      break;
    }

  /* FIXME UTF8 */
  if (g_ascii_tolower(title[0]) >= lo && g_ascii_tolower(title[0]) <= hi)
    return TRUE;

  return FALSE;
}

static void
view_af_selected (WHSliderMenu *menu,
		  ClutterActor      *actor,
		  gpointer           userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_alpha_filter, 
			     GINT_TO_POINTER(FILTER_AF));
  wh_video_model_set_sort_func (wh->model, model_sort_alpha, NULL);
}

static void
view_gl_selected (WHSliderMenu *menu,
		  ClutterActor      *actor,
		  gpointer           userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_alpha_filter, 
			     GINT_TO_POINTER(FILTER_GL));
  wh_video_model_set_sort_func (wh->model, model_sort_alpha, NULL);
}

static void
view_mr_selected (WHSliderMenu *menu,
		  ClutterActor      *actor,
		  gpointer           userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_alpha_filter, 
			     GINT_TO_POINTER(FILTER_MR));
  wh_video_model_set_sort_func (wh->model, model_sort_alpha, NULL);
}

static void
view_sz_selected (WHSliderMenu *menu,
		  ClutterActor      *actor,
		  gpointer           userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_alpha_filter, 
			     GINT_TO_POINTER(FILTER_SZ));
  wh_video_model_set_sort_func (wh->model, model_sort_alpha, NULL);
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
view_recently_viewed_selected (WHSliderMenu  *menu,
			       ClutterActor  *actor,
			       gpointer       userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_recently_viewed_filter, wh);
  wh_video_model_set_sort_func (wh->model, model_sort_vtime, NULL);
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
view_not_viewed_selected (WHSliderMenu *menu,
			  ClutterActor      *actor,
			  gpointer           userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_not_viewed_filter, wh);
  wh_video_model_set_sort_func (wh->model, model_sort_mtime, NULL);
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
view_popular_selected (WHSliderMenu *menu,
		       ClutterActor *actor,
		       gpointer      userdata)
{
  WooHaa *wh = (WooHaa*)userdata;

  wh_video_model_set_filter (wh->model, model_recently_viewed_filter, wh);
  wh_video_model_set_sort_func (wh->model, model_sort_popular, NULL);
}

void
on_db_row_created (WHDB *db, WHVideoModelRow *row, gpointer data)
{
  WooHaa *wh = (WooHaa *)data;

  wh_video_model_row_set_renderer (row, wh_video_row_renderer_new (row)); 
  wh_video_model_append_row (wh->model, row);
}


int
main (int argc, char *argv[])
{
  WooHaa        *wh;
  ClutterActor  *stage, *bg, *screen_start, *desktop, *label;
  gchar         **pathv;
  gint           i = 0;
  ClutterColor   stage_color = { 0x4a, 0x52, 0x5a, 0xff },
                 white_col   = { 0xff, 0xff, 0xff, 0xff};

  gst_init (&argc, &argv);

  clutter_init (&argc, &argv);

  if (argc < 2)
    {
      g_print ("Usage: %s <path to movie files>\n", argv[0]);
      exit(-1);
    }

  wh = g_new0(WooHaa, 1);

  wh->model = wh_video_model_new ();
  wh->db    = wh_db_new ();

  stage = clutter_stage_get_default ();
  g_object_set (stage, "fullscreen", TRUE, NULL);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  /* General setup */

  bg = util_actor_from_file (PKGDATADIR "/bg.png", -1, -1);

  if (bg == NULL)
        g_error ("unable to load resource '%s'", PKGDATADIR "/bg.png");

  clutter_actor_set_size (bg, CSW(), CSH());
  clutter_group_add (CLUTTER_GROUP(stage), bg);
  clutter_actor_show (bg);


  /* Layout not really needed .. */
  wh->screen_browse = clutter_simple_layout_new();
  clutter_actor_set_size (wh->screen_browse, CSW(), CSH());
  
  /* Slider menu */

  wh->menu = wh_slider_menu_new ();
  clutter_actor_set_size (wh->menu, CSW(), 100);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "Not Viewed",
			     view_not_viewed_selected, 
			     wh);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "Recently Added",
			     view_recently_added_selected, 
			     wh);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "A-F",
			     view_af_selected,
			     wh);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "G-L",
			     view_gl_selected,
			     wh);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "M-R",
			     view_mr_selected,
			     wh);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "S-Z",
			     view_sz_selected,
			     wh);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "Popular",
			     view_popular_selected,
			     wh);

  wh_slider_menu_add_option (WH_SLIDER_MENU (wh->menu), 
			     "Recently Viewed",
			     view_recently_viewed_selected, 
			     wh);

  /* Sort the model to correspond to insitial menu option */
  view_not_viewed_selected (WH_SLIDER_MENU (wh->menu), NULL, (gpointer)wh);

  clutter_group_add (CLUTTER_GROUP(wh->screen_browse), wh->menu);
  g_object_set(wh->menu, "y", 25, NULL); 

  /* Video screen */

  wh->screen_video = wh_screen_video_new ();

  g_signal_connect(wh->screen_video, 
		   "playback-started",
		   G_CALLBACK(on_video_playback_start), 
		   wh);

  g_signal_connect(wh->screen_video, 
		   "playback-finished",
		   G_CALLBACK(on_video_playback_finish), 
		   wh);

  clutter_group_add (CLUTTER_GROUP(stage), wh->screen_video);

  /* Startup screen */

  screen_start = clutter_group_new();

  desktop = util_texture_from_root_window ();
  clutter_group_add (CLUTTER_GROUP(stage), desktop);
  clutter_actor_show (desktop);

  wh->busy = wh_busy_new();
  clutter_group_add (CLUTTER_GROUP(screen_start), wh->busy);
  
  label = clutter_label_new_full ("Sans 72px", "Importing Media..", 
				  &white_col);
  clutter_group_add (CLUTTER_GROUP(screen_start), label);
  clutter_actor_set_position (label,
			      (CSW() - clutter_actor_get_width (label))/2,
			      CSH() - (3*clutter_actor_get_height (label)));

  clutter_group_add (CLUTTER_GROUP(stage), screen_start);

  util_actor_fade_in (screen_start, NULL, NULL);
  util_actor_zoom (screen_start, NULL, 0.5, 1.0, NULL);
  util_actor_fade (desktop, NULL, 0xff, 0x0, NULL);
  util_actor_zoom (desktop, NULL, 1.0, 0.5, NULL);

  /* show stage for desktop zoom  */
  clutter_actor_show (stage);

  /* below actually spins main loop (for busy icon)  */

  g_signal_connect (wh->db, 
		    "row-created", 
		    G_CALLBACK (on_db_row_created), 
		    wh);

  pathv = g_strsplit (argv[1], ":", 0);
  while (pathv[i] != NULL)
    {
      printf("importing '%s'\n", pathv[i]);
      wh_db_import_uri (wh->db, pathv[i++]);
    }
  g_strfreev(pathv) ;

  /* view widget */

  /* FIXME: Should be able to do this before importing 
   * and get a smoother transition, but something appears wrong
   * with add signal.      
  */

  wh->view = wh_video_view_new (wh->model, 5);
  clutter_actor_set_size (wh->view, CSW(), 600);
  clutter_group_add (CLUTTER_GROUP(wh->screen_browse), wh->view);

  g_object_set(wh->view, "y", 
	       clutter_actor_get_y(wh->menu) 
	         + clutter_actor_get_height(wh->menu) + 25, NULL); 

  clutter_group_add (CLUTTER_GROUP (stage), wh->screen_browse);

  /* Zoom to browse screen */

  clutter_actor_raise_top (screen_start);

  util_actor_zoom (screen_start, NULL, 1.0, 0.5, NULL);
  util_actor_zoom (wh->screen_browse, NULL, 0.5, 1.0, NULL);
  util_actor_fade_in (wh->screen_browse, NULL, NULL);
  util_actor_fade_out (screen_start, 
		       (UtilAnimCompleteFunc)screen_fadeout_complete, 
		       wh);

  g_signal_connect (stage, 
		    "input-event",
		    G_CALLBACK (browse_input_cb),
		    wh);
  clutter_main();

  return 0;
}
