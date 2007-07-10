/******************************************************************************/
/*                                                                            */
/* GPL license, Copyright (c) 2005-2006 by:                                   */
/*                                                                            */
/* Authors:                                                                   */
/*      Michael Dominic K. <michaldominik@gmail.com>                          */
/*                                                                            */
/* This program is free software; you can redistribute it and/or modify it    */
/* under the terms of the GNU General Public License as published by the      */
/* Free Software Foundation; either version 2, or (at your option) any later  */
/* version.                                                                   */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   */
/* for more details.                                                          */
/*                                                                            */
/* You should have received a copy of the GNU General Public License along    */
/* with this program; if not, write to the Free Software Foundation, Inc., 59 */
/* Temple Place - Suite 330, Boston, MA 02111-1307, USA.                      */
/*                                                                            */
/******************************************************************************/

#include "nflick-info-response.h"
#include "nflick-info-response-private.h"

GType                           
nflick_info_response_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickInfoResponseClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_info_response_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickInfoResponse), 
                        4, 
                        (GInstanceInitFunc) nflick_info_response_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_API_RESPONSE, "NFlickInfoResponse",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     
nflick_info_response_class_init (NFlickInfoResponseClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickApiResponseClass *apiresponseclass = (NFlickApiResponseClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_info_response_dispose;
        gobjectclass->finalize = (gpointer) nflick_info_response_finalize;
        gobjectclass->get_property = (gpointer) nflick_info_response_get_property;
        
        g_object_class_install_property (gobjectclass, ARG_TOKEN,
                                         g_param_spec_string
                                         ("token", "Token", "Unique flick full token",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_USER_NAME,
                                         g_param_spec_string
                                         ("username", "UserName", "Flickr user name",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_FULL_NAME,
                                         g_param_spec_string
                                         ("fullname", "FullName", "Flickr full user name",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_USER_NSID,
                                         g_param_spec_string
                                         ("usernsid", "UserNsid", "Unique nsid identyfying user in flickr",
                                         NULL, G_PARAM_READABLE));

        apiresponseclass->ParseFunc = (gpointer) parse_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_API_RESPONSE);
}

static void                     
nflick_info_response_init (NFlickInfoResponse *self)
{
        g_return_if_fail (NFLICK_IS_INFO_RESPONSE (self));
        self->Private = NULL;

        NFlickInfoResponsePrivate *priv = g_new0 (NFlickInfoResponsePrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 
private_init (NFlickInfoResponse *self, NFlickInfoResponsePrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_INFO_RESPONSE (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->UserName = NULL;
        private->FullName = NULL;
        private->Token = NULL;
        private->UserNsid = NULL;

        return TRUE;
}

static void                     
private_dispose (NFlickInfoResponsePrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->UserName != NULL) {
                g_free (private->UserName);
                private->UserName = NULL;
        }

        if (private->FullName != NULL) {
                g_free (private->FullName);
                private->FullName = NULL;
        }

        if (private->Token != NULL) {
                g_free (private->Token);
                private->Token = NULL;
        }

        if (private->UserNsid != NULL) {
                g_free (private->UserNsid);
                private->UserNsid = NULL;
        }
}


static void                     
nflick_info_response_dispose (NFlickInfoResponse *self)
{
        g_return_if_fail (NFLICK_IS_INFO_RESPONSE (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     
nflick_info_response_finalize (NFlickInfoResponse *self)
{
        g_return_if_fail (NFLICK_IS_INFO_RESPONSE (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

void
nflick_info_response_get (NFlickInfoResponse *self,
                          gchar **rotation,
                          gchar **realname,
                          gchar **desc)
{
  g_return_if_fail (NFLICK_IS_INFO_RESPONSE (self));

  *rotation = self->Private->rotation;
  *realname = self->Private->realname;
  *desc = self->Private->desc;
}


static gboolean                 
all_fields_valid (NFlickInfoResponse *self)
{
        g_return_val_if_fail (NFLICK_IS_INFO_RESPONSE (self), FALSE);

        if (self->Private->UserNsid != NULL && self->Private->Token != NULL)
                return TRUE;
        else
                return FALSE;
}

static void                     
fill_blanks (NFlickInfoResponse *self)
{
        g_return_if_fail (NFLICK_IS_INFO_RESPONSE (self));

        if (self->Private->UserName == NULL)
                self->Private->UserName = g_strdup (gettext ("anonymous"));
        
        if (self->Private->FullName == NULL)
                self->Private->FullName = g_strdup (gettext ("Anonymous"));
}

static void                     
parse_func (NFlickInfoResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error)
{
  g_return_if_fail (NFLICK_IS_INFO_RESPONSE (self));
  g_return_if_fail (children != NULL);
  g_return_if_fail (doc != NULL);
  g_return_if_fail (result != NULL && parse_error != NULL);

  xmlNode *cur_node = NULL;

  for (cur_node = children; cur_node; cur_node = cur_node->next) 
  {
    if (cur_node->type == XML_ELEMENT_NODE 
      && strcmp (cur_node->name, "photo") == 0) 
    {
      xmlNode *auth_node = NULL;
      self->Private->rotation = xmlGetProp (cur_node, "rotation");
            
      for (auth_node = cur_node->children; auth_node; auth_node = auth_node->next) 
      {
        if (auth_node->type == XML_ELEMENT_NODE 
              && strcmp (auth_node->name, "owner") == 0) 
        {
          /* Nsid */
          self->Private->realname = xmlGetProp (auth_node, "realname");
        }

        /* <token> */
        if (auth_node->type == XML_ELEMENT_NODE 
          && strcmp (auth_node->name, "description") == 0) 
        {
          self->Private->desc=xmlNodeListGetString (doc, auth_node->xmlChildrenNode, 1);
        }                
      }
    }
  }
}
static void                     
nflick_info_response_get_property (NFlickInfoResponse *self, guint propid, 
                                                                  GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_INFO_RESPONSE (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                case ARG_USER_NAME:
                        g_value_set_string (value, self->Private->UserName);
                break;
        
                case ARG_FULL_NAME:
                        g_value_set_string (value, self->Private->FullName);
                break;
 
                case ARG_TOKEN:
                        g_value_set_string (value, self->Private->Token);
                break;
       
                case ARG_USER_NSID:
                        g_value_set_string (value, self->Private->UserNsid);
                break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
