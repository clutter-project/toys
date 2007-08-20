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

#define CSW 240
#define CSH 320

typedef struct Button
{
  ClutterActor     *actor;
  ClutterTimeline  *pressed_fadeout;
  ClutterTimeline  *pressed_scale;
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
  ClutterEffectTemplate *button_effect_tmpl, *call_effect_tmpl;
  gboolean               dialing_state;
  ClutterTimeline       *dialing_timeline;
} 
App;

void
on_call_deactivate_complete (ClutterTimeline *timeline,
			     gpointer         user_data) 
{
  App *app = (App*)user_data;
  /* reset the now hidden dialing screen */
  clutter_actor_rotate_y (app->screen_dial, 0, CSW/2, 0);
  g_object_unref (timeline);
}

void
on_call_deactivate_new_frame (ClutterTimeline *timeline,
			      gint             frame_num,
			      gpointer         user_data)
{
  App *app = (App*)user_data;
  gint i;
  
  if (frame_num >= (clutter_timeline_get_n_frames(timeline)/2)-1)
    {
      /* switch visibility half way through rotation. shouldn't
       * really keep calling this though.	 
      */
      clutter_actor_rotate_y (app->screen_dial, 0.0, CSW/2, 0);
      clutter_actor_hide_all (app->screen_dial);

      clutter_actor_show_all (app->screen_dialpad);
      for (i=0; i<12; i++)
	clutter_actor_show (app->buttons[i]->actor);
    }

  clutter_actor_rotate_y (CLUTTER_ACTOR(app->screen_dial),
			  - (float)frame_num * 6, /* 180/30 = 6 */
			  CSW/2,
			  0);

  clutter_actor_rotate_y (CLUTTER_ACTOR(app->screen_dialpad),
                            180 - (frame_num * 6),
                            CSW/2,
                            0);
}

void
call_deactivate (App *app)
{
  ClutterTimeline  *timeline;

  /* stop the flashing text */
  clutter_timeline_stop (app->dialing_timeline);

  /* clear dialpad entry ready */
  clutter_entry_set_text (CLUTTER_ENTRY(app->dpy_entry), "");

  /* setup fli anim */
  timeline = clutter_timeline_new (30, 120);

  /* do the anim manually - it seems easier in this case than using behaviours
   * or rotation behaiours are buggy. Needs investigation
  */
  g_signal_connect (timeline, 
		    "new-frame", 
		    G_CALLBACK(on_call_deactivate_new_frame),
		    app);

  g_signal_connect (timeline, 
		    "completed", 
		    G_CALLBACK(on_call_deactivate_complete),
		    app);

  clutter_timeline_start (timeline);

  app->dialing_state = FALSE;
}

void
on_call_activate_complete (ClutterActor *actor,
			   gpointer user_data)
{
  ClutterAlpha     *alpha;
  ClutterBehaviour *behave;
  App *app = (App*)user_data;
  gint i;

  /* reset positions of dialer actors and hide, needed back for flip */
  for (i=0; i<12; i++)
    {
      clutter_actor_set_position (app->buttons[i]->actor,
				  app->buttons[i]->sx,
				  app->buttons[i]->sy);
      clutter_actor_set_opacity (app->buttons[i]->actor, 0xff);
      clutter_actor_hide (app->buttons[i]->actor);
    }

  clutter_actor_hide (app->dpy);
  clutter_actor_set_position (app->dpy, app->dpyx, app->dpyy);

  /* Setup the pulsing 'calling..' text if need be */
  if (app->dialing_timeline == NULL)
    {
      app->dialing_timeline = clutter_timeline_new (60, 60);
      clutter_timeline_set_loop (app->dialing_timeline, TRUE);
      alpha 
	= clutter_alpha_new_full (app->dialing_timeline, 
				  CLUTTER_ALPHA_SINE, NULL, NULL);
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
  ClutterKnot knots[2];

  /* Doh effect should set initial values */
  clutter_actor_set_scale_with_gravity (app->screen_dial,
					0.1,
					0.1,
					CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_opacity (app->screen_dial, 0);
  clutter_actor_show_all (app->screen_dial);

  /* zoom in the dialing window */
  clutter_effect_fade (app->call_effect_tmpl,
		       app->screen_dial,
		       0x66,
		       0xff,
		       NULL,
		       NULL);

  clutter_effect_scale (app->call_effect_tmpl,
			app->screen_dial,
			0.1,
			1.0,
			CLUTTER_GRAVITY_CENTER,
			NULL,
			NULL);

  /* Set up effects to shoot everything offscreen */
  for (i=0; i<12; i++)
    {
      knots[0].x = app->buttons[i]->sx;
      knots[0].y = app->buttons[i]->sy;

      switch ((i+1) % 3)
	{
	case 0:
	  knots[1].x = CSW;
	  knots[1].y = app->buttons[i]->sy;
	  break;
	case 1:
	  knots[1].x = -clutter_actor_get_width (app->buttons[i]->actor);
	  knots[1].y = app->buttons[i]->sy;
	  break;
	case 2:
	  knots[1].x = app->buttons[i]->sx;
	  if (i < 3)
	    knots[1].y =  -clutter_actor_get_height (app->buttons[i]->actor);
	  else
	    knots[1].y = CSH;
	  break;
	}

      clutter_effect_move (app->call_effect_tmpl,
			   app->buttons[i]->actor,
			   knots,
			   2,
			   NULL,
			   NULL);

      clutter_effect_fade (app->call_effect_tmpl,
			   app->buttons[i]->actor,
			   0xff,
			   0x0,
			   NULL,
			   NULL);
      }

  knots[0].x = app->dpyx;
  knots[0].y = app->dpyy;
  knots[1].x = app->dpyx;
  knots[1].y = -clutter_actor_get_height (app->dpy);

  /* HACK: Note, here we kind of assume this timeline will complete
   * last but that is not guarenteed because of dropped frames etc.
  */
  clutter_effect_move (app->call_effect_tmpl,
		       app->dpy,
		       knots,
		       2,
		       on_call_activate_complete,
		       app);
}

void  
on_button_effect_complete (ClutterActor *actor,
			   gpointer user_data)
{
  Button *b = (Button*)user_data;

  /* reset after effect */
  clutter_actor_set_opacity (b->actor, 0xff);
  clutter_actor_set_scale_with_gravity (b->actor, 1.0, 1.0, 
					CLUTTER_GRAVITY_CENTER);
  b->pressed_fadeout = NULL;
}

void
button_activate (App *app, Button *b)
{
  clutter_entry_insert_text (CLUTTER_ENTRY(app->dpy_entry), b->face, -1);

  if (b->pressed_fadeout)
    return;

  b->pressed_fadeout = clutter_effect_fade (app->button_effect_tmpl,
					    b->actor,
					    0xff,
					    0,
					    NULL,
					    NULL);

  /* HACK: Note, here we kind of assume this timeline will complete
   * last but that is not guarenteed because of dropped frames etc.
  */
  b->pressed_scale = clutter_effect_scale (app->button_effect_tmpl,
					   b->actor,
					   1.0,
					   2.0,
					   CLUTTER_GRAVITY_CENTER,
					   on_button_effect_complete,
					   b);
}

void 
on_input (ClutterStage *stage,
	  ClutterEvent *event,
	  gpointer      user_data)
{
  App *app = (App*)user_data;

  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      gint                x, y, i;
      ClutterActor       *actor;

      if (app->dialing_state == TRUE)
	{
	  call_deactivate(app);
	  return;
	}

      /* will be nice when actors themselves handle events... */
      clutter_event_get_coords (event, &x, &y);

      actor = clutter_stage_get_actor_at_pos (stage, x, y);

      while (actor &&
	     clutter_actor_get_parent (actor) != app->screen_dialpad &&
	     (actor = clutter_actor_get_parent (actor)));
	
      if (!actor)
	return;
      
      for (i=0; i<12; i++)
	if (app->buttons[i]->actor == actor)
	  {
	    if (i == 11)
	      call_activate (app);
	    else
	      button_activate (app, app->buttons[i]);
	    return;
	  }
    }
}

void
make_ui (App *app)
{
  gint          i, xpad, ypad, x ,y, xinit, xdpy, ydpy;
  ClutterActor *button_texture, *a;
  GdkPixbuf    *pixb;
  ClutterColor  text_color = { 0xff, 0xff, 0xff, 0xff },
                rect_color = { 0, 0, 0, 0x99 },
		black_color = { 0, 0, 0, 0xff };

  app->screen_dialpad = clutter_group_new();

  if ((pixb = gdk_pixbuf_new_from_file ("display.png", NULL)) == NULL)
    g_error ("Unable to load '%s'", "display.png");

  app->dpy = clutter_group_new();
  a = clutter_texture_new_from_pixbuf (pixb);
  clutter_group_add (CLUTTER_GROUP(app->dpy), a);
  g_object_unref (pixb);

  if ((pixb = gdk_pixbuf_new_from_file ("button.png", NULL)) == NULL)
    g_error ("Unable to load '%s'", "button.png");

  button_texture =  clutter_texture_new_from_pixbuf (pixb);
  g_object_unref (pixb);

  xpad = (CSW-(3*clutter_actor_get_width(button_texture)))/4;
  app->dpyx = xdpy = x = xinit = xpad;
  ypad = xpad/2;
  y = (CSH - (4 * (ypad + clutter_actor_get_height(button_texture))));
  app->dpyy = ydpy = (y - clutter_actor_get_height(app->dpy))/2;

  clutter_group_add (CLUTTER_GROUP(app->screen_dialpad), app->dpy);
  clutter_actor_set_position (app->dpy, xdpy, ydpy);
  clutter_actor_show_all (app->dpy);

  app->dpy_entry = clutter_entry_new_full ("Sans Bold 32px", "", &text_color);
  clutter_group_add (CLUTTER_GROUP(app->dpy), app->dpy_entry);
  clutter_actor_set_position (app->dpy_entry, 8, 8);
  clutter_actor_set_size (app->dpy_entry, 
			  clutter_actor_get_width (app->dpy) - 16,
			  32);
  clutter_actor_show (app->dpy_entry);

  for (i=1; i<13; i++)
    {
      gchar         buf[8];
      int           j = i-1;

      app->buttons[j] = g_new0(Button, 1);
      app->buttons[j]->actor = clutter_group_new ();
      
      a = clutter_clone_texture_new (CLUTTER_TEXTURE(button_texture));

      clutter_group_add (CLUTTER_GROUP(app->buttons[j]->actor), a);
      
      switch (i)
	{
	case 10:
	  g_snprintf(buf, 8, "#");
	  break;
	case 11:
	  g_snprintf(buf, 8, "0");
	  break;
	case 12:
	  g_snprintf(buf, 8, "*");
	  break;
	default:
	  g_snprintf(buf, 8, "%i", i);
	  break;
	}

      a = clutter_label_new_with_text ("Sans Bold 32px", buf);
      clutter_label_set_color (CLUTTER_LABEL(a), &text_color);
      clutter_actor_set_position (a, 
				  (clutter_actor_get_width (button_texture)
				   - clutter_actor_get_width (a))/2,
				  (clutter_actor_get_height (button_texture)
				   - clutter_actor_get_height (a))/2);
      clutter_group_add (CLUTTER_GROUP(app->buttons[j]->actor), a);

      clutter_group_add (CLUTTER_GROUP(app->screen_dialpad), 
			 app->buttons[j]->actor);
      clutter_actor_set_position (app->buttons[j]->actor, x, y);

      /* need to remember positions for anim - sucky */
      app->buttons[j]->sx = x;
      app->buttons[j]->sy = y;

      /* Really we should use a Clutter*Box here.. */
      if (i % 3 == 0)
	{
	  x = xinit;
	  y += (ypad + clutter_actor_get_height(button_texture));
	}
      else
	x += (xpad + clutter_actor_get_width(button_texture));

      app->buttons[j]->face = g_strdup (buf);

      clutter_actor_show_all (app->buttons[j]->actor);
    }

  /* for button animations */
  app->button_effect_tmpl 
    = clutter_effect_template_new (clutter_timeline_new (10, 120),
				   CLUTTER_ALPHA_RAMP_INC);

  /* dial 'screen' */

  app->screen_dial = clutter_group_new();

  a = clutter_rectangle_new_with_color (&black_color);
  clutter_actor_set_size (a, CSW, CSH);
  clutter_group_add (CLUTTER_GROUP(app->screen_dial), a);

  if ((pixb = gdk_pixbuf_new_from_file ("call-background.png", NULL)) == NULL)
    g_error ("Unable to load '%s'", "call-background.png");

  a = clutter_texture_new_from_pixbuf (pixb);
  g_object_unref (pixb);

  clutter_group_add (CLUTTER_GROUP(app->screen_dial), a);

  a = clutter_rectangle_new_with_color (&rect_color);
  clutter_actor_set_size (a, CSW, CSH/6);
  clutter_actor_set_position (a, 0, (CSH - (CSH/6))/2);
  clutter_group_add (CLUTTER_GROUP(app->screen_dial), a);

  a = clutter_label_new_full ("Sans Bold 32px", "Calling...", &text_color);
  clutter_actor_set_position (a, 10, (CSH - (CSH/6))/2 + 10);
  clutter_group_add (CLUTTER_GROUP(app->screen_dial), a);
  app->dial_label = a;

  app->call_effect_tmpl
    = clutter_effect_template_new (clutter_timeline_new (30, 120),
				   CLUTTER_ALPHA_SINE_INC);
}

int
main (int argc, char *argv[])
{
  ClutterActor    *stage;
  ClutterColor     stage_color = { 0x0, 0x0, 0x0, 0xff };

  App             *app;

  clutter_init (&argc, &argv);

  app = g_new0(App, 1);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, CSW, CSH);

  make_ui (app);

  clutter_group_add (CLUTTER_GROUP(stage), app->screen_dial);

  clutter_group_add (CLUTTER_GROUP(stage), app->screen_dialpad);
  clutter_actor_show_all (app->screen_dialpad);
  clutter_actor_show_all (stage);

  g_signal_connect (stage, 
		    "event",
		    G_CALLBACK (on_input),
		    app);

  printf("\n..Press '*' to dial..\n\n");

  clutter_main ();

  return 0;
}
