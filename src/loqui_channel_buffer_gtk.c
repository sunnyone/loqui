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

#include <libloqui/loqui-utils.h>

#include "loqui_channel_buffer_gtk.h"
#include <time.h>
#include <string.h>
#include "gtkutils.h"
#include "loqui-core-gtk.h"
#include <loqui.h>

#include <loqui-general-pref-groups.h>
#include <loqui-general-pref-default.h>
#include "loqui-general-pref-gtk-groups.h"
#include "loqui-general-pref-gtk-default.h"

/*
enum {
	LAST_SIGNAL
	}; */

struct _LoquiChannelBufferGtkPrivate
{
	LoquiPrefPartial *ppref_channel_buffer;
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

static void loqui_channel_buffer_gtk_apply_tag_cb(GtkTextBuffer *buffer,
					GtkTextTag *tag,
					GtkTextIter *start,
					GtkTextIter *end,
					gpointer user_data);
static void loqui_channel_buffer_gtk_delete_old_lines(LoquiChannelBufferGtk *buffer);

static void loqui_channel_buffer_gtk_load_styles(LoquiChannelBufferGtk *buffer);
static void loqui_channel_buffer_gtk_ppref_changed_cb(LoquiPrefPartial *pref, const gchar *key, gpointer data);


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
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("info");
	gtk_text_tag_table_add(default_tag_table, tag);
	
	tag = gtk_text_tag_new("normal");
	gtk_text_tag_table_add(default_tag_table, tag);
	
	tag = gtk_text_tag_new("error");
	gtk_text_tag_table_add(default_tag_table, tag);	

	tag = gtk_text_tag_new("notice");
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("action");
	g_object_set(tag, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("link");
	gtk_text_tag_table_add(default_tag_table, tag);
	
	tag = gtk_text_tag_new("hover");
	g_object_set(tag, "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("transparent");
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("highlight");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, NULL);
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
	LoquiChannelBufferGtkPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(object));

        channel_buffer = LOQUI_CHANNEL_BUFFER_GTK(object);

	priv = channel_buffer->priv;

	g_signal_handlers_disconnect_by_func(priv->ppref_channel_buffer, loqui_channel_buffer_gtk_ppref_changed_cb, channel_buffer);

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->ppref_channel_buffer);

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
	gchar **highlight_array;
	int i;

	channel_buffer = LOQUI_CHANNEL_BUFFER_GTK(buffer);
	priv = channel_buffer->priv;

	if(tag != highlight_area_tag)
		return;

	tmp_end = *end;
	highlight_array = loqui_pref_get_string_list(loqui_get_general_pref(),
						     LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "HighlightList", NULL, NULL);
	if (highlight_array) {
		for(i = 0; (word = highlight_array[i]) != NULL; i++) {
			tmp_start = *start;
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
		g_strfreev(highlight_array);
	}

	gtk_text_buffer_remove_tag(buffer, tag,
				   start, end);
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
		loqui_debug_puts("Can't backward iter for uri");
		return;
	}

	end_iter = start_iter;
	while(*cur && loqui_utils_search_uri(cur, NULL, &start_uri, &end_uri)) {
		len = g_utf8_strlen(cur, start_uri - cur);
		if(len > 0 && !gtk_text_iter_forward_chars(&start_iter, len)) {
			loqui_debug_puts("Can't forward iter to start_uri");
			break;
		}

		end_iter = start_iter;
		len = g_utf8_strlen(start_uri, end_uri - start_uri + 1);
		if(len > 0 && !gtk_text_iter_forward_chars(&end_iter, len)) {
			loqui_debug_puts("Can't forward iter to end_uri");
			break;
		}
		
		gtk_text_buffer_apply_tag_by_name(buffer, "link", &start_iter, &end_iter);

		start_iter = end_iter;
		cur = end_uri + 1;
	}
}

static void
loqui_channel_buffer_gtk_delete_old_lines(LoquiChannelBufferGtk *buffer)
{
	LoquiChannelBufferGtkPrivate *priv;
	GtkTextIter cut_iter_start, cut_iter_end;
	gint line_num;
	gint max_line_number;
		
	g_return_if_fail(buffer != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));

	priv = buffer->priv;
	
	max_line_number = priv->is_common_buffer ?
			  loqui_pref_get_with_default_integer(loqui_get_general_pref(),
							      LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "CommonBufferMaxLineNumber",
							      LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_COMMON_BUFFER_MAX_LINE_NUMBER, NULL) :
			  loqui_pref_get_with_default_integer(loqui_get_general_pref(),
							      LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "ChannelBufferMaxLineNumber",
							      LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_CHANNEL_BUFFER_MAX_LINE_NUMBER, NULL);
			  
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
}
LoquiChannelBufferGtk*
loqui_channel_buffer_gtk_new(LoquiPrefPartial *ppref_channel_buffer)
{
        LoquiChannelBufferGtk *channel_buffer;
	LoquiChannelBufferGtkPrivate *priv;
	GtkTextBuffer *textbuf;
	GtkTextIter iter;
	
	if(default_tag_table == NULL)
		loqui_channel_buffer_gtk_init_tags();

	channel_buffer = g_object_new(loqui_channel_buffer_gtk_get_type(), "tag_table", default_tag_table, NULL);
	priv = channel_buffer->priv;

	priv->ppref_channel_buffer = g_object_ref(ppref_channel_buffer);

	textbuf = GTK_TEXT_BUFFER(channel_buffer);

	gtk_text_buffer_get_start_iter(textbuf, &iter);
	gtk_text_buffer_create_mark(textbuf, "end", &iter, FALSE);
	gtk_text_buffer_create_mark(textbuf, "hover", &iter, FALSE);

	g_signal_connect_after(G_OBJECT(textbuf), "insert-text",
			       G_CALLBACK(loqui_channel_buffer_gtk_text_inserted_cb), NULL);
	g_signal_connect_after(G_OBJECT(textbuf), "apply-tag",
			       G_CALLBACK(loqui_channel_buffer_gtk_apply_tag_cb), NULL);

	loqui_channel_buffer_gtk_load_styles(channel_buffer);
	loqui_pref_partial_connect__changed_partial(priv->ppref_channel_buffer, loqui_channel_buffer_gtk_ppref_changed_cb, channel_buffer);

	return channel_buffer;
}
static void
loqui_channel_buffer_gtk_ppref_changed_cb(LoquiPrefPartial *ppref, const gchar *key, gpointer data)
{
	LoquiChannelBufferGtk *buffer;

	GtkTextTag *tag;

	g_return_if_fail(key != NULL);

	buffer = LOQUI_CHANNEL_BUFFER_GTK(data);

#define GET_TAG(buffer, name) gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer)), name)
#define SET_STRING_IF_MATCHED(tag_name, tag_attribute, _key) { \
	if (strcmp(key, _key) == 0) { \
		tag = GET_TAG(buffer, tag_name); \
if (tag == NULL) { g_warning("null tag: %s\n", tag_name); } \
                g_return_if_fail(tag != NULL); \
		g_object_set(tag, tag_attribute, \
			     loqui_pref_partial_get_string(ppref, _key, NULL), NULL); \
		return; \
	} \
}

	SET_STRING_IF_MATCHED("time", "foreground", "TimeColor");
	SET_STRING_IF_MATCHED("info", "foreground", "InfoColor");
	SET_STRING_IF_MATCHED("normal", "foreground", "NormalColor");
	SET_STRING_IF_MATCHED("error", "foreground", "ErrorColor");
	SET_STRING_IF_MATCHED("notice", "foreground", "NoticeColor");
	SET_STRING_IF_MATCHED("link", "foreground", "LinkColor");
	SET_STRING_IF_MATCHED("highlight", "foreground", "HighlightColor");

#undef SET_STRING_IF_MATCHED
#undef GET_TAG

}
static void
loqui_channel_buffer_gtk_load_styles(LoquiChannelBufferGtk *buffer)
{
	LoquiChannelBufferGtkPrivate *priv;

	priv = buffer->priv;

#define SET_STRING_DEFAULT(key, value) loqui_pref_partial_set_default_string(priv->ppref_channel_buffer, key, value)

	SET_STRING_DEFAULT("TimeColor", "blue");
	SET_STRING_DEFAULT("InfoColor", "#079107");
	SET_STRING_DEFAULT("NormalColor", "black");
	SET_STRING_DEFAULT("ErrorColor", "red");
	SET_STRING_DEFAULT("NoticeColor", "#555555");
	SET_STRING_DEFAULT("LinkColor", "blue");
	SET_STRING_DEFAULT("HighlightColor", "purple");

#undef SET_STRING_DEFAULT

	loqui_pref_partial_foreach(priv->ppref_channel_buffer, loqui_channel_buffer_gtk_ppref_changed_cb, buffer);
}
static void
loqui_channel_buffer_gtk_append_current_time(LoquiChannelBufferGtk *buffer)
{
	gchar *buf = NULL;
	time_t t;
	gchar *time_format;
	
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));

	t = time(NULL);
	time_format = loqui_pref_get_with_default_string(loqui_get_general_pref(),
							 LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "TimeFormat",
							 LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_TIME_FORMAT, NULL);
	if (time_format) {
		buf = loqui_utils_strftime_epoch(time_format, t);
		g_free(time_format);
	}

	if (buf == NULL) {
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

	if (loqui_channel_buffer_gtk_get_show_account_name(buffer) &&
	    loqui_message_text_get_account_name(msgtext)) {
		buf = g_strdup_printf("[%s] ", loqui_message_text_get_account_name(msgtext));
		loqui_channel_buffer_gtk_append(buffer, type, buf, FALSE);
		g_free(buf);
	}

	if(loqui_message_text_get_is_remark(msgtext)) {
		buf = loqui_message_text_get_nick_string(msgtext, loqui_channel_buffer_gtk_get_show_channel_name(buffer));
		loqui_channel_buffer_gtk_append(buffer, type, buf, FALSE);
		g_free(buf);
	}

	buf = g_strdup_printf("%s\n", loqui_message_text_get_text(msgtext));
	loqui_channel_buffer_gtk_append(buffer, type, buf,
					loqui_message_text_get_is_remark(msgtext));
	g_free(buf);

	loqui_channel_buffer_gtk_delete_old_lines(buffer);
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
