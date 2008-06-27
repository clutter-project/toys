#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include "clutter-texture-odo.h"

struct distort_data
{
  ClutterFixed turn;
  ClutterFixed radius;
  ClutterFixed angle;
};

struct app_data
{
  gint                 count;
  GList               *files;
  ClutterTimeline     *timeline;
  ClutterAlpha        *alpha;
  ClutterTextureOdo   *odo;
  ClutterTexture      *bg;
  ClutterActor        *next_page;
  struct distort_data  d;
};

/* Effects copied from odo-pageturn, odo-cloth and odo-bowtie */
static gboolean
page_turn_func (ClutterTextureOdo * otex,
                ClutterFixed x, ClutterFixed y, ClutterFixed z,
                ClutterFixed *x2, ClutterFixed *y2, ClutterFixed *z2,
                ClutterColor *color, gpointer data)
{
  struct distort_data * d = data;
  ClutterFixed cx, cy, rx, ry;
  guint w, h, shade;
  ClutterFixed width, height, turn_angle;
  
  clutter_actor_get_size (CLUTTER_ACTOR (otex), &w, &h);

  width = CLUTTER_INT_TO_FIXED (w);
  height = CLUTTER_INT_TO_FIXED (h);
  
  /* Rotate the point around the centre of the page-curl ray to align it with
   * the y-axis.
   */
  
  cx = clutter_qmulx (CFX_ONE - d->turn, width);
  cy = clutter_qmulx (CFX_ONE - d->turn, height);
  
  rx = clutter_qmulx (x - cx, clutter_cosx (-d->angle)) -
       clutter_qmulx (y - cy, clutter_sinx (-d->angle)) - d->radius;
  ry = clutter_qmulx (x - cx, clutter_sinx (-d->angle)) +
       clutter_qmulx (y - cy, clutter_cosx (-d->angle));
  
  if (rx > -d->radius)
    {
      /* Calculate the curl angle as a function from the distance of the curl
       * ray (i.e. the page crease)
       */
      turn_angle = (clutter_qmulx (clutter_qdivx (rx, d->radius),
                                   CFX_PI_2) - CFX_PI_2);

      /* Add a gradient that makes it look like lighting and hides the switch
       * between textures.
       */
      shade = CLUTTER_FIXED_TO_INT (
        clutter_qmulx (clutter_sinx (turn_angle), CLUTTER_INT_TO_FIXED (96))) +
        159;
      color->red = shade;
      color->green = shade;
      color->blue = shade;
    }
  else
    *color = (ClutterColor){ 0xff, 0xff, 0xff, 0xff };
  
  if (rx > 0)
    {
      /* Make the curl radius smaller as more circles are formed (stops
       * z-fighting and looks cool)
       */
      ClutterFixed small_radius = d->radius -
              clutter_qdivx (clutter_qmulx (turn_angle,
                                            CLUTTER_INT_TO_FIXED (2)), CFX_PI);
      
      /* Calculate a point on a cylinder (maybe make this a cone at some point)
       * and rotate it by the specified angle.
       */
      rx = clutter_qmulx (small_radius, clutter_cosx (turn_angle)) + d->radius;
      *x2 = clutter_qmulx (rx, clutter_cosx (d->angle)) -
            clutter_qmulx (ry, clutter_sinx (d->angle)) + cx;
      *y2 = clutter_qmulx (rx, clutter_sinx (d->angle)) +
            clutter_qmulx (ry, clutter_cosx (d->angle)) + cy;
      *z2 = clutter_qmulx (small_radius, clutter_sinx (turn_angle)) + d->radius;
    }
  else
    {
      *x2 = x;
      *y2 = y;
      *z2 = z;
    }
  
  return TRUE;
}

static gboolean
cloth_func (ClutterTextureOdo * otex,
            ClutterFixed x, ClutterFixed y, ClutterFixed z,
            ClutterFixed *x2, ClutterFixed *y2, ClutterFixed *z2,
            ClutterColor *color, gpointer data)
{
  struct distort_data * d = data;
  ClutterFixed cx, cy, rx, ry;
  guint w, h, shade;
  ClutterFixed width, height, turn_angle, height_radius;
  
  clutter_actor_get_size (CLUTTER_ACTOR (otex), &w, &h);

  width = CLUTTER_INT_TO_FIXED (w);
  height = CLUTTER_INT_TO_FIXED (h);
  
  /* Rotate the point around the centre of the curl ray to align it with
   * the y-axis.
   */
  
  cx = clutter_qmulx (CFX_ONE - d->turn, width);
  cy = clutter_qmulx (CFX_ONE - d->turn, height);
  
  rx = clutter_qmulx (x - cx, clutter_cosx (-d->angle)) -
       clutter_qmulx (y - cy, clutter_sinx (-d->angle)) - d->radius;
  ry = clutter_qmulx (x - cx, clutter_sinx (-d->angle)) +
       clutter_qmulx (y - cy, clutter_cosx (-d->angle));
  
  /* Calculate the angle as a function of the distance from the curl ray */
  turn_angle = (clutter_qmulx (clutter_qdivx (rx, d->radius),
                               CFX_PI_2) - CFX_PI_2);

  /* Add a gradient that makes it look like lighting and hides the switch
   * between textures.
   */
  shade = CLUTTER_FIXED_TO_INT (
    clutter_qmulx (clutter_sinx (turn_angle), CLUTTER_INT_TO_FIXED (96))) +
    159;
  color->red = shade;
  color->green = shade;
  color->blue = shade;
  
  /* Make the wave amplitude lower as its distance from the curl ray increases.
   * Not really necessary, but looks a little nicer I think.
   */
  height_radius = clutter_qmulx (CFX_ONE - clutter_qdivx (rx, width),
                                 d->radius);
  
  *x2 = clutter_qmulx (rx, clutter_cosx (d->angle)) -
        clutter_qmulx (ry, clutter_sinx (d->angle)) + cx;
  *y2 = clutter_qmulx (rx, clutter_sinx (d->angle)) +
        clutter_qmulx (ry, clutter_cosx (d->angle)) + cy;
  *z2 = clutter_qmulx (height_radius, clutter_sinx (turn_angle));
  
  return TRUE;
}

static gboolean
bowtie_func (ClutterTextureOdo * otex,
             ClutterFixed x, ClutterFixed y, ClutterFixed z,
             ClutterFixed *x2, ClutterFixed *y2, ClutterFixed *z2,
             ClutterColor *color, gpointer data)
{
  struct distort_data * d = data;
  ClutterFixed cx, cy, rx, ry;
  guint w, h, shade;
  ClutterFixed width, height, turn_angle;
  
  clutter_actor_get_size (CLUTTER_ACTOR (otex), &w, &h);

  width = CLUTTER_INT_TO_FIXED (w);
  height = CLUTTER_INT_TO_FIXED (h);
  
  cx = clutter_qmulx (d->turn, width + width/2) - width/4;
  cy = height/2;
  
  rx = clutter_qmulx (x - cx, clutter_cosx (0)) -
       clutter_qmulx (y - cy, clutter_sinx (0));
  ry = clutter_qmulx (x - cx, clutter_sinx (0)) +
       clutter_qmulx (y - cy, clutter_cosx (0));
  
  /* Make angle as a function of distance from the curl ray */
  turn_angle = MAX (-CFX_PI,
                    MIN (0,
                         (clutter_qmulx (clutter_qdivx (rx, width/4),
                                         CFX_PI_2) - CFX_PI_2)));

  /* Add a gradient that makes it look like lighting */
  shade = CLUTTER_FIXED_TO_INT (
    clutter_qmulx (clutter_cosx (turn_angle*2), CLUTTER_INT_TO_FIXED (96))) +
    159;
  color->red = shade;
  color->green = shade;
  color->blue = shade;

  /* Calculate the point on a cone (note, a cone, not a right cone) */
  ClutterFixed height_radius =
    clutter_qmulx (clutter_qdivx (ry, height/2), height/2);
  
  ry = clutter_qmulx (height_radius, clutter_cosx (turn_angle));
  *x2 = clutter_qmulx (rx, clutter_cosx (0)) -
        clutter_qmulx (ry, clutter_sinx (0)) + cx;
  *y2 = clutter_qmulx (rx, clutter_sinx (0)) +
        clutter_qmulx (ry, clutter_cosx (0)) + cy;
  *z2 = clutter_qmulx (height_radius, clutter_sinx (turn_angle));
  
  return TRUE;
}

static void
new_frame_cb (ClutterTimeline *timeline,
              gint             frame_num,
              gpointer         data)
{
  struct app_data *d = data;
  guint32 alpha = clutter_alpha_get_alpha (d->alpha);
  gdouble turn;
  
  if (alpha)
    turn = alpha/(gdouble)CLUTTER_ALPHA_MAX_ALPHA;
  else
    turn = 0.0;
  
  d->d.turn = CLUTTER_FLOAT_TO_FIXED (turn);

  clutter_actor_queue_redraw (clutter_stage_get_default ());
}

static void
turn_completed_cb (ClutterTimeline *timeline, struct app_data *d)
{
  ClutterActor *stage;
  
  stage = clutter_stage_get_default ();
  clutter_texture_odo_set_parent_texture (d->odo,
                                          CLUTTER_TEXTURE (d->next_page));
  clutter_actor_show (CLUTTER_ACTOR (d->odo));
  clutter_actor_set_size (CLUTTER_ACTOR (d->odo),
                          clutter_actor_get_width (stage),
                          clutter_actor_get_height (stage));
  clutter_container_remove_actor (CLUTTER_CONTAINER (stage), d->next_page);
  
  clutter_texture_odo_set_backface_texture (d->odo, NULL);
  d->next_page = NULL;
  d->d.turn = 0;
  
  g_signal_handlers_disconnect_by_func (timeline, turn_completed_cb, d);
}

static void
bowtie_completed_cb (ClutterTimeline *timeline, struct app_data *d)
{
  clutter_texture_odo_set_parent_texture (d->odo,
                                          CLUTTER_TEXTURE (d->next_page));
  clutter_actor_show (CLUTTER_ACTOR (d->odo));
  clutter_actor_set_size (CLUTTER_ACTOR (d->odo),
                          CLUTTER_STAGE_WIDTH (),
                          CLUTTER_STAGE_HEIGHT ());

  clutter_texture_odo_set_backface_texture (d->odo, NULL);
  d->next_page = NULL;
  d->d.turn = 0;

  g_signal_handlers_disconnect_by_func (timeline, bowtie_completed_cb, d);
}

static void
toggle_cloth (struct app_data *d)
{
  gpointer func;
  
  g_object_get (G_OBJECT (d->odo), "distort-func", &func, NULL);
  
  if (clutter_timeline_is_playing (d->timeline) && (func != cloth_func))
    return;
  
  if (clutter_timeline_is_playing (d->timeline))
    {
      g_object_set (G_OBJECT (d->odo), "distort-func", NULL, NULL);
      clutter_timeline_pause (d->timeline);
      clutter_timeline_set_loop (d->timeline, FALSE);
      clutter_alpha_set_func (d->alpha, CLUTTER_ALPHA_SINE_INC, NULL, NULL);
      clutter_actor_queue_redraw (CLUTTER_ACTOR (d->odo));
    }
  else
    {
      d->d.turn = 0;
      g_object_set (G_OBJECT (d->odo), "distort-func", cloth_func, NULL);
      clutter_alpha_set_func (d->alpha, CLUTTER_ALPHA_SINE, NULL, NULL);
      clutter_timeline_set_loop (d->timeline, TRUE);
      clutter_timeline_start (d->timeline);
    }
}

static void
next_page (struct app_data *d)
{
  if (clutter_timeline_is_playing (d->timeline))
    return;
  
  if (!d->next_page)
    {
      d->next_page = g_object_new (CLUTTER_TYPE_TEXTURE,
                                   "disable-slicing", TRUE,
                                   "filename",
                                   g_list_nth_data (d->files, d->count),
                                   "visible", TRUE,
                                   NULL);
      clutter_actor_set_size (d->next_page,
                              CLUTTER_STAGE_WIDTH (),
                              CLUTTER_STAGE_HEIGHT ());
    }

  if (!(d->count % 2))
    {
      clutter_actor_set_depth (d->next_page, -1);
      clutter_container_add_actor (
        CLUTTER_CONTAINER (clutter_stage_get_default ()), d->next_page);
      clutter_texture_odo_set_backface_texture (d->odo, d->bg);
      g_object_set (G_OBJECT (d->odo),
                    "distort-func", page_turn_func,
                    NULL);
      g_signal_connect (d->timeline, "completed",
                        G_CALLBACK (turn_completed_cb), d);
    }
  else
    {
      clutter_texture_odo_set_backface_texture (d->odo,
                                                CLUTTER_TEXTURE (d->next_page));
      g_object_set (G_OBJECT (d->odo),
                    "distort-func", bowtie_func,
                    NULL);
      g_signal_connect (d->timeline, "completed",
                        G_CALLBACK (bowtie_completed_cb), d);
    }
  
  d->count ++;
  d->d.turn = 0;
  
  clutter_timeline_rewind (d->timeline);
  clutter_timeline_start (d->timeline);
}

static void 
on_event (ClutterStage *stage,
          ClutterEvent *event,
          gpointer      data)
{
  struct app_data *d = data;
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
          
          case CLUTTER_space:
            next_page (d);
            break;
          
          case CLUTTER_Return:
            toggle_cloth (d);
            break;
             
          default:
            break;
          }
      }
      break;
    default:
      break;
    }
}

int
main (int argc, char **argv)
{
  GDir               *dir;
  const ClutterColor  stage_color = { 0x00, 0x00, 0x00, 0xff };
  ClutterActor       *stage, *odo;
  const gchar        *file;
  struct app_data     data;

  GList *files = NULL;
  GError *error = NULL;
  
  if (argc < 2)
    {
      g_message ("Usage: %s <directory>", argv[0]);
      return 1;
    }
  
  /* Compile list of images */
  if (!(dir = g_dir_open (argv[1], 0, &error)))
    {
      g_critical ("Error opening directory: %s", error->message);
      g_error_free (error);
      return 2;
    }

  while ((file = g_dir_read_name (dir)))
    {
      gchar *full_file;
      
      full_file = g_build_filename (argv[1], file, NULL);
      
      if (!g_file_test (full_file, G_FILE_TEST_IS_REGULAR))
        goto skip_file;
      
      if ((!g_str_has_suffix (full_file, ".png")) &&
          (!g_str_has_suffix (full_file, ".jpg")))
        goto skip_file;
      
      files = g_list_prepend (files, full_file);
      continue;

skip_file:
      g_free (full_file);
    }

  files = g_list_sort (files, (GCompareFunc)strcmp);

  /* Clutter initialisation */
  clutter_init (&argc, &argv);
  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 640, 480);
  //clutter_stage_fullscreen (CLUTTER_STAGE (stage));
  
  /* Initialise appdata variables */
  memset (&data, 0, sizeof (data));
  
  data.files = files;
  
  data.timeline = clutter_timeline_new_for_duration (1500);
  g_signal_connect (data.timeline, "new-frame",
                    G_CALLBACK (new_frame_cb), &data);
  data.alpha = clutter_alpha_new_full (data.timeline,
                                       CLUTTER_ALPHA_SINE_INC,
                                       &data,
                                       NULL);
  
  data.d.turn = 0;
  data.d.radius = CLUTTER_INT_TO_FIXED (CLUTTER_STAGE_WIDTH ()/10);
  data.d.angle = CFX_PI / 6;

  /* Hook onto key-press signal */
  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (on_event), &data);

  /* Create textures and odo texture */
  data.bg = g_object_new (CLUTTER_TYPE_TEXTURE,
                          "disable-slicing", TRUE,
                          "filename", "oh-logo.png",
                          "visible", TRUE,
                          NULL);
  
  odo = clutter_texture_odo_new (CLUTTER_TEXTURE (data.bg));
  data.odo = CLUTTER_TEXTURE_ODO (odo);
  g_object_set (G_OBJECT (odo),
                "tile-width", 32,
                "tile-height", 32,
                "distort-func-data", &data.d,
                "distort-func", page_turn_func,
                "vflip", TRUE,
                NULL);
  clutter_actor_set_size (odo, CLUTTER_STAGE_WIDTH (), CLUTTER_STAGE_HEIGHT ());
  clutter_texture_odo_set_cull_mode (CLUTTER_TEXTURE_ODO (odo), ODO_CULL_BACK);
  
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), odo);

  /* Show everything */
  clutter_actor_show_all (stage);

  cogl_enable_depth_test (TRUE);
  clutter_main ();
  
  while (files)
    {
      g_free (files->data);
      files = g_list_delete_link (files, files);
    }

  return 0;
}

