/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2 <http://loqui.good-day.net/>
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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

#include "channel_text.h"
#include "account_manager.h"
#include <time.h>
#include <string.h>
#include <ctype.h>

struct _ChannelTextPrivate
{
};

typedef struct _URIChunk {
	gboolean is_uri;

	gchar *str;
} URIChunk;

static GtkScrolledWindowClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_SCROLLED_WINDOW

static void channel_text_class_init(ChannelTextClass *klass);
static void channel_text_init(ChannelText *channel_text);
static void channel_text_finalize(GObject *object);
static void channel_text_destroy(GtkObject *object);

static void channel_text_insert_current_time(ChannelText *channel_text, GtkTextBuffer *textbuf, GtkTextIter *iter);
static GSList* channel_text_get_uri_chunk(const gchar *buf);

#define TIME_LEN 11

GType
channel_text_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelTextClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_text_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ChannelText),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_text_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ChannelText",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_text_class_init (ChannelTextClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_text_finalize;
        gtk_object_class->destroy = channel_text_destroy;
}
static void 
channel_text_init (ChannelText *channel_text)
{
	ChannelTextPrivate *priv;

	priv = g_new0(ChannelTextPrivate, 1);

	channel_text->priv = priv;
}
static void 
channel_text_finalize (GObject *object)
{
	ChannelText *channel_text;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_TEXT(object));

        channel_text = CHANNEL_TEXT(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_text->priv);
}
static void 
channel_text_destroy (GtkObject *object)
{
        ChannelText *channel_text;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_TEXT(object));

        channel_text = CHANNEL_TEXT(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

GtkWidget*
channel_text_new(void)
{
        ChannelText *channel_text;
	ChannelTextPrivate *priv;
	GtkTextBuffer *textbuf;

	channel_text = g_object_new(channel_text_get_type(), NULL);
	priv = channel_text->priv;

	channel_text->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(channel_text->text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(channel_text->text), GTK_WRAP_CHAR);

	textbuf = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(channel_text->text)));
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

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(channel_text), 
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(channel_text), channel_text->text);

	return GTK_WIDGET(channel_text);
}
static void
channel_text_insert_current_time(ChannelText *channel_text, GtkTextBuffer *textbuf, GtkTextIter *iter)
{
	gchar buf[TIME_LEN];
	time_t t;
	struct tm tm;

        g_return_if_fail(channel_text != NULL);
        g_return_if_fail(IS_CHANNEL_TEXT(channel_text));

	t = time(NULL);
	localtime_r(&t, &tm);
	strftime(buf, TIME_LEN, "%H:%M ", &tm);

	gtk_text_buffer_insert_with_tags_by_name(textbuf, iter, buf, -1, "time", NULL);
}

static GSList *
channel_text_get_uri_chunk(const gchar *buf)
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
channel_text_append(ChannelText *channel_text, TextType type, gchar *str)
{
	GtkTextIter iter;
	GtkTextBuffer *textbuf;
	GtkTextView *text;
	gchar *style;
	gchar *buf;

        g_return_if_fail(channel_text != NULL);
        g_return_if_fail(IS_CHANNEL_TEXT(channel_text));
	g_return_if_fail(channel_text->text != NULL);

	text = GTK_TEXT_VIEW(channel_text->text);

	textbuf = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(text));
	gtk_text_buffer_get_end_iter(textbuf, &iter);

	channel_text_insert_current_time(channel_text, textbuf, &iter);
	channel_text_get_uri_chunk(str);

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

	if(account_manager_whether_scroll(account_manager_get())) {
		gtk_adjustment_set_value(text->vadjustment, text->vadjustment->upper);
	}
}
