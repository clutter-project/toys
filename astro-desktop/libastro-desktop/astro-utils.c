#include "astro-utils.h"


void 
astro_utils_set_clip (ClutterActor *actor,
                      gint          xoff, 
                      gint          yoff,
                      gint          width,
                      gint          height)
{
#if 1
  clutter_actor_set_clip (actor, xoff, yoff, width, height);
#endif
}
