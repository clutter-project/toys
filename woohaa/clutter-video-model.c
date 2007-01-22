#include "clutter-video-model.h"
#include <string.h>

G_DEFINE_TYPE (ClutterVideoModel, clutter_video_model, G_TYPE_OBJECT);

#define VIDEO_MODEL_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CLUTTER_TYPE_VIDEO_MODEL, ClutterVideoModelPrivate))

typedef struct _ClutterVideoModelPrivate ClutterVideoModelPrivate;

struct _ClutterVideoModelPrivate
{
  GList         *items;
  ClutterActor  *default_video_image;
};

static gboolean
model_read_path (ClutterVideoModel *model, 
		 const gchar       *uri);

static void
clutter_video_model_get_property (GObject *object, guint property_id,
				  GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
clutter_video_model_set_property (GObject *object, guint property_id,
				  const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
clutter_video_model_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (clutter_video_model_parent_class)->dispose)
    G_OBJECT_CLASS (clutter_video_model_parent_class)->dispose (object);
}

static void
clutter_video_model_finalize (GObject *object)
{
  G_OBJECT_CLASS (clutter_video_model_parent_class)->finalize (object);
}

static void
clutter_video_model_class_init (ClutterVideoModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ClutterVideoModelPrivate));

  object_class->get_property = clutter_video_model_get_property;
  object_class->set_property = clutter_video_model_set_property;
  object_class->dispose = clutter_video_model_dispose;
  object_class->finalize = clutter_video_model_finalize;
}

static void
clutter_video_model_init (ClutterVideoModel *self)
{
}


typedef enum ClutterVideoModelView
{
  CLUTTER_VIDEO_MODEL_VIEW_NEW = 0,

  N_CLUTTER_VIDEO_MODEL_VIEWS
}
ClutterVideoModelView;

guint
clutter_video_model_item_count (ClutterVideoModel *model)
{
  ClutterVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);  

  return g_list_length (priv->items);
}

ClutterVideoModelItem*
clutter_video_model_get_item (ClutterVideoModel *model, gint index)
{
  ClutterVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);

  return (ClutterVideoModelItem*)g_list_nth_data (priv->items, index);
}

void
clutter_video_model_set_cell_size (ClutterVideoModel *model,
				   gint               width,
				   gint               height)
{
  /* FIXME: if this wasn't a quick hack we'd have some concept
   *        of cell renders or the like....
  */

#define PAD 10

  ClutterVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);
  GList                    *item;
  GdkPixbuf                *pixbuf;
  gchar                     font_desc[32];

  pixbuf = gdk_pixbuf_new_from_file_at_size ("video.png", 
					     height - (height/4), 
					     height - (height/4), 
					     NULL);

  if (priv->default_video_image != NULL)
    g_object_unref (priv->default_video_image);

  priv->default_video_image = clutter_texture_new_from_pixbuf (pixbuf);

  /* FIXME nicer way for sizing fonts */
  g_snprintf(font_desc, 32, "Sans Bold %ipx", height/3); 

  for (item = priv->items;
       item != NULL;
       item = item->next)
    {
      ClutterVideoModelItem *row;

      row = (ClutterVideoModelItem*)item->data;

      if (row->image != NULL)
	g_object_unref (row->image);

      row->image  = clutter_clone_texture_new 
	                 (CLUTTER_TEXTURE(priv->default_video_image));
      
      clutter_group_add (CLUTTER_GROUP(row->group), row->image);
      clutter_actor_set_position (row->image, height/8, height/8);

      g_object_set (row->title, "width", width - height - PAD, NULL);
      clutter_label_set_font_name (CLUTTER_LABEL(row->title), font_desc); 
      clutter_label_set_line_wrap (CLUTTER_LABEL(row->title), FALSE);
      clutter_label_set_ellipsize  (CLUTTER_LABEL(row->title), 
				    PANGO_ELLIPSIZE_END);
      clutter_actor_set_position (row->title, 
				  height + PAD, 
				  height/8);
    }
}


void
clutter_video_model_add_uri (ClutterVideoModel *model,
			     const gchar       *uri,
			     time_t             mtime)
{
  ClutterVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);
  ClutterVideoModelItem    *item;
  ClutterColor              color = { 0xff, 0xff, 0xff, 0xff };
  gchar                    *basename;

  item = g_new0(ClutterVideoModelItem, 1);
  item->uri   = g_strdup(uri);
  item->mtime = mtime;

  item->group = clutter_group_new();
  
  basename = g_path_get_basename (uri);

  item->title = clutter_label_new_with_text ("Sans Bold 24px", basename);
  clutter_label_set_color (CLUTTER_LABEL(item->title), &color);
  clutter_group_add (CLUTTER_GROUP(item->group), item->title);

  priv->items = g_list_append (priv->items, item);

  g_free (basename);
}

void
clutter_video_model_view_foreach (ClutterVideoModel     *model,
				  ClutterVideoModelFunc  func,
				  gpointer               userdata)
{
  ClutterVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);
  GList *item;

  for (item = priv->items;
       item != NULL;
       item = item->next)
    {
      func (model, item->data, userdata);
    }
}

static gboolean
model_add_dir (ClutterVideoModel *model, 
	       const gchar       *uri)
{
  GnomeVFSResult           vfs_result;
  GnomeVFSDirectoryHandle *vfs_handle = NULL;
  GnomeVFSFileInfoOptions  vfs_options;
  GnomeVFSFileInfo        *vfs_info = NULL;
  gboolean                 ret = TRUE;

  vfs_options = 
    GNOME_VFS_FILE_INFO_DEFAULT
    |GNOME_VFS_FILE_INFO_FOLLOW_LINKS
    |GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS;

  vfs_result = gnome_vfs_directory_open (&vfs_handle, uri, vfs_options);

  if (vfs_result != GNOME_VFS_OK)	
    goto cleanup;

  vfs_info = gnome_vfs_file_info_new ();

  while (gnome_vfs_directory_read_next(vfs_handle, vfs_info) == GNOME_VFS_OK)
    {
      if (vfs_info->name
	  && strcmp(vfs_info->name, ".")
	  && strcmp(vfs_info->name, ".."))
	{
	  gchar *entry_uri = NULL;

	  entry_uri = g_strconcat(uri, "/", vfs_info->name, NULL);

	  if (entry_uri)
	    {
	      ret |= model_read_path(model, entry_uri); 
	      g_free(entry_uri);
	    }
	}
    }

 cleanup:
  if (vfs_info)
    gnome_vfs_file_info_unref (vfs_info);
  
  if (vfs_handle)
    gnome_vfs_directory_close (vfs_handle);

 return ret;
}


static gboolean
model_read_path (ClutterVideoModel *model, 
		 const gchar         *uri)
{
  GnomeVFSResult          vfs_result;
  GnomeVFSFileInfo       *vfs_info = NULL;
  GnomeVFSFileInfoOptions vfs_options;
  gboolean                ret = FALSE;

  vfs_options = 
    GNOME_VFS_FILE_INFO_DEFAULT
    |GNOME_VFS_FILE_INFO_FOLLOW_LINKS
    |GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS;

  vfs_info = gnome_vfs_file_info_new ();
    
  vfs_result = gnome_vfs_get_file_info (uri, vfs_info, vfs_options);

  if (vfs_result != GNOME_VFS_OK)	
    goto cleanup;

  if (! (vfs_info->valid_fields & (GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS
				   |GNOME_VFS_FILE_INFO_FIELDS_TYPE)))
    goto cleanup;

  if (! (vfs_info->permissions & GNOME_VFS_PERM_ACCESS_READABLE))
    goto cleanup;


  if (vfs_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY)
    {
      ret = model_add_dir (model, uri);
    }
  else if (vfs_info->type == GNOME_VFS_FILE_TYPE_REGULAR)
    {
      time_t mtime = 0;

      if (vfs_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MTIME)
	mtime = vfs_info->mtime;

      /* FIXME: use gstreamer tag foo to identify */
      if (g_str_has_suffix(uri, ".avi")
	  || g_str_has_suffix(uri, ".mpeg")
	  || g_str_has_suffix(uri, ".wmv")
	  || g_str_has_suffix(uri, ".mpg"))
	{
	  clutter_video_model_add_uri (model, uri, mtime);	  
	}
      
      ret = TRUE;
    }

 cleanup:
 
 if (vfs_info)
    gnome_vfs_file_info_unref (vfs_info);
  
  return ret;
}

ClutterVideoModel*
clutter_video_model_new (const gchar* path)
{
  ClutterVideoModel *model;

  model = g_object_new (CLUTTER_TYPE_VIDEO_MODEL, NULL);

  gnome_vfs_init();

  model_add_dir (model, path);

  return model;
}

