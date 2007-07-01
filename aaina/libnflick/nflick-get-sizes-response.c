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

#include "nflick-get-sizes-response.h"
#include "nflick-get-sizes-response-private.h"

GType                           nflick_get_sizes_response_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickGetSizesResponseClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_get_sizes_response_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickGetSizesResponse), 
                        4, 
                        (GInstanceInitFunc) nflick_get_sizes_response_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_API_RESPONSE, "NFlickGetSizesResponse",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_get_sizes_response_class_init (NFlickGetSizesResponseClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickApiResponseClass *apiresponseclass = (NFlickApiResponseClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_get_sizes_response_dispose;
        gobjectclass->finalize = (gpointer) nflick_get_sizes_response_finalize;

        apiresponseclass->ParseFunc = (gpointer) parse_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_API_RESPONSE);
}

static void                     nflick_get_sizes_response_init (NFlickGetSizesResponse *self)
{
        g_return_if_fail (NFLICK_IS_GET_SIZES_RESPONSE (self));
        self->Private = NULL;

        NFlickGetSizesResponsePrivate *priv = g_new0 (NFlickGetSizesResponsePrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickGetSizesResponse *self, NFlickGetSizesResponsePrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_GET_SIZES_RESPONSE (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->SizesList = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickGetSizesResponsePrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->SizesList != NULL) {

                GList *iterator;
        
                for (iterator = private->SizesList; iterator; iterator = g_list_next (iterator)) {
                        SizeData *data = (SizeData *) iterator->data;
                        if (data != NULL) {
                                if (data->Uri != NULL)
                                        g_free (data->Uri);

                                g_free (data);
                        }
                }

                g_list_free (private->SizesList);
                private->SizesList = NULL;
        }
}

static void                     nflick_get_sizes_response_dispose (NFlickGetSizesResponse *self)
{
        g_return_if_fail (NFLICK_IS_GET_SIZES_RESPONSE (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_get_sizes_response_finalize (NFlickGetSizesResponse *self)
{
        g_return_if_fail (NFLICK_IS_GET_SIZES_RESPONSE (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static void                     parse_func (NFlickGetSizesResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error)
{
        g_return_if_fail (NFLICK_IS_GET_SIZES_RESPONSE (self));
        g_return_if_fail (children != NULL);
        g_return_if_fail (doc != NULL);
        g_return_if_fail (result != NULL && parse_error != NULL);

        xmlNode *cur_node = NULL;

        for (cur_node = children; cur_node; cur_node = cur_node->next) {
      
                if (cur_node->type == XML_ELEMENT_NODE && strcmp (cur_node->name, "sizes") == 0) {

                        xmlNode *sizes_node = NULL;
                        for (sizes_node = cur_node->children; sizes_node; sizes_node = sizes_node->next) {
                                
                                if (sizes_node->type == XML_ELEMENT_NODE && strcmp (sizes_node->name, "size") == 0) {

                                        gint32 width_val = -1;
                                        gint32 height_val = -1;
                                        gchar *width = xmlGetProp (sizes_node, "width");
                                        gchar *height = xmlGetProp (sizes_node, "height");
                                        gchar *source = xmlGetProp (sizes_node, "source");

                                        if (width != NULL)
                                                width_val = atoi (width);

                                        if (height != NULL)
                                                height_val = atoi (height);

                                        if (width != NULL && height != NULL && source != NULL && 
                                            width_val > 0 && height_val > 0) {
                                                SizeData *data = g_new0 (SizeData, 1);
                                                data->Uri = g_strdup (source);
                                                data->Width = width_val;
                                                data->Height = height_val;
                                                self->Private->SizesList = g_list_append (self->Private->SizesList, data);
                                        }

                                        if (width != NULL)
                                                g_free (width);
                                        if (height != NULL)
                                                g_free (height);
                                        if (source != NULL)
                                                g_free (source);
                                }
                        }
                }
        }

        /* Finished */
        *result = TRUE;
        *parse_error = FALSE;
}

/* FIXME: Make private */
gint32                          nflick_get_sizes_response_height_for (gint32 width, gint32 height, gint32 fit_width)
{
        g_return_val_if_fail (width > 0, -1);
        g_return_val_if_fail (height > 0, -1);
        g_return_val_if_fail (fit_width > 0, -1);

        gdouble aspect = (gdouble) height / (gdouble) width;
        return aspect * (gdouble) fit_width;
}

/* FIXME: Make private */
gint32                          nflick_get_sizes_response_width_for (gint32 width, gint32 height, gint32 fit_height)
{
        g_return_val_if_fail (width > 0, -1);
        g_return_val_if_fail (height > 0, -1);
        g_return_val_if_fail (fit_height > 0, -1);

        gdouble aspect = (gdouble) width / (gdouble) height;
        return aspect * (gdouble) fit_height;
}

gchar*                          nflick_get_sizes_response_find_match (NFlickGetSizesResponse *self, gint32 *width, gint32 *height, gboolean *rotated)
{
        g_return_val_if_fail (NFLICK_IS_GET_SIZES_RESPONSE (self), NULL);
        g_return_val_if_fail (width != NULL, NULL);
        g_return_val_if_fail (height != NULL, NULL);
        g_return_val_if_fail (rotated != NULL, NULL);
        g_return_val_if_fail (*width > 0, NULL);
        g_return_val_if_fail (*height > 0, NULL);

        GList *iterator;
        gchar *current_source = NULL;
        gint32 current_distance = 10000; /* FIXME: Max int */
        gdouble out_aspect = (gdouble) *height / (gdouble) *width;
        gint32 out_width = -1;
        gint32 out_height = -1;
        gboolean out_rotated = FALSE;
        
        for (iterator = self->Private->SizesList; iterator; iterator = g_list_next (iterator)) {
                SizeData *data = (SizeData *) iterator->data;
                g_assert (data != NULL);

                gdouble in_aspect = (gdouble) data->Height / (gdouble) data->Width;

                gint32 x_distance = 0;
                gint32 y_distance = 0;
                gint32 distance = 0;

                // FIXME: We should analyze the input width and height here!
                if (in_aspect > 1.0) {
                        x_distance = abs (data->Width - *height);
                        y_distance = abs (data->Height - *width);

                        if (data->Width < *height)
                                x_distance *= 2;
                        if (data->Height < *width)
                                y_distance *= 2;

                        distance = x_distance + y_distance;

                        if (distance < current_distance) {
                                current_distance = distance;
                                current_source = data->Uri;
                                out_rotated = TRUE;

                                /* Now let's try doing the fitting */
                                in_aspect = (gdouble) data->Width / (gdouble) data->Height;
                                if (in_aspect > out_aspect) {
                                        out_width = *height;
                                        out_height = nflick_get_sizes_response_height_for (data->Width, data->Height, out_width);
                                } else {
                                        out_height = *width;
                                        out_width= nflick_get_sizes_response_width_for (data->Width, data->Height, out_height);
                                }
                        }
                } else {
                        x_distance = abs (data->Width - *width);
                        y_distance = abs (data->Height - *height);

                        if (data->Width < *width)
                                x_distance *= 2;
                        if (data->Height < *height)
                                y_distance *= 2;

                        distance = x_distance + y_distance;

                        if (distance < current_distance) {
                                current_distance = distance;
                                current_source = data->Uri;
                                out_rotated = FALSE;
                                
                                /* Now let's try doing the fitting */
                                if (in_aspect > out_aspect) {
                                        out_height = *height;
                                        out_width = nflick_get_sizes_response_width_for (data->Width, data->Height, out_height);
                                } else {
                                        out_width = *width;
                                        out_height = nflick_get_sizes_response_height_for (data->Width, data->Height, out_width);
                                }
                        }


                }
        }

        *width = out_width;
        *height = out_height;
        *rotated = out_rotated;

        if (current_source != NULL)
                return g_strdup (current_source);
        else
                return NULL;
}
