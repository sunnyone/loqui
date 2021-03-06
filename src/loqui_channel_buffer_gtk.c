/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk
 * Copyright (C) 2002-2003 Yoichi Imai <sunnyone41@gmail.com>
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

/* static guint loqui_channel_buffer_gtk_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_channel_buffer_gtk_interface_buffer_init(LoquiChannelBufferIface *iface);
static void loqui_channel_buffer_gtk_class_init(LoquiChannelBufferGtkClass *klass);
static void loqui_channel_buffer_gtk_init(LoquiChannelBufferGtk *channel_buffer);
static void loqui_channel_buffer_gtk_finalize(GObject *object);

static void loqui_channel_buffer_gtk_append_message_text(LoquiChannelBuffer *buffer_p, LoquiMessageText *msgtext);
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

	tag = gtk_text_tag_new("global");
	gtk_text_tag_table_add(default_tag_table, tag);

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

	tag = gtk_text_tag_new("account_name");
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("channel_name");
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("hover");
	g_object_set(tag, "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("transparent");
	gtk_text_tag_table_add(default_tag_table, tag);

	tag = gtk_text_tag_new("highlight");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_tag_table_add(default_tag_table, tag);
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
LoquiChannelBufferGtk*
loqui_channel_buffer_gtk_new(LoquiPrefPartial *ppref_channel_buffer)
{
        LoquiChannelBufferGtk *channel_buffer;
	LoquiChannelBufferGtkPrivate *priv;
	GtkTextBuffer *textbuf;
	GtkTextIter iter;
	GtkTextIter start_iter, end_iter;

	if(default_tag_table == NULL)
		loqui_channel_buffer_gtk_init_tags();

	channel_buffer = g_object_new(loqui_channel_buffer_gtk_get_type(), "tag_table", default_tag_table, NULL);
	priv = channel_buffer->priv;

	priv->ppref_channel_buffer = g_object_ref(ppref_channel_buffer);

	textbuf = GTK_TEXT_BUFFER(channel_buffer);

	gtk_text_buffer_get_start_iter(textbuf, &iter);
	gtk_text_buffer_create_mark(textbuf, "end", &iter, FALSE);
	gtk_text_buffer_create_mark(textbuf, "hover", &iter, FALSE);

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
	SET_STRING_IF_MATCHED("global", "font", "GlobalFont");

#undef SET_STRING_IF_MATCHED
#undef GET_TAG

}
static void
loqui_channel_buffer_gtk_load_styles(LoquiChannelBufferGtk *buffer)
{
	LoquiChannelBufferGtkPrivate *priv;
	PangoFontDescription *font_desc;
	gchar *font_name;

	priv = buffer->priv;

	font_desc = gtkutils_get_default_font_desc();
	font_name = pango_font_description_to_string(font_desc);
	pango_font_description_free(font_desc);

#define SET_STRING_DEFAULT(key, value) loqui_pref_partial_set_default_string(priv->ppref_channel_buffer, key, value)

	SET_STRING_DEFAULT("TimeColor", "blue");
	SET_STRING_DEFAULT("InfoColor", "#079107");
	SET_STRING_DEFAULT("NormalColor", "black");
	SET_STRING_DEFAULT("ErrorColor", "red");
	SET_STRING_DEFAULT("NoticeColor", "#555555");
	SET_STRING_DEFAULT("LinkColor", "blue");
	SET_STRING_DEFAULT("HighlightColor", "purple");

	SET_STRING_DEFAULT("GlobalFont", font_name);

#undef SET_STRING_DEFAULT

	loqui_pref_partial_foreach(priv->ppref_channel_buffer, loqui_channel_buffer_gtk_ppref_changed_cb, buffer);
}
static gchar *
loqui_channel_buffer_gtk_get_time_string(LoquiChannelBufferGtk *buffer)
{
	gchar *buf = NULL;
	time_t t;
	gchar *time_format;

        g_return_val_if_fail(buffer != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer), NULL);

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
		return NULL;
	}

	return buf;
}

static const gchar *
loqui_channel_buffer_gtk_get_tag_name(LoquiChannelBufferGtk *buffer, LoquiTextType type)
{
	const gchar *tag_name;

        g_return_val_if_fail(buffer != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer), NULL);

	switch(type) {
	case LOQUI_TEXT_TYPE_NOTICE:
		tag_name = "notice";
		break;
	case LOQUI_TEXT_TYPE_ACTION:
		tag_name = "action";
		break;
	case LOQUI_TEXT_TYPE_ERROR:
		tag_name = "error";
		break;
	case LOQUI_TEXT_TYPE_INFO:
		tag_name = "info";
		break;
	case LOQUI_TEXT_TYPE_TIME:
		tag_name = "time";
		break;
	case LOQUI_TEXT_TYPE_TRANSPARENT:
		tag_name = "transparent";
		break;
	default:
		tag_name = "normal";
	}

	return tag_name;
}

static void
loqui_channel_buffer_gtk_tag_regions(LoquiChannelBufferGtk *buffer, GtkTextIter *text_start_iter_in, GtkTextTag *tag,
				     const gchar *text, GList *region_list)
{
	GList *cur;
	LoquiMessageTextRegion *region;
	GtkTextIter text_start_iter, region_start_iter, region_end_iter;
	gint start_pos, offset;

	text_start_iter = *text_start_iter_in;
	for (cur = region_list; cur != NULL; cur = cur->next) {
		region = LOQUI_MESSAGE_TEXT_REGION(cur->data);

		start_pos = loqui_message_text_region_get_start_pos(region);
		offset = loqui_message_text_region_get_offset(region);

		region_start_iter = text_start_iter;
		gtk_text_iter_forward_chars(&region_start_iter, g_utf8_strlen(text, start_pos));

		region_end_iter = region_start_iter;
		gtk_text_iter_forward_chars(&region_end_iter, g_utf8_strlen(text + start_pos, offset));

		gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), tag, &region_start_iter, &region_end_iter);
	}
}

static void
loqui_channel_buffer_gtk_append_message_text(LoquiChannelBuffer *buffer_p, LoquiMessageText *msgtext)
{
	gchar *buf;
	const gchar *text;
	LoquiTextType type;
	LoquiChannelBufferGtk *buffer;
	GtkTextIter iter, text_start_iter;
	const gchar *tag_name;
	gsize len;

        g_return_if_fail(buffer_p != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer_p));
        g_return_if_fail(msgtext != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE_TEXT(msgtext));

	buffer = LOQUI_CHANNEL_BUFFER_GTK(buffer_p);

	if (loqui_message_text_get_is_ignored(msgtext)) {
		return;
	}

	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &iter);
	if ((buf = loqui_channel_buffer_gtk_get_time_string(buffer)) != NULL) {
		gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, buf, -1, "time", "global", NULL);
		g_free(buf);
	}

	type = loqui_message_text_get_text_type(msgtext);
	tag_name = loqui_channel_buffer_gtk_get_tag_name(buffer, type);

	if (loqui_channel_buffer_gtk_get_show_account_name(buffer) &&
	    loqui_message_text_get_account_name(msgtext)) {
		/* Insert "[%s] " */
	    	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, "[", -1, tag_name, "global", NULL);
	    	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter,
		     loqui_message_text_get_account_name(msgtext), -1, tag_name, "global", "account_name", NULL);
     	    	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, "] ", -1, tag_name, "global", NULL);
	}

	if (loqui_message_text_get_is_remark(msgtext)) {
                gchar *prefix;
                gchar *channel_name;
                gchar *separator;
                gchar *nick;
                gchar *suffix;

		loqui_message_text_get_nick_string_parts(msgtext, loqui_channel_buffer_gtk_get_show_channel_name(buffer),
                      &prefix, &channel_name, &separator, &nick, &suffix);

		gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, prefix, -1, tag_name, "global", NULL);
		if (channel_name != NULL)
			gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, channel_name, -1, tag_name, "global", "channel_name", NULL);
		if (separator != NULL)
			gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, separator, -1, tag_name, "global", NULL);
		gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, nick, -1, tag_name, "global", NULL);
		gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, suffix, -1, tag_name, "global", NULL);
	}

	text = loqui_message_text_get_text(msgtext);
	len = g_utf8_strlen(text, -1);
	gtk_text_buffer_insert_with_tags_by_name(GTK_TEXT_BUFFER(buffer), &iter, text, -1, tag_name, "global", NULL);

	text_start_iter = iter;
	gtk_text_iter_backward_chars(&text_start_iter, len);

	loqui_channel_buffer_gtk_tag_regions(buffer, &text_start_iter,
					     gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer)), "link"),
					     text, msgtext->uri_region_list);
	if (loqui_message_text_get_is_remark(msgtext)) {
		loqui_channel_buffer_gtk_tag_regions(buffer, &text_start_iter,
						     gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer)), "highlight"),
						     text, msgtext->highlight_region_list);
	}

	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &iter, "\n", -1);
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
