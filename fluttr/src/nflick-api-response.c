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

#include "nflick-api-response.h"
#include "nflick-api-response-private.h"

GType                           nflick_api_response_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickApiResponseClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_api_response_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickApiResponse), 
                        4, 
                        (GInstanceInitFunc) nflick_api_response_init,
                };
                /* FIXME Make abstract type */
                objecttype = g_type_register_static (G_TYPE_OBJECT, "NFlickApiResponse",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_api_response_class_init (NFlickApiResponseClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_api_response_dispose;
        gobjectclass->finalize = (gpointer) nflick_api_response_finalize;
        gobjectclass->get_property = (gpointer) nflick_api_response_get_property;
        
        g_object_class_install_property (gobjectclass, ARG_ERROR,
                                         g_param_spec_string
                                         ("error", "Error", "Message describing the error",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_SUCCESS,
                                         g_param_spec_boolean 
                                         ("success", "Success", "If the response is succesfull",
                                         TRUE, G_PARAM_READABLE));
 
        g_object_class_install_property (gobjectclass, ARG_PARSE_ERROR,
                                         g_param_spec_boolean 
                                         ("parseerror", "ParseError", "If the error was an xml parsing error",
                                         FALSE, G_PARAM_READABLE));
                                        
        g_object_class_install_property (gobjectclass, ARG_XML,
                                         g_param_spec_string
                                         ("xml", "Xml", "Xml message source",
                                         NULL, G_PARAM_READABLE));

        klass->ParseFunc = NULL;

        ParentClass = g_type_class_ref (G_TYPE_OBJECT);
}

static void                     nflick_api_response_init (NFlickApiResponse *self)
{
        g_return_if_fail (NFLICK_IS_API_RESPONSE (self));

        self->Private = NULL;

        NFlickApiResponsePrivate *priv = g_new0 (NFlickApiResponsePrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickApiResponse *self, NFlickApiResponsePrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_API_RESPONSE (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->Error = NULL;
        private->Xml = NULL;
        private->Success = TRUE;

        return TRUE;
}

NFlickApiResponse*              nflick_api_response_new_from_request (GType type, NFlickApiRequest *request)
{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (request), NULL);

        NFlickApiResponse *self = NULL;

        gchar *buffer = nflick_api_request_take_buffer (request);
        if (buffer == NULL)
                goto Done;

        self = g_object_new (type, NULL);
        if (self == NULL)
                goto Done;
                
        if (self->Private == NULL) {
                g_object_unref (self);
                self = NULL;
                goto Done;
        }

        nflick_api_response_parse (self, buffer);

Done:
        if (buffer != NULL)
                g_free (buffer);

        if (self != NULL)
                return self;
        else
                g_return_val_if_reached (NULL);
}

static void                     private_dispose (NFlickApiResponsePrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->Error != NULL) {
                g_free (private->Error);
                private->Error = NULL;
        }

        if (private->Xml != NULL) {
                g_free (private->Xml);
                private->Xml = NULL;
        }
}

static void                     nflick_api_response_dispose (NFlickApiResponse *self)
{
        g_return_if_fail (NFLICK_IS_API_RESPONSE (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_api_response_finalize (NFlickApiResponse *self)
{
        g_return_if_fail (NFLICK_IS_API_RESPONSE (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

void                            nflick_api_response_set_error (NFlickApiResponse *self, const gchar *error)
{
        g_return_if_fail (NFLICK_IS_API_RESPONSE (self));
        
        if (self->Private->Error != NULL)
                g_free (self->Private->Error);

        self->Private->Error = (error != NULL) ? g_strdup (error) : NULL;
}

void                            nflick_api_response_add_error (NFlickApiResponse *self, const gchar *error)
{
        g_return_if_fail (NFLICK_IS_API_RESPONSE (self));

        if (self->Private->Error == NULL)
                nflick_api_response_set_error (self, error);
        else if (error != NULL) {
                gchar *sum = g_strdup_printf ("%s\n%s", self->Private->Error, error);
                g_free (self->Private->Error);
                self->Private->Error = sum;
        } else
                self->Private->Error = NULL;
}

gboolean                        nflick_api_response_parse (NFlickApiResponse *self, const gchar *xml)
{
        g_return_val_if_fail (NFLICK_IS_API_RESPONSE (self), FALSE);
        g_return_val_if_fail (xml != NULL, FALSE);
        g_return_val_if_fail (self->Private->Xml == NULL, FALSE);
        g_return_val_if_fail (NFLICK_API_RESPONSE_GET_CLASS (self)->ParseFunc != NULL, FALSE);

        self->Private->Xml = g_strdup (xml);

        xmlDoc *doc = NULL;           /* The xml tree element */
        xmlNode *root_element = NULL; /* Root element to start parsing */
        gboolean result = TRUE;       /* If we were sucesfull */
        gboolean parse_error = FALSE; /* If the error was a parsing error */
        gchar *stat = NULL;           /* Response stat */

        /* Start here */
        doc = xmlReadMemory (xml, strlen (xml), NULL, NULL, 0); 
        if (doc == NULL) {
                nflick_api_response_add_error (self, gettext ("Couldn't create the xml tree."));
                result = FALSE;
                parse_error = TRUE;
                goto Done;
        }

        root_element = xmlDocGetRootElement(doc);
        if (root_element == NULL) {
                nflick_api_response_add_error (self, gettext ("Couldn't get xml root element."));
                result = FALSE;
                parse_error = TRUE;
                goto Done;
        }

        if (root_element->type != XML_ELEMENT_NODE || 
            strcmp (root_element->name, "rsp") != 0) {
                nflick_api_response_add_error (self, gettext ("Rsp xml root expected, but was not found."));
                parse_error = TRUE;
                result = FALSE;
                goto Done;
        }

        stat = xmlGetProp (root_element, "stat");
        if (stat == NULL) {
                nflick_api_response_add_error (self, gettext ("Response has not stat property."));
                parse_error = TRUE;
                result = FALSE;
                goto Done;
        }
        
        if (strcmp (stat, "ok") == 0) 
                result = TRUE;
        else if (strcmp (stat, "fail") == 0) 
                result = FALSE;
        else {
                nflick_api_response_add_error (self, gettext ("Unknown response."));
                parse_error = TRUE;
                result = FALSE;
                goto Done;
        }

        if (root_element->children == NULL)
                goto Done;

        xmlNode *cur_node = NULL;

        /* Do the main parsing */
        for (cur_node = root_element->children; cur_node; cur_node = cur_node->next) {
                if (cur_node->type == XML_ELEMENT_NODE && strcmp (cur_node->name, "err") == 0) {
                        gchar *err = xmlGetProp (cur_node, "msg");
                        result = FALSE;
                        if (err != NULL) { 
                                nflick_api_response_set_error (self, err);
                                g_free (err);
                        }
                }
        }

        if (result == FALSE)
                goto Done;

        /* Forward to our parse func */
        NFLICK_API_RESPONSE_GET_CLASS (self)->ParseFunc (self, doc, root_element->children, &result, &parse_error);

Done:
        /* Free */
        if (doc != NULL) 
                xmlFreeDoc (doc);

        if (stat != NULL)
                g_free (stat);

        if (result == FALSE && self->Private->Error == NULL)
                nflick_api_response_set_error (self, gettext ("Failed to parse xml tree. Unknown error"));

        if (result == FALSE && parse_error == TRUE)
                g_warning ("Failed to parse xml tree. Error: %s", self->Private->Error);
        
        self->Private->Success = result;
        self->Private->ParseError = parse_error;
        return result;
}

static void                     nflick_api_response_get_property (NFlickApiResponse *self, guint propid, 
                                                                  GValue *value, GParamSpec *pspec)

{
        g_return_if_fail (NFLICK_IS_API_RESPONSE (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                case ARG_ERROR:
                        g_value_set_string (value, self->Private->Error);
                break;
        
                case ARG_PARSE_ERROR:
                        g_value_set_boolean (value, self->Private->ParseError);
                break;
 
                case ARG_SUCCESS:
                        g_value_set_boolean (value, self->Private->Success);
                break;
       
                case ARG_XML:
                        g_value_set_string (value, self->Private->Xml);
                break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
