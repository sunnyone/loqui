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
#include <time.h>
#include <string.h>
#include "prefs_general.h"
#include "utils.h"
#include "gtkutils.h"
#include "loqui_gtk.h"

enum {
	APPEND,
	LAST_SIGNAL
};

struct _ChannelBufferPrivate
{
	gboolean is_common_buffer;
};

static GtkTextBufferClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TEXT_BUFFER

static GtkTextTagTable *default_tag_table;

static GtkTextTag *highlight_area_tag;
static GtkTextTag *notification_area_tag;

static guint channel_buffer_signals[LAST_SIGNAL] = { 0 };

static void channel_buffer_class_init(ChannelBufferClass *klass);
static void channel_buffer_init(ChannelBuffer *channel_buffer);
static void channel_buffer_finalize(GObject *object);

static void channel_buffer_append_current_time(ChannelBuffer *channel_buffer);
static void channel_buffer_append(ChannelBuffer *buffer, LoquiTextType type, gchar *str,
				  gboolean enable_highlight, gboolean exec_notification);

static void channel_buffer_text_inserted_cb(GtkTextBuffer *buffer,
					    GtkTextIter *pos,
					    const gchar *text,
					    gint length,
					    gpointer data);

static void channel_buffer_tag_uri(GtkTextBuffer *buffer,
				   GtkTextIter *iter_in,
				   const gchar *text);

static gboolean channel_buffer_link_tag_event_cb(GtkTextTag *texttag,
						 GObject *arg1,
						 GdkEvent *event,
						 GtkTextIter *arg2,
						 gpointer user_data);

static void channel_buffer_apply_tag_cb(GtkTextBuffer *buffer,
					GtkTextTag *tag,
					GtkTextIter *start,
					GtkTextIter *end,
					gpointer user_data);
static gboolean channel_buffer_delete_old_lines(ChannelBuffer *buffer);

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
channel_buffer_init_tags(void)
{
	GtkTextTag *tag;
	
	if(default_tag_table)
		return;
		
	default_tag_table = gtk_text_tag_table_new();
	
	tag = gtk_text_tag_new("time");
	g_object_set(tag, "foreground", "blue", NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("info");
	g_object_set(tag, "foreground", "green3", NULL);
	gtk_text_tag_table_add(default_tag_table, tag);
	
	tag = gtk_text_tag_new("normal");
	g_object_set(tag, "foreground", "black", NULL);
	gtk_text_tag_table_add(default_tag_table, tag);
	
	tag = gtk_text_tag_new("error");
	g_object_set(tag, "foreground", "red", NULL);
	gtk_text_tag_table_add(default_tag_table, tag);	

	tag = gtk_text_tag_new("notice");
	g_object_set(tag, "foreground", "#555555", NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("action");
	g_object_set(tag, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("link");
	g_object_set(tag, "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	g_signal_connect(G_OBJECT(tag), "event",
			 G_CALLBACK(channel_buffer_link_tag_event_cb), NULL);
	gtk_text_tag_table_add(default_tag_table, tag);
	
	tag = gtk_text_tag_new("transparent");
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("highlight");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, "foreground", HIGHLIGHT_COLOR, NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	highlight_area_tag = gtk_text_tag_new("highlight-area");
	gtk_text_tag_table_add(default_tag_table, highlight_area_tag);
	
	notification_area_tag = gtk_text_tag_new("notification-area");
	gtk_text_tag_table_add(default_tag_table, notification_area_tag);	
}

static void
channel_buffer_class_init (ChannelBufferClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_buffer_finalize;

	if(default_tag_table == NULL)
		channel_buffer_init_tags();	
	klass->tag_table = default_tag_table;

	channel_buffer_signals[APPEND] = g_signal_new("append",
						      G_OBJECT_CLASS_TYPE(object_class),
						      G_SIGNAL_RUN_FIRST,
						      G_STRUCT_OFFSET(ChannelBufferClass, append),
						      NULL, NULL,
						      g_cclosure_marshal_VOID__OBJECT,
						      G_TYPE_NONE, 1,
						      LOQUI_TYPE_MESSAGE_TEXT);
}
static void 
channel_buffer_init (ChannelBuffer *channel_buffer)
{
	ChannelBufferPrivate *priv;

	priv = g_new0(ChannelBufferPrivate, 1);

	channel_buffer->priv = priv;

	priv->is_common_buffer = FALSE;
	channel_buffer->show_account_name = FALSE;
	channel_buffer->show_channel_name = FALSE;
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
static void channel_buffer_apply_tag_cb(GtkTextBuffer *buffer,
					GtkTextTag *tag,
					GtkTextIter *start,
					GtkTextIter *end,
					gpointer user_data)
{
	ChannelBuffer *channel_buffer;
	ChannelBufferPrivate *priv;
	GtkTextIter tmp_start, tmp_end;
	GtkTextIter region_start, region_end;
	gboolean matched = FALSE;
	gchar *word;
	GList *cur;

	channel_buffer = CHANNEL_BUFFER(buffer);
	priv = channel_buffer->priv;

	if(!(tag == highlight_area_tag || tag == notification_area_tag))
		return;

	tmp_end = *end;

	for(cur = prefs_general.highlight_list; cur != NULL; cur = cur->next) {
		tmp_start = *start;
		word = (gchar *) cur->data;
		while(gtk_text_iter_forward_search(&tmp_start,
						   word,
						   GTK_TEXT_SEARCH_VISIBLE_ONLY,
						   &region_start,
						   &region_end,
						   &tmp_end)) {
			gtk_text_buffer_apply_tag_by_name(buffer,
							  "highlight",
							  &region_start,
							  &region_end);
			tmp_start = region_end;
			matched = TRUE;
		}
	}

	if(tag == notification_area_tag && 
	   matched &&
	   prefs_general.use_notification &&
	   prefs_general.notification_command &&
	   strlen(prefs_general.notification_command) > 0) {
		gtkutils_exec_command_with_error_dialog(prefs_general.notification_command);
	}

	gtk_text_buffer_remove_tag(buffer, tag,
				   start, end);
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
		gtkutils_exec_command_argument_with_error_dialog(prefs_general.browser_command, str);
		return TRUE;
	}
	return FALSE;
}
static void
channel_buffer_tag_uri(GtkTextBuffer *buffer,
		       GtkTextIter *iter_in,
		       const gchar *text)
{
	GtkTextIter start_iter, end_iter;
	const gchar *cur;
	glong len;
	const gchar *start_uri, *end_uri;

	cur = text;
	start_iter = *iter_in;
	len = g_utf8_strlen(cur, -1);
	if(!gtk_text_iter_backward_chars(&start_iter, len)) {
		debug_puts("Can't backward iter for uri");
		return;
	}

	end_iter = start_iter;
	while(*cur && utils_search_uri(cur, NULL, &start_uri, &end_uri)) {
		len = g_utf8_strlen(cur, start_uri - cur);
		if(len > 0 && !gtk_text_iter_forward_chars(&start_iter, len)) {
			debug_puts("Can't forward iter to start_uri");
			break;
		}

		end_iter = start_iter;
		len = g_utf8_strlen(start_uri, end_uri - start_uri + 1);
		if(len > 0 && !gtk_text_iter_forward_chars(&end_iter, len)) {
			debug_puts("Can't forward iter to end_uri");
			break;
		}
		
		gtk_text_buffer_apply_tag_by_name(buffer, "link", &start_iter, &end_iter);

		start_iter = end_iter;
		cur = end_uri + 1;
	}
}

static gboolean
channel_buffer_delete_old_lines(ChannelBuffer *buffer)
{
	ChannelBufferPrivate *priv;
	GtkTextIter cut_iter_start, cut_iter_end;
	gint line_num;
	gint max_line_number;
		
	g_return_val_if_fail(buffer != NULL, FALSE);
	g_return_val_if_fail(IS_CHANNEL_BUFFER(buffer), FALSE);

	priv = buffer->priv;
	
	max_line_number = priv->is_common_buffer ?
			  prefs_general.common_buffer_max_line_number :
			  prefs_general.channel_buffer_max_line_number;
			  
	line_num = gtk_text_buffer_get_line_count(GTK_TEXT_BUFFER(buffer)) - 1; // except last return code
	if (0 < max_line_number && line_num > max_line_number) {
		gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &cut_iter_start);
		cut_iter_end = cut_iter_start;
		
		if(gtk_text_iter_forward_lines(&cut_iter_end, line_num - max_line_number))
			gtk_text_buffer_delete(GTK_TEXT_BUFFER(buffer), &cut_iter_start, &cut_iter_end);
		else {
			g_warning("Can't delete buffer.");
		}
	}
	
	return FALSE;
}

static void
channel_buffer_text_inserted_cb(GtkTextBuffer *buffer,
  			        GtkTextIter *pos,
				const gchar *text,
				gint length,
				gpointer data)
{
	ChannelBuffer *channel_buffer;
	ChannelBufferPrivate *priv;	
	GtkTextIter tmp_iter;
	
	g_return_if_fail(buffer != NULL);
	g_return_if_fail(IS_CHANNEL_BUFFER(buffer));
	g_return_if_fail(g_utf8_validate(text, -1, NULL));
	
	channel_buffer = CHANNEL_BUFFER(buffer);
	priv = channel_buffer->priv;
	
	tmp_iter = *pos;
	channel_buffer_tag_uri(buffer, &tmp_iter, text);
	
	/* FIXME: In reality, this should not be done in this function,
	 *        but I can't find any reasonable ways... */
	g_idle_add_full(G_PRIORITY_LOW, (GSourceFunc) channel_buffer_delete_old_lines, buffer, NULL);
}
ChannelBuffer*
channel_buffer_new(void)
{
        ChannelBuffer *channel_buffer;
	ChannelBufferPrivate *priv;
	GtkTextBuffer *textbuf;
	GtkTextIter iter;
	
	if(default_tag_table == NULL)
		channel_buffer_init_tags();	
	channel_buffer = g_object_new(channel_buffer_get_type(), "tag_table", default_tag_table, NULL);
	priv = channel_buffer->priv;

	textbuf = GTK_TEXT_BUFFER(channel_buffer);

	gtk_text_buffer_get_start_iter(textbuf, &iter);
	gtk_text_buffer_create_mark(textbuf, "end", &iter, FALSE);

	g_signal_connect_after(G_OBJECT(textbuf), "insert-text",
			       G_CALLBACK(channel_buffer_text_inserted_cb), NULL);
	g_signal_connect_after(G_OBJECT(textbuf), "apply-tag",
			       G_CALLBACK(channel_buffer_apply_tag_cb), NULL);

	return channel_buffer;
}
static void
channel_buffer_append_current_time(ChannelBuffer *buffer)
{
	gchar *buf;
	time_t t;
	
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	t = time(NULL);
	buf = utils_strftime_epoch(prefs_general.time_format, t);
	if(buf == NULL) {
		g_warning("Failed to strftime time string");
		return;
	}
	
	channel_buffer_append(buffer, LOQUI_TEXT_TYPE_TIME, buf, FALSE, FALSE);
	g_free(buf);
}

static void
channel_buffer_append(ChannelBuffer *buffer, LoquiTextType type, gchar *str, 
		      gboolean enable_highlight,
		      gboolean exec_notification)
{
	GtkTextIter iter;
	gchar *style;
	gchar *highlight;

        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &iter);

	switch(type) {
	case LOQUI_TEXT_TYPE_NOTICE:
		style = "notice";
		break;
	case LOQUI_TEXT_TYPE_ACTION:
		style = "action";
		break;
	case LOQUI_TEXT_TYPE_ERROR:
		style = "error";
		break;
	case LOQUI_TEXT_TYPE_INFO:
		style = "info";
		break;
	case LOQUI_TEXT_TYPE_TIME:
		style = "time";
		break;
	case LOQUI_TEXT_TYPE_TRANSPARENT:
		style = "transparent";
		break;
	default:
		style = "normal";
	}
	if(enable_highlight) {
		if(exec_notification)
			highlight = "notification-area";
		else
			highlight = "highlight-area";
	} else {
		highlight = NULL;
	}
	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, str, -1, 
						 style, highlight, NULL);
}
void
channel_buffer_append_message_text(ChannelBuffer *buffer, LoquiMessageText *msgtext, gboolean exec_notification)
{
	gchar *buf;
	LoquiTextType type;

        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));
        g_return_if_fail(msgtext != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE_TEXT(msgtext));

	channel_buffer_append_current_time(buffer);
	
	type = loqui_message_text_get_text_type(msgtext);

	if(loqui_message_text_get_is_remark(msgtext)) {
		buf = loqui_message_text_get_nick_string(msgtext, channel_buffer_get_show_channel_name(buffer));
		channel_buffer_append(buffer, type, buf, FALSE, FALSE);
		g_free(buf);
	}

	if(channel_buffer_get_show_account_name(buffer) &&
	   loqui_message_text_get_account_name(msgtext))
		buf = g_strdup_printf("[%s] %s\n",
				      loqui_message_text_get_account_name(msgtext),
				      loqui_message_text_get_text(msgtext));
	else
		buf = g_strconcat(loqui_message_text_get_text(msgtext), "\n", NULL);

	channel_buffer_append(buffer, type, buf,
			      loqui_message_text_get_is_remark(msgtext), 
			      exec_notification);
	g_free(buf);

	g_signal_emit(buffer, channel_buffer_signals[APPEND], 0, msgtext);
}
/* FIXME: this should do with max_line_number */
void
channel_buffer_set_whether_common_buffer(ChannelBuffer *buffer, gboolean is_common_buffer)
{
	ChannelBufferPrivate *priv;
	
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	priv = buffer->priv;
	
	priv->is_common_buffer = TRUE;
}
void
channel_buffer_set_show_account_name(ChannelBuffer *buffer, gboolean show_account_name)
{
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	buffer->show_account_name = show_account_name;
}
gboolean
channel_buffer_get_show_account_name(ChannelBuffer *buffer)
{
	g_return_val_if_fail(buffer != NULL, FALSE);
	g_return_val_if_fail(IS_CHANNEL_BUFFER(buffer), FALSE);
	
	return buffer->show_account_name;
}
void
channel_buffer_set_show_channel_name(ChannelBuffer *buffer, gboolean show_channel_name)
{
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(IS_CHANNEL_BUFFER(buffer));

	buffer->show_channel_name = show_channel_name;
}
gboolean
channel_buffer_get_show_channel_name(ChannelBuffer *buffer)
{
	g_return_val_if_fail(buffer != NULL, FALSE);
	g_return_val_if_fail(IS_CHANNEL_BUFFER(buffer), FALSE);
	
	return buffer->show_channel_name;
}
