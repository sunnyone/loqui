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

#include "loqui_channel_buffer_gtk.h"
#include <time.h>
#include <string.h>
#include "prefs_general.h"
#include "utils.h"
#include "gtkutils.h"
#include "loqui_gtk.h"

/*
enum {
	LAST_SIGNAL
	}; */

struct _LoquiChannelBufferGtkPrivate
{
	gboolean is_common_buffer;
};

static GtkTextBufferClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TEXT_BUFFER

static GtkTextTagTable *default_tag_table;

static GtkTextTag *highlight_area_tag;

/* static guint loqui_channel_buffer_gtk_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_channel_buffer_gtk_interface_buffer_init(LoquiChannelBufferIface *iface);
static void loqui_channel_buffer_gtk_class_init(LoquiChannelBufferGtkClass *klass);
static void loqui_channel_buffer_gtk_init(LoquiChannelBufferGtk *channel_buffer);
static void loqui_channel_buffer_gtk_finalize(GObject *object);

static void loqui_channel_buffer_gtk_append_current_time(LoquiChannelBufferGtk *channel_buffer);
static void loqui_channel_buffer_gtk_append(LoquiChannelBufferGtk *buffer, LoquiTextType type, gchar *str,
					    gboolean enable_highlight);

static void loqui_channel_buffer_gtk_append_message_text(LoquiChannelBuffer *buffer_p, LoquiMessageText *msgtext);
static void loqui_channel_buffer_gtk_text_inserted_cb(GtkTextBuffer *buffer,
					    GtkTextIter *pos,
					    const gchar *text,
					    gint length,
					    gpointer data);

static void loqui_channel_buffer_gtk_tag_uri(GtkTextBuffer *buffer,
				   GtkTextIter *iter_in,
				   const gchar *text);

static gboolean loqui_channel_buffer_gtk_link_tag_event_cb(GtkTextTag *texttag,
						 GObject *arg1,
						 GdkEvent *event,
						 GtkTextIter *arg2,
						 gpointer user_data);

static void loqui_channel_buffer_gtk_apply_tag_cb(GtkTextBuffer *buffer,
					GtkTextTag *tag,
					GtkTextIter *start,
					GtkTextIter *end,
					gpointer user_data);
static gboolean loqui_channel_buffer_gtk_delete_old_lines(LoquiChannelBufferGtk *buffer);

#define TIME_LEN 11

GType
loqui_channel_buffer_gtk_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelBufferGtkClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_buffer_gtk_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelBufferGtk),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_buffer_gtk_init
			};
		static const GInterfaceInfo ibuf_info = {
                        (GInterfaceInitFunc) loqui_channel_buffer_gtk_interface_buffer_init,    /* interface_init */
                        NULL,                                                                   /* interface_finalize */
                        NULL                                                                    /* interface_data */
                };
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiChannelBufferGtk",
					      &our_info,
					      0);
		g_type_add_interface_static(type, LOQUI_TYPE_CHANNEL_BUFFER, &ibuf_info);
	}
	
	return type;
}
static void
loqui_channel_buffer_gtk_interface_buffer_init(LoquiChannelBufferIface *iface)
{
	iface->append_message_text = loqui_channel_buffer_gtk_append_message_text;
}
static void
loqui_channel_buffer_gtk_init_tags(void)
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
			 G_CALLBACK(loqui_channel_buffer_gtk_link_tag_event_cb), NULL);
	gtk_text_tag_table_add(default_tag_table, tag);
	
	tag = gtk_text_tag_new("transparent");
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("highlight");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, "foreground", HIGHLIGHT_COLOR, NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	highlight_area_tag = gtk_text_tag_new("highlight-area");
	gtk_text_tag_table_add(default_tag_table, highlight_area_tag);
}

static void
loqui_channel_buffer_gtk_class_init(LoquiChannelBufferGtkClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_buffer_gtk_finalize;

	if(default_tag_table == NULL)
		loqui_channel_buffer_gtk_init_tags();	
	klass->tag_table = default_tag_table;
}
static void
loqui_channel_buffer_gtk_init(LoquiChannelBufferGtk *channel_buffer)
{
	LoquiChannelBufferGtkPrivate *priv;

	priv = g_new0(LoquiChannelBufferGtkPrivate, 1);

	channel_buffer->priv = priv;

	priv->is_common_buffer = FALSE;
	channel_buffer->show_account_name = FALSE;
	channel_buffer->show_channel_name = FALSE;
}
static void 
loqui_channel_buffer_gtk_finalize(GObject *object)
{
	LoquiChannelBufferGtk *channel_buffer;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(object));

        channel_buffer = LOQUI_CHANNEL_BUFFER_GTK(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_buffer->priv);
}
static void
loqui_channel_buffer_gtk_apply_tag_cb(GtkTextBuffer *buffer,
				      GtkTextTag *tag,
				      GtkTextIter *start,
				      GtkTextIter *end,
				      gpointer user_data)
{
	LoquiChannelBufferGtk *channel_buffer;
	LoquiChannelBufferGtkPrivate *priv;
	GtkTextIter tmp_start, tmp_end;
	GtkTextIter region_start, region_end;
	gboolean matched = FALSE;
	gchar *word;
	GList *cur;

	channel_buffer = LOQUI_CHANNEL_BUFFER_GTK(buffer);
	priv = channel_buffer->priv;

	if(tag != highlight_area_tag)
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

	gtk_text_buffer_remove_tag(buffer, tag,
				   start, end);
}

static gboolean
loqui_channel_buffer_gtk_link_tag_event_cb(GtkTextTag *texttag,
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
loqui_channel_buffer_gtk_tag_uri(GtkTextBuffer *buffer,
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
loqui_channel_buffer_gtk_delete_old_lines(LoquiChannelBufferGtk *buffer)
{
	LoquiChannelBufferGtkPrivate *priv;
	GtkTextIter cut_iter_start, cut_iter_end;
	gint line_num;
	gint max_line_number;
		
	g_return_val_if_fail(buffer != NULL, FALSE);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer), FALSE);

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
loqui_channel_buffer_gtk_text_inserted_cb(GtkTextBuffer *buffer,
					  GtkTextIter *pos,
					  const gchar *text,
					  gint length,
					  gpointer data)
{
	LoquiChannelBufferGtk *channel_buffer;
	LoquiChannelBufferGtkPrivate *priv;	
	GtkTextIter tmp_iter;
	
	g_return_if_fail(buffer != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));
	g_return_if_fail(g_utf8_validate(text, -1, NULL));
	
	channel_buffer = LOQUI_CHANNEL_BUFFER_GTK(buffer);
	priv = channel_buffer->priv;
	
	tmp_iter = *pos;
	loqui_channel_buffer_gtk_tag_uri(buffer, &tmp_iter, text);
	
	/* FIXME: In reality, this should not be done in this function,
	 *        but I can't find any reasonable ways... */
	g_idle_add_full(G_PRIORITY_LOW, (GSourceFunc) loqui_channel_buffer_gtk_delete_old_lines, buffer, NULL);
}
LoquiChannelBufferGtk*
loqui_channel_buffer_gtk_new(void)
{
        LoquiChannelBufferGtk *channel_buffer;
	LoquiChannelBufferGtkPrivate *priv;
	GtkTextBuffer *textbuf;
	GtkTextIter iter;
	
	if(default_tag_table == NULL)
		loqui_channel_buffer_gtk_init_tags();	
	channel_buffer = g_object_new(loqui_channel_buffer_gtk_get_type(), "tag_table", default_tag_table, NULL);
	priv = channel_buffer->priv;

	textbuf = GTK_TEXT_BUFFER(channel_buffer);

	gtk_text_buffer_get_start_iter(textbuf, &iter);
	gtk_text_buffer_create_mark(textbuf, "end", &iter, FALSE);

	g_signal_connect_after(G_OBJECT(textbuf), "insert-text",
			       G_CALLBACK(loqui_channel_buffer_gtk_text_inserted_cb), NULL);
	g_signal_connect_after(G_OBJECT(textbuf), "apply-tag",
			       G_CALLBACK(loqui_channel_buffer_gtk_apply_tag_cb), NULL);

	return channel_buffer;
}
static void
loqui_channel_buffer_gtk_append_current_time(LoquiChannelBufferGtk *buffer)
{
	gchar *buf;
	time_t t;
	
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));

	t = time(NULL);
	buf = utils_strftime_epoch(prefs_general.time_format, t);
	if(buf == NULL) {
		g_warning("Failed to strftime time string");
		return;
	}
	
	loqui_channel_buffer_gtk_append(buffer, LOQUI_TEXT_TYPE_TIME, buf, FALSE);
	g_free(buf);
}

static void
loqui_channel_buffer_gtk_append(LoquiChannelBufferGtk *buffer, LoquiTextType type, gchar *str, 
		      gboolean enable_highlight)
{
	GtkTextIter iter;
	gchar *style;
	gchar *highlight;

        g_return_if_fail(buffer != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));

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
	highlight = enable_highlight ? "highlight-area" : NULL;

	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, str, -1, 
						 style, highlight, NULL);
}
static void
loqui_channel_buffer_gtk_append_message_text(LoquiChannelBuffer *buffer_p, LoquiMessageText *msgtext)
{
	gchar *buf;
	LoquiTextType type;
	LoquiChannelBufferGtk *buffer;

        g_return_if_fail(buffer_p != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer_p));
        g_return_if_fail(msgtext != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE_TEXT(msgtext));

	buffer = LOQUI_CHANNEL_BUFFER_GTK(buffer_p);

	loqui_channel_buffer_gtk_append_current_time(buffer);
	
	type = loqui_message_text_get_text_type(msgtext);

	if(loqui_message_text_get_is_remark(msgtext)) {
		buf = loqui_message_text_get_nick_string(msgtext, loqui_channel_buffer_gtk_get_show_channel_name(buffer));
		loqui_channel_buffer_gtk_append(buffer, type, buf, FALSE);
		g_free(buf);
	}

	if(loqui_channel_buffer_gtk_get_show_account_name(buffer) &&
	   loqui_message_text_get_account_name(msgtext))
		buf = g_strdup_printf("[%s] %s\n",
				      loqui_message_text_get_account_name(msgtext),
				      loqui_message_text_get_text(msgtext));
	else
		buf = g_strconcat(loqui_message_text_get_text(msgtext), "\n", NULL);

	loqui_channel_buffer_gtk_append(buffer, type, buf,
					loqui_message_text_get_is_remark(msgtext));
	g_free(buf);
}
/* FIXME: this should do with max_line_number */
void
loqui_channel_buffer_gtk_set_whether_common_buffer(LoquiChannelBufferGtk *buffer, gboolean is_common_buffer)
{
	LoquiChannelBufferGtkPrivate *priv;
	
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));

	priv = buffer->priv;
	
	priv->is_common_buffer = TRUE;
}
void
loqui_channel_buffer_gtk_set_show_account_name(LoquiChannelBufferGtk *buffer, gboolean show_account_name)
{
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));

	buffer->show_account_name = show_account_name;
}
gboolean
loqui_channel_buffer_gtk_get_show_account_name(LoquiChannelBufferGtk *buffer)
{
	g_return_val_if_fail(buffer != NULL, FALSE);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer), FALSE);
	
	return buffer->show_account_name;
}
void
loqui_channel_buffer_gtk_set_show_channel_name(LoquiChannelBufferGtk *buffer, gboolean show_channel_name)
{
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));

	buffer->show_channel_name = show_channel_name;
}
gboolean
loqui_channel_buffer_gtk_get_show_channel_name(LoquiChannelBufferGtk *buffer)
{
	g_return_val_if_fail(buffer != NULL, FALSE);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer), FALSE);
	
	return buffer->show_channel_name;
}
