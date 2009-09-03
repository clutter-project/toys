#include <clutter/clutter.h>
#include <math.h>

#define CSW() CLUTTER_STAGE_WIDTH()
#define CSH() CLUTTER_STAGE_HEIGHT()

#define N_ITEMS 8
#define STEP (360.0/N_ITEMS)
#define CLAMP_ANG(x) (((x) > 360.0) ? ((x) - 360.0) : (x))

struct { gchar *img; gchar *title; } ItemDetails[] =
{
  { "accessories-text-editor.png", "Text Editor" },
  { "applications-games.png", "Game" },
  { "dates.png", "Dates" },
  { "im-client.png", "Chat" },
  { "preferences-desktop-theme.png", "Preferences" },
  { "tasks.png", "Todo List" },
  { "utilities-terminal.png", "Terminal" },
  { "web-browser.png", "Browser"},
};

typedef struct Item
{
  ClutterActor     *actor;
  ClutterBehaviour *ellipse_behave, *opacity_behave, *scale_behave;
}
Item;

typedef struct App
{
  ClutterTimeline *timeline;
  ClutterAlpha    *alpha_sine_inc, *alpha_ramp;
  GSList          *items;
  Item            *active;
  gdouble          off;
  int              selected_index;
  ClutterActor    *label;
}
App;

void
introduce_items (App *app)
{
  gint     i;
  GSList  *node;
  gdouble  ang_start, ang_end;

  node = app->items;

  for (i=0; i<N_ITEMS; i++)
    {
      Item *item = node->data;

      ang_start = -90.0;
      ang_end   = (STEP * i);

      clutter_behaviour_ellipse_set_angle_start
	(CLUTTER_BEHAVIOUR_ELLIPSE(item->ellipse_behave), ang_start);

      clutter_behaviour_ellipse_set_angle_end
	(CLUTTER_BEHAVIOUR_ELLIPSE(item->ellipse_behave), ang_end);

      if (i == app->selected_index)
	{
	  g_object_set (item->opacity_behave,
			"opacity-start", 0x66,
			"opacity-end", 0xff,
			NULL);
	  g_object_set (item->scale_behave,
			"x-scale-start", 0.6,
			"y-scale-start", 0.6,
			"x-scale-end", 1.0,
			"y-scale-end", 1.0,
			NULL);
	}
      node = node->next;
    }

  clutter_timeline_start (app->timeline);
}


void
rotate_items (App *app, int step)
{
  gint     i, from_index;
  GSList  *node;
  gdouble  ang = 0.0, ang_start, ang_end;

  from_index = app->selected_index;

  app->selected_index += (-1 * step);
  if (app->selected_index < 0) app->selected_index = 7;
  if (app->selected_index > 7) app->selected_index = 0;

  ang = app->off;

  node = app->items;

  for (i=0; i<N_ITEMS; i++)
    {
      Item *item = node->data;

      ang_start = ang;
      ang_end   = ang + (STEP * step);

      clutter_behaviour_ellipse_set_direction
	(CLUTTER_BEHAVIOUR_ELLIPSE(item->ellipse_behave),
	 step > 0 ? CLUTTER_ROTATE_CW : CLUTTER_ROTATE_CCW);

      clutter_behaviour_ellipse_set_angle_start
	(CLUTTER_BEHAVIOUR_ELLIPSE(item->ellipse_behave), ang_start);

      clutter_behaviour_ellipse_set_angle_end
	(CLUTTER_BEHAVIOUR_ELLIPSE(item->ellipse_behave), ang_end);

      if (i == from_index)
	{
	  g_object_set (item->opacity_behave,
			"opacity-start", 0xff,
			"opacity-end", 0x66,
			NULL);

	  g_object_set (item->scale_behave,
			"x-scale-start", 1.0,
			"y-scale-start", 1.0,
			"x-scale-end", 0.6,
			"y-scale-end", 0.6,
			NULL);
	}
      else if (i == app->selected_index)
	{
	  g_object_set (item->opacity_behave,
			"opacity-start", 0x66,
			"opacity-end", 0xff,
			NULL);
	  g_object_set (item->scale_behave,
			"x-scale-start", 0.6,
			"y-scale-start", 0.6,
			"x-scale-end", 1.0,
			"y-scale-end", 1.0,
			NULL);
	}
      else
	{
	  g_object_set (item->opacity_behave,
			"opacity-start", 0x66,
			"opacity-end", 0x66,
			NULL);
	  g_object_set (item->scale_behave,
			"x-scale-start", 0.6,
			"y-scale-start", 0.6,
			"x-scale-end", 0.6,
			"y-scale-end", 0.6,
			NULL);
	}

      ang += STEP;
      node = node->next;
    }

  clutter_timeline_start (app->timeline);

  app->off += (STEP * step);
  app->off = CLAMP_ANG(app->off);
}

static gboolean
on_input (ClutterActor *stage,
	  ClutterEvent *event,
	  gpointer      user_data)
{
  App *app = user_data;

  if (event->type == CLUTTER_KEY_RELEASE)
    {
      ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

      if (clutter_timeline_is_playing(app->timeline))
	return FALSE;

      switch (clutter_key_event_symbol (kev))
	{
	case CLUTTER_Left:
	  rotate_items (app, -1);
	  break;
	case CLUTTER_Right:
	  rotate_items (app, 1);
	  break;
	case CLUTTER_Return:
	  break;
	case CLUTTER_q:
	  clutter_main_quit();
	  break;
	default:
	  break;
	}
    }

  return FALSE;
}

void
on_timeline_new_frame (ClutterTimeline *timeline,
		       gint             frame_num,
		       App             *app)
{
  if (frame_num > clutter_timeline_get_n_frames (timeline)/2)
    clutter_text_set_text (CLUTTER_TEXT(app->label),
                           ItemDetails[app->selected_index].title);
}

/* An alpha function that goes from 0->1->0 along a sine. */
gdouble
label_opacity_alpha_func (ClutterAlpha *alpha,
                          gpointer unused)
{
  ClutterTimeline *timeline = clutter_alpha_get_timeline (alpha);
  return sin (clutter_timeline_get_progress (timeline) * M_PI);
}

int
main (int argc, char *argv[])
{
  ClutterActor    *stage;
  ClutterColor     stage_color = { 0x34, 0x39, 0x39, 0xff };
  ClutterColor     white = { 0x72, 0x9f, 0xcf, 0xff };
  gint             i = 0;
  Item            *item;
  App             *app;
  gdouble          ang = 0.0;
  ClutterBehaviour *behave;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 800, 600);

  app = g_new0(App, 1);
  app->off = 0.0;
  app->timeline = clutter_timeline_new (20, 60);
  app->alpha_sine_inc
    = clutter_alpha_new_full (app->timeline, CLUTTER_EASE_OUT_SINE);

  app->alpha_ramp
    = clutter_alpha_new_with_func (app->timeline, label_opacity_alpha_func,
                                   NULL, NULL);

  for (i=0; i<N_ITEMS; i++)
    {
      item = g_new0 (Item, 1);

      item->actor = clutter_texture_new_from_file (ItemDetails[i].img, NULL);
      if (!item->actor)
	g_error ("Unable to load '%s'", ItemDetails[i].img);

      clutter_group_add (CLUTTER_GROUP(stage), item->actor);

      item->ellipse_behave
	= clutter_behaviour_ellipse_new (app->alpha_sine_inc,
					 CSW()/4,   /* center x */
					 CSH() - (CSH()/3),   /* center y */
					 CSW()/2,   /* width */
					 CSH() - (CSH()/4),   /* height */
					 CLUTTER_ROTATE_CW,
					 ang,
					 ang + STEP);
      item->opacity_behave
	= clutter_behaviour_opacity_new (app->alpha_sine_inc, 0x66, 0x66);

      item->scale_behave
	= clutter_behaviour_scale_new (app->alpha_sine_inc,
				       0.6, 0.6, 0.6, 0.6);

      clutter_behaviour_apply (item->ellipse_behave, item->actor);
      clutter_behaviour_apply (item->opacity_behave, item->actor);
      clutter_behaviour_apply (item->scale_behave, item->actor);

      app->items = g_slist_append (app->items, item);

      ang += STEP;
    }

  app->label = clutter_text_new_full ("Bitstream Vera Sans 60px", "", &white);
  clutter_actor_set_position (app->label, CSW()/2 - 30, CSH()/3 - 40);
  clutter_group_add (CLUTTER_GROUP(stage), app->label);

  behave = clutter_behaviour_opacity_new (app->alpha_ramp, 0xff, 0);
  clutter_behaviour_apply (behave, app->label);

  g_signal_connect (app->timeline,
		    "new-frame",
		    G_CALLBACK(on_timeline_new_frame),
		    app);

  g_signal_connect (stage,
		    "event",
		    G_CALLBACK (on_input),
		    app);

  introduce_items (app);

  clutter_actor_show_all (stage);

  clutter_main();

  return 0;
}
