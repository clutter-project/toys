/*
 * Copyright (C) 2007 OpenedHand Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Neil Jagdish Patel <njp@o-hand.com>
 */


#include "astro-appicon.h"

#include <clutter/clutter-shader.h>
#include <libastro-desktop/astro-defines.h>

G_DEFINE_TYPE (AstroAppicon, astro_appicon, CLUTTER_TYPE_GROUP);

#define ASTRO_APPICON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_APPICON, AstroAppiconPrivate))
	
struct _AstroAppiconPrivate
{
  AstroApplication *application;
  ClutterActor     *texture;
  ClutterShader     *shader;
};

enum
{
  CLICKED,

  LAST_SIGNAL
};
static guint _appicon_signals[LAST_SIGNAL] = { 0 };

static gchar *source = "uniform float radius ;"
        "uniform sampler2DRect rectTexture;"
        ""
        "void main()"
        "{"
        "    vec4 color = texture2DRect(rectTexture, gl_TexCoord[0].st);"
        "    float u;"
        "    float v;"
        "    int count = 1;"
        "    for (u=-radius;u<radius;u++)"
        "      for (v=-radius;v<radius;v++)"
        "        {"
        "          color += texture2DRect(rectTexture, vec2(gl_TexCoord[0].s + u * 2, gl_TexCoord[0].t +v * 2));"
        "          count ++;"
        "        }"
        ""
        "    gl_FragColor = color / count;"
        "}" ;


/* Callbacks */
static gboolean
on_clicked (ClutterActor *home, ClutterEvent *event, AstroAppicon *appicon)
{
  g_return_val_if_fail (ASTRO_APPICON (appicon), FALSE);
  g_debug ("app button clicked");

  g_signal_emit (appicon, _appicon_signals[CLICKED], 
                 0, appicon->priv->application);
  return FALSE;
}


/* Public Functions */
const gchar  *
astro_appicon_get_title (AstroAppicon     *icon)
{
  g_return_val_if_fail (ASTRO_IS_APPICON (icon), NULL);

  return astro_application_get_title (icon->priv->application);
}
  
static void
astro_appicon_set_application (AstroAppicon *appicon, AstroApplication *app)
{
  AstroAppiconPrivate *priv;
  ClutterShader *shader;
  ClutterActor *texture;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  g_return_if_fail (ASTRO_IS_APPICON (appicon));
  priv = appicon->priv;

  priv->application = app;

  pixbuf = astro_application_get_icon (app);
  if (pixbuf)
    {
      priv->texture = texture = clutter_texture_new_from_pixbuf (pixbuf);
      clutter_container_add_actor (CLUTTER_CONTAINER (appicon), texture);
      
      clutter_actor_set_position (texture, 0, 0);
      clutter_actor_set_reactive (texture, TRUE);

      g_signal_connect (texture, "button-release-event",
                        G_CALLBACK (on_clicked), appicon);

    }
  else
    return;
  /* Set up shader */
  priv->shader = shader = clutter_shader_new ();
  clutter_shader_set_fragment_source (shader, source, -1);
   
  /* We try and bind the source, we'll catch and error if there are issues */
  clutter_shader_bind (shader, &error);
  if (error)
    {
      g_warning ("Unable to init shader: %s", error->message);
      g_error_free (error);
    }
  else
    {
      clutter_actor_set_shader (texture, shader);
      clutter_actor_set_shader_param (texture, "radius", 5.0);
    }

  clutter_actor_show_all (CLUTTER_ACTOR (appicon));
}

void            
astro_appicon_set_blur  (AstroAppicon     *appicon,
                         gfloat            blur)
{
  AstroAppiconPrivate *priv;

  g_return_if_fail (ASTRO_IS_APPICON (appicon));
  priv = appicon->priv;

  clutter_actor_set_shader_param (priv->texture, "radius", blur);
}  

/* GObject stuff */
static void
astro_appicon_class_init (AstroAppiconClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  _appicon_signals[CLICKED] = 
    g_signal_new ("clicked",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AstroAppiconClass, clicked),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, ASTRO_TYPE_APPLICATION);

  g_type_class_add_private (gobject_class, sizeof (AstroAppiconPrivate));
}

static void
astro_appicon_init (AstroAppicon *appicon)
{
  AstroAppiconPrivate *priv;

  priv = appicon->priv = ASTRO_APPICON_GET_PRIVATE (appicon);

  priv->application = NULL;
  priv->texture = NULL;
  priv->shader = NULL;
}

ClutterActor * 
astro_appicon_new (AstroApplication *app)
{
  AstroAppicon *appicon =  g_object_new (ASTRO_TYPE_APPICON,
									                       NULL);
  astro_appicon_set_application (appicon, app);

  return CLUTTER_ACTOR (appicon);
}

