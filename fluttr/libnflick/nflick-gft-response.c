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

#include "nflick-gft-response.h"
#include "nflick-gft-response-private.h"

GType                           nflick_gft_response_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickGftResponseClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_gft_response_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickGftResponse), 
                        4, 
                        (GInstanceInitFunc) nflick_gft_response_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_API_RESPONSE, "NFlickGftResponse",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_gft_response_class_init (NFlickGftResponseClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickApiResponseClass *apiresponseclass = (NFlickApiResponseClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_gft_response_dispose;
        gobjectclass->finalize = (gpointer) nflick_gft_response_finalize;
        gobjectclass->get_property = (gpointer) nflick_gft_response_get_property;
        
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

static void                     nflick_gft_response_init (NFlickGftResponse *self)
{
        g_return_if_fail (NFLICK_IS_GFT_RESPONSE (self));
        self->Private = NULL;

        NFlickGftResponsePrivate *priv = g_new0 (NFlickGftResponsePrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickGftResponse *self, NFlickGftResponsePrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_GFT_RESPONSE (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->UserName = NULL;
        private->FullName = NULL;
        private->Token = NULL;
        private->UserNsid = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickGftResponsePrivate *private)
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


static void                     nflick_gft_response_dispose (NFlickGftResponse *self)
{
        g_return_if_fail (NFLICK_IS_GFT_RESPONSE (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_gft_response_finalize (NFlickGftResponse *self)
{
        g_return_if_fail (NFLICK_IS_GFT_RESPONSE (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static gboolean                 all_fields_valid (NFlickGftResponse *self)
{
        g_return_val_if_fail (NFLICK_IS_GFT_RESPONSE (self), FALSE);

        if (self->Private->UserNsid != NULL && self->Private->Token != NULL)
                return TRUE;
        else
                return FALSE;
}

static void                     fill_blanks (NFlickGftResponse *self)
{
        g_return_if_fail (NFLICK_IS_GFT_RESPONSE (self));

        if (self->Private->UserName == NULL)
                self->Private->UserName = g_strdup (gettext ("anonymous"));
        
        if (self->Private->FullName == NULL)
                self->Private->FullName = g_strdup (gettext ("Anonymous"));
}

static void                     parse_func (NFlickGftResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error)
{
        g_return_if_fail (NFLICK_IS_GFT_RESPONSE (self));
        g_return_if_fail (children != NULL);
        g_return_if_fail (doc != NULL);
        g_return_if_fail (result != NULL && parse_error != NULL);

        xmlNode *cur_node = NULL;

        for (cur_node = children; cur_node; cur_node = cur_node->next) {
      
                if (cur_node->type == XML_ELEMENT_NODE && strcmp (cur_node->name, "auth") == 0) {
                        
                        xmlNode *auth_node = NULL;
                        for (auth_node = cur_node->children; auth_node; auth_node = auth_node->next) {
                                
                                /* <user> */
                                if (auth_node->type == XML_ELEMENT_NODE && strcmp (auth_node->name, "user") == 0) {

                                        /* Nsid */
                                        gchar *nsid = xmlGetProp (auth_node, "nsid");
                                        if (nsid != NULL) {
                                                if (self->Private->UserNsid != NULL)
                                                        g_free (self->Private->UserNsid);
                                                self->Private->UserNsid = nsid;
                                        }

                                        /* UserName */
                                        gchar *username = xmlGetProp (auth_node, "username");
                                        if (username != NULL) {
                                                if (self->Private->UserName != NULL)
                                                        g_free (self->Private->UserName);
                                                self->Private->UserName = username;
                                        }

                                        /* FullName */
                                        gchar *fullname = xmlGetProp (auth_node, "fullname");
                                        if (fullname != NULL) {
                                                if (self->Private->FullName != NULL)
                                                        g_free (self->Private->FullName);
                                                self->Private->FullName = fullname;
                                        }
                                }

                                /* <token> */
                                if (auth_node->type == XML_ELEMENT_NODE && strcmp (auth_node->name, "token") == 0) {
                                        char *token = xmlNodeListGetString (doc, auth_node->xmlChildrenNode, 1);
                                        if (token != NULL) {
                                                if (self->Private->Token != NULL)
                                                        g_free (self->Private->Token);
                                                self->Private->Token = token;
                                        }
                                }               
                        }
                }
        }

        /* Finished */
        if (all_fields_valid (self) == TRUE) {
                fill_blanks (self);
                *result = TRUE;
                *parse_error = FALSE;
        } else {
                *result = FALSE;
                *parse_error = TRUE;
                nflick_api_response_add_error ((NFlickApiResponse *) self, 
                                               gettext ("Some of the required info is missing from the response!"));
        }
}

static void                     nflick_gft_response_get_property (NFlickGftResponse *self, guint propid, 
                                                                  GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_GFT_RESPONSE (self));
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
