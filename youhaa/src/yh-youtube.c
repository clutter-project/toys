
#include "yh-youtube.h"

#include <stdlib.h>
#include <string.h>
#include <clutter/clutter.h>
#include "glibcurl.h"

typedef enum {
  TYPE_QUERY,
  TYPE_THUMB,
  TYPE_LINK,
} YHYoutubeRequestType;

typedef struct {
  gchar *url;
  gchar *data;
  gint size;
  YHYoutubeRequestType type;
} YHYoutubeRequest;

enum
{
  COMPLETE,
  MODEL,
  THUMBNAIL,
  LINK,

  LAST_SIGNAL
};

#define YOUTUBE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YH_TYPE_YOUTUBE, YHYoutubePrivate))

struct _YHYoutubePrivate {
  JsonParser *parser;
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (YHYoutube, yh_youtube, G_TYPE_OBJECT)

static void yh_youtube_curl_close (void *userp);

static void
yh_youtube_get_property (GObject *object, guint property_id,
                         GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
yh_youtube_set_property (GObject *object, guint property_id,
                         const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
yh_youtube_dispose (GObject *object)
{
  YHYoutubePrivate *priv = YOUTUBE_PRIVATE (object);
  
  if (priv->parser)
    {
      g_object_unref (priv->parser);
      priv->parser = NULL;
    }
  
  if (G_OBJECT_CLASS (yh_youtube_parent_class)->dispose)
    G_OBJECT_CLASS (yh_youtube_parent_class)->dispose (object);
}

static void
yh_youtube_finalize (GObject *object)

{
  G_OBJECT_CLASS (yh_youtube_parent_class)->finalize (object);
}

static void
yh_youtube_class_init (YHYoutubeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YHYoutubePrivate));

  object_class->get_property = yh_youtube_get_property;
  object_class->set_property = yh_youtube_set_property;
  object_class->dispose = yh_youtube_dispose;
  object_class->finalize = yh_youtube_finalize;

  signals[COMPLETE] =
    g_signal_new ("complete",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YHYoutubeClass, complete),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);

  signals[MODEL] =
    g_signal_new ("model",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YHYoutubeClass, model),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, G_TYPE_OBJECT);

  signals[THUMBNAIL] =
    g_signal_new ("thumbnail",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YHYoutubeClass, thumbnail),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, G_TYPE_OBJECT);

  signals[LINK] =
    g_signal_new ("link",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YHYoutubeClass, link),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
yh_youtube_init (YHYoutube *self)
{
  static gboolean first_call = TRUE;
  
  YHYoutubePrivate *priv = self->priv = YOUTUBE_PRIVATE (self);
  
  if (first_call)
    {
	    glibcurl_init ();
	    glibcurl_set_callback (yh_youtube_curl_close, self);
	    
	    first_call = FALSE;
    }
  else
    {
      g_warning ("This class is a singleton, it can only be created once");
      return;
    }
  
  priv->parser = json_parser_new ();
	    
}

static ClutterModel *
yh_youtube_create_model (YHYoutube *youtube)
{
  gint i;
  JsonNode *node;
  JsonArray *array;
  JsonObject *object;
  ClutterModel *model;
  
  if (!(node = json_parser_get_root (youtube->priv->parser)))
    {
      g_warning ("Error retrieving root node");
      return NULL;
    }
  
  if (!(object = json_node_get_object (node)))
    {
      g_warning ("Error retrieving object from root node");
      return NULL;
    }
  
  if (!(node = json_object_get_member (object, "feed")))
    {
      g_warning ("Error retrieving 'feed' member");
      return NULL;
    }
  
  if (!(object = json_node_get_object (node)))
    {
      g_warning ("Error retreiving 'feed' as an object");
      return NULL;
    }
  
  if (!(node = json_object_get_member (object, "entry")))
    {
      /* No error message, no entry means zero results */
      return NULL;
    }
  
  if (!(array = json_node_get_array (node)))
    {
      g_warning ("Error retrieving 'entry' as an array");
      return NULL;
    }

  model = clutter_list_model_new (YH_YOUTUBE_COL_LAST,
                                  G_TYPE_STRING, "Title",
                                  G_TYPE_STRING, "Author",
                                  G_TYPE_STRING, "Description",
                                  G_TYPE_DOUBLE, "Rating",
                                  G_TYPE_STRV, "Thumbnails",
                                  G_TYPE_STRV, "MIME types",
                                  G_TYPE_STRV, "URIs");
  
  for (i = 0; i < json_array_get_length (array); i++)
    {
      ClutterModelIter *iter;
      JsonNode *prop_node;
      JsonArray *prop_array;
      JsonObject *prop_object;
      
      if (!(node = json_array_get_element (array, i)))
        continue;
      
      if (!(object = json_node_get_object (node)))
        continue;

      clutter_model_insert (model, -1,
                            YH_YOUTUBE_COL_TITLE, NULL,
                            YH_YOUTUBE_COL_AUTHOR, NULL,
                            YH_YOUTUBE_COL_DESCRIPTION, NULL,
                            YH_YOUTUBE_COL_RATING, 0.0,
                            YH_YOUTUBE_COL_THUMBS, NULL,
                            YH_YOUTUBE_COL_MIMES, NULL,
                            YH_YOUTUBE_COL_URIS, NULL,
                            -1);
      iter = clutter_model_get_last_iter (model);
      
      /* The 'JSON' that GData returns is really horrible :( */
      
      /* Title */
      if ((prop_node = json_object_get_member (object, "title")))
        if ((prop_object = json_node_get_object (prop_node)))
          if ((prop_node = json_object_get_member (prop_object, "$t")))
            clutter_model_iter_set (iter,
                                    YH_YOUTUBE_COL_TITLE,
                                    json_node_get_string (prop_node),
                                    -1);
      
      /* Author */
      if ((prop_node = json_object_get_member (object, "author")))
        if ((prop_array = json_node_get_array (prop_node)))
          if ((prop_node = json_array_get_element (prop_array, 0)))
            if ((prop_object = json_node_get_object (prop_node)))
              if ((prop_node = json_object_get_member (prop_object, "name")))
                if ((prop_object = json_node_get_object (prop_node)))
                  if ((prop_node = json_object_get_member (prop_object, "$t")))
                    clutter_model_iter_set (iter,
                                            YH_YOUTUBE_COL_AUTHOR,
                                            json_node_get_string (prop_node),
                                            -1);
      
      /* Description */
      if ((prop_node = json_object_get_member (object, "content")))
        if ((prop_object = json_node_get_object (prop_node)))
          if ((prop_node = json_object_get_member (prop_object, "$t")))
            clutter_model_iter_set (iter,
                                    YH_YOUTUBE_COL_DESCRIPTION,
                                    json_node_get_string (prop_node),
                                    -1);
      
      /* Rating */
      if ((prop_node = json_object_get_member (object, "gd$rating")))
        if ((prop_object = json_node_get_object (prop_node)))
          if ((prop_node = json_object_get_member (prop_object, "average")))
            {
              /* FIXME: This is probably insecure? */
              gdouble rating = atof (json_node_get_string (prop_node));
              clutter_model_iter_set (iter,
                                      YH_YOUTUBE_COL_RATING,
                                      rating, -1);
            }
      
      if ((prop_node = json_object_get_member (object, "media$group")))
        if ((prop_object = json_node_get_object (prop_node)))
          {
            JsonObject *media_object;
            JsonNode *media_node;
            gint j;
            
            /* Formats/URIs */
            if ((prop_node = json_object_get_member (prop_object,
                                                     "media$content")))
              if ((prop_array = json_node_get_array (prop_node)))
                {
                  GList *uris = NULL;
                  GList *formats = NULL;
                  
                  for (j = 0; j < json_array_get_length (prop_array); j++)
                    {
                      const gchar *format, *uri;
                      
                      if (!(prop_node = json_array_get_element (prop_array, j)))
                        continue;
                      
                      if (!(media_object = json_node_get_object (prop_node)))
                        continue;
                      
                      if (!(media_node = json_object_get_member (media_object,
                                                                 "type")))
                        continue;
                      
                      if (!(format = json_node_get_string (media_node)))
                        continue;
                      
                      if (!(media_node = json_object_get_member (media_object,
                                                                "url")))
                        continue;
                      
                      if (!(uri = json_node_get_string (media_node)))
                        continue;
                      
                      uris = g_list_append (uris, (gpointer)uri);
                      formats = g_list_append (formats, (gpointer)format);
                    }
                  
                  if (uris)
                    {
                      GList *l;
                      gchar **string_list;
                      
                      string_list = g_new0 (gchar *, g_list_length (uris) + 1);
                      
                      /* Set URI list */
                      for (j = 0, l = uris; l; l = l->next, j++)
                        {
                          string_list[j] = (gchar *)l->data;
                        }
                      clutter_model_iter_set (iter,
                                              YH_YOUTUBE_COL_URIS,
                                              string_list,
                                              -1);

                      /* Set format (MIME type) list */
                      for (j = 0, l = formats; l; l = l->next, j++)
                        {
                          string_list[j] = (gchar *)l->data;
                        }
                      clutter_model_iter_set (iter,
                                              YH_YOUTUBE_COL_MIMES,
                                              string_list,
                                              -1);
                      
                      g_free (string_list);
                      g_list_free (uris);
                      g_list_free (formats);
                    }
                }

            /* Thumbnails */
            if ((prop_node = json_object_get_member (prop_object,
                                                     "media$thumbnail")))
              if ((prop_array = json_node_get_array (prop_node)))
                {
                  GList *urls = NULL;
                  
                  for (j = 0; j < json_array_get_length (prop_array); j++)
                    {
                      const gchar *url;
                      
                      if (!(prop_node = json_array_get_element (prop_array, j)))
                        continue;
                      
                      if (!(media_object = json_node_get_object (prop_node)))
                        continue;
                      
                      if (!(media_node = json_object_get_member (media_object,
                                                                 "url")))
                        continue;
                      
                      if (!(url = json_node_get_string (media_node)))
                        continue;
                      
                      urls = g_list_append (urls, (gpointer)url);
                    }
                  
                  if (urls)
                    {
                      GList *l;
                      gchar **string_list;
                      
                      string_list = g_new0 (gchar *, g_list_length (urls) + 1);
                      
                      /* Set URL list */
                      for (j = 0, l = urls; l; l = l->next, j++)
                        {
                          string_list[j] = (gchar *)l->data;
                        }
                      clutter_model_iter_set (iter,
                                              YH_YOUTUBE_COL_THUMBS,
                                              string_list,
                                              -1);
                      
                      g_free (string_list);
                      g_list_free (urls);
                    }
                }
          }
      
      g_object_unref (iter);
    }
  
  return model;
}

static void
yh_youtube_curl_close (void *userp)
{
  CURLMsg *msg;
  int in_queue;
  CURL *handle;
  YHYoutubeRequest *request;

  YHYoutube *youtube = YH_YOUTUBE (userp);
  YHYoutubePrivate *priv = youtube->priv;
  
  while ((msg = curl_multi_info_read (glibcurl_handle (), &in_queue))) {
    GError *error = NULL;
    
    if (msg->msg != CURLMSG_DONE)
      continue;
    
    handle = msg->easy_handle;
    
    if (curl_easy_getinfo (msg->easy_handle,
                           CURLINFO_PRIVATE,
                           &request) == CURLE_OK)
      {
        switch (request->type)
          {
          case TYPE_QUERY : {
              ClutterModel *model = NULL;
              
              /* Parse the data into the model */
              if (request->data)
                {
                  if (!json_parser_load_from_data (priv->parser,
                                                   request->data,
                                                   request->size,
                                                   &error))
                    {
                      g_warning ("Error parsing JSON: %s", error->message);
                      g_error_free (error);
                    }
                  else
                    model = yh_youtube_create_model (youtube);
                }
              
              g_signal_emit (youtube, signals[MODEL], 0, model);
              if (model)
                g_object_unref (model);
            }
            break;
          case TYPE_THUMB : {
              GdkPixbuf *pixbuf = NULL;
              
              /* Create a GdkPixbuf from the data */
              if (request->data)
                {
                  GdkPixbufLoader *loader;
                  
                  loader = gdk_pixbuf_loader_new ();
                  if (!gdk_pixbuf_loader_write (loader,
                                                (const guchar *)request->data,
                                                request->size,
                                                &error))
                    {
                      g_warning ("Error decoding image: %s", error->message);
                      g_error_free (error);
                    }
                  else
                    {
                      if (!gdk_pixbuf_loader_close (loader, &error))
                        {
                          g_warning ("Error closing pixbuf loader: %s",
                                     error->message);
                          g_error_free (error);
                        }
                      else
                        pixbuf = g_object_ref (
                          gdk_pixbuf_loader_get_pixbuf (loader));
                    }
                    g_object_unref (loader);
                }
              
              g_signal_emit (youtube, signals[THUMBNAIL], 0, pixbuf);
              if (pixbuf)
                g_object_unref (pixbuf);
            }
            break;
          case TYPE_LINK : {
              gchar *url;
              long error_code;
              
              if (curl_easy_getinfo (handle,
                                     CURLINFO_EFFECTIVE_URL,
                                     &url) == CURLE_OK)
                {
                  if (url && (strcmp (request->url, url) != 0))
                    g_free (url);
                }
              
              /* If we can't get the error code for whatever reason, just 
               * assume success.
               */
              if (curl_easy_getinfo (handle,
                                     CURLINFO_RESPONSE_CODE,
                                     &error_code) != CURLE_OK)
                error_code = 200;
              
              /* Recursively solve redirects */
              if ((error_code >= 300) && (error_code < 400))
                yh_youtube_get_http_link (youtube, request->url);
              else
                g_signal_emit (youtube, signals[LINK], 0, request->url);
            }
            break;
          }
        
        g_free (request->data);
        g_free (request->url);
        g_slice_free (YHYoutubeRequest, request);
      }
    else
      g_warning ("Error retrieving user data, something has gone wrong...");
    
    g_signal_emit (youtube, signals[COMPLETE], 0, handle);
    glibcurl_remove (handle);
  }
}

static size_t
yh_youtube_curl_read (void *buffer, size_t size, size_t nmemb, void *userp)
{
  YHYoutubeRequest *request = (YHYoutubeRequest *) userp;
  gint real_size = (gint)(size * nmemb);
  
  if (!request->data) {
      request->data = g_memdup (buffer, real_size);
      request->size = real_size;
    }
  else
    {
      request->data = g_realloc (request->data, request->size + real_size);
      g_memmove (request->data + request->size, buffer, real_size);
      request->size += real_size;
    }
  
  return (size_t)real_size;
}

YHYoutube *
yh_youtube_get_default ()
{
  static YHYoutube *youtube = NULL;
  
  if (!youtube)
    {
      youtube = YH_YOUTUBE (g_object_new (YH_TYPE_YOUTUBE, NULL));
    }

  return youtube;
}

void *
yh_youtube_query (YHYoutube *youtube, const gchar *search_string)
{
  CURL *handle;
  YHYoutubeRequest *request;

  /* Make request to Youtube GData url */
  request = g_slice_new0 (YHYoutubeRequest);
  search_string = curl_escape (search_string, 0);
  request->type = TYPE_QUERY;
  request->url =
    g_strconcat ("http://gdata.youtube.com/feeds/api/videos?alt=json&vq=",
                 search_string, NULL);
  curl_free ((char *)search_string);
  
  /* Don't free url, CURL doesn't make a copy */
  handle = curl_easy_init ();
  curl_easy_setopt (handle, CURLOPT_URL, request->url);
  curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, yh_youtube_curl_read);
  curl_easy_setopt (handle, CURLOPT_WRITEDATA, request);
  curl_easy_setopt (handle, CURLOPT_PRIVATE, request);
  
  glibcurl_add (handle);
  
  return handle;
}

void *
yh_youtube_get_thumb (YHYoutube *youtube, const gchar *url)
{
  CURL *handle;
  YHYoutubeRequest *request;

  /* Download image */
  request = g_slice_new0 (YHYoutubeRequest);
  request->type = TYPE_THUMB;
  request->url = g_strdup (url);
  
  handle = curl_easy_init ();
  curl_easy_setopt (handle, CURLOPT_URL, request->url);
  curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, yh_youtube_curl_read);
  curl_easy_setopt (handle, CURLOPT_WRITEDATA, request);
  curl_easy_setopt (handle, CURLOPT_PRIVATE, request);
  
  glibcurl_add (handle);
  
  return handle;
}

void
yh_youtube_cancel (YHYoutube *youtube, void *handle)
{
  YHYoutubeRequest *request;
  
  CURL *curl_handle = (CURL *)handle;
  
  if (curl_easy_getinfo (curl_handle,
                         CURLINFO_PRIVATE,
                         &request) == CURLE_OK)
    {
      g_free (request->data);
      g_free (request->url);
      g_slice_free (YHYoutubeRequest, request);
    }
  
	glibcurl_remove(curl_handle);
	curl_easy_cleanup (curl_handle);
}

static size_t
yh_youtube_header_cb (void *buffer, size_t size, size_t nmemb, void *userp)
{
  YHYoutubeRequest *request = (YHYoutubeRequest *)userp;
  gint real_size = (gint)(size *nmemb);
  gchar *header = g_strstrip (g_strndup (buffer, real_size));
  
  if (header && strncmp (header, "Location: ", 10) == 0)
    {
      const gchar *video_id;
      const gchar *url = header + 10;
      
      /* Hacky URL mangling */
      if ((video_id = strstr (url, "/swf/l.swf?video_id=")))
        {
          /* NOTE: This URL subject to change */
          request->url = g_strconcat (
            /*"http://www.youtube.com/get_video?video_id="*/
            "http://cache.googlevideo.com/get_video?video_id=",
            video_id + 20, "&origin=youtube.com", NULL);
        }
      else if (url[0] == '/')
        {
          request->url = g_strconcat (
            "http://www.youtube.com",
            url, NULL);
        }
      else
        {
          request->url = g_strdup (url);
        }
      
      /* Set the size to -1 (cancels transfer) -
       * this is the header we were looking for
       */
      real_size = -1;
    }
  g_free (header);

  return real_size;
}

static size_t
yh_youtube_minus_one ()
{
  return -1;
}

/* This is a nasty function required because YouTube uses HTTP 303's
 * to 'redirect' :( (but even then, the location needs mangling)
 */
void *
yh_youtube_get_http_link  (YHYoutube *youtube, const gchar *url)
{
  CURL *handle;
  YHYoutubeRequest *request;
  
  /* Download image */
  request = g_slice_new0 (YHYoutubeRequest);
  request->type = TYPE_LINK;
  request->url = g_strdup (url);
  
  handle = curl_easy_init ();
  curl_easy_setopt (handle, CURLOPT_URL, request->url);
  curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, yh_youtube_minus_one);
  curl_easy_setopt (handle, CURLOPT_WRITEDATA, request);
  curl_easy_setopt (handle, CURLOPT_PRIVATE, request);
	curl_easy_setopt (handle, CURLOPT_HEADERFUNCTION,
                    yh_youtube_header_cb);
  curl_easy_setopt (handle, CURLOPT_HEADERDATA, request);
  
  glibcurl_add (handle);
  
  return handle;
}

