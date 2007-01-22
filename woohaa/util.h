#ifndef _FOO_FOO_UTIL
#define _FOO_FOO_UTIL

#include <clutter/clutter.h>

G_BEGIN_DECLS

typedef void (*UtilFadeCompleteFunc) (ClutterActor *actor,
				      gpointer       user_data);

guint32 
alpha_sine_inc_func (ClutterAlpha *alpha,
		     gpointer      dummy);

ClutterActor*
util_actor_from_file (const gchar *path, int width, int height);

ClutterTimeline*
util_actor_fade_in (ClutterActor *actor);

ClutterTimeline*
util_actor_fade_out (ClutterActor *actor);

G_END_DECLS

#endif
