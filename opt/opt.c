#include "opt.h"
#include <stdlib.h> 		/* for exit() */

void input_cb (ClutterStage *stage,
               ClutterEvent *event,
               gpointer      user_data)
{
  OptShow  *show = (OptShow*)user_data;

  if (event->type == CLUTTER_KEY_RELEASE)
    {
      if (clutter_key_event_symbol ((ClutterKeyEvent*)event) == CLUTTER_q)
	exit(0); 		/* FIXME: need clutter exit func */
      opt_show_advance (show, 0);
    }
}

void
opt_init (void)
{
  clutter_init(NULL, NULL);
}

int 
main(int argc, char **argv)
{
  OptShow  *show;
  OptSlide *slide;
  ClutterElement *pic;

  opt_init ();

  show = opt_show_new();

  /* Slide 1 */

  slide = opt_slide_new ();
  opt_slide_set_title  (slide, "This is slide 1");
  opt_slide_add_bullet_text_item (slide, "Hey!, the first bullet!");
  opt_slide_add_bullet_text_item (slide, "And Im the second :)");
  opt_slide_add_bullet_text_item (slide, "( any key to advance, q to quit )");
  opt_show_add_slide (show, slide);

  /* Slide 2 with image */

  slide = opt_slide_new ();
  opt_slide_set_title (slide, "And this be slide 2");
  opt_slide_add_bullet_text_item (slide, "With a kitten ! ( For Iain )");
  pic = clutter_texture_new_from_pixbuf 
              (gdk_pixbuf_new_from_file ("kitten.jpg", NULL));

  if (pic != NULL)
    opt_slide_add_bullet (slide, pic);

  opt_show_add_slide (show, slide);

  /* Slide 3 */

  slide = opt_slide_new ();
  opt_slide_set_title (slide, "Slide 3 is the Best");
  opt_slide_add_bullet_text_item (slide, "Because");
  opt_slide_add_bullet_text_item (slide, "It");
  opt_slide_add_bullet_text_item (slide, "Has");
  opt_slide_add_bullet_text_item (slide, "Most");
  opt_slide_add_bullet_text_item (slide, "Bullets");
  opt_show_add_slide (show, slide);

  /* Slide 4 */

  slide = opt_slide_new ();
  opt_slide_set_title (slide, "No! No! slide 4 is best!");
  opt_slide_add_bullet_text_item (slide, "As more kittens!");
  pic = clutter_texture_new_from_pixbuf 
              (gdk_pixbuf_new_from_file ("kitten2.jpg", NULL));

  if (pic != NULL)
    opt_slide_add_bullet (slide, pic);

  opt_slide_add_bullet_text_item (slide, 
				  "Thats all for now apart from "
				  "see me wrap some text here.");
  opt_show_add_slide (show, slide);

  /* Connect up for input event */

  g_signal_connect (clutter_stage(), "input-event",
                    G_CALLBACK (input_cb),
                    show);

  opt_show_run (show);

  return 0;
}
