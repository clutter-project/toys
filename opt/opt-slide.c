#include "opt-slide.h"

G_DEFINE_TYPE (OptSlide, opt_slide, CLUTTER_TYPE_GROUP);

/* Note: percentages of total stage size */
#define TITLE_BORDER_SIZE    5  /* all round */
#define TITLE_BULLET_PAD     5  /* between title and bullet */
#define BULLET_BORDER_SIZE  10  /* sides */
#define BULLET_BULLET_PAD    5  /* between bullets */

#define TITLE_FONT "Sans Bold 50"
#define BULLET_FONT "Sans Bold 40"

#define PERCENT_TO_PIXELS(p) \
   (( (p) * CLUTTER_STAGE_WIDTH() ) / 100)

struct OptSlidePrivate
{
  ClutterElement *title;
  ClutterElement *bg;
  GList          *bullets;
};

static void 
opt_slide_dispose (GObject *object)
{
  OptSlide *self = OPT_SLIDE(object); 

  if (self->priv)
    {

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

  /*
  element_class->request_coords  = opt_slide_request_coords;
  element_class->allocate_coords = opt_slide_allocate_coords;
  */

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

  /* FIXME: set this via API call. show would usually set
  priv->bg = clutter_rectangle_new (0x999999FF);
  clutter_element_set_size (priv->bg, 
			    CLUTTER_STAGE_WIDTH(), 
			    CLUTTER_STAGE_HEIGHT());

  clutter_group_add (CLUTTER_GROUP(self), priv->bg);
  clutter_element_show(priv->bg);
  */
}

OptSlide*
opt_slide_new (void)
{
  return g_object_new (OPT_TYPE_SLIDE, NULL);
}

void
opt_slide_set_title (OptSlide *slide, const gchar *title)
{
  OptSlidePrivate *priv;
  gint             avail_w, border;

  priv = slide->priv;

  if (priv->title == NULL)
    {
      priv->title = clutter_label_new_with_text (TITLE_FONT, title);
      clutter_group_add (CLUTTER_GROUP(slide), priv->title);
    }
  else
    clutter_label_set_text (CLUTTER_LABEL(priv->title), title);

  border = PERCENT_TO_PIXELS (TITLE_BORDER_SIZE);

  avail_w = CLUTTER_STAGE_WIDTH() - (2 * border) ; 

  clutter_label_set_text_extents (CLUTTER_LABEL(priv->title),
				  avail_w,
				  0);

  clutter_element_set_position (priv->title, border, border);

  clutter_element_show(priv->title);
}

void
get_next_bullet_offsets (OptSlide *slide,
			 gint     *x,
			 gint     *y,
			 gint    *max_width)
{
  OptSlidePrivate *priv;
  GList           *last_bullet_item;

  priv = slide->priv;

  if ((last_bullet_item = g_list_last (priv->bullets)) == NULL)
    {
      *y = clutter_element_get_y (priv->title)
	+ clutter_element_get_height (priv->title);

      *y += PERCENT_TO_PIXELS (TITLE_BULLET_PAD);
    }
  else
    {
      ClutterElement *last_bullet = CLUTTER_ELEMENT(last_bullet_item->data);

      *y = clutter_element_get_y (last_bullet)
	+ clutter_element_get_height (last_bullet);

      *y += PERCENT_TO_PIXELS (BULLET_BULLET_PAD);
    }

  *x = PERCENT_TO_PIXELS (BULLET_BORDER_SIZE);

  *max_width = CLUTTER_STAGE_WIDTH() 
                  - (2 * PERCENT_TO_PIXELS (BULLET_BORDER_SIZE)) ; 
}

void
opt_slide_add_bullet_text_item (OptSlide *slide, const gchar *title)
{
  OptSlidePrivate *priv;
  ClutterElement  *bullet;
  gint             x, y, width;
  gchar           *buf;

  priv = slide->priv;

  buf = g_strdup_printf ("• %s", title);

  bullet = clutter_label_new_with_text (BULLET_FONT, buf);

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

