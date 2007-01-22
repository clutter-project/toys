#include "clutter-slider-menu.h"
#include "util.h"

#define CLUTTER_TYPE_BEHAVIOUR_SLIDER (clutter_behaviour_slider_get_type ())

#define CLUTTER_BEHAVIOUR_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_BEHAVIOUR_SLIDER, ClutterBehaviourSlider))

#define CLUTTER_BEHAVIOUR_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_BEHAVIOUR_SLIDER, ClutterBehaviourSliderClass))

#define CLUTTER_IS_BEHAVIOUR_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_BEHAVIOUR_SLIDER))

#define CLUTTER_IS_BEHAVIOUR_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_BEHAVIOUR_SLIDER))

#define CLUTTER_BEHAVIOUR_SLIDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_BEHAVIOUR_SLIDER, ClutterBehaviourSliderClass))

typedef struct _ClutterBehaviourSlider        ClutterBehaviourSlider;
typedef struct _ClutterBehaviourSliderClass   ClutterBehaviourSliderClass;
 
struct _ClutterBehaviourSlider
{
  ClutterBehaviour             parent;
  gint                         start;
  gint                         end;
  ClutterSliderMenu           *menu;
};

struct _ClutterBehaviourSliderClass
{
  ClutterBehaviourClass   parent_class;
};

GType clutter_behaviour_slider_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (ClutterBehaviourSlider, clutter_behaviour_slider, CLUTTER_TYPE_BEHAVIOUR);

static ClutterBehaviour*
clutter_behaviour_slider_new (ClutterSliderMenu *menu,
			      guint8             offset_start,
			      guint8             offset_end);


G_DEFINE_TYPE (ClutterSliderMenu, clutter_slider_menu, CLUTTER_TYPE_ACTOR);

enum
{
  PROP_0,
};

#define CLUTTER_SLIDER_MENU_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_SLIDER_MENU, ClutterSliderMenuPrivate))

typedef struct ClutterSliderMenuEntry
{
  ClutterActor                   *actor;
  ClutterSliderMenuSelectedFunc   selected_func;
  gpointer                        userdata;
  gint                            offset;
}
ClutterSliderMenuEntry;

struct _ClutterSliderMenuPrivate
{
  GList           *entrys;
  gint             entry_width;
  gint             entry_height;
  gint             n_entrys;
  gint             active_entry_num;

  gint             offset;

  ClutterTimeline  *timeline;
  ClutterAlpha     *alpha;
  ClutterBehaviour *behave;

};

static void
clutter_slider_menu_set_property (GObject      *object, 
				  guint         prop_id,
				  const GValue *value, 
				  GParamSpec   *pspec)
{
  ClutterSliderMenu        *disk;
  ClutterSliderMenuPrivate *priv;

  disk = CLUTTER_SLIDER_MENU(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
clutter_slider_menu_get_property (GObject    *object, 
				  guint       prop_id,
				  GValue     *value, 
				  GParamSpec *pspec)
{
  ClutterSliderMenu        *disk;
  ClutterSliderMenuPrivate *priv;

  disk = CLUTTER_SLIDER_MENU(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}

static void
clutter_slider_menu_request_coords (ClutterActor    *self,
				    ClutterActorBox *box)
{
  ClutterActorBox cbox;

  clutter_actor_allocate_coords (self, &cbox);

  /* Only positioning works.*/
  box->x2 = box->x1 + (cbox.x2 - cbox.x1);
  box->y2 = box->y1 + (cbox.y2 - cbox.y1);
}

static void
clutter_slider_menu_allocate_coords (ClutterActor    *self,
				     ClutterActorBox *box)
{
  ClutterSliderMenuPrivate *priv;

  priv = CLUTTER_SLIDER_MENU(self)->priv;

  box->x2 = box->x1 + priv->entry_width;
  box->y2 = box->y1 + priv->entry_height;
}

static void
clutter_slider_menu_paint (ClutterActor *actor)
{
  ClutterSliderMenu *menu = CLUTTER_SLIDER_MENU(actor);
  GList             *entry;

  glPushMatrix();

  glTranslatef((float)-1.0 * menu->priv->offset, 
	       0.0 /* (float)clutter_actor_get_y(actor), */,
	       0.0);

  for (entry = menu->priv->entrys;
       entry != NULL;
       entry = entry->next)
    {
      ClutterSliderMenuEntry *data = (ClutterSliderMenuEntry*)entry->data;
      ClutterActor *child = data->actor;

      g_assert (child != NULL);

      if (CLUTTER_ACTOR_IS_MAPPED (child))
	  clutter_actor_paint (child);
    }

  glPopMatrix();
}

static void
clutter_slider_menu_realize (ClutterActor *self)
{
  ClutterSliderMenu        *menu;

  menu = CLUTTER_SLIDER_MENU(self);
}

static void 
clutter_slider_menu_dispose (GObject *object)
{
  ClutterSliderMenu         *self = CLUTTER_SLIDER_MENU(object);
  ClutterSliderMenuPrivate  *priv;  

  priv = self->priv;
  
  G_OBJECT_CLASS (clutter_slider_menu_parent_class)->dispose (object);
}

static void 
clutter_slider_menu_finalize (GObject *object)
{
  G_OBJECT_CLASS (clutter_slider_menu_parent_class)->finalize (object);
}

static void
clutter_slider_menu_class_init (ClutterSliderMenuClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
  ClutterActorClass   *parent_class; 

  parent_class = CLUTTER_ACTOR_CLASS (clutter_slider_menu_parent_class);

  actor_class->paint      = clutter_slider_menu_paint;
  actor_class->realize    = clutter_slider_menu_realize;
  actor_class->request_coords  = clutter_slider_menu_request_coords;
  actor_class->allocate_coords = clutter_slider_menu_allocate_coords;

  actor_class->unrealize  = parent_class->unrealize;
  actor_class->show       = parent_class->show;
  actor_class->hide       = parent_class->hide;

  gobject_class->finalize     = clutter_slider_menu_finalize;
  gobject_class->dispose      = clutter_slider_menu_dispose;
  gobject_class->set_property = clutter_slider_menu_set_property;
  gobject_class->get_property = clutter_slider_menu_get_property;

  g_type_class_add_private (gobject_class, sizeof (ClutterSliderMenuPrivate));
}

static void
clutter_slider_menu_init (ClutterSliderMenu *self)
{
  ClutterSliderMenuPrivate *priv;

  self->priv = priv = CLUTTER_SLIDER_MENU_GET_PRIVATE (self);

  priv->timeline = clutter_timeline_new (20, 120);

  priv->alpha = clutter_alpha_new_full (priv->timeline,
					alpha_sine_inc_func,
					NULL, NULL);

  priv->behave = clutter_behaviour_slider_new (self, 0, 0);
}

/**
 * clutter_slider_menu_new
 *
 * Creates a new #ClutterSliderMenu displaying @text using @font_name.
 *
 * Return value: a #ClutterSliderMenu
 */
ClutterActor*
clutter_slider_menu_new (void)
{
  ClutterActor              *menu;
  ClutterSliderMenuPrivate  *priv;

  menu = g_object_new (CLUTTER_TYPE_SLIDER_MENU, NULL);
  priv = CLUTTER_SLIDER_MENU_GET_PRIVATE (menu);

  return menu;
}

void
clutter_slider_menu_append_actor (ClutterSliderMenu            *menu, 
				  ClutterActor                 *actor,
				  ClutterSliderMenuSelectedFunc selected,
				  gpointer                      userdata)
{
  ClutterSliderMenuEntry *entry;
  gboolean                update = FALSE;

  entry = g_new0(ClutterSliderMenuEntry, 1);
  entry->actor = actor;
  entry->selected_func = selected;
  entry->userdata = userdata;

  if (clutter_actor_get_width(actor) > menu->priv->entry_width)
    {
      menu->priv->entry_width = clutter_actor_get_width(actor);
      update = TRUE;
    }

  if (clutter_actor_get_height(actor) > menu->priv->entry_height)
    {
      menu->priv->entry_height = clutter_actor_get_height(actor);
      update = TRUE;
    }
  
  if (update)
    {
      gint   i = 0;
      GList *entry;

      for (entry = menu->priv->entrys;
	   entry != NULL;
	   entry = entry->next)
	{
	  ClutterSliderMenuEntry *data = (ClutterSliderMenuEntry*)entry->data;
	  ClutterActor *child = data->actor;
	  gint xoff = 0, yoff = 0;

	  xoff = (menu->priv->entry_width - clutter_actor_get_width(child))/2;
	  yoff = (menu->priv->entry_height - clutter_actor_get_height(child))/2;

	  clutter_actor_set_position (child,
				      (i * menu->priv->entry_width) + xoff,
				      yoff);
	  i++;
	}

      clutter_actor_set_clip(CLUTTER_ACTOR(menu), 0, 0, 
			     menu->priv->entry_width, 
			     menu->priv->entry_height);

    }

  menu->priv->entrys = g_list_append (menu->priv->entrys, entry);

  clutter_actor_set_position (actor, 
			      (menu->priv->n_entrys * menu->priv->entry_width) 
			        + ((menu->priv->entry_width 
				    - clutter_actor_get_width(actor))/2), 
			      (menu->priv->entry_height 
			            - clutter_actor_get_height(actor))/2);

  menu->priv->n_entrys++;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (menu));
  clutter_actor_show (actor);
}

void
clutter_slider_menu_activate (ClutterSliderMenu *menu,
			      gint               entry_num)
{
  ClutterSliderMenuEntry *selected;
  
  if (entry_num < 0 || entry_num >= menu->priv->n_entrys)
    return;

  if (!clutter_timeline_is_playing(menu->priv->timeline))
    clutter_timeline_start (menu->priv->timeline);    

  CLUTTER_BEHAVIOUR_SLIDER(menu->priv->behave)->start = menu->priv->offset;
  CLUTTER_BEHAVIOUR_SLIDER(menu->priv->behave)->end   = entry_num * menu->priv->entry_width;

  menu->priv->active_entry_num = entry_num;

  selected = (ClutterSliderMenuEntry *)g_list_nth_data(menu->priv->entrys, 
						       entry_num);
  /* FIXME: Should be a signal */
  if (selected->selected_func)
    selected->selected_func(menu, selected->actor, selected->userdata);
}

void
clutter_slider_menu_advance (ClutterSliderMenu *menu, gint n)
{
  clutter_slider_menu_activate (menu, menu->priv->active_entry_num + n);
}

/* Custom behaviour */


static void
clutter_behaviour_alpha_notify (ClutterBehaviour *behave,
                                guint32           alpha_value)
{
  ClutterBehaviourSlider *slide = CLUTTER_BEHAVIOUR_SLIDER(behave);
  ClutterSliderMenu      *menu;

  menu = slide->menu;

  menu->priv->offset = slide->start + 
                      (((gint)alpha_value * (slide->end - slide->start))
			       / CLUTTER_ALPHA_MAX_ALPHA);
  /*
  printf("%i -> %i (%i) %i %i\n", slide->start, slide->end, 
	 (slide->end - slide->start),
	 alpha_value, menu->priv->offset);
  */

  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(menu)))
    clutter_actor_queue_redraw (CLUTTER_ACTOR(menu));
}

static void
clutter_behaviour_slider_class_init (ClutterBehaviourSliderClass *klass)
{
  ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

  behave_class->alpha_notify = clutter_behaviour_alpha_notify;
}

static void
clutter_behaviour_slider_init (ClutterBehaviourSlider *self)
{
  ;
}

static ClutterBehaviour*
clutter_behaviour_slider_new (ClutterSliderMenu *menu,
			      guint8             offset_start,
			      guint8             offset_end)
{
  ClutterBehaviourSlider *slide_behave;

  slide_behave = g_object_new (CLUTTER_TYPE_BEHAVIOUR_SLIDER, 
			       "alpha", menu->priv->alpha,
			       NULL);

  slide_behave->start = offset_start;
  slide_behave->end   = offset_end;
  slide_behave->menu  = menu;

  return CLUTTER_BEHAVIOUR(slide_behave);
}
