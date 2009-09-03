/* 
 * foofone
 *
 * Foofone is a quick 3 hour hack to experiment with effects and create a
 * dummy phone interface. Thats all it is.
 *
 * Copyright 2007 OpenedHand Ltd
 * Authored by Matthew Allum <mallum@o-hand.com>
 * Licensed under the GPL v2 or greater.
 *
 */

#include <clutter/clutter.h>
#include <stdlib.h>
#include <math.h>

#define CSW 240
#define CSH 320

typedef struct Button
{
  ClutterActor     *actor;
  gchar            *face;
  gint              sx, sy;
} 
Button;

typedef struct App
{
  Button                *buttons[12];
  ClutterActor          *dpy, *dpy_entry;
  gint                   dpyx, dpyy;
  ClutterActor          *screen_dialpad, *screen_dial, *dial_label;
  gboolean               dialing_state;
  ClutterTimeline       *dialing_timeline;
} 
App;

/* An alpha function that goes from 0->1->0 along a sine. */
static gdouble
alpha_sine_func (ClutterAlpha *alpha,
                 gpointer unused)
{
  ClutterTimeline *timeline = clutter_alpha_get_timeline (alpha);
  return sin(clutter_timeline_get_progress (timeline) * M_PI);
}

gulong ALPHA_SINE;

/* A boolean 'interpolator', switching from 'a' to 'b' when 'progress' = 0.5 */ 
static gboolean
boolean_progress (const GValue *a,
                  const GValue *b,
                  gdouble       progress,
                  GValue       *retval)
{
  gboolean ba = g_value_get_boolean (a);
  gboolean bb = g_value_get_boolean (b);
  gboolean res = (progress <= 0.5) ? ba : bb;
  g_value_set_boolean (retval, res);
  return TRUE;
}

void
on_call_deactivate_complete (ClutterTimeline *timeline,
			     gpointer         user_data)
{
  App *app = (App*)user_data;
  /* reset the now hidden dialing screen */
  clutter_actor_set_rotation (app->screen_dial, CLUTTER_Y_AXIS, 0, 0, 0, 0);
}

void
call_deactivate (App *app)
{
  int               i;
  ClutterAnimation *anim;
  ClutterAlpha     *alpha;

  /* stop the flashing text */
  clutter_timeline_stop (app->dialing_timeline);

  /* clear dialpad entry ready */
  clutter_text_set_text (CLUTTER_TEXT(app->dpy_entry), "");

  /* rotate screen_dial, and hide it at mid-animation */
  clutter_actor_set_rotation (app->screen_dial, CLUTTER_Y_AXIS, 0.0, 0.0, 0.0, 0.0);
  anim = clutter_actor_animate (app->screen_dial, CLUTTER_LINEAR, 150,
                                "rotation-angle-y", -180.0,
                                "visible", FALSE,
                                NULL);
  alpha = clutter_animation_get_alpha (anim);

  /* reset positions of dialer actors, needed back for flip */
  for (i=0; i<12; i++)
    {
      clutter_actor_set_position (app->buttons[i]->actor,
                                  app->buttons[i]->sx, app->buttons[i]->sy);
      clutter_actor_set_opacity (app->buttons[i]->actor, 0xff);
    }
  clutter_actor_set_position (app->dpy, app->dpyx, app->dpyy);

  /* rotate hidden screen_dialpad, and show it at mid-animation */
  clutter_actor_set_rotation (app->screen_dialpad, CLUTTER_Y_AXIS, 180.0, 0.0, 0.0, 0.0);
  clutter_actor_animate_with_alpha (app->screen_dialpad, alpha,
                                    "rotation-angle-y", 0.0,
                                    "visible", TRUE,
                                    "signal-after::completed",
                                      G_CALLBACK(on_call_deactivate_complete),
                                      app,
                                    NULL);

  app->dialing_state = FALSE;
}

void
on_call_activate_complete (ClutterActor *actor,
			   gpointer user_data)
{
  ClutterAlpha     *alpha;
  ClutterBehaviour *behave;
  App *app = (App*)user_data;

  clutter_actor_hide (app->screen_dialpad);

  /* Setup the pulsing 'calling..' text if need be */
  if (app->dialing_timeline == NULL)
    {
      app->dialing_timeline = clutter_timeline_new (1000);
      clutter_timeline_set_loop (app->dialing_timeline, TRUE);
      alpha = clutter_alpha_new_full (app->dialing_timeline, ALPHA_SINE);
      behave = clutter_behaviour_opacity_new (alpha, 0xff, 0);
      clutter_behaviour_apply (behave, app->dial_label);
    }
  clutter_timeline_start (app->dialing_timeline);

  app->dialing_state = TRUE;
}

void
call_activate (App *app)
{
  gint i;
  gfloat x, y;
  ClutterAnimation *anim;
  ClutterAlpha     *alpha;

  /* zoom in the dialing window */
  clutter_actor_set_scale (app->screen_dial, 0.1, 0.1);
  clutter_actor_set_opacity (app->screen_dial, 0x66);
  clutter_actor_show_all (app->screen_dial);

  anim = clutter_actor_animate (app->screen_dial, CLUTTER_EASE_OUT_SINE, 150,
                                "opacity", 0xff,
                                "scale-x", 1.0,
                                "scale-y", 1.0,
                                NULL);
  alpha = clutter_animation_get_alpha (anim);

  /* Set up effects to shoot everything offscreen, synchronized with screen_dial animation */
  for (i=0; i<12; i++)
    {
      clutter_actor_set_position (app->buttons[i]->actor,
                                  app->buttons[i]->sx,
                                  app->buttons[i]->sy);

      switch ((i+1) % 3)
	  {
	  case 0:
            x = CSW + clutter_actor_get_width (app->buttons[i]->actor) / 2;
	    y = app->buttons[i]->sy;
	    break;
	  case 1:
            x = -clutter_actor_get_width (app->buttons[i]->actor) / 2;
	    y = app->buttons[i]->sy;
	    break;
	  case 2:
	    x = app->buttons[i]->sx;
	    if (i < 3)
              y = -clutter_actor_get_height (app->buttons[i]->actor) / 2;
	    else
              y = CSH + clutter_actor_get_height (app->buttons[i]->actor) / 2;
	    break;
	  }

      clutter_actor_animate_with_alpha (app->buttons[i]->actor, alpha,
                                        "opacity", 0x00,
                                        "x", x,
                                        "y", y,
			                NULL);
    }

  clutter_actor_set_position (app->dpy, app->dpyx, app->dpyy);
  clutter_actor_animate_with_alpha(app->dpy, alpha,
                                   "x", (float)app->dpyx,
                                   "y", -clutter_actor_get_height (app->dpy),
                                   "signal-after::completed",
                                     on_call_activate_complete,
                                     app,
                                   NULL);
}

void  
on_button_effect_complete (ClutterAnimation *animation,
			               gpointer user_data)
{
  ClutterActor *actor = (ClutterActor*)user_data;

  /* reset after effect */
  clutter_actor_set_opacity (actor, 0xff);
  clutter_actor_set_scale (actor, 1.0, 1.0);
}

void
button_activate (App *app, Button *b)
{
  // Wait for the previous animation to end
  if (clutter_actor_get_animation (b->actor))
    return;

  clutter_text_insert_text (CLUTTER_TEXT(app->dpy_entry), b->face, -1);

  clutter_actor_set_opacity (b->actor, 0xff);
  clutter_actor_set_scale (b->actor, 1.0, 1.0);
  clutter_actor_animate (b->actor, CLUTTER_LINEAR, 50,
                         "opacity", 0x00,
                         "scale-x", 1.5,
                         "scale-y", 1.5,
                         "signal-after::completed", on_button_effect_complete, b->actor,
                         NULL);
}

static gboolean 
on_input (ClutterStage *stage,
	  ClutterEvent *event,
	  gpointer      user_data)
{
  App *app = (App*)user_data;

  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      ClutterActor *actor = clutter_event_get_source (event);
      const gchar  *label = clutter_actor_get_name (actor);
      int          label_val;

      if (app->dialing_state == TRUE)
	    {
	      call_deactivate(app);
	      return TRUE;
	    }

      /* retrieve button id (stored in the Actor's name) */
      if ( !label )
        return FALSE;
      label_val = atoi(label);
      if ( label_val < 1 || label_val > 12 )
        return FALSE;
      --label_val;

      if (label_val == 11) /* 'dial' key */
        call_activate (app);
      else
        button_activate (app, app->buttons[label_val]);

      return TRUE;
    }

  return FALSE;
}

void
make_ui (App *app)
{
  gint          i, xpad, ypad, x ,y, xinit, xdpy, ydpy;
  ClutterActor *button_texture, *a;
  ClutterColor  text_color = { 0xff, 0xff, 0xff, 0xff },
                rect_color = { 0, 0, 0, 0x99 },
                black_color = { 0, 0, 0, 0xff };

  button_texture =  clutter_texture_new_from_file ("button.png", NULL);

  xpad = (CSW-(3*clutter_actor_get_width(button_texture)))/4;
  x = xinit = xpad;
  ypad = xpad/2;
  y = (CSH - (4 * (ypad + clutter_actor_get_height(button_texture))));

  /*
   * screen_dialpad (group)
   *  +----dpy (group)
   *        +---- (texture:display.png)
   *        +----dpy_entry (text)
   *        +----buttons[0:11]->actor (group)
   *              +---- (texture:button.png)
   *              +---- (text)
   */

  app->screen_dialpad = clutter_group_new();
  clutter_actor_set_size (app->screen_dialpad, CSW, CSH);
  clutter_actor_set_anchor_point_from_gravity (app->screen_dialpad, CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (app->screen_dialpad, CSW/2, CSH/2);

  app->dpy = clutter_group_new();

  a = clutter_texture_new_from_file ("display.png", NULL);
  clutter_group_add (CLUTTER_GROUP(app->dpy), a);
  app->dpyx = xdpy = x;
  app->dpyy = ydpy = (y - clutter_actor_get_height(app->dpy))/2;
  clutter_actor_set_position (app->dpy, xdpy, ydpy);

  clutter_group_add(CLUTTER_GROUP(app->screen_dialpad), app->dpy);

  app->dpy_entry = clutter_text_new_full ("Sans Bold 32px", "", &text_color);
  clutter_text_set_editable (CLUTTER_TEXT(app->dpy_entry), TRUE);
  clutter_actor_set_position (app->dpy_entry, 8, 8);
  clutter_actor_set_size (app->dpy_entry, clutter_actor_get_width (app->dpy) - 16, 32);
  clutter_group_add (CLUTTER_GROUP(app->dpy), app->dpy_entry);

  for (i=0; i<12; i++)
    {
      gchar       buf[8];
      gchar       label[8];

      app->buttons[i] = g_new0(Button, 1);
      app->buttons[i]->actor = clutter_group_new ();
      g_snprintf (label, 8, "%d", i+1);
      clutter_actor_set_name (app->buttons[i]->actor, label);
      clutter_actor_set_reactive (app->buttons[i]->actor, TRUE);
      clutter_actor_set_anchor_point_from_gravity (app->buttons[i]->actor,
                                                   CLUTTER_GRAVITY_CENTER);
      
      if ( i == 0 )
        a = button_texture;
      else
        a = clutter_clone_new(button_texture);
      clutter_group_add(CLUTTER_GROUP(app->buttons[i]->actor), a);
      
      switch (i)
        {
        case 9:
          g_snprintf(buf, 8, "#");
          break;
        case 10:
          g_snprintf(buf, 8, "0");
          break;
        case 11:
          g_snprintf(buf, 8, "*");
          break;
        default:
          g_snprintf(buf, 8, "%i", i+1);
          break;
        }

      a = clutter_text_new_full("Sans Bold 32px", buf, &text_color);
      clutter_actor_set_position (a, 
        (clutter_actor_get_width (button_texture)  - clutter_actor_get_width (a))/2,
        (clutter_actor_get_height (button_texture) - clutter_actor_get_height (a))/2);
      clutter_group_add (CLUTTER_GROUP (app->buttons[i]->actor), a);

      clutter_group_add (CLUTTER_GROUP (app->screen_dialpad), app->buttons[i]->actor);

      /* need to remember positions for anim - sucky */
      app->buttons[i]->sx = x + clutter_actor_get_width (app->buttons[i]->actor)/2;
      app->buttons[i]->sy = y + clutter_actor_get_height (app->buttons[i]->actor)/2;
      clutter_actor_set_position (app->buttons[i]->actor,
                                  app->buttons[i]->sx,
                                  app->buttons[i]->sy);

      /* Really we should use a Clutter*Box here.. */
      if (i % 3 == 2)
        {
          x = xinit;
          y += (ypad + clutter_actor_get_height (button_texture));
        }
      else
        x += (xpad + clutter_actor_get_width(button_texture));

      app->buttons[i]->face = g_strdup (buf);
    }

    /*
     * screen_dial
     *  +---- (rectangle:black)
     *  +---- (texture:call-background.png)
     *  +---- (rectangle:semi transparent)
     *  +----dial_label (text:"Calling...")
     */

  app->screen_dial = clutter_group_new();
  clutter_actor_set_anchor_point_from_gravity (app->screen_dial, CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (app->screen_dial, CSW/2, CSH/2);

  a = clutter_rectangle_new_with_color (&black_color);
  clutter_actor_set_size (a, CSW, CSH);
  clutter_group_add (CLUTTER_GROUP(app->screen_dial), a);

  a = clutter_texture_new_from_file ("call-background.png", NULL);
  clutter_group_add (CLUTTER_GROUP(app->screen_dial), a);

  a = clutter_rectangle_new_with_color (&rect_color);
  clutter_actor_set_size (a, CSW, CSH/6);
  clutter_actor_set_position (a, 0, (CSH - (CSH/6))/2);
  clutter_group_add (CLUTTER_GROUP(app->screen_dial), a);

  app->dial_label = clutter_text_new_full ("Sans Bold 32px", "Calling...", &text_color);
  clutter_actor_set_position (app->dial_label, 10, (CSH - (CSH/6))/2 + 10);
  clutter_group_add (CLUTTER_GROUP (app->screen_dial), app->dial_label);
}

int
main (int argc, char *argv[])
{
  ClutterActor    *stage;
  ClutterColor     stage_color = { 0x0, 0x0, 0x0, 0xff };

  App             *app;

  clutter_init (&argc, &argv);

  ALPHA_SINE = clutter_alpha_register_func (alpha_sine_func, NULL);
  clutter_interval_register_progress_func (G_TYPE_BOOLEAN, boolean_progress);

  app = g_new0(App, 1);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, CSW, CSH);

  make_ui (app);

  clutter_group_add (CLUTTER_GROUP(stage), app->screen_dial);
  clutter_group_add (CLUTTER_GROUP(stage), app->screen_dialpad);

  clutter_actor_hide_all (app->screen_dial);
  clutter_actor_show_all (stage);

  g_signal_connect (stage, 
		    "event",
		    G_CALLBACK (on_input),
		    app);

  printf("\n..Press '*' to dial..\n\n");

  clutter_main ();

  return 0;
}
