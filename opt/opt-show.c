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
  ClutterActor    *bullet_texture;
  GdkPixbuf       *background;

  ClutterActor    *position_label;
  ClutterActor    *position_rect;
  guint            position_label_visible;
  
  ClutterTimeline *transition;
  ClutterActor    *bg;

  gulong           trans_signal_id;

  OptMenu         *menu;
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

  g_object_unref (G_OBJECT (self->priv->menu));
  
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
      priv->background = g_value_get_object (value);
      /* refs */
      clutter_texture_set_pixbuf (CLUTTER_TEXTURE(priv->bg),
                                  priv->background,
                                  NULL);
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
      g_value_set_object (value, priv->background);
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
     g_param_spec_object ("background",
			  "Pixbuf source for default show background.",
			  "Pixbuf source for default show background.",
			  GDK_TYPE_PIXBUF, G_PARAM_READWRITE));
}

static void
opt_show_init (OptShow *self)
{
  OptShowPrivate *priv;

  priv           = g_new0 (OptShowPrivate, 1);

  self->priv  = priv;
}

OptShow*
opt_show_new (void)
{
  OptShow       *show;
  ClutterColor col = { 0, 0, 0, 0xff };

  show = g_object_new (OPT_TYPE_SHOW, NULL);

  show->priv->bullet_texture 
    = clutter_label_new_with_text (show->priv->bullet_font, "•");
  clutter_label_set_color (CLUTTER_LABEL(show->priv->bullet_texture), &col);

  show->priv->bg = g_object_new (CLUTTER_TYPE_TEXTURE, NULL);

  show->priv->menu = opt_menu_new (show);
  g_object_ref (G_OBJECT (show->priv->menu));
  
  return show;
}

/* bullet hacks, needs redoing */
ClutterActor*
opt_show_bullet_clone (OptShow *show)
{
  return clutter_label_new_with_text (show->priv->bullet_font, "•");
}

void
opt_show_set_bullet_color (OptShow *show, ClutterColor *col)
{
  clutter_label_set_color (CLUTTER_LABEL(show->priv->bullet_texture), col);
}

void
opt_show_add_slide (OptShow *self, OptSlide *slide)
{
  ClutterActor   *bg, *stage;

  self->priv->slides = g_list_append(self->priv->slides, slide);
  self->priv->num_slides++;

  stage = clutter_stage_get_default();

  bg = CLUTTER_ACTOR(opt_slide_get_background_texture (slide));

  if (bg == NULL)
    bg = clutter_clone_texture_new(CLUTTER_TEXTURE(self->priv->bg));

  clutter_actor_set_size (bg, 
                          clutter_actor_get_width (stage),
                          clutter_actor_get_height (stage));
  
  clutter_group_add (CLUTTER_GROUP(slide), bg);
  
  clutter_actor_lower_bottom(bg);
  clutter_actor_show(bg);

  opt_menu_add_slide (self->priv->menu, slide);
}

void
opt_show_run (OptShow *self)
{
  OptSlide       *slide;
  OptShowPrivate *priv;
  ClutterActor *stage;
  ClutterColor    col = { 0x22, 0x22, 0x22, 0xff };

  priv = self->priv;
  priv->current_slide_num = 0;

  slide = g_list_nth_data (priv->slides, 0);
  stage = clutter_stage_get_default();

  clutter_stage_set_color (CLUTTER_STAGE(stage), &col);
  clutter_group_add (CLUTTER_GROUP(stage), CLUTTER_ACTOR(slide));
  clutter_actor_show_all (stage);

  clutter_main();
}

static void
opt_show_update_position_label (OptShow *show)
{
  OptShowPrivate *priv = show->priv;
  ClutterActor *stage;
  ClutterGeometry stage_geom;
  ClutterGeometry rect_geom;
  gint label_width, label_height;
  gchar *pos;
  
  if (!priv->position_label)
    return;

  stage = clutter_stage_get_default ();
  clutter_actor_get_geometry (stage, &stage_geom);
  
  pos = g_strdup_printf ("%d/%d",
		         priv->current_slide_num + 1,
			 priv->num_slides);

  clutter_label_set_text (CLUTTER_LABEL (priv->position_label), pos);
  clutter_texture_get_base_size (CLUTTER_TEXTURE (priv->position_label),
		                 &label_width,
				 &label_height);

  rect_geom.width = label_width + 50;
  rect_geom.height = label_height + 20;
  rect_geom.x = (stage_geom.width / 2) - (rect_geom.width / 2);
  rect_geom.y = stage_geom.height - rect_geom.height - 10;
  
  clutter_actor_set_geometry (priv->position_rect, &rect_geom);
  clutter_actor_set_position (priv->position_label,
		              rect_geom.x + 25,
			      rect_geom.y + 10);
  
  g_free (pos);
}

static void
transition_completed_cb (OptTransition   *trans,
			 gpointer         data)
{
  OptShow        *show = (OptShow *)data;
  OptSlide       *from;
  OptShowPrivate *priv;
  ClutterActor *stage;

  priv = show->priv;

  from = opt_transition_get_from (trans);
  stage = clutter_stage_get_default();

  /* Remove as to free up resources. */

  clutter_actor_hide_all (CLUTTER_ACTOR(from));
  clutter_container_remove_actor (CLUTTER_CONTAINER(stage),
                                  CLUTTER_ACTOR(from));


  /* Reset any tranforms to be safe */
  clutter_actor_rotate_x (CLUTTER_ACTOR(from), 0, 0, 0);
  clutter_actor_rotate_y (CLUTTER_ACTOR(from), 0, 0, 0);
  clutter_actor_rotate_z (CLUTTER_ACTOR(from), 0, 0, 0);

  /* If needed, update the position */
  if (priv->position_label_visible)
    opt_show_update_position_label (show);
  
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
  ClutterActor *stage;

  priv = self->priv;

  /* transition already running */
  if (priv->trans_signal_id != 0)
    return;

  stage = clutter_stage_get_default();

  from = g_list_nth_data (priv->slides, priv->current_slide_num);
  to   = g_list_nth_data (priv->slides, priv->current_slide_num + step);

  if (from == NULL)
    from = priv->slides->data;
  
  /* Nowhere to go */
  if (to == NULL)
    return;

  /* Add next slide to stage */
  clutter_group_add (CLUTTER_GROUP(stage), CLUTTER_ACTOR(to));

  trans = opt_slide_get_transition ( step < 0 ? to : from);

  /* 
   * Make sure any textures are loaded before the transitions is started .
  */
  clutter_container_foreach (CLUTTER_CONTAINER (to), 
                             (ClutterCallback)clutter_actor_realize,
                             NULL);

  if (trans != NULL)
    {
      if (step < 0)
	opt_transition_set_direction (trans, OPT_TRANSITION_BACKWARD);
      else
	opt_transition_set_direction (trans, OPT_TRANSITION_FORWARD);

      /* Set up transition and start it */
      opt_transition_set_to (trans, to);
      opt_transition_set_from (trans, from);

      priv->trans_signal_id 
	= g_signal_connect (trans,
			    "completed",  
			    G_CALLBACK (transition_completed_cb), 
			    self);

      /* lower it out of view */
      clutter_actor_lower_bottom (CLUTTER_ACTOR(to));

      clutter_timeline_start (CLUTTER_TIMELINE(trans));
    }
  else
    {
      /* No transition just hide current slide*/
      clutter_group_remove (CLUTTER_GROUP(stage), CLUTTER_ACTOR(from));
      clutter_actor_hide_all (CLUTTER_ACTOR(from));
    }
  
  /* Advance */
    priv->current_slide_num += step;

  priv->current_slide_num = 
      CLAMP(priv->current_slide_num, 0, priv->num_slides-1);

  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (priv->menu)))
      opt_menu_popdown (priv->menu);
  
  opt_menu_set_current_slide (priv->menu, priv->current_slide_num);
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

void
opt_show_skip (OptShow *self, gint n_slides)
{
  opt_show_step (self, n_slides);
}

gboolean
opt_show_export (OptShow *self, const char *path, GError **error)
{
#define HTML "<html><head><title>Slide %i</title></head>\n"       \
             "<body><p><center><img src=\"%s\"></center></p>\n"   \
             "<p><center><strong>%s%s</strong></center></p>\n"    \
             "</body></html>"

  GList          *slide;
  OptShowPrivate *priv;
  ClutterActor   *stage;
  gint            i = 0;

  priv = self->priv;

  stage = clutter_stage_get_default();

  g_object_set (stage, "offscreen", TRUE, NULL);

  clutter_actor_show_all (stage);

  slide = priv->slides;

  while (slide)
    {
      ClutterActor *e;
      GdkPixbuf      *pixb = NULL;
      gchar           name[32];
      gchar          *filename = NULL;
      gchar           html[2048], html_next[512], html_prev[512];

      e = CLUTTER_ACTOR(slide->data);

      clutter_container_add_actor (CLUTTER_CONTAINER(stage), e);
      clutter_actor_show_all (stage);
      clutter_actor_show_all (e);

      clutter_redraw();
      
      pixb = clutter_stage_snapshot (CLUTTER_STAGE(stage),
				     0,
				     0,
				     clutter_actor_get_width (stage),
				     clutter_actor_get_height (stage));

      if (pixb == NULL)
	{
	  g_warning("Failed to grab pixels from stage");
	  return FALSE;
	}

      g_snprintf(name, 32, "slide-%02i.png", i);

      filename = g_build_filename(path, name, NULL);

      if (!gdk_pixbuf_save (pixb, filename, "png", error, NULL))
	{
	  if (filename) g_free (filename);
	  return FALSE;
	}

      html_next[0] = html_prev[0] = '\0';
      
      if (i > 0)
	snprintf(html_prev, 512, 
		 "<a href=\"slide-%02i.html\">Prev</a> |", i-1);

      if (slide->next)
	snprintf(html_next, 512, 
		 " <a href=\"slide-%02i.html\">Next</a>", i+1);

      g_snprintf(html, 2048, HTML, i, name, html_prev, html_next);
      g_snprintf(name, 32, "slide-%02i.html", i);
      g_free (filename);

      filename = g_build_filename(path, name, NULL);

      g_file_set_contents (filename, html, -1, NULL);

      g_print ("wrote '%s'\n", filename);

      clutter_actor_hide_all (e);
      clutter_group_remove (CLUTTER_GROUP(stage), e);

      if (filename) g_free (filename);
      slide = slide->next;
      i++;
    }

  return TRUE;
}

void
opt_show_toggle_position (OptShow *show)
{
  OptShowPrivate *priv;
  ClutterActor *stage;
  ClutterGeometry stage_geom;
  
  g_return_if_fail (OPT_IS_SHOW (show));

  priv = show->priv;

  stage = clutter_stage_get_default ();
  clutter_actor_get_geometry (stage, &stage_geom);

  if (!priv->position_label)
    {
      ClutterActor *rect;
      ClutterActor *label;
      ClutterColor rect_color = { 0x00, 0x00, 0x00, 0x33 };
      ClutterColor label_color = { 0xff, 0xff, 0xff, 0xee };
      ClutterGeometry rect_geom;

      rect = clutter_rectangle_new ();
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (rect),
		                   &rect_color);

      rect_geom.width = 102;
      rect_geom.height = 77;
      rect_geom.x = stage_geom.width / 2 - rect_geom.width / 2;
      rect_geom.y = stage_geom.height - rect_geom.height - 20;

      clutter_actor_set_geometry (rect, &rect_geom);

      label = clutter_label_new_with_text ("Sans Bold 20", "0/0");
      clutter_label_set_color (CLUTTER_LABEL (label),
		               &label_color);
      clutter_actor_set_position (label, rect_geom.x + 10, rect_geom.y + 10);

      clutter_group_add_many (CLUTTER_GROUP (stage),
		              rect,
			      label,
			      NULL);
      
      priv->position_label = label;
      priv->position_rect = rect;
      priv->position_label_visible = FALSE;
    }

  if (!priv->position_label_visible)
    {
      priv->position_label_visible = TRUE;

      opt_show_update_position_label (show);
      
      clutter_actor_show (priv->position_rect);
      clutter_actor_show (priv->position_label);
    }
  else
    {
      clutter_actor_hide (priv->position_label);
      clutter_actor_hide (priv->position_rect);

      priv->position_label_visible = FALSE;
    }
}

void
opt_show_pop_menu (OptShow *show)
{
    opt_menu_pop (show->priv->menu);
}

