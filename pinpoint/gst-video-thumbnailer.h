/*
 * Bickley - a meta data management framework.
 * Copyright © 2008 - 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __GST_VIDEO_THUMBNAILER_H__
#define __GST_VIDEO_THUMBNAILER_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

GdkPixbuf * gst_video_thumbnailer_get_shot (const gchar *location, GCancellable *cancellable);
#endif
