#include "opt.h"

G_DEFINE_TYPE (OptShow, opt_show, G_TYPE_OBJECT);

#define TITLE_BORDER_SIZE    8  /* all round */
#define TITLE_BULLET_PAD     5  /* between title and bullet */
#define BULLET_BORDER_SIZE  10  /* sides */
#define BULLET_PAD    5  /* between bullets */

#define TITLE_FONT "VistaSansMed 50"
#define BULLET_FONT "VistaSansMed 40"

struct OptShowPrivate
{
  GList           *slides;
  gint             current_slide_num;
  guint            num_slides;

  gint             title_border_size;
  gint             title_bullet_pad;
  gint             bullet_border_size;
  gint             bullet_pad;
  gchar*           title_font;
  gchar*           bullet_font;
  GdkPixbuf       *background;

  ClutterTimeline *transition;
  ClutterElement  *bg;

  gulong           trans_signal_id;  
};

enum
{
  PROP_0,
  PROP_TITLE_BORDER_SIZE,
  PROP_TITLE_BULLET_PAD,  
  PROP_BULLET_BORDER_SIZE,
  PROP_BULLET_PAD,  
  PROP_TITLE_FONT,
  PROP_BULLET_FONT,
  PROP_BACKGROUND
};


static void 
opt_show_dispose (GObject *object)
{
  OptShow *self = OPT_SHOW(object); 

  if (self->priv)
    {

    }

  G_OBJECT_CLASS (opt_show_parent_class)->dispose (object);
}

static void 
opt_show_finalize (GObject *object)
{
  OptShow *self = OPT_SHOW(object); 

  if (self->priv)
    {
      g_free(self->priv);
      self->priv = NULL;
    }

  G_OBJECT_CLASS (opt_show_parent_class)->finalize (object);
}

static void 
opt_show_set_property (GObject      *object, 
		       guint         prop_id,
		       const GValue *value, 
		       GParamSpec   *pspec)
{

  OptShow *show = OPT_SHOW(object);
  OptShowPrivate *priv;

  priv = show->priv;

  switch (prop_id) 
    {
    case PROP_TITLE_BORDER_SIZE:
      priv->title_border_size = g_value_get_int (value);
      break;
    case PROP_TITLE_BULLET_PAD:
      priv->title_bullet_pad = g_value_get_int (value);
      break;
    case PROP_BULLET_BORDER_SIZE:
      priv->bullet_border_size = g_value_get_int (value);
      break;
    case PROP_BULLET_PAD:
      priv->bullet_pad = g_value_get_int (value);
      break;
    case PROP_TITLE_FONT:
      if (priv->title_font) g_free (priv->title_font);
      priv->title_font = g_value_dup_string (value);
      break;
    case PROP_BULLET_FONT:
      if (priv->bullet_font) g_free (priv->bullet_font);
      priv->bullet_font = g_value_dup_string (value);
      break;
    case PROP_BACKGROUND:
      priv->background = (GdkPixbuf*)g_value_get_pointer(value);
      /* refs */
      clutter_texture_set_pixbuf (CLUTTER_TEXTURE(priv->bg), priv->background);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void 
opt_show_get_property (GObject    *object, 
		       guint       prop_id,
		       GValue     *value, 
		       GParamSpec *pspec)
{
  OptShow *show = OPT_SHOW(object);
  OptShowPrivate *priv;

  priv = show->priv;

  switch (prop_id) 
    {
    case PROP_TITLE_BORDER_SIZE:
      g_value_set_int (value, priv->title_border_size);
      break;
    case PROP_TITLE_BULLET_PAD:
      g_value_set_int (value, priv->title_bullet_pad);
      break;
    case PROP_BULLET_BORDER_SIZE:
      g_value_set_int (value, priv->bullet_border_size);
      break;
    case PROP_BULLET_PAD:
      g_value_set_int (value, priv->bullet_pad);
      break;
    case PROP_TITLE_FONT:
      g_value_set_string (value, priv->title_font);
      break;
    case PROP_BULLET_FONT:
      g_value_set_string (value, priv->bullet_font);
      break;
    case PROP_BACKGROUND:
      g_value_set_pointer (value, priv->background);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
opt_show_class_init (OptShowClass *klass)
{
  GObjectClass        *object_class;

  object_class = (GObjectClass*) klass;

  /* GObject */
  object_class->finalize     = opt_show_finalize;
  object_class->dispose      = opt_show_dispose;
  object_class->set_property = opt_show_set_property;
  object_class->get_property = opt_show_get_property;

  g_object_class_install_property
    (object_class, PROP_TITLE_BORDER_SIZE,
     g_param_spec_int ("title-border-size",
		       "percentage",
		       "percentage",
		       0,
		       100,
		       TITLE_BORDER_SIZE,
		       G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (object_class, PROP_TITLE_BULLET_PAD,
     g_param_spec_int ("title-bullet-pad",
		       "percentage",
		       "percentage",
		       0,
		       100,
		       TITLE_BULLET_PAD,
		       G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (object_class, PROP_BULLET_BORDER_SIZE,
     g_param_spec_int ("bullet-border-size",
		       "percentage",
		       "percentage",
		       0,
		       100,
		       BULLET_BORDER_SIZE,
		       G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (object_class, PROP_BULLET_PAD,
     g_param_spec_int ("bullet-pad",
		       "percentage",
		       "percentage",
		       0,
		       100,
		       BULLET_PAD,
		       G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (object_class, PROP_BULLET_FONT,
     g_param_spec_string ("bullet-font",
			  "bullet font name",
			  "bullet font name",
			  BULLET_FONT,
			  G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (object_class, PROP_TITLE_FONT,
     g_param_spec_string ("title-font",
			  "title font name",
			  "title font name",
			  TITLE_FONT,
			  G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (object_class, PROP_BACKGROUND,
     g_param_spec_pointer ("background",
			   "Pixbuf source for default show background.",
			   "Pixbuf source for default show background.",
			   G_PARAM_READWRITE));
}

static void
opt_show_init (OptShow *self)
{
  OptShowPrivate *priv;

  priv           = g_new0 (OptShowPrivate, 1);

  self->priv  = priv;

  g_object_set (clutter_stage(),
		"fullscreen", TRUE, 
		"hide-cursor", TRUE,
		NULL);

  /* FIXME: should be prop */
#if 0
  priv->bg = clutter_texture_new_from_pixbuf 
              (gdk_pixbuf_new_from_file ("bg.png", NULL));
#endif

  priv->bg = g_object_new (CLUTTER_TYPE_TEXTURE, NULL);
  /* FIXME: construct only prop to set bullet style */
}

OptShow*
opt_show_new (void)
{
  return g_object_new (OPT_TYPE_SHOW, NULL);
}

void
opt_show_add_slide (OptShow *self, OptSlide *slide)
{
  ClutterElement *clone;

  self->priv->slides = g_list_append(self->priv->slides, slide);
  self->priv->num_slides++;

  clone = clutter_clone_texture_new(CLUTTER_TEXTURE(self->priv->bg));

  clutter_element_set_size (clone, 
			    CLUTTER_STAGE_WIDTH(), CLUTTER_STAGE_HEIGHT()); 
  clutter_group_add (CLUTTER_GROUP(slide), clone);

  clutter_element_lower_bottom(clone);
  clutter_element_show(clone);
}

void
opt_show_run (OptShow *self)
{
  OptSlide       *slide;
  OptShowPrivate *priv;

  priv = self->priv;
  priv->current_slide_num = 0;

  slide = g_list_nth_data (priv->slides, 0);

  clutter_stage_set_color (CLUTTER_STAGE(clutter_stage()), 0x222222ff);
  clutter_group_add (clutter_stage(), CLUTTER_ELEMENT(slide));
  clutter_group_show_all(clutter_stage());

  clutter_main();
}

static void
transition_completed_cb (OptTransition   *trans,
			 gpointer         data)
{
  OptShow        *show = (OptShow *)data;
  OptSlide       *from;
  OptShowPrivate *priv;

  priv = show->priv;

  from = opt_transition_get_from (trans);

  /* Remove as to free up resources. */
  clutter_group_remove (clutter_stage(), CLUTTER_ELEMENT(from));
  clutter_group_hide_all (CLUTTER_GROUP(from));

  /* Reset any tranforms to be safe */
  clutter_element_rotate_x (CLUTTER_ELEMENT(from), 0, 0, 0);
  clutter_element_rotate_y (CLUTTER_ELEMENT(from), 0, 0, 0);
  clutter_element_rotate_z (CLUTTER_ELEMENT(from), 0, 0, 0);

  /* Disconnect the handler */
  g_signal_handler_disconnect (trans, priv->trans_signal_id);
  priv->trans_signal_id = 0;
}

void
opt_show_step (OptShow *self, gint step)
{
  OptSlide       *from, *to;
  OptShowPrivate *priv;
  OptTransition  *trans;

  priv = self->priv;

  /* transition already running */
  if (priv->trans_signal_id != 0)
    return;

  from = g_list_nth_data (priv->slides, priv->current_slide_num);
  to   = g_list_nth_data (priv->slides, priv->current_slide_num + step);

  /* Nowhere to go */
  if (to == NULL)
    return;

  /* Add next slide to stage */
  clutter_group_add (clutter_stage(), CLUTTER_ELEMENT(to));

  trans = opt_slide_get_transition (from);

  if (trans != NULL)
    {
      /* Set up transition and start it */
      opt_transition_set_to (trans, to);

      priv->trans_signal_id 
	= g_signal_connect (trans,
			    "completed",  
			    G_CALLBACK (transition_completed_cb), 
			    self);

      /* lower it out of view */
      clutter_element_lower_bottom (CLUTTER_ELEMENT(to));

      clutter_timeline_start (CLUTTER_TIMELINE(trans));
    }
  else
    {
      /* No transition just hide current slide*/
      clutter_group_remove (clutter_stage(), CLUTTER_ELEMENT(from));
      clutter_group_hide_all (CLUTTER_GROUP(from));
    }
  
  /* Advance */
  priv->current_slide_num += step;

  priv->current_slide_num = 
      CLAMP(priv->current_slide_num, 0, priv->num_slides-1);
}

void
opt_show_advance (OptShow *self)
{
  opt_show_step (self, 1);
}

void
opt_show_retreat (OptShow *self)
{
  opt_show_step (self, -1);
}

gboolean
opt_show_export (OptShow *self, const char *path, GError **error)
{
  GList          *slide;
  OptShowPrivate *priv;
  gint            i = 0;

  priv = self->priv;

  g_object_set (clutter_stage(), "offscreen", TRUE, NULL);

  clutter_group_show_all(clutter_stage());

  slide = priv->slides;

  while (slide)
    {
      ClutterElement *e = CLUTTER_ELEMENT(slide->data);
      GdkPixbuf      *pixb = NULL;
      gchar           name[32];
      gchar          *filename = NULL;

      clutter_group_add (clutter_stage(), e);
      clutter_group_show_all(clutter_stage());

      clutter_redraw();
      
      pixb = clutter_stage_snapshot (CLUTTER_STAGE(clutter_stage()),
				     0,
				     0,
				     CLUTTER_STAGE_WIDTH(),
				     CLUTTER_STAGE_HEIGHT());

      g_snprintf(name, 32, "slide-%i.png", i);

      filename = g_build_filename(path, name, NULL);

      if (!gdk_pixbuf_save (pixb, filename, "png", error, NULL))
	{
	  if (filename) g_free (filename);
	  return FALSE;
	}

      g_print ("wrote '%s'\n", filename);

      clutter_group_remove (clutter_stage(), e);
      clutter_group_hide_all (CLUTTER_GROUP(e));

      if (filename) g_free (filename);
      slide = slide->next;
      i++;
    }

  return TRUE;
}
