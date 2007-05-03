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

#include "nflick-photo-data.h"

GType                           nflick_photo_data_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {
                objecttype = g_boxed_type_register_static 
                        ("NFlickPhotoData", 
                         (GBoxedCopyFunc) nflick_photo_data_copy,
                         (GBoxedFreeFunc) nflick_photo_data_free);
        }
        
        return objecttype;
}

NFlickPhotoData*                nflick_photo_data_copy (const NFlickPhotoData *self)
{
        g_return_val_if_fail (self != NULL, NULL);

        NFlickPhotoData *new = g_new (NFlickPhotoData, 1);
        g_return_val_if_fail (new != NULL, NULL);

        new->Id = (self->Id != NULL) ? g_strdup (self->Id) : NULL;
        new->Name = (self->Name != NULL) ? g_strdup (self->Name) : NULL;
 
        return new;
}

void                            nflick_photo_data_free (NFlickPhotoData *self)
{
        if (self == NULL)
                return;
        else {
                if (self->Id != NULL)
                        g_free (self->Id);
                if (self->Name != NULL)
                        g_free (self->Name);
                g_free (self);
        }
}

NFlickPhotoData*                nflick_photo_data_new (const gchar *id, const gchar *name)
{
        NFlickPhotoData *self = g_new (NFlickPhotoData, 1);
        g_return_val_if_fail (self != NULL, NULL);

        self->Id = (id != NULL) ? g_strdup (id) : NULL;
        self->Name = (name != NULL) ? g_strdup (name) : NULL;

        return self;
}
