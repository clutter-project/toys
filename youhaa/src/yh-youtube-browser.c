
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "yh-youtube-browser.h"
#include "yh-youtube.h"
#include "yh-theme.h"

#include <clutter/clutter.h>
#include <clutter-gst/clutter-gst.h>
#include <string.h>

enum
{
  PROP_0,
  
  PROP_MODEL,
  PROP_YOUTUBE,
};

struct _YHYoutubeBrowserPrivate {
  ClutterModel     *model;
  YHYoutube        *youtube;
  ClutterModelIter *iter;
  
  ClutterActor     *group;
  
  ClutterActor     *frame;
  ClutterActor     *prev;
  ClutterActor     *next;
  ClutterActor     *thumb;
  ClutterActor     *title;
  ClutterActor     *author;
  ClutterActor     *rating;
  ClutterActor     *description;
  
  GList            *thumb_handles;
  GList            *thumbs;
  GList            *current_thumb;
  ClutterTimeline  *timeline_in;
  ClutterTimeline  *timeline_out;
  guint             fade_timeout;
  ClutterEffectTemplate *template;
  gboolean          loading;
};

G_DEFINE_TYPE (YHYoutubeBrowser, yh_youtube_browser, CLUTTER_TYPE_ACTOR)

#define YOUTUBE_BROWSER_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        YH_TYPE_YOUTUBE_BROWSER, \
        YHYoutubeBrowserPrivate))

static void
video_out_complete_cb (ClutterActor *actor, YHYoutubeBrowser *browser)
{
  clutter_actor_destroy (actor);
}

static gboolean
true_function ()
{
  return TRUE;
}

static gboolean
video_bg_button_press_cb (ClutterActor       *actor,
                          ClutterButtonEvent *event,
                          YHYoutubeBrowser   *browser)
{
  YHYoutubeBrowserPrivate *priv = browser->priv;

  clutter_effect_fade (priv->template,
                       actor,
                       0x00,
                       (ClutterEffectCompleteFunc)video_out_complete_cb,
                       browser);
  
  /* Disconnect handler and just block events until we're faded out */
  g_signal_handlers_disconnect_by_func (actor,
                                        video_bg_button_press_cb,
                                        browser);
  g_signal_connect (actor, "button-press-event",
                    G_CALLBACK (true_function), NULL);
  
  return TRUE;
}

static void
video_in_complete_cb (ClutterActor *actor, YHYoutubeBrowser *browser)
{
  g_signal_handlers_disconnect_by_func (actor,
                                        true_function,
                                        browser);
  g_signal_connect (actor, "button-press-event",
                    G_CALLBACK (video_bg_button_press_cb), browser);
}

static void
video_error_cb (ClutterMedia *media, GError *error)
{
  g_warning ("Error from video: %s", error->message);
}

static void
video_buffer_notify_cb (ClutterMedia *media,
                        GParamSpec *pspec,
                        YHYoutubeBrowser *browser)
{
  if ((clutter_media_get_buffer_percent (media) == 100) &&
      (!clutter_media_get_playing (media)))
    clutter_media_set_playing (media, TRUE);
}

static void
link_cb (YHYoutube *youtube, const gchar *url, YHYoutubeBrowser *browser)
{
  ClutterUnit width, height;
  ClutterActor *group, *rect, *player, *player_bg;
  
  YHYoutubeBrowserPrivate *priv = browser->priv;

  priv->loading = FALSE;
  
  clutter_actor_get_sizeu (priv->frame, &width, &height);
  
  group = clutter_group_new ();
  
  rect = clutter_rectangle_new_with_color (&stage_color);
  clutter_actor_set_opacity (rect, 128);
  clutter_actor_set_size (rect, CLUTTER_STAGE_WIDTH(), CLUTTER_STAGE_HEIGHT());
  
  player_bg = clutter_rectangle_new_with_color (&frame_color);
  clutter_actor_set_sizeu (player_bg,
                           (width*4)/5 + UFRAME,
                           (height*4)/5 + UFRAME);
  
  player = clutter_gst_video_texture_new ();
  g_signal_connect (player, "error", G_CALLBACK (video_error_cb), browser);
  g_signal_connect (player, "notify::buffer-percent",
                    G_CALLBACK (video_buffer_notify_cb), browser);

  g_object_set (G_OBJECT (player),
                "sync-size", FALSE,
                "uri", url,
                "playing", FALSE,
                NULL);
  
  clutter_actor_set_sizeu (player,
                           (width*4)/5,
                           (height*4)/5);
  
  clutter_container_add (CLUTTER_CONTAINER (group),
                         rect, player_bg, player, NULL);
  clutter_actor_show_all (group);
  
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->group), group);
  clutter_actor_set_position (group, 0, 0);
  
  clutter_actor_set_anchor_point_from_gravity (player,
                                               CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_anchor_point_from_gravity (player_bg,
                                               CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_positionu (player, width/2, height/2);
  clutter_actor_set_positionu (player_bg, width/2, height/2);
  
  clutter_actor_set_opacity (group, 0x00);
  clutter_effect_fade (priv->template,
                       group,
                       0xFF,
                       (ClutterEffectCompleteFunc)video_in_complete_cb,
                       browser);
  clutter_actor_set_reactive (group, TRUE);
  g_signal_connect (group, "button-press-event",
                    G_CALLBACK (true_function), browser);
}

static gboolean
thumb_button_press_cb (ClutterActor       *actor,
                       ClutterButtonEvent *event,
                       YHYoutubeBrowser   *browser)
{
  gchar **uris;
  
  YHYoutubeBrowserPrivate *priv = browser->priv;

  if (priv->loading)
    return TRUE;

  clutter_model_iter_get (priv->iter, YH_YOUTUBE_COL_URIS, &uris, -1);
  
  if (!uris)
    return TRUE;
  
  if (!uris[0])
    {
      g_strfreev (uris);
      return TRUE;
    }

  /* We assume the http link comes first
   * (which it does, but should probably check)
   */
  yh_youtube_get_http_link (priv->youtube, uris[0]);
  
  g_strfreev (uris);
  priv->loading = TRUE;
  
  return TRUE;
}

static void
in_complete_cb (ClutterActor *actor, YHYoutubeBrowser *browser)
{
  YHYoutubeBrowserPrivate *priv = browser->priv;
  
  priv->timeline_in = NULL;
}

static void
out_complete_cb (ClutterActor *actor, YHYoutubeBrowser *browser)
{
  YHYoutubeBrowserPrivate *priv = browser->priv;
  
  priv->timeline_out = NULL;
}

static gboolean
crossfade_timeout (YHYoutubeBrowser *browser)
{
  YHYoutubeBrowserPrivate *priv = browser->priv;
  
  if (!priv->current_thumb)
    return FALSE;
  
  if (priv->current_thumb->next ||
      (priv->current_thumb != priv->thumbs))
    {
      ClutterActor *in, *out;
      
      out = CLUTTER_ACTOR (priv->current_thumb->data);
      
      priv->current_thumb = priv->current_thumb->next ?
        priv->current_thumb->next : priv->thumbs;
      
      in = CLUTTER_ACTOR (priv->current_thumb->data);
      
      /* Cross-fade effect */
      priv->timeline_out = clutter_effect_fade (priv->template,
                                                out,
                                                0x00,
                                                (ClutterEffectCompleteFunc)
                                                 out_complete_cb,
                                                browser);
      priv->timeline_in = clutter_effect_fade (priv->template,
                                               in,
                                               0xFF,
                                               (ClutterEffectCompleteFunc)
                                                in_complete_cb,
                                               browser);
    }
  
  return TRUE;
}

static void
complete_cb (YHYoutube *youtube, void *handle, YHYoutubeBrowser *browser)
{
  GList *l;
  YHYoutubeBrowserPrivate *priv = browser->priv;
  
  if ((l = g_list_find (priv->thumb_handles, handle)))
    priv->thumb_handles = g_list_delete_link (priv->thumb_handles, l);
}

static void
thumbnail_cb (YHYoutube *youtube, GdkPixbuf *pixbuf, YHYoutubeBrowser *browser)
{
  ClutterActor *thumb;
  YHYoutubeBrowserPrivate *priv = browser->priv;
  
  if (!pixbuf)
    return;
  
  thumb = clutter_texture_new_from_pixbuf (pixbuf);
  if (!thumb)
    return;
  
  priv->thumbs = g_list_append (priv->thumbs, thumb);
  
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->group), thumb);
  clutter_actor_set_positionu (thumb,
                               clutter_actor_get_xu (priv->prev),
                               UBORDER/2);
  clutter_actor_set_sizeu (thumb,
                           clutter_actor_get_widthu (priv->frame)/2 -
                           (UBORDER*3)/2,
                           (clutter_actor_get_heightu (priv->frame)*3)/4 -
                           UBORDER);
  clutter_actor_show (thumb);
  clutter_actor_set_opacity (thumb, 0x00);
  
  clutter_actor_set_reactive (thumb, TRUE);
  g_signal_connect (thumb, "button-press-event",
                    G_CALLBACK (thumb_button_press_cb), browser);

  if (!priv->current_thumb)
    {
      /* Fade in */
      priv->current_thumb = priv->thumbs;
      priv->timeline_in = clutter_effect_fade (priv->template,
                                               thumb,
                                               0xFF,
                                               (ClutterEffectCompleteFunc)
                                                in_complete_cb,
                                               browser);
      priv->fade_timeout = g_timeout_add_seconds (5,
                                                  (GSourceFunc)crossfade_timeout,
                                                  browser);
    }
}

static void
free_thumbs (YHYoutubeBrowser *self)
{
  YHYoutubeBrowserPrivate *priv = self->priv;
  
  if (priv->fade_timeout)
    {
      g_source_remove (priv->fade_timeout);
      priv->fade_timeout = 0;
    }
  
  if (priv->timeline_in)
    {
      clutter_timeline_pause (priv->timeline_in);
      g_object_unref (priv->timeline_in);
      priv->timeline_in = NULL;
    }

  if (priv->timeline_out)
    {
      clutter_timeline_pause (priv->timeline_out);
      g_object_unref (priv->timeline_out);
      priv->timeline_out = NULL;
    }
  
  while (priv->thumb_handles)
    {
      yh_youtube_cancel (priv->youtube, priv->thumb_handles->data);
      priv->thumb_handles = g_list_delete_link (priv->thumb_handles,
                                                priv->thumb_handles);
    }

  while (priv->thumbs)
    {
      clutter_actor_destroy (CLUTTER_ACTOR (priv->thumbs->data));
      priv->thumbs = g_list_delete_link (priv->thumbs, priv->thumbs);
    }
  
  priv->current_thumb = NULL;
}

static void
fill_details (YHYoutubeBrowser *self)
{
  gchar *title, *author, *description, **thumbs;
  ClutterModelIter *next_iter;
  gdouble rating;
  guint row;
  
  YHYoutubeBrowserPrivate *priv = self->priv;
  
  free_thumbs (self);
  if (!priv->iter)
    return;
  
  clutter_model_iter_get (priv->iter,
                          YH_YOUTUBE_COL_TITLE, &title,
                          YH_YOUTUBE_COL_AUTHOR, &author,
                          YH_YOUTUBE_COL_DESCRIPTION, &description,
                          YH_YOUTUBE_COL_RATING, &rating,
                          YH_YOUTUBE_COL_THUMBS, &thumbs,
                          -1);
  
  clutter_label_set_text (CLUTTER_LABEL (priv->title), title);
  clutter_label_set_text (CLUTTER_LABEL (priv->author), author);
  clutter_label_set_text (CLUTTER_LABEL (priv->description), description);
  
  switch ((gint)(rating + 0.5))
    {
    case 1 :
      clutter_label_set_text (CLUTTER_LABEL (priv->rating), "★••••");
      break;
    case 2 :
      clutter_label_set_text (CLUTTER_LABEL (priv->rating), "★★•••");
      break;
    case 3 :
      clutter_label_set_text (CLUTTER_LABEL (priv->rating), "★★★••");
      break;
    case 4 :
      clutter_label_set_text (CLUTTER_LABEL (priv->rating), "★★★★•");
      break;
    case 5 :
      clutter_label_set_text (CLUTTER_LABEL (priv->rating), "★★★★★");
      break;
    default :
      clutter_label_set_text (CLUTTER_LABEL (priv->rating), "No rating");
    }
  
  if (clutter_model_iter_is_first (priv->iter))
    {
      clutter_actor_set_opacity (priv->prev, 128);
      clutter_actor_set_reactive (priv->prev, FALSE);
    }
  else
    {
      clutter_actor_set_opacity (priv->prev, 255);
      clutter_actor_set_reactive (priv->prev, TRUE);
    }

  row = clutter_model_iter_get_row (priv->iter);
  next_iter = clutter_model_get_iter_at_row (priv->model, row + 1);
  if (!next_iter)
    {
      clutter_actor_set_opacity (priv->next, 128);
      clutter_actor_set_reactive (priv->next, FALSE);
    }
  else
    {
      clutter_actor_set_opacity (priv->next, 255);
      clutter_actor_set_reactive (priv->next, TRUE);
      g_object_unref (next_iter);
    }
  
  if (thumbs)
    {
      gint i;
      for (i = 0; thumbs[i]; i++)
        {
          priv->thumb_handles = g_list_append (priv->thumb_handles,
                                        yh_youtube_get_thumb (priv->youtube,
                                                              thumbs[i]));
        }
      g_strfreev (thumbs);
    }
}

static void
yh_youtube_browser_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  YHYoutubeBrowser *self = YH_YOUTUBE_BROWSER (object);
  
  switch (property_id)
    {
    case PROP_MODEL :
      g_value_set_object (value, self->priv->model);
      break;
    
    case PROP_YOUTUBE :
      g_value_set_object (value, self->priv->youtube);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yh_youtube_browser_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  YHYoutubeBrowser *self = YH_YOUTUBE_BROWSER (object);
  
  switch (property_id)
    {
    case PROP_MODEL :
      self->priv->model = CLUTTER_MODEL (g_value_dup_object (value));
      self->priv->iter = clutter_model_get_first_iter (self->priv->model);
      if (self->priv->youtube)
        fill_details (self);
      break;
    
    case PROP_YOUTUBE :
      self->priv->youtube = YH_YOUTUBE (g_value_dup_object (value));
      g_signal_connect (self->priv->youtube, "complete",
                        G_CALLBACK (complete_cb), self);
      g_signal_connect (self->priv->youtube, "thumbnail",
                        G_CALLBACK (thumbnail_cb), self);
      g_signal_connect (self->priv->youtube, "link",
                        G_CALLBACK (link_cb), self);
      if (self->priv->model)
        fill_details (self);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yh_youtube_browser_dispose (GObject *object)
{
  YHYoutubeBrowser *browser = YH_YOUTUBE_BROWSER (object);
  YHYoutubeBrowserPrivate *priv = browser->priv;
  
  free_thumbs (browser);
  
  if (priv->iter)
    {
      g_object_unref (priv->iter);
      priv->iter = NULL;
    }
  
  if (priv->model)
    {
      g_object_unref (priv->model);
      priv->model = NULL;
    }
  
  if (priv->template)
    {
      g_object_unref (priv->template);
      priv->template = NULL;
    }
  
  if (priv->group)
    {
      clutter_actor_unparent (priv->group);
      priv->group = NULL;
    }
  
  if (priv->youtube)
    {
      g_signal_handlers_disconnect_by_func (priv->youtube,
                                            complete_cb,
                                            browser);
      g_signal_handlers_disconnect_by_func (priv->youtube,
                                            thumbnail_cb,
                                            browser);
      g_signal_handlers_disconnect_by_func (priv->youtube,
                                            link_cb,
                                            browser);
      g_object_unref (priv->youtube);
      priv->youtube = NULL;
    }
  
  if (G_OBJECT_CLASS (yh_youtube_browser_parent_class)->dispose)
    G_OBJECT_CLASS (yh_youtube_browser_parent_class)->dispose (object);
}

static void
yh_youtube_browser_paint (ClutterActor *actor)
{
  YHYoutubeBrowserPrivate *priv = YH_YOUTUBE_BROWSER (actor)->priv;
  
  clutter_actor_paint (priv->group);
}

static void
yh_youtube_browser_pick (ClutterActor *actor, const ClutterColor *color)
{
  yh_youtube_browser_paint (actor);
}

static void
yh_youtube_browser_request_coords (ClutterActor *actor, ClutterActorBox *box)
{
  ClutterUnit width, height;
  
  YHYoutubeBrowserPrivate *priv = YH_YOUTUBE_BROWSER (actor)->priv;
  
  width = box->x2 - box->x1;
  height = box->y2 - box->y1;
  
  clutter_actor_set_sizeu (priv->frame, width, height);
  
  clutter_actor_set_widthu (priv->title, width/2 - (UBORDER*3)/2);
  clutter_actor_set_widthu (priv->author, width/2 - (UBORDER*3)/2);
  clutter_actor_set_widthu (priv->rating, width/2 - (UBORDER*3)/2);
  clutter_actor_set_widthu (priv->description,
                           width/2 - (UBORDER*3)/2);
  clutter_actor_set_clipu (priv->description, 0, 0,
                           clutter_actor_get_widthu (priv->description),
                           height - clutter_actor_get_yu (priv->description) -
                           UBORDER/2);
  
  clutter_actor_set_positionu (priv->prev,
                               width/2 + UBORDER/2,
                               (height*3)/4 + UBORDER/2);
  clutter_actor_set_sizeu (priv->prev, width/4 - UBORDER, height/4 - UBORDER);

  clutter_actor_set_positionu (priv->next,
                       clutter_actor_get_xu (priv->prev) +
                       clutter_actor_get_widthu (priv->prev) + UBORDER/2,
                       (height*3)/4 + UBORDER/2);
  clutter_actor_set_sizeu (priv->next, width/4 - UBORDER, height/4 - UBORDER);

  CLUTTER_ACTOR_CLASS (yh_youtube_browser_parent_class)->
    request_coords (actor, box);
}

static void
yh_youtube_browser_class_init (YHYoutubeBrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YHYoutubeBrowserPrivate));

  object_class->get_property = yh_youtube_browser_get_property;
  object_class->set_property = yh_youtube_browser_set_property;
  object_class->dispose = yh_youtube_browser_dispose;
  
  actor_class->paint = yh_youtube_browser_paint;
  actor_class->pick = yh_youtube_browser_pick;
  actor_class->request_coords = yh_youtube_browser_request_coords;

  g_object_class_install_property (object_class,
                                   PROP_MODEL,
                                   g_param_spec_object ("model",
                                                        "Model",
                                                        "ClutterModel",
                                                        CLUTTER_TYPE_MODEL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_YOUTUBE,
                                   g_param_spec_object ("youtube",
                                                        "YHYoutube",
                                                        "Youtube data provider",
                                                        YH_TYPE_YOUTUBE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static gboolean
prev_pressed_cb (ClutterActor     *actor,
                 ClutterEvent     *event,
                 YHYoutubeBrowser *self)
{
  self->priv->iter = clutter_model_iter_prev (self->priv->iter);
  fill_details (self);
  return TRUE;
}

static gboolean
next_pressed_cb (ClutterActor     *actor,
                 ClutterEvent     *event,
                 YHYoutubeBrowser *self)
{
  self->priv->iter = clutter_model_iter_next (self->priv->iter);
  fill_details (self);
  return TRUE;
}

static void
yh_youtube_browser_init (YHYoutubeBrowser *self)
{
  GdkPixbuf *pixbuf;
  
  GError *error = NULL;
  YHYoutubeBrowserPrivate *priv = self->priv = YOUTUBE_BROWSER_PRIVATE (self);
  
  priv->group = clutter_group_new ();
  clutter_actor_set_parent (priv->group, CLUTTER_ACTOR (self));
  
  /* Frame */
  priv->frame = clutter_rectangle_new_with_color (&entry_color);
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (priv->frame), 6);
  clutter_rectangle_set_border_color (CLUTTER_RECTANGLE (priv->frame),
                                      &frame_color);
  
  /* Previous arrow */
  pixbuf = gdk_pixbuf_new_from_file_at_size (PKGDATADIR "/go-previous.svg",
                                             CLUTTER_STAGE_WIDTH () / 5,
                                             CLUTTER_STAGE_HEIGHT () / 4,
                                             &error);
  if (!pixbuf)
    {
      g_warning ("Error loading pixbuf: %s", error->message);
      g_error_free (error);
      error = NULL;
    }
  
  priv->prev = clutter_texture_new_from_pixbuf (pixbuf);
  g_signal_connect (priv->prev, "button-press-event",
                    G_CALLBACK (prev_pressed_cb), self);
  
  /* Next arrow */
  pixbuf = gdk_pixbuf_new_from_file_at_size (PKGDATADIR "/go-next.svg",
                                             CLUTTER_STAGE_WIDTH () / 5,
                                             CLUTTER_STAGE_HEIGHT () / 4,
                                             &error);
  if (!pixbuf)
    {
      g_warning ("Error loading pixbuf: %s", error->message);
      g_error_free (error);
    }
  
  priv->next = clutter_texture_new_from_pixbuf (pixbuf);
  g_signal_connect (priv->next, "button-press-event",
                    G_CALLBACK (next_pressed_cb), self);
  
  /* Title */
  priv->title = clutter_label_new_full (font, "", &text_color);
  clutter_label_set_ellipsize (CLUTTER_LABEL (priv->title),
                               PANGO_ELLIPSIZE_END);
  
  /* Author */
  priv->author = clutter_label_new_full (small_font, "", &text_color);
  clutter_label_set_ellipsize (CLUTTER_LABEL (priv->author),
                               PANGO_ELLIPSIZE_END);

  /* Rating */
  priv->rating = clutter_label_new_full (small_font, "", &text_color);
  clutter_label_set_ellipsize (CLUTTER_LABEL (priv->rating),
                               PANGO_ELLIPSIZE_END);

  /* Description */
  priv->description = clutter_label_new_full (small_font, "", &text_color);
  clutter_label_set_line_wrap (CLUTTER_LABEL (priv->description), TRUE);
  
  /* Add widgets to group, they'll be sized (mostly) by request-coords */
  clutter_container_add (CLUTTER_CONTAINER (priv->group),
                         priv->frame,
                         priv->prev,
                         priv->next,
                         priv->title,
                         priv->author,
                         priv->rating,
                         priv->description,
                         NULL);
  clutter_actor_set_position (priv->title, BORDER, BORDER/2);
  clutter_actor_set_position (priv->author,
                              BORDER,
                              clutter_actor_get_y (priv->title) +
                              clutter_actor_get_height (priv->title) +
                              BORDER/2);
  clutter_actor_set_position (priv->rating,
                              BORDER,
                              clutter_actor_get_y (priv->author) +
                              clutter_actor_get_height (priv->author) +
                              BORDER/2);
  clutter_actor_set_position (priv->description,
                              BORDER,
                              clutter_actor_get_y (priv->rating) +
                              clutter_actor_get_height (priv->rating) +
                              BORDER/2);
  
  clutter_actor_show_all (priv->group);
  
  /* Create template for cycling preview image cross-fades */
  priv->template = clutter_effect_template_new (
                   clutter_timeline_new_for_duration (750),
                   CLUTTER_ALPHA_RAMP_INC);
}

ClutterActor *
yh_youtube_browser_new (ClutterModel *model, YHYoutube *youtube)
{
  return CLUTTER_ACTOR (g_object_new (YH_TYPE_YOUTUBE_BROWSER,
                                      "model", model,
                                      "youtube", youtube, NULL));
}

