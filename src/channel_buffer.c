/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include "channel_buffer.h"
#include "account_manager.h"
#include <time.h>
#include <string.h>
#include <ctype.h>

struct _ChannelBufferPrivate
{
};

typedef struct _URIChunk {
	gboolean is_uri;

	gchar *str;
} URIChunk;

static GtkTextBufferClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TEXT_BUFFER

static void channel_buffer_class_init(ChannelBufferClass *klass);
static void channel_buffer_init(ChannelBuffer *channel_buffer);
static void channel_buffer_finalize(GObject *object);

static void channel_buffer_insert_current_time(ChannelBuffer *channel_buffer, GtkTextIter *iter);
static GSList* channel_buffer_get_uri_chunk(const gchar *buf);

#define TIME_LEN 11

GType
channel_buffer_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelBufferClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_buffer_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ChannelBuffer),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_buffer_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ChannelBuffer",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_buffer_class_init (ChannelBufferClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_buffer_finalize;
}
static void 
channel_buffer_init (ChannelBuffer *channel_buffer)
{
	ChannelBufferPrivate *priv;

	priv = g_new0(ChannelBufferPrivate, 1);

	channel_buffer->priv = priv;
}
static void 
channel_buffer_finalize (GObject *object)
{
	ChannelBuffer *channel_buffer;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(object));

        channel_buffer = CHANNEL_BUFFER(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_buffer->priv);
}

ChannelBuffer*
channel_buffer_new(void)
{
        ChannelBuffer *channel_buffer;
	ChannelBufferPrivate *priv;
	GtkTextBuffer *textbuf;
	GtkTextIter iter;
	
	channel_buffer = g_object_new(channel_buffer_get_type(), NULL);
	priv = channel_buffer->priv;

	textbuf = GTK_TEXT_BUFFER(channel_buffer);

	gtk_text_buffer_get_start_iter(textbuf, &iter);
	gtk_text_buffer_create_mark(textbuf, "end", &iter, FALSE);

        gtk_text_buffer_create_tag(textbuf, "time", 
				   "foreground", "blue", 
				   NULL);
        gtk_text_buffer_create_tag(textbuf, "info", 
				   "foreground", "green", 
				   NULL);
        gtk_text_buffer_create_tag(textbuf, "normal", 
				   "foreground", "black", 
				   NULL);
        gtk_text_buffer_create_tag(textbuf, "error", 
				   "foreground", "red", 
				   NULL);
        gtk_text_buffer_create_tag(textbuf, "notice", 
				   "foreground", "grey", 
				   NULL);

	return channel_buffer;
}
static void
channel_buffer_insert_current_time(ChannelBuffer *channel_buffer, GtkTextIter *iter)
{
	gchar buf[TIME_LEN];
	time_t t;
	struct tm tm;

        g_return_if_fail(channel_buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(channel_buffer));

	t = time(NULL);
	localtime_r(&t, &tm);
	strftime(buf, TIME_LEN, "%H:%M ", &tm);

	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(channel_buffer), iter, buf, -1, "time", NULL);
}

static GSList *
channel_buffer_get_uri_chunk(const gchar *buf)
{
	GSList *chunk_list = NULL;
	gchar *tmp, *str;
	const gchar *cur;
	URIChunk *chunk;
	GSList *cl;
	gsize len;

	g_return_val_if_fail(buf != NULL, NULL);

	cur = buf;
	while(*cur) {
		if((tmp = strstr(cur, "http://")) == NULL &&
		   (tmp = strstr(cur, "https://")) == NULL &&
		   (tmp = strstr(cur, "ftp://")) == NULL)
			break;
		if(cur < tmp) {
			len = tmp - cur;
			str = g_malloc0(len + 1);
			memmove(str, cur, len);

			chunk = g_new0(URIChunk, 1);
			chunk->str = str;
			chunk->is_uri = FALSE;

			chunk_list = g_slist_append(chunk_list, chunk);
		}
		cur = tmp;
		
		while(*cur) {
			if(!isascii(*cur) ||
			   !g_ascii_isgraph(*cur) ||
			   strchr("()<>\"", *cur))
				break;
			   
			cur++;
		}
		if(cur > tmp) {
			len = cur - tmp;
			str = g_malloc0(len + 1);
			memmove(str, tmp, len);

			chunk = g_new0(URIChunk, 1);
			chunk->str = str;
			chunk->is_uri = TRUE;
			
			chunk_list = g_slist_append(chunk_list, chunk);
		}
	}
	if(*cur != '\0') {
		chunk = g_new0(URIChunk, 1);
		chunk->str = g_strdup(cur);
		chunk->is_uri = FALSE;

		chunk_list = g_slist_append(chunk_list, chunk);
	}

#if 0
	for(cl = chunk_list; cl != NULL; cl = cl->next) {
		chunk = (URIChunk *) cl->data;
		if(chunk->is_uri)
			g_print("uri: ");
		else
			g_print("nonuri: ");
		
		g_print("%s\n", chunk->str);
	}
#endif

	return chunk_list;
}
void
channel_buffer_append(ChannelBuffer *channel_buffer, TextType type, gchar *str)
{
	GtkTextIter iter;
	GtkTextBuffer *textbuf;
	gchar *style;
	gchar *buf;

        g_return_if_fail(channel_buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(channel_buffer));

	textbuf = GTK_TEXT_BUFFER(channel_buffer);
	gtk_text_buffer_get_end_iter(textbuf, &iter);

	channel_buffer_insert_current_time(channel_buffer, &iter);
/*	channel_buffer_get_uri_chunk(str); */

	switch(type) {
	case TEXT_TYPE_NOTICE:
		style = "notice";
		break;
	case TEXT_TYPE_ERROR:
		style = "error";
		break;
	case TEXT_TYPE_INFO:
		style = "info";
		break;
	default:
		style = "normal";
	}
	buf = g_strconcat(str, "\n", NULL);
	gtk_text_buffer_insert_with_tags_by_name(textbuf, &iter, buf, -1, style, NULL);
	g_free(buf);
}
