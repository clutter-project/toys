#include <clutter/clutter.h>
#include "clutter-reflect-texture.h"

int
main (int argc, char *argv[])
{
  ClutterActor  *stage, *tex, *reflect;
  GdkPixbuf     *pixbuf;
  ClutterColor   stage_color = { 0x0, 0x0, 0x0, 0xff };
  gint           x;

  clutter_init (&argc, &argv);

  if (argc < 2)
    {
      g_error ("No image argument supplied");
    }

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage),
			   &stage_color);

  pixbuf = gdk_pixbuf_new_from_file (argv[1], NULL);

  tex = clutter_texture_new_from_pixbuf (pixbuf);

  reflect = clutter_reflect_texture_new (CLUTTER_TEXTURE(tex), 100);
  clutter_actor_set_opacity (reflect, 100);

  x = (CLUTTER_STAGE_WIDTH() - clutter_actor_get_width(tex))/2;

  clutter_group_add (CLUTTER_GROUP(stage), tex);
  clutter_group_add (CLUTTER_GROUP(stage), reflect);
  clutter_actor_set_position (tex, x, 20);
  clutter_actor_set_position (reflect, x, clutter_actor_get_height(tex) + 20);

  /* clutter_actor_rotate_y (stage, 60.0, CLUTTER_STAGE_WIDTH()/2, 0); */

  clutter_actor_show_all (stage);

  clutter_main();

  return 1;
}
