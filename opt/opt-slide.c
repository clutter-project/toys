#include "opt.h"

G_DEFINE_TYPE (OptSlide, opt_slide, CLUTTER_TYPE_GROUP);

/* Note: percentages of total stage size */
#define TITLE_BORDER_SIZE    8  /* all round */
#define TITLE_BULLET_PAD     5  /* between title and bullet */
#define BULLET_BORDER_SIZE  10  /* sides */
#define BULLET_BULLET_PAD    5  /* between bullets */

#define TITLE_FONT "VistaSansMed 50"
#define BULLET_FONT "VistaSansMed 40"

#define PERCENT_TO_PIXELS(p) \
   (( (p) * CLUTTER_STAGE_WIDTH() ) / 100)

struct OptSlidePrivate
{
  ClutterElement *title;
  ClutterElement *bg;
  GList          *bullets;
  OptShow        *show;
  OptTransition  *trans;
};

static void 
opt_slide_dispose (GObject *object)
{
  OptSlide *self = OPT_SLIDE(object); 

  if (self->priv)
    {
      if (self->priv->trans != NULL)
	g_object_unref(self->priv->trans);
      self->priv->trans = NULL;
    }

  G_OBJECT_CLASS (opt_slide_parent_class)->dispose (object);
}


static void 
opt_slide_finalize (GObject *object)
{
  OptSlide *self = OPT_SLIDE(object); 

  if (self->priv)
    {
      g_free(self->priv);
      self->priv = NULL;
    }

  G_OBJECT_CLASS (opt_slide_parent_class)->finalize (object);
}

static void
opt_slide_class_init (OptSlideClass *klass)
{
  GObjectClass        *object_class;
  ClutterElementClass *element_class;

  object_class = (GObjectClass*) klass;
  element_class = (ClutterElementClass*)klass;

  /* GObject */
  object_class->finalize     = opt_slide_finalize;
  object_class->dispose      = opt_slide_dispose;
}

static void
opt_slide_init (OptSlide *self)
{
  OptSlidePrivate *priv;

  priv = g_new0 (OptSlidePrivate, 1);

  self->priv  = priv;
}

OptSlide*
opt_slide_new (OptShow *show)
{
  OptSlide *slide;

  g_return_val_if_fail(OPT_IS_SHOW(show), NULL);

  slide = g_object_new (OPT_TYPE_SLIDE, NULL);

  slide->priv->show = show; 

  return slide;
}

void
opt_slide_set_title (OptSlide     *slide, 
		     const gchar  *title,
		     const gchar  *font,
		     ClutterColor *col)
{
  OptSlidePrivate *priv;
  gint             avail_w, border;
  gint             title_border_size;

  g_return_if_fail(OPT_IS_SLIDE(slide));

  priv = slide->priv;

  if (priv->title != NULL)
    {
      clutter_group_remove (CLUTTER_GROUP(slide), priv->title);
      g_object_unref (priv->title);
    }

  if (font == NULL)
    {
      gchar *default_font = NULL;
      g_object_get (priv->show, "title-font", &default_font, NULL);
      priv->title = clutter_label_new_with_text (default_font, title);
      g_free (default_font);
    }
  else
    priv->title = clutter_label_new_with_text (font, title);

  clutter_group_add (CLUTTER_GROUP(slide), priv->title);

  g_object_get (priv->show, 
		"title-border-size", &title_border_size,
		NULL);

  border = PERCENT_TO_PIXELS (title_border_size);

  avail_w = CLUTTER_STAGE_WIDTH() - (2 * border) ; 

  clutter_label_set_text_extents (CLUTTER_LABEL(priv->title),
				  avail_w,
				  0);

  clutter_label_set_color (CLUTTER_LABEL(priv->title), col);

  clutter_element_set_position (priv->title, border, border);

  clutter_element_show (priv->title);
}

void
get_next_bullet_offsets (OptSlide *slide,
			 gint     *x,
			 gint     *y,
			 gint    *max_width)
{
  OptSlidePrivate *priv;
  GList           *last_bullet_item;
  gint             title_bullet_pad, bullet_border_size, bullet_pad;

  priv = slide->priv;

  g_object_get (priv->show, 
		"title-bullet-pad", &title_bullet_pad,
		"bullet-pad", &bullet_pad,
		"bullet-border-size", &bullet_border_size,
		NULL);

  if ((last_bullet_item = g_list_last (priv->bullets)) == NULL)
    {
      *y = clutter_element_get_y (priv->title)
	+ clutter_element_get_height (priv->title);

      *y += PERCENT_TO_PIXELS (title_bullet_pad);
    }
  else
    {
      ClutterElement *last_bullet = CLUTTER_ELEMENT(last_bullet_item->data);

      *y = clutter_element_get_y (last_bullet)
	+ clutter_element_get_height (last_bullet);

      *y += PERCENT_TO_PIXELS (bullet_pad);
    }

  *x = PERCENT_TO_PIXELS (bullet_border_size);

  *max_width = CLUTTER_STAGE_WIDTH() 
                  - (2 * PERCENT_TO_PIXELS (bullet_border_size)) ; 
}

void
opt_slide_add_bullet_text_item (OptSlide     *slide, 
				const gchar  *title,
				const gchar  *font,
				ClutterColor *col)
{
  OptSlidePrivate *priv;
  ClutterElement  *bullet;
  gint             x, y, width;
  gchar           *buf;

  priv = slide->priv;

  buf = g_strdup_printf ("• %s", title);

  if (font == NULL)
    {
      gchar *default_font = NULL;

      g_object_get (priv->show, "bullet-font", &default_font, NULL);
      bullet = clutter_label_new_with_text (default_font, buf);
      g_free (default_font);
    }
  else
    bullet = clutter_label_new_with_text (font, buf);

  clutter_label_set_color (CLUTTER_LABEL(bullet), col);

  get_next_bullet_offsets (slide, &x, &y, &width);

  priv->bullets = g_list_append(priv->bullets, bullet);

  clutter_label_set_text_extents (CLUTTER_LABEL(bullet),
				  width,
				  0);

  clutter_element_set_position (bullet, x, y);

  clutter_group_add (CLUTTER_GROUP(slide), bullet);

  clutter_element_show(bullet);

  g_free(buf);
}

void
opt_slide_add_bullet (OptSlide *slide, ClutterElement *element)
{
  OptSlidePrivate *priv;
  gint             x, y, width;

  priv = slide->priv;

  get_next_bullet_offsets (slide, &x, &y, &width);

  priv->bullets = g_list_append(priv->bullets, element);

  clutter_group_add (CLUTTER_GROUP(slide), element);

  clutter_element_set_position (element, 
				x + (width -clutter_element_get_width(element))
				                      /2, 
				y);

  clutter_element_show(element);
}

const ClutterElement*
opt_slide_get_title (OptSlide *slide)
{
  return slide->priv->title;
}

GList*
opt_slide_get_bullets (OptSlide *slide)
{
  return slide->priv->bullets;
}

void
opt_slide_set_transition (OptSlide *slide, OptTransition *trans)
{
  OptSlidePrivate *priv;

  priv = slide->priv;

  if (priv->trans == trans)
    return;

  if (priv->trans != NULL)
    g_object_unref(priv->trans);

  if (trans)
    {
      priv->trans = trans;
      g_object_ref(slide);
    }
}

OptTransition*
opt_slide_get_transition (OptSlide *slide)
{
  return slide->priv->trans;
}

