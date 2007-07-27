#include <clutter/clutter.h>
#include <clutter-cairo/clutter-cairo.h>
#include <math.h>
#include <X11/keysym.h>

#define PADDLE_SIZE 48.0
#define PADDLE_THICKNESS 8.0
#define PADDLE_SPEED 4
#define BALL_SIZE 12.0
#define DASH_LENGTH 12.0
#define ARENA_WIDTH 320.0
#define ARENA_HEIGHT 240.0
#define FPS 60.0
#define MINPOS (PADDLE_THICKNESS * 2)
#define MAXPOS (ARENA_HEIGHT - PADDLE_SIZE - (PADDLE_THICKNESS * 2))

/*
 * NOTE: This is a completely brain-dead way to implement pong, but helped
 *       me familiarise with Clutter paths and such.
 */

static const ClutterColor green = { 0x0, 0xff, 0x0, 0xff };

typedef struct {
	/* First paddle */
	gint score1;
	gint position1;
	ClutterActor *paddle1;
	gboolean up1;
	gboolean down1;
	
	/* Second paddle */
	gint score2;
	gint position2;
	ClutterActor *paddle2;
	gboolean up2;
	gboolean down2;
	
	/* Paddle independent */
	gdouble angle;
	gdouble speed;
	ClutterActor *ball;
	ClutterActor *arena;
	ClutterTimeline *timeline;
	ClutterAlpha *alpha;
	ClutterBehaviour *path;
	ClutterKnot start;
	ClutterKnot end;
	gboolean pause;
} PongData;

static ClutterActor *
pong_arena_actor_create (PongData *data)
{
	ClutterActor *group, *actor;
	ClutterGeometry geom;
	gint i;
	
	group = clutter_group_new ();
	
	/* Top border */
	actor = clutter_rectangle_new_with_color (&green);
	geom.x = 0; geom.y = 0;
	geom.width = ARENA_WIDTH; geom.height = PADDLE_THICKNESS;
	clutter_actor_set_geometry (actor, &geom);
	clutter_actor_show (actor);
	clutter_group_add (CLUTTER_GROUP (group), actor);
	
	/* Bottom border */
	actor = clutter_rectangle_new_with_color (&green);
	geom.y = ARENA_HEIGHT - PADDLE_THICKNESS;
	clutter_actor_set_geometry (actor, &geom);
	clutter_actor_show (actor);
	clutter_group_add (CLUTTER_GROUP (group), actor);
	
	/* Dotted line down the middle */
	geom.x = (ARENA_WIDTH / 2) - (PADDLE_THICKNESS / 2);
	geom.width = PADDLE_THICKNESS;
	geom.height = PADDLE_THICKNESS * 2;
	for (i = 0; i < ARENA_HEIGHT / (PADDLE_THICKNESS * 2); i+= 2) {
		geom.y = i * PADDLE_THICKNESS * 2;
		actor = clutter_rectangle_new_with_color (&green);
		clutter_actor_set_geometry (actor, &geom);
		clutter_actor_show (actor);
		clutter_group_add (CLUTTER_GROUP (group), actor);
	}
	
	return group;
}

static ClutterActor *
pong_ball_actor_create (PongData *data)
{
	ClutterActor *actor, *group;
	cairo_t *cr;
	
	actor = clutter_cairo_new (BALL_SIZE, BALL_SIZE);
	cr = clutter_cairo_create (CLUTTER_CAIRO (actor));
	
	/* Clear */
	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgba (cr, 0, 1.0, 0, 1.0);
	
	cairo_new_path (cr);
	cairo_arc (cr, BALL_SIZE/2.0, BALL_SIZE/2.0,
		BALL_SIZE/2.0, 0, 2*M_PI);
	cairo_fill (cr);
	
	cairo_destroy (cr);

	group = clutter_group_new ();
	clutter_group_add (CLUTTER_GROUP (group), actor);
	clutter_actor_set_position (actor, -BALL_SIZE/2, -BALL_SIZE/2);
	clutter_actor_show (actor);

	return group;
}

static ClutterActor *
pong_paddle_actor_create (PongData *data)
{
	ClutterGeometry geom;
	ClutterActor *actor;

	actor = clutter_rectangle_new_with_color (&green);
	geom.x = 0; geom.y = 0;
	geom.width = PADDLE_THICKNESS;
	geom.height = PADDLE_SIZE;
	clutter_actor_set_geometry (actor, &geom);
	
	return actor;
}

static void
pong_ball_path_calculate (PongData *data, gdouble ax, gdouble ay, gdouble w,
			  gdouble h)
{
	gdouble x, y, dx, dy, angle;
	
	x = clutter_actor_get_x (data->ball);
	y = clutter_actor_get_y (data->ball);
	
	data->start.x = x;
	data->start.y = y;

	/* Work out destination */
	if (data->angle < M_PI * 0.5) {
		/* Travelling up-right */
		dx = x + (tan (data->angle) * (y - ay - (BALL_SIZE/2)));
		if (dx > w - (BALL_SIZE/2)) {
			dx = w - (BALL_SIZE/2);
			dy = y - ((dx - x) / tan (data->angle));
		} else {
			dy = ay + (BALL_SIZE / 2);
		}
	} else if (data->angle < M_PI) {
		/* Travelling down-right */
		angle = M_PI - data->angle;
		dx = x + (tan (angle) * (h - y));
		if (dx > w - (BALL_SIZE/2)) {
			dx = w - (BALL_SIZE/2);
			dy = y + ((dx - x) / tan (angle));
		} else {
			dy = h - (BALL_SIZE/2);
		}
	} else if (data->angle < M_PI * 1.5) {
		/* Travelling down-left */
		angle = data->angle - M_PI;
		dx = x - (tan (angle) * (h - y));
		if (dx < (BALL_SIZE/2)) {
			dx = (BALL_SIZE/2);
			dy = y + ((x - ax) / tan (angle));
		} else {
			dy = h - (BALL_SIZE/2);
		}
	} else {
		/* Travelling up-left */
		angle = (M_PI * 2) - data->angle;
		dx = x - (tan (angle) * (y - ay - (BALL_SIZE/2)));
		if (dx <  (BALL_SIZE/2)) {
			dx = (BALL_SIZE/2);
			dy = y - ((x - ax) / tan (angle));
		} else {
			dy = ay + (BALL_SIZE / 2);
		}
	}
	
	clutter_timeline_set_n_frames (data->timeline, MAX (1,
		(guint)((ABS (dx - x)/w) * FPS * data->speed)));
	clutter_timeline_rewind (data->timeline);
	
	data->end.x = (gint)dx;
	data->end.y = (gint)dy;
}

static void
pong_knot_reached_cb (ClutterBehaviourPath *pathb, const ClutterKnot *knot,
		      PongData *data)
{
	/* Figure out the new angle of the ball after a collision */
	if ((knot->x == data->end.x) && (knot->y == data->end.y)) {
		gint x, y;

		clutter_behaviour_path_clear (pathb);

		x = clutter_actor_get_x (data->ball);
		y = clutter_actor_get_y (data->ball);
		
		/*g_debug ("%d, %d, %lf", x, y, data->angle);*/
		
		/* Work out new travel angle after collisions */
		if ((x >= (ARENA_WIDTH - (BALL_SIZE/2))) ||
		    (x <= (BALL_SIZE/2)))
			data->angle = -data->angle;

		while (data->angle > M_PI*2) data->angle -= M_PI*2;
		while (data->angle < 0) data->angle += M_PI*2;

		if (y <= PADDLE_THICKNESS + (BALL_SIZE/2)) {
			if (data->angle < M_PI * 0.5) {
				data->angle = M_PI - data->angle;
			} else if (data->angle > M_PI * 1.5) {
				data->angle = M_PI +
					((M_PI * 2.0) - data->angle);
			}
		} else if (y >= ARENA_HEIGHT - PADDLE_THICKNESS -
			   (BALL_SIZE/2)) {
			if (data->angle < M_PI) {
				data->angle = M_PI - data->angle;
			} else if (data->angle < M_PI * 1.5) {
				data->angle = (M_PI * 2.0) -
					(data->angle - M_PI);
			}
		}
		
		while (data->angle > M_PI*2) { data->angle -= M_PI*2; }
		while (data->angle < 0) { data->angle += M_PI*2; }
		
		pong_ball_path_calculate (data,
			0, PADDLE_THICKNESS,
			ARENA_WIDTH,
			ARENA_HEIGHT - PADDLE_THICKNESS);
		
		clutter_behaviour_path_clear (
			CLUTTER_BEHAVIOUR_PATH (data->path));
		clutter_behaviour_path_append_knots (
			CLUTTER_BEHAVIOUR_PATH (data->path), &data->start,
			&data->end, NULL);

		/*g_debug ("%d, %d, %lf", data->end.x,
			data->end.y, data->angle);*/
	}
}

static void
pong_key_press_event_cb (ClutterStage *stage, ClutterEvent *event,
			 PongData *data)
{
	ClutterKeyEvent *keyevent = (ClutterKeyEvent *)event;

	if ((keyevent->keyval != XK_p) && (data->pause)) {
		data->pause = FALSE;
		clutter_timeline_start (data->timeline);
	}
		
	switch (keyevent->keyval)
	{
	    case XK_Escape:
	    case XK_q:
		clutter_main_quit ();
		break;
	    case XK_a:
		data->up1 = TRUE;
		break;
	    case XK_z:
		data->down1 = TRUE;
		break;
	    case XK_k:
		data->up2 = TRUE;
		break;
	    case XK_m:
		data->down2 = TRUE;
		break;
	    case XK_p:
		data->pause = !data->pause;
		if (data->pause) {
			clutter_timeline_pause (data->timeline);
		} else
			clutter_timeline_start (data->timeline);
	    default:
		break;
	}
}

static void
pong_key_release_event_cb (ClutterStage *stage, ClutterEvent *event,
			   PongData *data)
{
	ClutterKeyEvent *keyevent = (ClutterKeyEvent *)event;

	switch (keyevent->keyval)
	{
	    case XK_a:
		data->up1 = FALSE;
		break;
	    case XK_z:
		data->down1 = FALSE;
		break;
	    case XK_k:
		data->up2 = FALSE;
		break;
	    case XK_m:
		data->down2 = FALSE;
		break;
	    default:
		break;
	}
}

static void pong_new_frame_cb (ClutterTimeline *timeline, gint frame_num,
			       PongData *data)
{
	if (data->up1 ^ data->down1) {
		data->position1 = MAX (MINPOS, MIN (MAXPOS, data->position1 +
			(data->down1 ? PADDLE_SPEED : 0) -
			(data->up1 ? PADDLE_SPEED : 0)));
		clutter_actor_set_position (data->paddle1, PADDLE_THICKNESS,
			data->position1);
	}

	if (data->up2 ^ data->down2) {
		data->position2 = MAX (MINPOS, MIN (MAXPOS, data->position2 +
			(data->down2 ? PADDLE_SPEED : 0) -
			(data->up2 ? PADDLE_SPEED : 0)));
		clutter_actor_set_position (data->paddle2, ARENA_WIDTH -
			(PADDLE_THICKNESS * 2), data->position2);
	}
}

int
main (int argc, char **argv)
{
	PongData data;
	ClutterActor *stage;
	const ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };
	
	clutter_init (&argc, &argv);
	
	data.arena = pong_arena_actor_create (&data);
	data.paddle1 = pong_paddle_actor_create (&data);
	data.paddle2 = pong_paddle_actor_create (&data);
	data.ball = pong_ball_actor_create (&data);
	
	clutter_actor_set_position (data.paddle1, PADDLE_THICKNESS,
		PADDLE_THICKNESS * 2);
	clutter_actor_set_position (data.paddle2,
		ARENA_WIDTH - (PADDLE_THICKNESS * 2),
		PADDLE_THICKNESS * 2);
	
	data.up1 = FALSE;
	data.down1 = FALSE;
	data.up2 = FALSE;
	data.down2 = FALSE;
	data.pause = TRUE;
	data.position1 = 0;
	data.position2 = 0;
	
	data.timeline = clutter_timeline_new (FPS * 2, FPS);
	data.alpha = clutter_alpha_new_full (data.timeline,
		CLUTTER_ALPHA_RAMP_INC, NULL, NULL);
	data.path = clutter_behaviour_path_new (data.alpha, NULL, 0);
	
	data.angle = ((M_PI * 1.8));
	data.speed = 2;
	
	clutter_actor_set_position (data.ball, ARENA_WIDTH/2, ARENA_HEIGHT/2);
	
	stage = clutter_stage_get_default ();
	clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
	
	clutter_group_add (CLUTTER_GROUP (stage), data.arena);
	clutter_group_add (CLUTTER_GROUP (stage), data.paddle1);
	clutter_group_add (CLUTTER_GROUP (stage), data.paddle2);
	clutter_group_add (CLUTTER_GROUP (stage), data.ball);
	
	clutter_actor_show_all (CLUTTER_ACTOR (stage));
	clutter_actor_set_scale (CLUTTER_ACTOR (stage),
		CLUTTER_STAGE_WIDTH () / ARENA_WIDTH,
		CLUTTER_STAGE_HEIGHT () / ARENA_HEIGHT);
	
	pong_ball_path_calculate (&data,
			0, PADDLE_THICKNESS,
			ARENA_WIDTH,
			ARENA_HEIGHT - PADDLE_THICKNESS);
	clutter_behaviour_apply (data.path, data.ball);
	clutter_behaviour_path_append_knots (CLUTTER_BEHAVIOUR_PATH (data.path),
		&data.start, &data.end, NULL);
	
	g_signal_connect (data.path, "knot_reached",
		G_CALLBACK (pong_knot_reached_cb), &data);
	g_signal_connect (data.timeline, "new_frame",
		G_CALLBACK (pong_new_frame_cb), &data);
	g_signal_connect (stage, "key-press-event",
		G_CALLBACK (pong_key_press_event_cb), &data);
	g_signal_connect (stage, "key-release-event",
		G_CALLBACK (pong_key_release_event_cb), &data);

	clutter_main ();

	return 0;
}

