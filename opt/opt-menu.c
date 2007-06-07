/* -*- mode:C; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "opt.h"

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_FONT "Sans Bold 20"
#define ITEM_HEIGHT 24
#define TEXT_BORDER 4
#define MENU_BORDER 1

G_DEFINE_TYPE (OptMenu, opt_menu, CLUTTER_TYPE_GROUP);

static void opt_menu_up (OptMenu * menu);
static void opt_menu_down (OptMenu * menu);
static void opt_menu_activate (OptMenu * menu);
static void opt_menu_select_item (OptMenu * menu, gint slide_no);

struct OptMenuPrivate
{
  guint             height;
  gint              current_slide;
  gint              active_item;
  gint              item_count;
  
  ClutterColor      color_normal;
  ClutterColor      color_sel;
  ClutterColor      color_bg;

  OptShow          *show;
  ClutterActor     *background;
  ClutterActor     *selection;

  ClutterTimeline  *timeline;
  ClutterAlpha     *alpha;
  ClutterBehaviour *behaviour_s;
  ClutterBehaviour *behaviour_o;
  
  gboolean          size_set;
  gboolean          hiding;
  guint             timeout_id;
  gulong            button_release_signal_id;
  gulong            key_release_signal_id;
};

/* Set sizes for background and selection -- called once the
 * menu is fully populated
 */
static void
opt_menu_init_size (OptMenu * menu)
{
  guint width, height;
  clutter_actor_get_size (CLUTTER_ACTOR (menu), &width, &height);

  width += 2 * TEXT_BORDER;
  
  clutter_actor_set_size (CLUTTER_ACTOR (menu),
                          width, height);
  
  clutter_actor_set_size (CLUTTER_ACTOR (menu->priv->background),
                          width, height);

  clutter_actor_set_size (CLUTTER_ACTOR (menu->priv->selection),
                          width - 2 * MENU_BORDER, ITEM_HEIGHT);

  menu->priv->height = height;
  menu->priv->size_set = TRUE;
}

/* Input callbacks
 */
static void 
opt_menu_key_release_cb (ClutterStage          *stage,
                         ClutterKeyEvent       *kev,
                         gpointer               user_data)
{
  OptMenu  *menu = OPT_MENU (user_data);

  if (!CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (menu)))
    return;

  switch (clutter_key_event_symbol (kev))
    {
      case CLUTTER_Up:
          opt_menu_up (menu);
          break;
      case CLUTTER_Down:
          opt_menu_down (menu);
          break;
      case CLUTTER_Return:
          opt_menu_activate (menu);
          break;

      default:
          opt_menu_popdown (menu);
          break;
    }
}

static void
opt_menu_button_release_cb (ClutterStage       *stage,
                            ClutterButtonEvent *bev,
                            gpointer            user_data)
{
  OptMenu  *menu = OPT_MENU (user_data);

  if (!CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (menu)))
    return;

  /* Allow a mouse wheel to control the menu (cannot handle
   * buttons 1 and 3 here, because those are used to control the slides).
   */

  if (bev->button == 4)
    opt_menu_up (menu);
  else if (bev->button == 5)
    opt_menu_down (menu);
  else if (bev->button == 2)
    opt_menu_activate (menu);
}

static void 
opt_menu_finalize (GObject *object)
{
  OptMenu *self = OPT_MENU(object); 

  g_object_unref (G_OBJECT (self->priv->behaviour_s));
  g_object_unref (G_OBJECT (self->priv->behaviour_o));
  g_object_unref (G_OBJECT (self->priv->timeline));
  
  if (self->priv)
    {
      g_free(self->priv);
      self->priv = NULL;
    }

  G_OBJECT_CLASS (opt_menu_parent_class)->finalize (object);
}

static void
opt_menu_class_init (OptMenuClass *klass)
{
  GObjectClass * object_class = (GObjectClass*) klass;
  object_class->finalize = opt_menu_finalize;
}

static void
opt_menu_init (OptMenu *self)
{
  OptMenuPrivate *priv = g_new0 (OptMenuPrivate, 1);
  self->priv = priv;
}

static void
opt_menu_hide_cb (ClutterTimeline * timeline, gpointer data)
{
  OptMenu  *menu = OPT_MENU (data);

  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (menu)) &&
      menu->priv->hiding)
    {
      ClutterActor * stage = clutter_stage_get_default();

      clutter_actor_hide_all (CLUTTER_ACTOR (menu));
      clutter_group_remove (CLUTTER_GROUP (stage), CLUTTER_ACTOR (menu));
      opt_menu_select_item (menu, 0);
      menu->priv->hiding = FALSE;
      
      if (menu->priv->timeout_id)
        {
          g_source_remove (menu->priv->timeout_id);
          menu->priv->timeout_id = 0;
        }
    }
}

OptMenu*
opt_menu_new (OptShow * show)
{
  OptMenu *menu = g_object_new (OPT_TYPE_MENU, NULL);

  /* TODO -- maybe allow these to be customised
   */
  menu->priv->color_normal.red   = 0xff;
  menu->priv->color_normal.green = 0xff;
  menu->priv->color_normal.blue  = 0xff;
  menu->priv->color_normal.alpha = 0xff;

  menu->priv->color_sel.red   = 0;
  menu->priv->color_sel.green = 0;
  menu->priv->color_sel.blue  = 0;
  menu->priv->color_sel.alpha = 0xff;

  menu->priv->color_bg.red   = 0x7f;
  menu->priv->color_bg.green = 0x7f;
  menu->priv->color_bg.blue  = 0x7f;
  menu->priv->color_bg.alpha = 0xcf;

  menu->priv->show = show;

  menu->priv->background =
    clutter_rectangle_new_with_color (&menu->priv->color_bg);
  
  clutter_rectangle_set_border_color(CLUTTER_RECTANGLE(menu->priv->background),
                                     &menu->priv->color_normal);
  clutter_rectangle_set_border_width(CLUTTER_RECTANGLE(menu->priv->background),
                                     1);
  
  clutter_group_add (CLUTTER_GROUP (menu),
                     CLUTTER_ACTOR (menu->priv->background));

  menu->priv->selection =
    clutter_rectangle_new_with_color (&menu->priv->color_normal);

  clutter_group_add (CLUTTER_GROUP (menu),
                     CLUTTER_ACTOR (menu->priv->selection));
  clutter_actor_set_position (CLUTTER_ACTOR (menu->priv->selection),
                              MENU_BORDER, 0);

  menu->priv->timeline = clutter_timeline_new (10, 26);

  g_signal_connect (menu->priv->timeline, "completed",
                    G_CALLBACK (opt_menu_hide_cb), menu);
  
  menu->priv->alpha = clutter_alpha_new_full (menu->priv->timeline,
                                              CLUTTER_ALPHA_RAMP_INC,
                                              NULL, NULL);

  menu->priv->behaviour_s =
    clutter_behaviour_scale_new (menu->priv->alpha,
                                 0.0, 1.0,
                                 CLUTTER_GRAVITY_NORTH_WEST); 

  clutter_behaviour_apply (menu->priv->behaviour_s, CLUTTER_ACTOR (menu));

  menu->priv->behaviour_o =
    clutter_behaviour_opacity_new (menu->priv->alpha, 0x00, 0xff); 

  clutter_behaviour_apply (menu->priv->behaviour_o, CLUTTER_ACTOR (menu));
  
  return menu;
}

/*
 * Adjusts the postition of the menu if the selected item is
 * off screen
 */
static void
opt_menu_adjust_postion (OptMenu * menu)
{
  if (menu->priv->height > CLUTTER_STAGE_HEIGHT ())
    {
      gint x           = clutter_actor_get_x (CLUTTER_ACTOR (menu));
      gint y           = clutter_actor_get_y (CLUTTER_ACTOR (menu));
      gint item_offset = menu->priv->active_item * ITEM_HEIGHT + y;

      if (item_offset < 0)
        {
          /* attemp to shift the item to the middle of screen, but no so that
           * the the menu would detach from the top of stage
           */
          gint screen_itms = CLUTTER_STAGE_HEIGHT () / ITEM_HEIGHT;
          gint shift = ITEM_HEIGHT * screen_itms / 2 - item_offset;

          y += shift;

          if (shift > 0)
            y = 0;
        }
      else if (item_offset > CLUTTER_STAGE_HEIGHT () - ITEM_HEIGHT)
        {
          /* attemp to shift the item to the middle of screen, but no so that
           * the the menu would detach from the bottom of stage
           */
          gint screen_itms = CLUTTER_STAGE_HEIGHT () / ITEM_HEIGHT;
          gint shift = ITEM_HEIGHT * screen_itms / 2 + item_offset;
          gint max_shft = (menu->priv->item_count - screen_itms)*ITEM_HEIGHT;
          
          if (shift > max_shft)
            shift = max_shft;
          
          y -= shift;
        }

      clutter_actor_set_position (CLUTTER_ACTOR (menu), x, y);
    }
}

/*
 * Selects nth item in the menu
 */
static void
opt_menu_select_item (OptMenu * menu, gint slide_no)
{
  if (slide_no < 0 || slide_no >= menu->priv->item_count)
    return;
  
  if (menu->priv->active_item != slide_no)
    {
      /* Plus two, because the first two children are the background
       * and selection rectangles
       */
      ClutterActor * active =
        clutter_group_get_nth_child (CLUTTER_GROUP (menu),
                                     menu->priv->active_item + 2);
  
      clutter_label_set_color (CLUTTER_LABEL (active),
                               &menu->priv->color_normal);

      active = clutter_group_get_nth_child (CLUTTER_GROUP (menu),
                                            slide_no + 2);
  
      clutter_label_set_color (CLUTTER_LABEL (active), &menu->priv->color_sel);

      clutter_actor_set_position (CLUTTER_ACTOR (menu->priv->selection),
                                  MENU_BORDER, slide_no * ITEM_HEIGHT);
      
      menu->priv->active_item = slide_no;

      opt_menu_adjust_postion (menu);
    }
}

/*
 * Callback to automatically close the menu after given period of inactivity
 */
static gboolean
opt_menu_timeout_cb (gpointer data)
{
  OptMenu * menu = data;

  opt_menu_popdown (menu);
  menu->priv->timeout_id = 0;
  
  return FALSE;
}

/*
 * move one item up in the menu
 */
static void
opt_menu_up (OptMenu * menu)
{
  opt_menu_select_item (menu, menu->priv->active_item - 1);

  if (menu->priv->timeout_id)
    {
      g_source_remove (menu->priv->timeout_id);
      menu->priv->timeout_id = g_timeout_add (5000, opt_menu_timeout_cb, menu);
    }
}

/* move one item down in the menu */
static void
opt_menu_down (OptMenu * menu)
{
  opt_menu_select_item (menu, menu->priv->active_item + 1);

  if (menu->priv->timeout_id)
    {
      g_source_remove (menu->priv->timeout_id);
      menu->priv->timeout_id = g_timeout_add (5000, opt_menu_timeout_cb, menu);
    }
}

/*
 * Jump to the slide represented by the active menu item
 */
static void
opt_menu_activate (OptMenu * menu)
{
  int step = menu->priv->active_item - menu->priv->current_slide;

  opt_menu_popdown (menu);
  
  if (step)
    opt_show_skip (menu->priv->show, step);
}

/*
 *  Called when we mode to a different slide
 */
void
opt_menu_set_current_slide (OptMenu * menu, gint slide_no)
{
  opt_menu_select_item (menu, slide_no);
  menu->priv->current_slide = slide_no;
}

/*
 * Adds a slide to the menu
 */
void
opt_menu_add_slide (OptMenu * menu, OptSlide * slide)
{
  static gint y = 0;
  
  gchar              * text = NULL;
  const gchar        * font = DEFAULT_FONT;
  const ClutterLabel * title = CLUTTER_LABEL (opt_slide_get_title (slide));
  ClutterActor       * label;
  
  if (title)
    text = g_strdup_printf ("Slide %d: %s", menu->priv->item_count + 1,
                            clutter_label_get_text ((ClutterLabel*)title));
  else
    text = g_strdup_printf ("Slide %d", menu->priv->item_count + 1);

  if (!menu->priv->item_count)
    label = clutter_label_new_full (font, text, &menu->priv->color_sel);
  else
    label = clutter_label_new_full (font, text, &menu->priv->color_normal);
  
  g_free (text);
  
  clutter_actor_set_position (label, TEXT_BORDER, y);
  y += ITEM_HEIGHT;
      
  clutter_group_add (CLUTTER_GROUP (menu), label);
  menu->priv->item_count++;
}

/*
 * Shows menu
 */
void
opt_menu_pop (OptMenu * menu)
{
  if (!CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (menu)))
    {
      guint width, height;
      
      ClutterActor * stage = clutter_stage_get_default();

      if (!menu->priv->size_set)
        opt_menu_init_size (menu);
      
      clutter_actor_get_size (CLUTTER_ACTOR (menu), &width, &height);

      clutter_actor_set_scale (CLUTTER_ACTOR (menu), 0.0, 0.0);
      clutter_alpha_set_func (menu->priv->alpha, CLUTTER_ALPHA_RAMP_INC,
                              NULL, NULL);
      
      clutter_group_add (CLUTTER_GROUP(stage), CLUTTER_ACTOR(menu));

      clutter_actor_set_position (CLUTTER_ACTOR (menu), 0, 0);

      /* Connect up for input event */
      menu->priv->button_release_signal_id =
        g_signal_connect (stage, "button-release-event",
                          G_CALLBACK (opt_menu_button_release_cb), menu);
      menu->priv->key_release_signal_id =
        g_signal_connect (stage, "key-release-event",
                          G_CALLBACK (opt_menu_key_release_cb), menu);

      opt_menu_select_item (menu, menu->priv->current_slide);
      clutter_actor_show_all (CLUTTER_ACTOR (menu));

      menu->priv->timeout_id = g_timeout_add (5000, opt_menu_timeout_cb, menu);
      menu->priv->hiding = FALSE;
      clutter_timeline_start (menu->priv->timeline);
    }
}

/*
 * Hides menu, if shown
 */
void
opt_menu_popdown (OptMenu * menu)
{
  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (menu)))
  {
    ClutterActor * stage = clutter_stage_get_default();

    if (menu->priv->button_release_signal_id)
      {
        g_signal_handler_disconnect (stage,
                                     menu->priv->button_release_signal_id);
        menu->priv->button_release_signal_id = 0;
      }

    if (menu->priv->key_release_signal_id)
      {
        g_signal_handler_disconnect (stage,
                                     menu->priv->key_release_signal_id);
        menu->priv->key_release_signal_id = 0;
      }
  
    clutter_actor_set_scale (CLUTTER_ACTOR (menu), 1.0, 1.0);
    clutter_alpha_set_func (menu->priv->alpha, CLUTTER_ALPHA_RAMP_DEC,
                            NULL, NULL);
    
    menu->priv->hiding = TRUE;
    clutter_timeline_start (menu->priv->timeline);
  }
}

