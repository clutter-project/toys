#include <clutter/clutter.h>
#include <clutter/clutter-event.h>

#include <stdio.h>
#include <stdlib.h>
#include <glib/gstdio.h>
#include <gst/gst.h>
#include <unistd.h>
#include "clutter-dominatrix.h"
#include "clutter-video-player.h"

#define SIZE_X 80
#define MARG 4

ClutterDominatrix  *ActiveDMX = NULL;

/* rand() is not that random, and tends to generate clustered values;
 * this function attempts to ensure that the images we load are spread more
 * evenly around the stage
 *
 * We divide the stage into n x m squares with size corresponsing to the
 * thumb width, and maintain a bit matrix indicating if an image has been
 * placed at a position in each square; then, for first n*m/2 number of thumbs,
 * we do not allow two thumbs in the same square.
 */
static void
get_xy_coords (ClutterActor * stage, gint * x, gint * y)
{
  static unsigned char * map = NULL;
  static gint            count = 0;
  static gint            size = 0;
  static gint            sw, sh;
  static gint            xdim, ydim;
  
  if (!map)
    {
      gint w, h;

      sw = clutter_actor_get_width  (stage) - SIZE_X;
      sh = clutter_actor_get_height (stage) - SIZE_X;
      
      w = sw + 2 * SIZE_X - 1;
      h = sh + 2 * SIZE_X - 1;

      xdim = w / (SIZE_X);
      ydim = h / (SIZE_X);

      size = xdim * ydim;
      
      map = g_malloc0 (size / 8);
    }

  *x = rand () % sw;
  *y = rand () % sh;
  
  if (count >= size / 2)
    return;
  
  do {
    gint          off;
    gint          indx;
    unsigned char mask;
    
    off  = *y/(SIZE_X) * xdim + *x/(SIZE_X);
    indx = off / 8;
    mask = 1 << (off % 8);

    if (!(map[indx] & mask))
      {
	map[indx] |= mask;
	count++;
	return;
      }

    *x = rand () % sw;
    *y = rand () % sh;
    }
  while (count < size / 2);
}

static ClutterDominatrix *
make_item (ClutterActor * stage, ClutterActor *actor)
{
  ClutterActor      * rect, * group, * shaddow;
  ClutterDominatrix * dmx;
  ClutterColor        bckg_clr = { 0xff, 0xff, 0xff, 0xff };
  ClutterColor        shdw_clr = { 0x44, 0x44, 0x44, 0x44 };
  gdouble             scale;
  gint                w, h, sw, sh, x, y;
  ClutterFixed        zang;
  
  scale = (double) SIZE_X / (double) clutter_actor_get_width (actor);
  w = SIZE_X;
  h = (gint)(scale * (double) clutter_actor_get_height (actor));

  sw = clutter_actor_get_width  (stage) - w;
  sh = clutter_actor_get_height (stage) - h;

  get_xy_coords (stage, &x, &y);
  
  group = clutter_group_new ();
  clutter_actor_set_position (group, x, y);
  clutter_actor_set_size (group, w + MARG, h + MARG);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), group);
  clutter_actor_show (group);
  
  rect = clutter_rectangle_new ();
  clutter_actor_set_position (rect, 0, 0);
  clutter_actor_set_size (rect, w + MARG, h + MARG);
  
  clutter_rectangle_set_color (CLUTTER_RECTANGLE (rect),
                               &bckg_clr);
  clutter_actor_show (rect);

  shaddow = clutter_rectangle_new ();
  clutter_actor_set_position (shaddow, 2, 2);
  clutter_actor_set_size (shaddow, w + MARG, h + MARG);
  clutter_rectangle_set_color (CLUTTER_RECTANGLE (shaddow), &shdw_clr);
  clutter_actor_show (shaddow);
  
  clutter_actor_set_position (actor, 2, 2);
  clutter_actor_set_size (actor, w, h);
  clutter_actor_show (actor);
  
  clutter_container_add (CLUTTER_CONTAINER (group),
			 shaddow, rect, actor, NULL);

  zang = CLUTTER_INT_TO_FIXED (rand()%360);
  clutter_actor_rotate_zx (group, zang, (w + MARG)/2, (h + MARG)/2);
  
  dmx = clutter_dominatrix_new (group);

  return dmx;
}

static ClutterDominatrix *
make_img_item (ClutterActor * stage, const gchar * name)
{
  ClutterActor      * img;
  GdkPixbuf         * pixbuf;
  
  pixbuf = gdk_pixbuf_new_from_file_at_size (name, 400, 400, NULL);

  if (!pixbuf)
    return NULL;

  img = clutter_texture_new_from_pixbuf (pixbuf);
  return make_item (stage, img);
}

static ClutterDominatrix *
make_vid_item (ClutterActor * stage, const gchar * name)
{
  ClutterActor      * vid;

  vid = clutter_video_player_new (name);
  return make_item (stage, vid);
}

static gboolean
is_supported_img (const gchar * name)
{
  GdkPixbufFormat * fmt = gdk_pixbuf_get_file_info (name, NULL, NULL);

  if (fmt)
    return (gdk_pixbuf_format_is_disabled (fmt) != TRUE);
  
  return FALSE;
}

static void
process_directory (const gchar * name,
		   ClutterActor * stage, ClutterActor * notice)
{
  GDir              * dir;
  const gchar       * fname;
  struct stat         sbuf;
  
  dir = g_dir_open (name, 0, NULL);

  if (!dir)
    return;

  g_chdir (name);
  
  while ((fname = g_dir_read_name (dir)))
    {
      while (g_main_context_pending (NULL))
	g_main_context_iteration (NULL, FALSE);

      if (is_supported_img (fname))
	{
 	  make_img_item (stage, fname);
 	  clutter_actor_raise_top (notice);
	}
      else if (g_str_has_suffix (fname, ".ogg"))
	{
	  make_vid_item (stage, fname);
	  clutter_actor_raise_top (notice);
	}
	       
      if (g_stat (fname, &sbuf) > -1 && S_ISDIR (sbuf.st_mode))
	process_directory (fname, stage, notice);
    }

  g_chdir ("..");
  
  g_dir_close (dir);
}

static ClutterActor *
make_busy_notice (ClutterActor * stage)
{
  ClutterActor     * label;
  ClutterActor     * rect;
  ClutterActor     * group;
  ClutterColor       text_clr = { 0xff, 0xff, 0xff, 0xff };
  ClutterColor       bckg_clr = { 0x5c, 0x54, 0x57, 0x9f };
  
  label = clutter_label_new_with_text ("Sans 54",
				       "Please wait, loading images ...");
  
  clutter_label_set_color (CLUTTER_LABEL (label), &text_clr);
  clutter_actor_set_position (label, 10, 10);
  clutter_actor_show (label);

  group = clutter_group_new ();
  clutter_actor_show (group);
  
  rect = clutter_rectangle_new ();
  clutter_actor_set_position (rect, 0, 0);
  clutter_actor_set_size (rect,
			  clutter_actor_get_width (label) + 20,
			  clutter_actor_get_height (label) + 20);
  
  clutter_rectangle_set_color (CLUTTER_RECTANGLE (rect),
                               &bckg_clr);
  clutter_actor_show (rect);

  clutter_container_add (CLUTTER_CONTAINER (group), rect, label, NULL);
  
  return group;
}

struct timeout_cb_data
{
  ClutterActor * stage;
  ClutterActor * notice;
  const gchar  * name;
};

static void
tmln_completed_cb (ClutterActor *actor, gpointer data)
{
  ClutterGroup * stage = data;

  clutter_group_remove (stage, actor);
}

static gboolean
timeout_cb (gpointer data)
{
  ClutterTimeline * tmln;
  ClutterEffectTemplate * tmpl;
  
  struct timeout_cb_data * d = data;
  
  process_directory (d->name, d->stage, d->notice);

  tmpl = clutter_effect_template_new (clutter_timeline_new (60, 60),
				      CLUTTER_ALPHA_SINE_DEC);
  
  tmln = clutter_effect_fade (tmpl, d->notice, 0, 0xff, tmln_completed_cb,
			      d->stage);

  g_object_unref (tmpl);
  
  clutter_actor_show_all (d->stage);
  clutter_actor_queue_redraw (d->stage);
  
  return FALSE;
}

static void 
on_event (ClutterStage *stage,
	  ClutterEvent *event,
	  gpointer      data)
{
  gint            x,y;
  ClutterActor   *actor;
  ClutterDominatrix *dmx;

  switch (event->type)
    {
    case CLUTTER_KEY_PRESS:
      {
	guint sym = clutter_key_event_symbol ((ClutterKeyEvent*)event);

	switch (sym)
	  {
	  case CLUTTER_Escape:
	  case CLUTTER_q:
	  case CLUTTER_Q:
	     clutter_main_quit ();
	     break;
	     
	  default:
	    break;
	  }
      }
      break;
    case CLUTTER_2BUTTON_PRESS:
    case CLUTTER_BUTTON_PRESS:
      clutter_event_get_coords (event, &x, &y);

      actor = clutter_stage_get_actor_at_pos (stage, x, y);
	
      while (actor &&
	     clutter_actor_get_parent (actor) != CLUTTER_ACTOR(stage) &&
	     (actor = clutter_actor_get_parent (actor)));
	
      if (!actor)
	return;
      
      dmx = g_object_get_data (G_OBJECT (actor), "dominatrix");
      
      if (!dmx)
	return;

      ActiveDMX = g_object_ref (dmx);
      break;
    case CLUTTER_BUTTON_RELEASE:
      if (ActiveDMX)
	{
	  clutter_dominatrix_handle_event (ActiveDMX, event);
	  g_object_unref (ActiveDMX);
	  ActiveDMX = NULL;
	}
      break;
    default:
      break;
    }

  if (ActiveDMX)
    clutter_dominatrix_handle_event (ActiveDMX, event);

  return;
}

int
main (int argc, char *argv[])
{
  ClutterActor      * stage, * notice;
  ClutterColor        stage_clr = { 0xed, 0xe8, 0xe1, 0xff };
  struct timeout_cb_data tcbd;
  
  if (argc != 2)
    {
      g_print ("\n    usage: %s image_directory\n\n", argv[0]);
      exit (1);
    }
  
  srand (time(NULL) + getpid());
  
  clutter_init (&argc, &argv);
  gst_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_clr);

  g_object_set (stage, "fullscreen", TRUE, NULL);

  notice = make_busy_notice (stage);
  clutter_actor_set_position (notice,
			      (clutter_actor_get_width (stage) -
			       clutter_actor_get_width (notice))/2,
			      (clutter_actor_get_height (stage) -
			       clutter_actor_get_height (notice))/2);
  
  clutter_group_add (CLUTTER_GROUP(stage), notice);
  clutter_actor_show_all (stage);

  tcbd.stage  = stage;
  tcbd.notice = notice;
  tcbd.name   = argv[1];

  g_timeout_add (100, timeout_cb, &tcbd);

  g_signal_connect (stage, "event", G_CALLBACK (on_event), NULL);
  
  clutter_main();

  return EXIT_SUCCESS;
}
