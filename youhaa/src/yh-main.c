
#include <string.h>
#include <clutter/clutter.h>
#include <clutter-gst/clutter-gst.h>

#include "yh-theme.h"
#include "yh-youtube.h"
#include "yh-youtube-browser.h"

typedef struct {
  ClutterActor *entry;
  ClutterActor *button;
  ClutterActor *throbber;
  ClutterActor *logo;

  ClutterActor *results;
  
  ClutterTimeline *throbber_timeline;
  ClutterTimeline *transition;
  
  ClutterEffectTemplate *template;
  
  gboolean query_changed;
  YHYoutube *youtube;
} YouhaaData;

static void related_cb (YHYoutubeBrowser *browser, ClutterModelIter *iter,
                        YouhaaData *data);

static void
button_in_complete (ClutterActor *actor, YouhaaData *data)
{
  data->transition = NULL;
}

static void
throbber_out_complete (ClutterActor *actor, YouhaaData *data)
{
  clutter_actor_hide (data->throbber);
  clutter_actor_show (data->button);

  clutter_timeline_stop (data->throbber_timeline);

  data->transition = clutter_effect_rotate (data->template,
                         data->button,
                         CLUTTER_Y_AXIS,
                         0,
                         clutter_actor_get_width (data->button)/2,
                         0,
                         0,
                         CLUTTER_ROTATE_CCW,
                         (ClutterEffectCompleteFunc)button_in_complete,
                         data);
}

static void
destroy_out_complete (ClutterActor *actor, YouhaaData *data)
{
  clutter_actor_destroy (actor);
}

static void
model_cb (YHYoutube *youtube, ClutterModel *model, YouhaaData *data)
{
  if (data->results)
    {
      /* Fade out old view */
      clutter_effect_fade (data->template,
                           data->results,
                           0x00,
                           (ClutterEffectCompleteFunc)destroy_out_complete,
                           data);
      data->results = NULL;
    }
  
  /* Animate out throbber */
  if (data->transition)
    clutter_timeline_stop (data->transition);
  data->transition = clutter_effect_rotate (data->template,
                         data->throbber,
                         CLUTTER_Y_AXIS,
                         90,
                         0,
                         0,
                         0,
                         CLUTTER_ROTATE_CW,
                         (ClutterEffectCompleteFunc)throbber_out_complete,
                         data);
  
  if (model)
    {
      /* Create and fade in new view */
      ClutterActor *view;
      
      view = yh_youtube_browser_new (model, data->youtube);
      data->results = clutter_group_new ();
      clutter_actor_show (data->results);
      clutter_actor_show (view);
      clutter_container_add_actor (CLUTTER_CONTAINER (data->results), view);
      clutter_container_add_actor (
        CLUTTER_CONTAINER (clutter_stage_get_default ()),
        data->results);
      
      clutter_actor_set_position (data->results, BORDER, BORDER);
      clutter_actor_set_size (view,
                              CLUTTER_STAGE_WIDTH() - (BORDER*2),
                              clutter_actor_get_y (data->button) - (BORDER*2));
      
      clutter_actor_set_opacity (data->results, 0x00);
      clutter_effect_fade (data->template,
                           data->results,
                           0xFF,
                           NULL,
                           NULL);
      
      /* Connected to related-videos button click */
      g_signal_connect (view, "related",
                        G_CALLBACK (related_cb), data);
    }
  else
    {
      /* TODO: Fade in 'No results' indicator */
      g_debug ("0 results");
    }
}

static void
throbber_in_complete (ClutterActor *actor, YouhaaData *data)
{
  data->transition = NULL;
}

static void
button_out_complete (ClutterActor *actor, YouhaaData *data)
{
  clutter_actor_hide (data->button);
  clutter_actor_show (data->throbber);

  clutter_timeline_start (data->throbber_timeline);

  data->transition = clutter_effect_rotate (data->template,
                         data->throbber,
                         CLUTTER_Y_AXIS,
                         0,
                         0,
                         0,
                         0,
                         CLUTTER_ROTATE_CCW,
                         (ClutterEffectCompleteFunc)throbber_in_complete,
                         data);
}

static void
animate_search (YouhaaData *data)
{
  static gboolean first_time = TRUE;
  
  if (first_time)
    {
      /* Fade out and destroy logo */
      clutter_effect_fade (data->template,
                           data->logo,
                           0x00,
                           (ClutterEffectCompleteFunc)destroy_out_complete,
                           data);
      first_time = FALSE;
    }
  
  /* Animate the throbber in */
  if (data->transition)
    clutter_timeline_stop (data->transition);
  data->transition = clutter_effect_rotate (data->template,
                         data->button,
                         CLUTTER_Y_AXIS,
                         90,
                         clutter_actor_get_width (data->button)/2,
                         0,
                         0,
                         CLUTTER_ROTATE_CW,
                         (ClutterEffectCompleteFunc)button_out_complete,
                         data);
}

static gboolean
button_pressed_cb (ClutterActor *button, ClutterEvent *event, YouhaaData *data)
{
  if (data->query_changed)
    {
      yh_youtube_query (data->youtube,
                        clutter_entry_get_text (CLUTTER_ENTRY (data->entry)));
      animate_search (data);
    }
  
  return TRUE;
}

static void
related_cb (YHYoutubeBrowser *browser, ClutterModelIter *iter, YouhaaData *data)
{
  gchar *url;
  
  clutter_model_iter_get (iter, YH_YOUTUBE_COL_RELATED, &url, -1);
  if (url)
    {
      yh_youtube_query_manual (data->youtube, url);
      g_free (url);
      animate_search (data);
    }
}

static gboolean
stage_key_press_event_cb (ClutterActor *actor,
                          ClutterKeyEvent *event,
                          YouhaaData *data)
{
  data->query_changed = TRUE;
  clutter_entry_handle_key_event (CLUTTER_ENTRY (data->entry), event);

  return TRUE;
}

int
main (int argc, char **argv)
{
  YouhaaData data;
  ClutterAlpha *alpha;
  ClutterBehaviour *behaviour;
  ClutterActor *stage, *button, *box, *label, *box2, *label2;
  
  clutter_init (&argc, &argv);
  clutter_gst_init (&argc, &argv);
  
  /* Setup stage */
  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 800, 600);
  
  /* Initialise data */
  data.query_changed = FALSE;
  data.results = NULL;
  data.transition = NULL;
  data.youtube = yh_youtube_get_default ();
  g_signal_connect (data.youtube,
                    "model",
                    G_CALLBACK (model_cb),
                    &data);
  data.template = clutter_effect_template_new (
                    clutter_timeline_new_for_duration (250),
                    CLUTTER_ALPHA_RAMP_INC);
  
  /* Create actors */
  
  /* Logo */
  data.logo = clutter_group_new ();
  box = clutter_rectangle_new_with_color (&white);
  label = clutter_label_new_full ("Sans 48", "You", &black);
  box2 = clutter_rectangle_new_with_color (&red);
  label2 = clutter_label_new_full ("Sans 48", "Tube", &white);
  clutter_container_add (CLUTTER_CONTAINER (data.logo),
                         box, label, box2, label2, NULL);
  clutter_actor_set_size (box,
                          clutter_actor_get_width (label2) + BORDER,
                          clutter_actor_get_height (label2) + BORDER/2);
  clutter_actor_set_size (box2,
                          clutter_actor_get_width (box),
                          clutter_actor_get_height (box));
  clutter_actor_set_y (box2, clutter_actor_get_height (box));
  clutter_actor_set_anchor_point_from_gravity (label, CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_anchor_point_from_gravity (label2, CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (label,
                              clutter_actor_get_width (box)/2,
                              clutter_actor_get_height (box)/2);
  clutter_actor_set_position (label2,
                              clutter_actor_get_width (box)/2,
                              (clutter_actor_get_height (box)*3)/2);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), data.logo);
  
  /* Search box + button */
  box = clutter_rectangle_new_with_color (&entry_color);
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (box), FRAME);
  clutter_rectangle_set_border_color (CLUTTER_RECTANGLE (box), &frame_color);
  
  data.entry = clutter_entry_new_full (font, "", &text_color);
  
  data.button = clutter_group_new ();
  
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               box);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               data.entry);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), data.button);
  
  clutter_actor_set_height (box,
                            clutter_actor_get_height (data.entry) + (BORDER/2));
  clutter_actor_set_width (box, CLUTTER_STAGE_WIDTH() - (BORDER*2) -
                           clutter_actor_get_height (box));
  clutter_actor_set_position (box, BORDER, CLUTTER_STAGE_HEIGHT() - BORDER -
                              clutter_actor_get_height (box));
  
  clutter_actor_set_width (data.entry,
                           clutter_actor_get_width (box) - (BORDER/2));
  clutter_actor_set_position (data.entry, 30, CLUTTER_STAGE_HEIGHT () -
                              clutter_actor_get_height (data.entry) -
                              BORDER + FRAME);
  
  button = clutter_rectangle_new_with_color (&bg_color);
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (button), FRAME);
  clutter_rectangle_set_border_color (CLUTTER_RECTANGLE (button), &frame_color);
  label = clutter_label_new_full (font, "Go!", &entry_color);
  clutter_actor_set_size (button, clutter_actor_get_height (box),
                          clutter_actor_get_height (box));
  clutter_container_add (CLUTTER_CONTAINER (data.button), button, label, NULL);
  clutter_actor_set_anchor_point_from_gravity (label, CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (label,
                              clutter_actor_get_width (button)/2,
                              clutter_actor_get_height (button)/2);
  
  clutter_actor_set_position (data.button, CLUTTER_STAGE_WIDTH() - BORDER -
                              clutter_actor_get_width (data.button),
                              CLUTTER_STAGE_HEIGHT() - BORDER -
                              clutter_actor_get_height (data.button));
  
  /* Set position of logo */
  clutter_actor_set_anchor_point_from_gravity (data.logo,
                                               CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (data.logo,
                              CLUTTER_STAGE_WIDTH () / 2,
                              clutter_actor_get_y (data.button) / 2);
  
  /* Throbber */
  data.throbber = clutter_label_new_full ("Sans 22", "+", &frame_color);
  clutter_actor_set_position (data.throbber,
                              clutter_actor_get_x (data.button) +
                              (clutter_actor_get_width (data.button) -
                               clutter_actor_get_width (data.throbber))/2,
                              clutter_actor_get_y (data.button) +
                              (clutter_actor_get_height (data.button) -
                               clutter_actor_get_height (data.throbber))/2);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), data.throbber);
  clutter_actor_move_anchor_point_from_gravity (data.throbber,
                                                CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_rotation (data.throbber,
                              CLUTTER_Y_AXIS,
                              90,
                              0,
                              0,
                              0);

  data.throbber_timeline = clutter_timeline_new_for_duration (500);
  alpha = clutter_alpha_new_full (data.throbber_timeline,
                                  CLUTTER_ALPHA_RAMP_INC, NULL, NULL);
  behaviour = clutter_behaviour_rotate_new (alpha,
                                            CLUTTER_Z_AXIS,
                                            CLUTTER_ROTATE_CW,
                                            0,
                                            360);
  clutter_timeline_set_loop (data.throbber_timeline, TRUE);
  clutter_behaviour_apply (behaviour, data.throbber);

  clutter_actor_show_all (stage);
  clutter_actor_show_all (data.button);
  clutter_actor_show_all (data.logo);
  clutter_actor_hide (data.throbber);
  
  clutter_actor_set_reactive (data.button, TRUE);
  g_signal_connect (data.button, "button-press-event",
                    G_CALLBACK (button_pressed_cb), &data);
  
  /* Hook up key events on stage to entry */
  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (stage_key_press_event_cb), &data);
  
  clutter_main ();
  
  return 0;
}

