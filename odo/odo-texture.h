/* odo-texture.h */

#ifndef _ODO_TEXTURE_H
#define _ODO_TEXTURE_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define ODO_TYPE_TEXTURE odo_texture_get_type()

#define ODO_TEXTURE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ODO_TYPE_TEXTURE, OdoTexture))

#define ODO_TEXTURE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  ODO_TYPE_TEXTURE, OdoTextureClass))

#define ODO_IS_TEXTURE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ODO_TYPE_TEXTURE))

#define ODO_IS_TEXTURE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ODO_TYPE_TEXTURE))

#define ODO_TEXTURE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  ODO_TYPE_TEXTURE, OdoTextureClass))

typedef struct _OdoTexture OdoTexture;
typedef struct _OdoTextureClass OdoTextureClass;
typedef struct _OdoTexturePrivate OdoTexturePrivate;

struct _OdoTexture
{
  ClutterActor parent;

  OdoTexturePrivate *priv;
};

struct _OdoTextureClass
{
  ClutterActorClass parent_class;
};

GType odo_texture_get_type (void);

typedef void (*OdoTextureCallback) (OdoTexture        *texture,
                                    CoglTextureVertex *vertex,
                                    gfloat             width,
                                    gfloat             height,
                                    gpointer           user_data);

ClutterActor *odo_texture_new (void);
ClutterActor *odo_texture_new_from_files (const gchar *front_face_filename,
                                          const gchar *back_face_filename);
ClutterActor *odo_texture_new_with_textures (ClutterTexture *front_face,
                                             ClutterTexture *back_face);

void odo_texture_get_resolution (OdoTexture *texture,
                                 gint       *tiles_x,
                                 gint       *tiles_y);

void odo_texture_set_resolution (OdoTexture *texture,
                                 gint        tiles_x,
                                 gint        tiles_y);

void odo_texture_set_callback (OdoTexture         *texture,
                               OdoTextureCallback  callback,
                               gpointer            user_data);

void odo_texture_set_textures (OdoTexture     *texture,
                               ClutterTexture *front_face,
                               ClutterTexture *back_face);

void odo_texture_invalidate (OdoTexture *texture);

G_END_DECLS

#endif /* _ODO_TEXTURE_H */

