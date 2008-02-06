#include "wh-slider-menu.h"
#include "util.h"

#include <clutter/cogl.h>

#define FONT "Sans 75px"
#define SELECTED_OFFSET (CLUTTER_STAGE_WIDTH()/5)

typedef struct WHSliderMenuEntry
{
  ClutterActor                   *actor;
  WHSliderMenuSelectedFunc        selected_func;
  gpointer                        userdata;
  gint                            offset;
}
WHSliderMenuEntry;

#define WH_TYPE_BEHAVIOUR_SLIDER (clutter_behaviour_slider_get_type ())

#define WH_BEHAVIOUR_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_BEHAVIOUR_SLIDER, WHBehaviourSlider))

#define WH_BEHAVIOUR_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_BEHAVIOUR_SLIDER, WHBehaviourSliderClass))

#define CLUTTER_IS_BEHAVIOUR_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_BEHAVIOUR_SLIDER))

#define CLUTTER_IS_BEHAVIOUR_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_BEHAVIOUR_SLIDER))

#define WH_BEHAVIOUR_SLIDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_BEHAVIOUR_SLIDER, WHBehaviourSliderClass))

typedef struct _WHBehaviourSlider        WHBehaviourSlider;
typedef struct _WHBehaviourSliderClass   WHBehaviourSliderClass;
 
struct _WHBehaviourSlider
{
  ClutterBehaviour        parent;
  WHSliderMenuEntry      *old;
  WHSliderMenuEntry      *new;
  WHSliderMenu           *menu;
};

struct _WHBehaviourSliderClass
{
  ClutterBehaviourClass   parent_class;
};

GType clutter_behaviour_slider_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (WHBehaviourSlider, clutter_behaviour_slider, CLUTTER_TYPE_BEHAVIOUR);

static ClutterBehaviour*
clutter_behaviour_slider_new (WHSliderMenu *menu,
			      WHSliderMenuEntry *start,
			      WHSliderMenuEntry *end);


G_DEFINE_TYPE (WHSliderMenu, wh_slider_menu, CLUTTER_TYPE_ACTOR);

enum
{
  PROP_0,
};

#define WH_SLIDER_MENU_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), WH_TYPE_SLIDER_MENU, WHSliderMenuPrivate))


struct _WHSliderMenuPrivate
{
  GList           *entrys;
  gint             entry_height;
  gint             menu_width;
  gint             n_entrys;
  gint             active_entry_num;
  gint             offset; 	    /* current offset */
  gint             unclipped_width;
  ClutterActor    *bg;

  ClutterTimeline        *timeline;
  ClutterAlpha           *alpha;
  ClutterBehaviour       *behave;
  ClutterEffectTemplate  *effect_template;

  gchar            *font;
  ClutterColor     *font_color;
  ClutterActor     *next, *prev;
};

static void
show_handler (ClutterActor *actor)
{
  WHSliderMenu        *disk;
  WHSliderMenuPrivate *priv;
  gint                 x;
  gint                 y;

  disk = WH_SLIDER_MENU(actor);
  priv = disk->priv;

  x = clutter_actor_get_x (actor);
  y = clutter_actor_get_y (actor);

  clutter_actor_set_position (actor,
			      clutter_actor_get_x (actor),
			      -clutter_actor_get_height (actor));

  clutter_effect_move (priv->effect_template,
		       actor,
		       x,
		       y,
		       NULL,
		       NULL);
}

static void
wh_slider_menu_set_property (GObject      *object, 
			     guint         prop_id,
			     const GValue *value, 
			     GParamSpec   *pspec)
{
  WHSliderMenu        *disk;
  WHSliderMenuPrivate *priv;

  disk = WH_SLIDER_MENU(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
wh_slider_menu_get_property (GObject    *object, 
				  guint       prop_id,
				  GValue     *value, 
				  GParamSpec *pspec)
{
  WHSliderMenu        *disk;
  WHSliderMenuPrivate *priv;

  disk = WH_SLIDER_MENU(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}

static void
wh_slider_menu_request_coords (ClutterActor    *self,
			       ClutterActorBox *box)
{
  WHSliderMenuPrivate *priv;
  ClutterActorBox     cbox;

  priv = WH_SLIDER_MENU(self)->priv;

  /* Only positioning + width works.*/
  priv->menu_width = CLUTTER_UNITS_TO_INT(box->x2 - box->x1);

  if (!priv->bg)
    {  
      gint w, h;

      w = priv->menu_width - (priv->menu_width/10);
      h = CLUTTER_UNITS_TO_INT(box->y2 - box->y1);

      priv->bg = util_actor_from_file (PKGDATADIR "/header.svg", w, h);

      if (priv->bg == NULL)
	g_warning ("Unable to load " PKGDATADIR "/header.svg");

      clutter_actor_set_size (priv->bg, w, h);
      clutter_actor_set_position (priv->bg, w/20, 0);
      clutter_actor_set_parent (priv->bg, self);
      clutter_actor_show (priv->bg);

      priv->next = util_actor_from_file (PKGDATADIR "/arrow-next.svg", h/8, h/4);
      // clutter_actor_set_size (priv->next, h/8, h/4);
      clutter_actor_set_parent (priv->next, CLUTTER_ACTOR (self));
      clutter_actor_show (priv->next);

      priv->prev = util_actor_from_file (PKGDATADIR "/arrow-prev.svg", h/8, h/4);
      // clutter_actor_set_size (priv->prev, h/8, h/4);
      clutter_actor_set_parent (priv->prev, CLUTTER_ACTOR (self));
      clutter_actor_show (priv->prev);
    }

  clutter_actor_query_coords (self, &cbox);

  box->x2 = box->x1 + (cbox.x2 - cbox.x1);
  box->y2 = box->y1 + (cbox.y2 - cbox.y1);
}

static void
wh_slider_menu_query_coords (ClutterActor    *self,
			     ClutterActorBox *box)
{
  WHSliderMenuPrivate *priv;

  priv = WH_SLIDER_MENU(self)->priv;

  box->x2 = box->x1 + CLUTTER_UNITS_FROM_INT(priv->menu_width);
  box->y2 = box->y1 + CLUTTER_UNITS_FROM_INT(priv->entry_height);
}

static void
wh_slider_menu_paint (ClutterActor *actor)
{
  WHSliderMenu      *menu = WH_SLIDER_MENU(actor);
  GList             *entry;

  cogl_push_matrix();

  clutter_actor_paint (menu->priv->bg);

  cogl_clip_set (CLUTTER_INT_TO_FIXED (menu->priv->menu_width / 16),
                 0,
                 CLUTTER_INT_TO_FIXED (menu->priv->menu_width -
                                       ( menu->priv->menu_width / 8 )),
                 CLUTTER_UNITS_TO_FIXED (clutter_actor_get_heightu (actor)));

  cogl_translate (SELECTED_OFFSET, 0, 0);

  if (menu->priv->active_entry_num > 0)
      clutter_actor_paint (menu->priv->prev);

  if (menu->priv->active_entry_num < menu->priv->n_entrys-1)
      clutter_actor_paint (menu->priv->next);

  cogl_translate (-1  * menu->priv->offset 
		  + clutter_actor_get_width(menu->priv->prev), 0, 0);

  for (entry = menu->priv->entrys;
       entry != NULL;
       entry = entry->next)
    {
      WHSliderMenuEntry *data = (WHSliderMenuEntry*)entry->data;
      ClutterActor *child = data->actor;

      g_assert (child != NULL);

      if (CLUTTER_ACTOR_IS_MAPPED (child))
	  clutter_actor_paint (child);
    }

  cogl_clip_unset();

  cogl_pop_matrix();
}

static void
wh_slider_menu_realize (ClutterActor *self)
{
  WHSliderMenu        *menu;

  menu = WH_SLIDER_MENU(self);
}

static void 
wh_slider_menu_dispose (GObject *object)
{
  WHSliderMenu         *self = WH_SLIDER_MENU(object);
  WHSliderMenuPrivate  *priv;  

  priv = self->priv;
  
  G_OBJECT_CLASS (wh_slider_menu_parent_class)->dispose (object);
}

static void 
wh_slider_menu_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_slider_menu_parent_class)->finalize (object);
}

static void
wh_slider_menu_class_init (WHSliderMenuClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
  ClutterActorClass   *parent_class; 

  parent_class = CLUTTER_ACTOR_CLASS (wh_slider_menu_parent_class);

  actor_class->paint      = wh_slider_menu_paint;
  actor_class->realize    = wh_slider_menu_realize;
  actor_class->request_coords  = wh_slider_menu_request_coords;
  actor_class->query_coords = wh_slider_menu_query_coords;

  actor_class->unrealize  = parent_class->unrealize;
  actor_class->show       = parent_class->show;
  actor_class->hide       = parent_class->hide;

  gobject_class->finalize     = wh_slider_menu_finalize;
  gobject_class->dispose      = wh_slider_menu_dispose;
  gobject_class->set_property = wh_slider_menu_set_property;
  gobject_class->get_property = wh_slider_menu_get_property;

  g_type_class_add_private (gobject_class, sizeof (WHSliderMenuPrivate));
}

static void
wh_slider_menu_init (WHSliderMenu *self)
{
  WHSliderMenuPrivate *priv;

  self->priv = priv = WH_SLIDER_MENU_GET_PRIVATE (self);

  priv->timeline = clutter_timeline_new (10, 60);

  priv->alpha = clutter_alpha_new_full (priv->timeline,
					alpha_sine_inc_func,
					NULL, NULL);

  priv->behave = clutter_behaviour_slider_new (self, 0, 0);

  priv->effect_template 
    = clutter_effect_template_new (clutter_timeline_new (20, 60),
				   CLUTTER_ALPHA_SINE_INC);

  g_signal_connect (self,
		    "show",
		    G_CALLBACK (show_handler),
		    NULL);

  /* FIXME: should be props */

  priv->font_color = g_new0(ClutterColor, 1);
  clutter_color_parse ("#ccccccff", priv->font_color);
}

ClutterActor*
wh_slider_menu_new (const gchar *font)
{
  ClutterActor         *menu;
  WHSliderMenuPrivate  *priv;

  menu = g_object_new (WH_TYPE_SLIDER_MENU, NULL);
  priv = WH_SLIDER_MENU_GET_PRIVATE (menu);
  priv->font = g_strdup(font);

  return menu;
}

void
wh_slider_menu_add_option (WHSliderMenu            *menu, 
			   const gchar             *text,
			   WHSliderMenuSelectedFunc selected,
			   gpointer                 userdata)
{
  WHSliderMenuPrivate  *priv = WH_SLIDER_MENU_GET_PRIVATE (menu);
  WHSliderMenuEntry    *entry;
  ClutterActor         *actor;
  gint                  pad;

  actor = clutter_label_new_with_text (priv->font, text);
  clutter_label_set_color (CLUTTER_LABEL(actor), priv->font_color);
  clutter_label_set_line_wrap (CLUTTER_LABEL(actor), FALSE);

  entry = g_new0(WHSliderMenuEntry, 1);
  entry->actor = actor;
  entry->selected_func = selected;
  entry->userdata = userdata;

  if (clutter_actor_get_height(actor) > menu->priv->entry_height)
      menu->priv->entry_height = clutter_actor_get_height(actor);

  pad = clutter_actor_get_width (priv->next) * 2;

  entry->offset = priv->unclipped_width + pad;

  if (menu->priv->entrys == NULL)
    priv->unclipped_width += pad;

  priv->unclipped_width += clutter_actor_get_width(actor) + pad;

  if (menu->priv->entrys == 0)
    {
      clutter_actor_set_opacity (actor, 0xff);
      g_object_set (menu->priv->next, 
		    "x", clutter_actor_get_width(actor) + pad +
		            clutter_actor_get_width(menu->priv->prev)
		         + clutter_actor_get_width(menu->priv->prev)/2,
		    "y", clutter_actor_get_height(menu->priv->next),
		    NULL);
    }
  else
    {
      clutter_actor_set_opacity (actor, 0x33);
      clutter_actor_set_scale (actor, 0.7, 0.7); 
    }

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (menu));

  menu->priv->entrys = g_list_append (menu->priv->entrys, entry);

  clutter_actor_set_position (actor, 
			      entry->offset, 
			      menu->priv->entry_height/12);

  menu->priv->n_entrys++;

  clutter_actor_show (actor);
}

void
wh_slider_menu_activate (WHSliderMenu *menu,
			 gint               entry_num)
{
  WHSliderMenuEntry *selected, *current;
  
  if (entry_num < 0 || entry_num >= menu->priv->n_entrys)
    return;

  if (clutter_timeline_is_playing(menu->priv->timeline))
    return;

  current 
    = (WHSliderMenuEntry *)g_list_nth_data(menu->priv->entrys, 
					   menu->priv->active_entry_num);

  selected = (WHSliderMenuEntry *)g_list_nth_data(menu->priv->entrys, 
						  entry_num);

  menu->priv->active_entry_num = entry_num;

  WH_BEHAVIOUR_SLIDER(menu->priv->behave)->old = current;
  WH_BEHAVIOUR_SLIDER(menu->priv->behave)->new = selected;

  g_object_set (menu->priv->next, 
		"x", clutter_actor_get_width(selected->actor) +
		         clutter_actor_get_width(menu->priv->prev) +
		         clutter_actor_get_width(menu->priv->prev) / 2,
		"y", clutter_actor_get_height(menu->priv->next),
		NULL);

  g_object_set (menu->priv->prev, 
		"x", - clutter_actor_get_width(menu->priv->prev)/2,
		"y", clutter_actor_get_height(menu->priv->next),
		NULL);


  /* FIXME: Should be a signal */
  if (selected->selected_func)
    selected->selected_func(menu, selected->actor, selected->userdata);

  clutter_timeline_start (menu->priv->timeline);    
}

void
wh_slider_menu_advance (WHSliderMenu *menu, gint n)
{
  wh_slider_menu_activate (menu, menu->priv->active_entry_num + n);
}

/* Custom behaviour */

static void
clutter_behaviour_alpha_notify (ClutterBehaviour *behave,
                                guint32           alpha_value)
{
  WHBehaviourSlider *slide = WH_BEHAVIOUR_SLIDER(behave);
  WHSliderMenu      *menu;
  gdouble            scale;

  menu = slide->menu;

  menu->priv->offset = slide->old->offset + 
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

  clutter_actor_set_scale (slide->old->actor,
			   1.0 - scale,
			   1.0 - scale);


  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(menu)))
    clutter_actor_queue_redraw (CLUTTER_ACTOR(menu));
}

static void
clutter_behaviour_slider_class_init (WHBehaviourSliderClass *klass)
{
  ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

  behave_class->alpha_notify = clutter_behaviour_alpha_notify;
}

static void
clutter_behaviour_slider_init (WHBehaviourSlider *self)
{
  ;
}

static ClutterBehaviour*
clutter_behaviour_slider_new (WHSliderMenu      *menu,
			      WHSliderMenuEntry *old,
			      WHSliderMenuEntry *new)
{
  WHBehaviourSlider *slide_behave;

  slide_behave = g_object_new (WH_TYPE_BEHAVIOUR_SLIDER, 
			       "alpha", menu->priv->alpha,
			       NULL);

  slide_behave->old   = old;
  slide_behave->new   = new;
  slide_behave->menu  = menu;

  return CLUTTER_BEHAVIOUR(slide_behave);
}
