#ifndef _FOO_FOO_UTIL
#define _FOO_FOO_UTIL

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CSW() CLUTTER_STAGE_WIDTH()
#define CSH() CLUTTER_STAGE_HEIGHT()

typedef void (*UtilAnimCompleteFunc) (ClutterActor *actor,
				      gpointer       user_data);

guint32 
alpha_sine_inc_func (ClutterAlpha *alpha,
		     gpointer      dummy);

ClutterActor*
util_actor_from_file (const gchar *path, int width, int height);

ClutterTimeline*
util_actor_fade_in (ClutterActor        *actor,
		    UtilAnimCompleteFunc func, 
		    gpointer             data);

ClutterTimeline*
util_actor_fade_out (ClutterActor        *actor,
		     UtilAnimCompleteFunc func, 
		     gpointer             data);

ClutterTimeline*
util_actor_fade (ClutterActor        *actor, 
		 UtilAnimCompleteFunc func,
		 guint8               start_opacity,
		 guint8               end_opacity,
		 gpointer             data);


ClutterTimeline*
util_actor_zoom (ClutterActor        *actor, 
		 UtilAnimCompleteFunc func,
		 gdouble              start_scale,
		 gdouble              end_scale,
		 gpointer             data);

ClutterActor*
util_texture_from_root_window (void);

G_END_DECLS

#endif
