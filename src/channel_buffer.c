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
#include "utils.h"

struct _ChannelBufferPrivate
{
};

static GtkTextBufferClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TEXT_BUFFER

static void channel_buffer_class_init(ChannelBufferClass *klass);
static void channel_buffer_init(ChannelBuffer *channel_buffer);
static void channel_buffer_finalize(GObject *object);

static void channel_buffer_append_current_time(ChannelBuffer *channel_buffer);
static void channel_buffer_append(ChannelBuffer *buffer, TextType type, gchar *str);

static void channel_buffer_text_inserted_cb(GtkTextBuffer *buffer,
					    GtkTextIter *pos,
					    const gchar *text,
					    gint length,
					    gpointer data);

static void channel_buffer_tag_uri(GtkTextBuffer *buffer,
				   GtkTextIter *iter,
				   const gchar *text);

static gboolean channel_buffer_link_tag_event_cb(GtkTextTag *texttag,
						 GObject *arg1,
						 GdkEvent *event,
						 GtkTextIter *arg2,
						 gpointer user_data);

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
static gboolean channel_buffer_link_tag_event_cb(GtkTextTag *texttag,
						 GObject *arg1,
						 GdkEvent *event,
						 GtkTextIter *arg2,
						 gpointer user_data)
{
	gchar *str;
	GtkTextIter start_iter, end_iter;

	if(event->type == GDK_2BUTTON_PRESS && ((GdkEventButton *) event)->button == 1) {
		start_iter = *arg2;

		if(!gtk_text_iter_backward_to_tag_toggle(&start_iter, texttag)) {
			debug_puts("Can't find start.");
			return FALSE;
		}
		end_iter = *arg2;
		if(!gtk_text_iter_forward_to_tag_toggle(&end_iter, texttag)) {
			debug_puts("Can't find end");
			return FALSE;
		}

		str = gtk_text_iter_get_text(&start_iter, &end_iter);
		g_print("uri: %s\n", str);
		return TRUE;
	}
	return FALSE;
}
static void channel_buffer_tag_uri(GtkTextBuffer *buffer,
				   GtkTextIter *iter,
				   const gchar *text)
{
	GtkTextIter end_iter;
	gchar *buf, *cur, *tmp;
	glong len;

	cur = buf = g_strdup(text);
	len = g_utf8_strlen(cur, -1);
	if(!gtk_text_iter_backward_chars(iter, len)) {
		debug_puts("Can't backward iter");
		return;
	}

	while(*cur &&
	      ((tmp = strstr(cur, "http://")) != NULL ||
	      (tmp = strstr(cur, "https://")) != NULL ||
	      (tmp = strstr(cur, "ftp://")) != NULL)) {
		*tmp = '\0';
		len = g_utf8_strlen(cur, -1);
		if(len > 0 && !gtk_text_iter_forward_chars(iter, len)) {
			debug_puts("Can't forward chars");
			break;
		}

		cur = tmp+1;
		len = 1;

		while ((cur = g_utf8_next_char(cur)) != NULL) {
			len++;
			if(!isascii(*cur) ||
			   !g_ascii_isgraph(*cur) ||
			   strchr("()<>\"", *cur))
				break;
		}

		end_iter = *iter;
		if(!gtk_text_iter_forward_chars(&end_iter, len))
			break;
		
		gtk_text_buffer_apply_tag_by_name(buffer, "link", iter, &end_iter);
	}

	g_free(buf);

}
				    
static void channel_buffer_text_inserted_cb(GtkTextBuffer *buffer,
					    GtkTextIter *pos,
					    const gchar *text,
					    gint length,
					    gpointer data)
{
	GtkTextIter tmp_iter;
	g_return_if_fail(g_utf8_validate(text, -1, NULL));

	tmp_iter = *pos;
	channel_buffer_tag_uri(buffer, &tmp_iter, text);
}
ChannelBuffer*
channel_buffer_new(void)
{
        ChannelBuffer *channel_buffer;
	ChannelBufferPrivate *priv;
	GtkTextBuffer *textbuf;
	GtkTextIter iter;
	GtkTextTag *tag;

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
				   "foreground", "#555555", 
				   NULL);
	tag = gtk_text_buffer_create_tag(textbuf, "link",
					 "foreground", "blue",
					 "underline", PANGO_UNDERLINE_SINGLE,
					 NULL);
	gtk_text_buffer_create_tag(textbuf, "emphasis",
				   "weight", PANGO_WEIGHT_BOLD,
				   NULL);

	g_signal_connect(G_OBJECT(tag), "event",
			 G_CALLBACK(channel_buffer_link_tag_event_cb), NULL);
	g_signal_connect_after(G_OBJECT(textbuf), "insert-text",
			       G_CALLBACK(channel_buffer_text_inserted_cb), NULL);
	
	return channel_buffer;
}
static void
channel_buffer_append_current_time(ChannelBuffer *buffer)
{
	gchar buf[TIME_LEN];
	time_t t;
	struct tm tm;

        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	t = time(NULL);
	localtime_r(&t, &tm);
	strftime(buf, TIME_LEN, "%H:%M ", &tm);

	channel_buffer_append(buffer, TEXT_TYPE_TIME, buf);
}

static void
channel_buffer_append(ChannelBuffer *buffer, TextType type, gchar *str)
{
	GtkTextIter iter;
	gchar *style;

        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &iter);

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
	case TEXT_TYPE_TIME:
		style = "time";
		break;
	default:
		style = "normal";
	}
	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, str, -1, style, NULL);
}
void
channel_buffer_append_line(ChannelBuffer *buffer, TextType type, gchar *str)
{
	gchar *buf;

        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	channel_buffer_append_current_time(buffer);

	buf = g_strconcat(str, "\n", NULL);
	channel_buffer_append(buffer, type, buf);
	g_free(buf);
}

void
channel_buffer_append_remark(ChannelBuffer *buffer, TextType type, gboolean exec_noticer,
			     gboolean is_self, gboolean is_priv, 
			     const gchar *channel_name, const gchar *nick, const gchar *remark)
{
	gchar *nick_str;
	gchar *buf;
	
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));
	g_return_if_fail(nick != NULL);
	g_return_if_fail(remark != NULL);

	channel_buffer_append_current_time(buffer);
	
	if(is_priv) {
		if(is_self)
			nick_str = g_strdup_printf(">%s< ", nick);
		else
			nick_str = g_strdup_printf("=%s= ", nick);
	} else if (channel_name) {
		if(is_self)
			nick_str = g_strdup_printf(">%s:%s< ", channel_name, nick);
		else
			nick_str = g_strdup_printf("<%s:%s> ", channel_name, nick);
	} else {
		if(is_self)
			nick_str = g_strdup_printf(">%s< ", nick);
		else
			nick_str = g_strdup_printf("<%s> ", nick);
	}
	
	channel_buffer_append(buffer, type, nick_str);
	
	buf = g_strconcat(remark, "\n", NULL);
	channel_buffer_append(buffer, type, buf);

	g_free(buf);
	g_free(nick_str);
}
