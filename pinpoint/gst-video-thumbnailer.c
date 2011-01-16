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

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include <gio/gio.h>
#include <gst/gst.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gst-video-thumbnailer.h"

static void
push_buffer (GstElement *element,
             GstBuffer  *out_buffer,
             GstPad     *pad,
             GstBuffer  *in_buffer)
{
    gst_buffer_set_caps (out_buffer, GST_BUFFER_CAPS (in_buffer));
    GST_BUFFER_SIZE (out_buffer) = GST_BUFFER_SIZE (in_buffer);
    memcpy (GST_BUFFER_DATA (out_buffer), GST_BUFFER_DATA (in_buffer),
            GST_BUFFER_SIZE (in_buffer));
}

static void
pull_buffer (GstElement *element,
             GstBuffer  *in_buffer,
             GstPad     *pad,
             GstBuffer **out_buffer)
{
    *out_buffer = gst_buffer_ref (in_buffer);
}

GdkPixbuf *
convert_buffer_to_pixbuf (GstBuffer    *buffer,
                          GCancellable *cancellable)
{
    GstCaps *pb_caps;
    GstElement *pipeline;
    GstBuffer *out_buffer = NULL;
    GstElement *src, *sink, *colorspace, *scale, *filter;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn state;
    gboolean ret;
    int width, height, dw, dh, i;
    GstStructure *s;

    s = gst_caps_get_structure (GST_BUFFER_CAPS (buffer), 0);
    gst_structure_get_int (s, "width", &dw);
    gst_structure_get_int (s, "height", &dh);

    pb_caps = gst_caps_new_simple ("video/x-raw-rgb",
                                   "bpp", G_TYPE_INT, 24,
                                   "depth", G_TYPE_INT, 24,
                                   "width", G_TYPE_INT, dw,
                                   "height", G_TYPE_INT, dh,
                                   "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                   NULL);

    pipeline = gst_pipeline_new ("pipeline");

    src = gst_element_factory_make ("fakesrc", "src");
    colorspace = gst_element_factory_make ("ffmpegcolorspace", "colorspace");
    scale = gst_element_factory_make ("videoscale", "scale");
    filter = gst_element_factory_make ("capsfilter", "filter");
    sink = gst_element_factory_make ("fakesink", "sink");

    gst_bin_add_many (GST_BIN (pipeline), src, colorspace, scale,
                      filter, sink, NULL);

    g_object_set (filter,
                  "caps", pb_caps,
                  NULL);
    g_object_set (src,
                  "num-buffers", 1,
                  "sizetype", 2,
                  "sizemax", GST_BUFFER_SIZE (buffer),
                  "signal-handoffs", TRUE,
                  NULL);
    g_signal_connect (src, "handoff",
                      G_CALLBACK (push_buffer), buffer);

    g_object_set (sink,
                  "signal-handoffs", TRUE,
                  "preroll-queue-len", 1,
                  NULL);
    g_signal_connect (sink, "handoff",
                      G_CALLBACK (pull_buffer), &out_buffer);

    ret = gst_element_link (src, colorspace);
    if (ret == FALSE) {
        g_warning ("Failed to link src->colorspace");
        return NULL;
    }

    ret = gst_element_link (colorspace, scale);
    if (ret == FALSE) {
        g_warning ("Failed to link colorspace->scale");
        return NULL;
    }

    ret = gst_element_link (scale, filter);
    if (ret == FALSE) {
        g_warning ("Failed to link scale->filter");
        return NULL;
    }

    ret = gst_element_link (filter, sink);
    if (ret == FALSE) {
        g_warning ("Failed to link filter->sink");
        return NULL;
    }

    bus = gst_element_get_bus (GST_ELEMENT (pipeline));
    state = gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

    i = 0;
    msg = NULL;
    while (msg == NULL && i < 5) {
        msg = gst_bus_timed_pop_filtered (bus, GST_SECOND,
                                          GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
        i++;
    }

    /* FIXME: Notify about error? */
    gst_message_unref (msg);

    gst_caps_unref (pb_caps);

    if (out_buffer) {
        GdkPixbuf *pixbuf;
        char *data;

        data = g_memdup (GST_BUFFER_DATA (out_buffer),
                         GST_BUFFER_SIZE (out_buffer));
        pixbuf = gdk_pixbuf_new_from_data ((guchar *) data,
                                           GDK_COLORSPACE_RGB, FALSE,
                                           8, dw, dh, GST_ROUND_UP_4 (dw * 3),
                                           (GdkPixbufDestroyNotify) g_free,
                                           NULL);

        gst_buffer_unref (buffer);
        return pixbuf;
    }

    /* FIXME: Check what buffers need freed */
    return NULL;
}

GdkPixbuf *
gst_video_thumbnailer_get_shot (const gchar  *location,
          GCancellable *cancellable)
{
    GstElement *playbin, *audio_sink, *video_sink;
    GstStateChangeReturn state;
    GdkPixbuf *shot = NULL;
    int count = 0;
    gchar *uri = g_strconcat ("file://", location, NULL);

    playbin = gst_element_factory_make ("playbin", "playbin");
    audio_sink = gst_element_factory_make ("fakesink", "audiosink");
    video_sink = gst_element_factory_make ("fakesink", "videosink");

    g_object_set (playbin,
                  "uri", uri,
                  "audio-sink", audio_sink,
                  "video-sink", video_sink,
                  NULL);
    g_object_set (video_sink,
                  "sync", TRUE,
                  NULL);
    state = gst_element_set_state (playbin, GST_STATE_PAUSED);
    while (state == GST_STATE_CHANGE_ASYNC
           && count < 5
           && !g_cancellable_is_cancelled (cancellable)) {
        g_print ("Waiting in loop %d\n", count);
        state = gst_element_get_state (playbin, NULL, 0, 1 * GST_SECOND);
        count++;

        /* Spin mainloop so we can pick up the cancels */
        while (g_main_context_pending (NULL)) {
            g_main_context_iteration (NULL, FALSE);
        }
    }

    if (g_cancellable_is_cancelled (cancellable)) {
        g_print ("Video %s was cancelled\n", uri);
        state = GST_STATE_CHANGE_FAILURE;
    }

    if (state != GST_STATE_CHANGE_FAILURE &&
        state != GST_STATE_CHANGE_ASYNC) {
        GstFormat format = GST_FORMAT_TIME;
        gint64 duration;

        if (gst_element_query_duration (playbin, &format, &duration)) {
            gint64 seekpos;
            GstBuffer *frame;

            if (duration > 0) {
                if (duration / (3 * GST_SECOND) > 90) {
                    seekpos = (rand () % (duration / (3 * GST_SECOND))) * GST_SECOND;
                } else {
                    seekpos = (rand () % (duration / (GST_SECOND))) * GST_SECOND;
                }
            } else {
                seekpos = 5 * GST_SECOND;
            }

            gst_element_seek_simple (playbin, GST_FORMAT_TIME,
                                     GST_SEEK_FLAG_FLUSH |
                                     GST_SEEK_FLAG_ACCURATE, seekpos);

            /* Wait for seek to complete */
            count = 0;
            state = gst_element_get_state (playbin, NULL, 0,
                                           0.2 * GST_SECOND);
            while (state == GST_STATE_CHANGE_ASYNC && count < 3) {
                g_print ("Waiting in loop %d\n", count);
                state = gst_element_get_state (playbin, NULL, 0, 1 * GST_SECOND);
                count++;
            }

            g_object_get (playbin,
                          "frame", &frame,
                          NULL);
            if (frame == NULL) {
                g_warning ("No frame for %s", uri);
                return NULL;
            }

            shot = convert_buffer_to_pixbuf (frame, cancellable);
        }
    }

    gst_element_set_state (playbin, GST_STATE_NULL);
    g_object_unref (playbin);
    g_free (uri);

    return shot;
}

static gboolean
is_interesting (GdkPixbuf *pixbuf)
{
    int width, height, r, rowstride;
    gboolean has_alpha;
    guint32 histogram[4][4][4] = {{{0,},},};
    guchar *pixels;
    int pxl_count = 0, count, i;

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);

    pixels = gdk_pixbuf_get_pixels (pixbuf);
    for (r = 0; r < height; r++) {
        guchar *row = pixels + (r * rowstride);
        int c;

        for (c = 0; c < width; c++) {
            guchar r, g, b;

            r = row[0];
            g = row[1];
            b = row[2];

            histogram[r / 64][g / 64][b / 64]++;

            if (has_alpha) {
                row += 4;
            } else {
                row += 3;
            }

            pxl_count++;
        }
    }

    count = 0;
    for (i = 0; i < 4; i++) {
        int j;
        for (j = 0; j < 4; j++) {
            int k;

            for (k = 0; k < 4; k++) {
                /* Count how many bins have more than
                   1% of the pixels in the histogram */
                if (histogram[i][j][k] > pxl_count / 100) {
                    count++;
                }
            }
        }
    }

    /* Image is boring if there is only 1 bin with > 1% of pixels */
    return count > 1;
}
