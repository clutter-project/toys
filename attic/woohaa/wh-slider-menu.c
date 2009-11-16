#include <glib.h>
#include "wh-slider-menu.h"

#define CSW() CLUTTER_STAGE_WIDTH()
#define CSH() CLUTTER_STAGE_HEIGHT()

#define SELECTED_OFFSET (CLUTTER_STAGE_WIDTH()/5)

typedef struct WoohaaSliderMenuEntry
{
  ClutterActor                   *actor;
  WoohaaSliderMenuSelectedFunc    selected_func;
  gpointer                        userdata;
  gint                            offset;
}
WoohaaSliderMenuEntry;

#define WOOHAA_TYPE_BEHAVIOUR_SLIDER (clutter_behaviour_slider_get_type ())

#define WOOHAA_BEHAVIOUR_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WOOHAA_TYPE_BEHAVIOUR_SLIDER, WoohaaBehaviourSlider))

#define WOOHAA_BEHAVIOUR_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WOOHAA_TYPE_BEHAVIOUR_SLIDER, WoohaaBehaviourSliderClass))

#define CLUTTER_IS_BEHAVIOUR_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WOOHAA_TYPE_BEHAVIOUR_SLIDER))

#define CLUTTER_IS_BEHAVIOUR_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WOOHAA_TYPE_BEHAVIOUR_SLIDER))

#define WOOHAA_BEHAVIOUR_SLIDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WOOHAA_TYPE_BEHAVIOUR_SLIDER, WoohaaBehaviourSliderClass))

typedef struct _WoohaaBehaviourSlider        WoohaaBehaviourSlider;
typedef struct _WoohaaBehaviourSliderClass   WoohaaBehaviourSliderClass;
 
struct _WoohaaBehaviourSlider
{
  ClutterBehaviour        parent;
  WoohaaSliderMenuEntry  *old;
  WoohaaSliderMenuEntry  *new;
  WoohaaSliderMenu       *menu;
};

struct _WoohaaBehaviourSliderClass
{
  ClutterBehaviourClass   parent_class;
};

GType clutter_behaviour_slider_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (WoohaaBehaviourSlider, clutter_behaviour_slider, CLUTTER_TYPE_BEHAVIOUR);

static ClutterBehaviour*
clutter_behaviour_slider_new (WoohaaSliderMenu *menu,
			      WoohaaSliderMenuEntry *start,
			      WoohaaSliderMenuEntry *end);

struct _WoohaaSliderMenuPrivate
{
  GList           *entrys;
  gint             entry_height;
  gint             menu_width;
  gint             n_entrys;
  gint             active_entry_num;
  gint             offset; 	    /* current offset */
  gint             unclipped_width;
  ClutterActor    *bg;
  ClutterActor    *entry_group;

  guint            alpha_value;

  ClutterTimeline        *timeline;
  ClutterAlpha           *alpha;
  ClutterBehaviour       *behave;
  ClutterEffectTemplate  *effect_template;

  gchar            *font;
  ClutterColor     *font_color;
  ClutterActor     *next, *prev;
};

G_DEFINE_TYPE (WoohaaSliderMenu, woohaa_slider_menu, CLUTTER_TYPE_ACTOR);

#define WOOHAA_SLIDER_MENU_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), WOOHAA_TYPE_SLIDER_MENU, WoohaaSliderMenuPrivate))

static void
woohaa_slider_menu_dispose (GObject *object)
{
  WoohaaSliderMenu        *self;
  WoohaaSliderMenuPrivate *priv; 

  self = WOOHAA_SLIDER_MENU(object); 
  priv = self->priv;

  G_OBJECT_CLASS (woohaa_slider_menu_parent_class)->dispose (object);
}

static void
woohaa_slider_menu_finalize (GObject *object)
{
  WoohaaSliderMenu        *self;
  WoohaaSliderMenuPrivate *priv; 

  self = WOOHAA_SLIDER_MENU(object); 
  priv = self->priv;

  G_OBJECT_CLASS (woohaa_slider_menu_parent_class)->finalize (object);
}

static void
woohaa_slider_menu_paint (ClutterActor *actor)
{
  WoohaaSliderMenuPrivate *priv = (WOOHAA_SLIDER_MENU (actor))->priv;
  
  clutter_actor_paint (priv->bg);
  clutter_actor_paint (priv->next);
  clutter_actor_paint (priv->prev);
  clutter_actor_paint (priv->entry_group);
}

static void
woohaa_slider_menu_get_preferred_width  (ClutterActor *actor,
					 ClutterUnit   for_height,
					 ClutterUnit  *min_width_p,
					 ClutterUnit  *natural_width_p)
{
  *min_width_p = CLUTTER_UNITS_FROM_INT (100);
  *natural_width_p = CLUTTER_UNITS_FROM_INT (CSW());
}

static void
woohaa_slider_menu_get_preferred_height (ClutterActor *actor,
					 ClutterUnit   for_width,
					 ClutterUnit  *min_height_p,
					 ClutterUnit  *natural_height_p)
{
  WoohaaSliderMenuPrivate *priv = (WOOHAA_SLIDER_MENU (actor))->priv;

  *min_height_p = CLUTTER_UNITS_FROM_INT (1);
  if (priv->entry_height)
	  *natural_height_p = CLUTTER_UNITS_FROM_INT (priv->entry_height * 2);
  else
	  *natural_height_p = CLUTTER_UNITS_FROM_INT (200);
}

static void
woohaa_slider_menu_allocate (ClutterActor *actor, 
			     const ClutterActorBox *box,
			     gboolean absolute_origin_changed)
{
  WoohaaSliderMenuPrivate *priv = (WOOHAA_SLIDER_MENU (actor))->priv;
  ClutterUnit natural_width, natural_height;
  ClutterActorBox child_box;
  ClutterUnit focal_x, focal_y;
  ClutterUnit entry_offset = 0, entry_width = 0;
  WoohaaSliderMenuEntry *current, *old;
  
  clutter_actor_get_preferred_size (priv->bg, NULL, NULL, 
				    &natural_width, &natural_height);
  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = natural_width;
  child_box.y2 = natural_height;
  clutter_actor_allocate (priv->bg, &child_box, absolute_origin_changed);

  focal_x = CLUTTER_UNITS_FROM_INT(CSW()/4);
  focal_y = 0;

  if (priv->entrys)
  {
    current = (WOOHAA_BEHAVIOUR_SLIDER (priv->behave))->new;
    old = (WOOHAA_BEHAVIOUR_SLIDER (priv->behave))->old;
    
    if (current && old)
      {
	entry_offset = (clutter_actor_get_xu (current->actor) - clutter_actor_get_xu (old->actor)) * 
	  ((gdouble)(priv->alpha_value) / (gdouble)CLUTTER_ALPHA_MAX_ALPHA) +
	  clutter_actor_get_xu (old->actor);

	entry_width = (clutter_actor_get_widthu (current->actor) - clutter_actor_get_widthu (old->actor)) * 
	  ((gdouble)(priv->alpha_value) / (gdouble)CLUTTER_ALPHA_MAX_ALPHA) +
	  clutter_actor_get_widthu (old->actor);
      }

  }

  child_box.x1 = focal_x - entry_offset;
  child_box.y1 = focal_y;
  child_box.x2 = natural_height/2 + child_box.x1 - entry_offset;
  child_box.y2 = natural_height/2 + child_box.y1;
  clutter_actor_allocate (priv->entry_group, 
			  &child_box, 
			  absolute_origin_changed);

  if (priv->active_entry_num > 0)
    {
      clutter_actor_set_opacity (priv->prev, 0xAA);

      child_box.x1 = focal_x - natural_height/2;
      child_box.y1 = focal_y + natural_height/10;
      child_box.x2 = natural_height/2 + child_box.x1;
      child_box.y2 = natural_height/2 + child_box.y1;
      clutter_actor_allocate (priv->prev, &child_box, absolute_origin_changed);
    }
  else
    {
      clutter_actor_set_opacity (priv->prev, 
				 0xff + (priv->alpha_value * (-0xff)
					 / CLUTTER_ALPHA_MAX_ALPHA));
    }

  if (priv->active_entry_num < priv->n_entrys - 1)
    {
      clutter_actor_set_opacity (priv->next, 0xAA);

      child_box.x1 = focal_x + entry_width;
      child_box.y1 = focal_y + natural_height/10;
      child_box.x2 = natural_height/2 + child_box.x1;
      child_box.y2 = natural_height/2 + child_box.y1;
      clutter_actor_allocate (priv->next, &child_box, absolute_origin_changed);
    }
  else
    {
      clutter_actor_set_opacity (priv->next, 
				 0xff + (priv->alpha_value * (-0xff)
					 / CLUTTER_ALPHA_MAX_ALPHA));
    }

  CLUTTER_ACTOR_CLASS (woohaa_slider_menu_parent_class)->
  	  allocate (actor, box, absolute_origin_changed);
}

static void
woohaa_slider_menu_class_init (WoohaaSliderMenuClass *klass)
{
  GObjectClass *object_class     = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (WoohaaSliderMenuPrivate));

  object_class->dispose      = woohaa_slider_menu_dispose;
  object_class->finalize     = woohaa_slider_menu_finalize;

  actor_class->get_preferred_width  = woohaa_slider_menu_get_preferred_width;
  actor_class->get_preferred_height = woohaa_slider_menu_get_preferred_height;
  actor_class->allocate             = woohaa_slider_menu_allocate;
  actor_class->paint                = woohaa_slider_menu_paint;
}

static void
woohaa_slider_menu_init (WoohaaSliderMenu *woohaa_slider_menu)
{
  WoohaaSliderMenuPrivate   *priv;

  woohaa_slider_menu->priv = priv =
    G_TYPE_INSTANCE_GET_PRIVATE (woohaa_slider_menu,
                                 WOOHAA_TYPE_SLIDER_MENU,
                                 WoohaaSliderMenuPrivate);

  priv->menu_width = CSW();

  priv->timeline = clutter_timeline_new (30, 60);

  priv->alpha = clutter_alpha_new_full (priv->timeline,
					CLUTTER_ALPHA_SINE_INC,
					NULL, NULL);

  priv->behave = clutter_behaviour_slider_new (woohaa_slider_menu, 0, 0);

  priv->effect_template 
    = clutter_effect_template_new (clutter_timeline_new (20, 60),
				   CLUTTER_ALPHA_SINE_INC);

  priv->font_color = g_new0(ClutterColor, 1);
  clutter_color_parse ("#ccccccff", priv->font_color);

  priv->bg = clutter_texture_new_from_file (PKGDATADIR "/header.svg", NULL);
  if (!priv->bg) g_warning ("Unable to load heaer.svg");

  clutter_actor_set_parent (priv->bg, CLUTTER_ACTOR (woohaa_slider_menu));

  clutter_actor_set_width (priv->bg, CSW());
  
  clutter_actor_show (priv->bg);

  priv->next = clutter_texture_new_from_file (PKGDATADIR "/arrow-next.svg", 
					      NULL);
  if (!priv->next) g_warning ("Unable to load arror-next.svg");

  clutter_actor_hide (priv->next);
  clutter_actor_set_parent (priv->next, CLUTTER_ACTOR (woohaa_slider_menu));

  priv->prev = clutter_texture_new_from_file (PKGDATADIR "/arrow-prev.svg", 
					      NULL);
  if (!priv->prev) g_warning ("Unable to load arror-prev.svg");

  clutter_actor_hide (priv->prev);
  clutter_actor_set_parent (priv->prev, CLUTTER_ACTOR (woohaa_slider_menu));

  priv->entry_group = clutter_group_new ();
  clutter_actor_set_parent (priv->entry_group, 
			    CLUTTER_ACTOR (woohaa_slider_menu));
  clutter_actor_show (priv->entry_group);
}

ClutterActor*
woohaa_slider_menu_new (const gchar *font)
{
  ClutterActor         *menu;
  WoohaaSliderMenuPrivate  *priv;

  menu = g_object_new (WOOHAA_TYPE_SLIDER_MENU, NULL);
  priv = WOOHAA_SLIDER_MENU_GET_PRIVATE (menu);
  priv->font = g_strdup(font);

  return menu;
}

void
woohaa_slider_menu_add_option (WoohaaSliderMenu            *menu, 
			       const gchar                 *text,
			       WoohaaSliderMenuSelectedFunc selected,
			       gpointer                     userdata)
{
  WoohaaSliderMenuPrivate  *priv = WOOHAA_SLIDER_MENU_GET_PRIVATE (menu);
  WoohaaSliderMenuEntry    *entry;
  ClutterActor             *actor;
  gint                      pad = 0;

  actor = clutter_label_new_with_text (priv->font, text);
  clutter_label_set_color (CLUTTER_LABEL(actor), priv->font_color);
  clutter_label_set_line_wrap (CLUTTER_LABEL(actor), FALSE);

  entry = g_new0(WoohaaSliderMenuEntry, 1);
  entry->actor = actor;
  entry->selected_func = selected;
  entry->userdata = userdata;

  if (clutter_actor_get_height(actor) > priv->entry_height)
    {
      priv->entry_height = clutter_actor_get_height(actor);

      clutter_actor_set_width (priv->bg, CSW());
      clutter_actor_set_height (priv->bg, priv->entry_height
	+ (priv->entry_height/2));
    }

  if (clutter_actor_get_height(priv->next) > priv->entry_height)
    {
      gint w, h;

      w = priv->entry_height/8;
      h = priv->entry_height/4;
      
      clutter_actor_set_size (priv->next, w, h);
      clutter_actor_set_size (priv->prev, w, h);
    }

  pad = clutter_actor_get_width (priv->next) * 2;

  entry->offset = priv->unclipped_width + pad;

  if (priv->entrys == NULL)
    priv->unclipped_width += pad;

  priv->unclipped_width += clutter_actor_get_width(actor) + pad;

  if (priv->entrys == 0)
    {
      /* First Entry */
      clutter_actor_set_opacity (actor, 0xff);
    }
  else
    {
      clutter_actor_set_opacity (actor, 0x33);
      clutter_actor_set_scale (actor, 0.7, 0.7);
    }

  clutter_group_add (CLUTTER_GROUP (priv->entry_group), actor);

  priv->entrys = g_list_append (priv->entrys, entry);

  clutter_actor_set_position (actor, 
			      entry->offset, 
			      priv->entry_height/12);

  priv->n_entrys++;
}

void
woohaa_slider_menu_activate (WoohaaSliderMenu *menu,
			     gint              entry_num)
{
  WoohaaSliderMenuPrivate  *priv = WOOHAA_SLIDER_MENU_GET_PRIVATE (menu);
  WoohaaSliderMenuEntry *selected, *current;

  if (entry_num < 0 || entry_num >= priv->n_entrys)
      return;

  if (clutter_timeline_is_playing(priv->timeline))
    return;

  current 
    = (WoohaaSliderMenuEntry *)g_list_nth_data(priv->entrys, 
					       priv->active_entry_num);

  selected = (WoohaaSliderMenuEntry *)g_list_nth_data(priv->entrys, 
						      entry_num);

  priv->active_entry_num = entry_num;

  WOOHAA_BEHAVIOUR_SLIDER(priv->behave)->old = current;
  WOOHAA_BEHAVIOUR_SLIDER(priv->behave)->new = selected;

  clutter_actor_queue_relayout (CLUTTER_ACTOR (menu));

  /* FIXME: Should be a signal */
  if (selected->selected_func)
    selected->selected_func(menu, selected->actor, selected->userdata);

  clutter_timeline_start (priv->timeline);    
}

void
woohaa_slider_menu_advance (WoohaaSliderMenu *menu, gint n)
{
  WoohaaSliderMenuPrivate  *priv = WOOHAA_SLIDER_MENU_GET_PRIVATE (menu);
  woohaa_slider_menu_activate (menu, priv->active_entry_num + n);
}

/* Custom behaviour */

static void
clutter_behaviour_alpha_notify (ClutterBehaviour *behave,
                                guint32           alpha_value)
{
  WoohaaBehaviourSlider *slide = WOOHAA_BEHAVIOUR_SLIDER(behave);
  WoohaaSliderMenu      *menu;
  gdouble                scale;
  WoohaaSliderMenuPrivate  *priv;

  if (!(slide->old) || !(slide->new))
    return;

  menu = slide->menu;
  priv = WOOHAA_SLIDER_MENU_GET_PRIVATE (menu);

  priv->offset = slide->old->offset + 
                      (((gint)alpha_value * 
                          (slide->new->offset - slide->old->offset))
			       / CLUTTER_ALPHA_MAX_ALPHA);

  clutter_actor_set_opacity (slide->old->actor, 
			     0xff + (alpha_value 
				     * (0x66 - 0xff)
				     / CLUTTER_ALPHA_MAX_ALPHA));

  clutter_actor_set_opacity (slide->new->actor, 
			     0x66 + (alpha_value 
				     * (0xff - 0x66)
				     / CLUTTER_ALPHA_MAX_ALPHA));

  scale = (0.3 * alpha_value) / (gdouble)CLUTTER_ALPHA_MAX_ALPHA;

  clutter_actor_set_scale (slide->new->actor,
			   0.7 + scale,
			   0.7 + scale);

  if (slide->new->actor != slide->old->actor)
    clutter_actor_set_scale (slide->old->actor,
			     1.0 - scale,
			     1.0 - scale);

  priv->alpha_value = alpha_value;

  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(menu)))
    clutter_actor_queue_relayout (CLUTTER_ACTOR (menu));
}

static void
clutter_behaviour_slider_class_init (WoohaaBehaviourSliderClass *klass)
{
  ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

  behave_class->alpha_notify = clutter_behaviour_alpha_notify;
}

static void
clutter_behaviour_slider_init (WoohaaBehaviourSlider *self)
{
}

static ClutterBehaviour*
clutter_behaviour_slider_new (WoohaaSliderMenu      *menu,
			      WoohaaSliderMenuEntry *old,
			      WoohaaSliderMenuEntry *new)
{
  WoohaaSliderMenuPrivate  *priv = WOOHAA_SLIDER_MENU_GET_PRIVATE (menu);
  WoohaaBehaviourSlider *slide_behave;

  slide_behave = g_object_new (WOOHAA_TYPE_BEHAVIOUR_SLIDER, 
			       "alpha", priv->alpha,
			       NULL);

  slide_behave->old   = old;
  slide_behave->new   = new;
  slide_behave->menu  = menu;

  return CLUTTER_BEHAVIOUR(slide_behave);
}
