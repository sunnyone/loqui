/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk
 * Copyright (C) 2003 Yoichi Imai <sunnyone41@gmail.com>
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

#include "remark_entry.h"
#include "gdk/gdkkeysyms.h"
#include <libloqui/loqui-utils.h>
#include "gtkutils.h"

#include "loqui_stock.h"
#include "loqui_channel_text_view.h"
#include "loqui_statusbar.h"

#include <loqui_sender.h>
#include <loqui_sender_irc.h>

#include "main.h"
#include <glib/gi18n.h>

#include <string.h>

#include "loqui-general-pref-gtk-groups.h"
#include "loqui-general-pref-gtk-default.h"

#include <loqui.h>

enum {
	CALL_HISTORY,
	SCROLL_CHANNEL_TEXTVIEW,
	SCROLL_COMMON_TEXTVIEW,
	COMPLETE_NICK,

        LAST_SIGNAL
};

struct _RemarkEntryPrivate
{
	GList *string_list;
	gint current_index;

	gboolean is_multiline;

	GtkWidget *vbox;

	GtkToggleAction *toggle_command_action;
	GtkWidget *toggle_command;

	GtkWidget *hbox_text;
	GtkWidget *textview;
	GtkWidget *button_ok;
	GtkWidget *button_notice;

	GtkWidget *toggle_multiline;
	guint toggle_multiline_toggled_id;

	GtkWidget *toggle_palette;

	LoquiApp *app;
};

static GtkHBoxClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_HBOX

static guint remark_entry_signals[LAST_SIGNAL] = { 0 };

static void remark_entry_class_init(RemarkEntryClass *klass);
static void remark_entry_init(RemarkEntry *remark_entry);
static void remark_entry_finalize(GObject *object);
static void remark_entry_destroy(GtkObject *object);
static void remark_entry_grab_focus(GtkWidget *widget);

static void remark_entry_entry_text_shown_cb(GtkWidget *widget, gpointer data);
static void remark_entry_entry_multiline_toggled_cb(GtkWidget *widget, gpointer data);
static void remark_entry_ok_clicked_cb(GtkWidget *widget, gpointer data);
static void remark_entry_activate_cb(GtkWidget *widget, gpointer data);
static void remark_entry_notice_clicked_cb(GtkWidget *widget, gpointer data);
static void remark_entry_entry_changed_cb(GtkEntry *widget, RemarkEntry *remark_entry);
static gboolean remark_entry_textview_or_entry_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, RemarkEntry *remark_entry);

static void remark_entry_history_add(RemarkEntry *entry, const gchar *str);

static void remark_entry_send_text(RemarkEntry *remark_entry, gboolean is_notice);
static void remark_entry_call_history(RemarkEntry *entry, gint count);
static void remark_entry_scroll_channel_textview(RemarkEntry *entry, gint pages);
static void remark_entry_scroll_common_textview(RemarkEntry *entry, gint pages);
static void remark_entry_complete_nick(RemarkEntry *entry);

GType
remark_entry_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(RemarkEntryClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) remark_entry_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(RemarkEntry),
				0,              /* n_preallocs */
				(GInstanceInitFunc) remark_entry_init
			};

		type = g_type_register_static(PARENT_TYPE,
					      "RemarkEntry",
					      &our_info,
					      0);
	}

	return type;
}
static void
remark_entry_class_init(RemarkEntryClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);
	GtkBindingSet *binding_set;

        parent_class = g_type_class_peek_parent(klass);

        object_class->finalize = remark_entry_finalize;
        gtk_object_class->destroy = remark_entry_destroy;

	klass->call_history = remark_entry_call_history;
	klass->scroll_channel_textview = remark_entry_scroll_channel_textview;
	klass->scroll_common_textview = remark_entry_scroll_common_textview;
	klass->complete_nick = remark_entry_complete_nick;

	GTK_WIDGET_CLASS(klass)->grab_focus = remark_entry_grab_focus;

        remark_entry_signals[CALL_HISTORY] = g_signal_new("call_history",
							  G_OBJECT_CLASS_TYPE(object_class),
							  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
							  G_STRUCT_OFFSET(RemarkEntryClass, call_history),
							  NULL, NULL,
							  g_cclosure_marshal_VOID__INT,
							  G_TYPE_NONE, 1, G_TYPE_INT);

        remark_entry_signals[SCROLL_CHANNEL_TEXTVIEW] = g_signal_new("scroll_channel_textview",
								     G_OBJECT_CLASS_TYPE(object_class),
								     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
								     G_STRUCT_OFFSET(RemarkEntryClass, scroll_channel_textview),
								     NULL, NULL,
								     g_cclosure_marshal_VOID__INT,
								     G_TYPE_NONE, 1, G_TYPE_INT);

        remark_entry_signals[SCROLL_COMMON_TEXTVIEW] = g_signal_new("scroll_common_textview",
								    G_OBJECT_CLASS_TYPE(object_class),
								    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
								    G_STRUCT_OFFSET(RemarkEntryClass, scroll_common_textview),
								    NULL, NULL,
								    g_cclosure_marshal_VOID__INT,
								    G_TYPE_NONE, 1, G_TYPE_INT);

        remark_entry_signals[COMPLETE_NICK] = g_signal_new("complete_nick",
						           G_OBJECT_CLASS_TYPE(object_class),
							   G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
							   G_STRUCT_OFFSET(RemarkEntryClass, complete_nick),
							   NULL, NULL,
							   g_cclosure_marshal_VOID__VOID,
							   G_TYPE_NONE, 0);

	binding_set = gtk_binding_set_by_class(klass);

	gtk_binding_entry_add_signal(binding_set, GDK_Up, 0,
				     "call_history", 1,
				     G_TYPE_INT, -1);
	gtk_binding_entry_add_signal(binding_set, GDK_Down, 0,
				     "call_history", 1,
				     G_TYPE_INT, 1);
	gtk_binding_entry_add_signal(binding_set, GDK_Page_Up, 0,
				     "scroll_channel_textview", 1,
				     G_TYPE_INT, -1);
	gtk_binding_entry_add_signal(binding_set, GDK_Page_Down, 0,
				     "scroll_channel_textview", 1,
				     G_TYPE_INT, 1);
	gtk_binding_entry_add_signal(binding_set, GDK_Page_Up, GDK_MOD1_MASK,
				     "scroll_common_textview", 1,
				     G_TYPE_INT, -1);
	gtk_binding_entry_add_signal(binding_set, GDK_Page_Down, GDK_MOD1_MASK,
				     "scroll_common_textview", 1,
				     G_TYPE_INT, 1);
	gtk_binding_entry_add_signal(binding_set, GDK_Tab, 0,
	                             "complete_nick", 0);
}
static void
remark_entry_init(RemarkEntry *remark_entry)
{
	RemarkEntryPrivate *priv;

	priv = g_new0(RemarkEntryPrivate, 1);

	priv->is_multiline = FALSE;
	priv->current_index = 0;

	remark_entry->priv = priv;
}
static void
remark_entry_finalize(GObject *object)
{
	RemarkEntry *remark_entry;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(object));

        remark_entry = REMARK_ENTRY(object);

        if(G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(remark_entry->priv);
}
static void
remark_entry_destroy(GtkObject *object)
{
        RemarkEntry *remark_entry;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(object));

        remark_entry = REMARK_ENTRY(object);

        if(GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
remark_entry_grab_focus(GtkWidget *widget)
{
	RemarkEntry *entry;
	RemarkEntryPrivate *priv;

        g_return_if_fail(widget != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(widget));

	entry = REMARK_ENTRY(widget);
	priv = entry->priv;

	if(remark_entry_get_multiline(entry))
		gtk_widget_grab_focus(priv->textview);
	else {
		gtk_widget_grab_focus(entry->entry);
		gtk_editable_select_region(GTK_EDITABLE(entry->entry), -1, -1);
	}
}

GtkWidget*
remark_entry_new(LoquiApp *app, GtkToggleAction *toggle_command_action)
{
        RemarkEntry *remark_entry;
	RemarkEntryPrivate *priv;
	GtkWidget *hbox;
	GtkWidget *image;
	gchar *text;

	GtkWidget *scwin;

	remark_entry = g_object_new(remark_entry_get_type(), NULL);

	priv = remark_entry->priv;
	priv->app = app;

	g_object_ref(toggle_command_action);
	priv->toggle_command_action = toggle_command_action;

	hbox = GTK_WIDGET(remark_entry);

	priv->toggle_command = gtk_toggle_button_new();
	gtk_action_connect_proxy(GTK_ACTION(toggle_command_action), priv->toggle_command);
	gtkutils_bin_remove_child_if_exist(GTK_BIN((priv->toggle_command)));
	gtk_button_set_focus_on_click(GTK_BUTTON(priv->toggle_command), FALSE);
	g_object_get(G_OBJECT(toggle_command_action), "tooltip", &text, NULL);
	gtk_tooltips_set_tip(app->tooltips, priv->toggle_command, text, NULL);

	image = gtk_image_new_from_stock(LOQUI_STOCK_COMMAND, GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(priv->toggle_command), image);
	gtk_widget_show(image);

	gtk_button_set_relief(GTK_BUTTON(priv->toggle_command), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_command, FALSE, FALSE, 0);

	priv->vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), priv->vbox, TRUE, TRUE, 0);

	remark_entry->entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(remark_entry->entry), "activate",
			 G_CALLBACK(remark_entry_activate_cb), remark_entry);
	g_signal_connect(G_OBJECT(remark_entry->entry), "key_press_event",
			 G_CALLBACK(remark_entry_textview_or_entry_key_press_event_cb), remark_entry);
	g_signal_connect(G_OBJECT(remark_entry->entry), "show",
			 G_CALLBACK(remark_entry_entry_text_shown_cb), remark_entry);
	g_signal_connect(G_OBJECT(remark_entry->entry), "changed",
			 G_CALLBACK(remark_entry_entry_changed_cb), remark_entry);

	gtk_box_pack_start(GTK_BOX(priv->vbox), remark_entry->entry, TRUE, TRUE, 0);

	priv->hbox_text = gtk_hbox_new(FALSE, 0);
	g_signal_connect(G_OBJECT(priv->hbox_text), "show",
			 G_CALLBACK(remark_entry_entry_text_shown_cb), remark_entry);
	gtk_box_pack_start(GTK_BOX(priv->vbox), priv->hbox_text, TRUE, TRUE, 2);

	scwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(priv->hbox_text), scwin, TRUE, TRUE, 0);

	priv->textview = gtk_text_view_new();
	g_signal_connect(G_OBJECT(priv->textview), "key_press_event",
			 G_CALLBACK(remark_entry_textview_or_entry_key_press_event_cb), remark_entry);
	gtk_container_add(GTK_CONTAINER(scwin), priv->textview);

	image = gtk_image_new_from_stock(GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON);
	priv->button_ok = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(priv->button_ok), image);
	g_signal_connect(G_OBJECT(priv->button_ok), "clicked",
			 G_CALLBACK(remark_entry_ok_clicked_cb), remark_entry);
	g_signal_connect_swapped(G_OBJECT(priv->button_ok), "clicked",
				 G_CALLBACK(remark_entry_grab_focus), remark_entry);
	gtk_tooltips_set_tip(app->tooltips, priv->button_ok, _("Send message (Shift+Enter)"), NULL);
	gtk_box_pack_start(GTK_BOX(priv->hbox_text), priv->button_ok, FALSE, FALSE, 0);


	image = gtk_image_new_from_stock(LOQUI_STOCK_NOTICE, GTK_ICON_SIZE_BUTTON);
	priv->button_notice = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(priv->button_notice), image);
	g_signal_connect(G_OBJECT(priv->button_notice), "clicked",
			 G_CALLBACK(remark_entry_notice_clicked_cb), remark_entry);
	gtk_tooltips_set_tip(app->tooltips, priv->button_notice, _("Send message with NOTICE command (Ctrl+Enter)"), NULL);
	gtk_button_set_focus_on_click(GTK_BUTTON(priv->button_notice), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), priv->button_notice, FALSE, FALSE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_BUTTON);
	priv->toggle_multiline = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(priv->toggle_multiline), image);
	priv->toggle_multiline_toggled_id = g_signal_connect(G_OBJECT(priv->toggle_multiline), "toggled",
						  	     G_CALLBACK(remark_entry_entry_multiline_toggled_cb), remark_entry);
	gtk_button_set_focus_on_click(GTK_BUTTON(priv->toggle_multiline), FALSE);
	gtk_tooltips_set_tip(app->tooltips, priv->toggle_multiline, _("Toggle multiline mode"), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_multiline, FALSE, FALSE, 0);

	/* TODO: color palette
	image = gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_BUTTON);
	priv->toggle_palette = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(priv->toggle_palette), image);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_palette, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(priv->toggle_palette, FALSE);
	*/

	priv->string_list = g_list_prepend(priv->string_list, NULL);

	return GTK_WIDGET(remark_entry);
}

G_CONST_RETURN gchar *
remark_entry_get_text(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	const gchar *str = NULL;

        g_return_val_if_fail(entry != NULL, NULL);
        g_return_val_if_fail(IS_REMARK_ENTRY(entry), NULL);

	priv = entry->priv;
	if(priv->is_multiline) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		str = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
	} else {
		str = gtk_entry_get_text(GTK_ENTRY(entry->entry));
	}

	return str;
}
void
remark_entry_clear_text(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	gtk_entry_set_text(GTK_ENTRY(entry->entry), "");
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview)), "", 0);
}

void
remark_entry_set_multiline(RemarkEntry *entry, gboolean is_multiline)
{
	RemarkEntryPrivate *priv;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	const gchar *str;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	if(priv->is_multiline == is_multiline)
		return;

	priv->is_multiline = is_multiline;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
	if(is_multiline) {
		str = gtk_entry_get_text(GTK_ENTRY(entry->entry));
		gtk_text_buffer_set_text(buffer, str, -1);
		gtk_widget_hide(entry->entry);
		gtk_widget_show_all(priv->hbox_text);
	} else {
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		str = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
		gtk_entry_set_text(GTK_ENTRY(entry->entry), str);
		gtk_widget_hide_all(priv->hbox_text);
		gtk_widget_show(entry->entry);
	}

	gtkutils_toggle_button_with_signal_handler_blocked(GTK_TOGGLE_BUTTON(priv->toggle_multiline),
							   priv->toggle_multiline_toggled_id,
							   is_multiline);
	gtk_widget_grab_focus(GTK_WIDGET(entry));
}
gboolean
remark_entry_get_multiline(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;

        g_return_val_if_fail(entry != NULL, FALSE);
        g_return_val_if_fail(IS_REMARK_ENTRY(entry), FALSE);

	priv = entry->priv;

	return priv->is_multiline;
}
void
remark_entry_set_command_mode(RemarkEntry *entry, gboolean command_mode)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	gtk_toggle_action_set_active(priv->toggle_command_action, command_mode);
}
gboolean
remark_entry_get_command_mode(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;

        g_return_val_if_fail(entry != NULL, FALSE);
        g_return_val_if_fail(IS_REMARK_ENTRY(entry), FALSE);

	priv = entry->priv;

	return gtk_toggle_action_get_active(priv->toggle_command_action);
}

static void
remark_entry_history_add(RemarkEntry *entry, const gchar *str)
{
	RemarkEntryPrivate *priv;
	gint diff;
	GList *cur, *prev;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	priv->string_list = g_list_insert(priv->string_list, g_strdup(str), 1);
	diff = (gint) g_list_length(priv->string_list) - (gint) loqui_pref_get_with_default_integer(loqui_get_general_pref(),
												    LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "RemarkHistoryNumber",
												    LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_REMARK_HISTORY_NUMBER, NULL) - 1;
	if(diff > 0) {
		cur = g_list_last(priv->string_list);
		while(cur && diff > 0) {
			prev = cur->prev;
			g_free((gchar *) cur->data);
			priv->string_list = g_list_delete_link(priv->string_list, cur);
			diff--;
			cur = prev;
		}
	}

}
static void
remark_entry_call_history(RemarkEntry *entry, gint count)
{
	RemarkEntryPrivate *priv;
	gint length, dest;
	GList *cur;
	gchar *new_str;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	length = g_list_length(priv->string_list);

	if (priv->current_index >= length)
		return;

	dest = priv->current_index - count;
	if (dest < 0 || length <= dest)
		return;

	cur = g_list_nth(priv->string_list, priv->current_index);
	if(cur->data)
		g_free(cur->data);
	cur->data = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry->entry)));

	priv->current_index = dest;
	new_str = g_list_nth_data(priv->string_list, dest);
	gtk_entry_set_text(GTK_ENTRY(entry->entry), new_str == NULL ? "" : new_str);
}

static GList *
remark_entry_find_completion_matches(RemarkEntry *entry, gchar *word) {
	RemarkEntryPrivate *priv;
	GList *candidates = NULL, *matched = NULL;
	LoquiAccount *account;
	LoquiChannel *channel;
	GList *cur;
	gchar *text;
	int i;

	g_return_if_fail(entry != NULL);
	g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	// at first, collect all candidate

	// channel names
	account = loqui_app_get_current_account(priv->app);
	if (account) {
		GList *channel_list = loqui_account_get_channel_list(account);
		for (cur = channel_list; cur != NULL; cur = cur->next) {
			gchar *name;
			g_object_get(G_OBJECT(cur->data), "name", &name, NULL);
			candidates = g_list_append(candidates, g_strdup(name));
		}
	}

	// nicks on the current channel
	channel = loqui_app_get_current_channel(priv->app);
	if (channel) {
		LoquiMember *member;
		gint num = loqui_channel_entry_get_member_number(LOQUI_CHANNEL_ENTRY(channel));
		for(i = 0; i < num; i++) {
			member = loqui_channel_entry_get_nth_member(LOQUI_CHANNEL_ENTRY(channel), i);
			candidates = g_list_append(candidates, g_strdup(loqui_user_get_nick(loqui_member_get_user(member))));
		}
	}

	for (cur = candidates; cur != NULL; cur = cur->next) {
		text = cur->data;
		if (strncmp(word, text, strlen(word)) == 0) {
			matched = g_list_append(matched, g_strdup(text));
		}
	}

	g_list_foreach(candidates, (GFunc) g_free, NULL);
	g_list_free(candidates);

	return matched;
}

static gchar*
remark_entry_find_common_prefix(GList *matched) {
	gchar *prefix;
	gchar *p1, *p2;
	gchar *lastcmp;
	gchar *old_prefix;
	gunichar u1, u2;
	gint len;
	GList *cur;

	if (matched == NULL) {
		return NULL;
	}

	if (matched->next == NULL) {
		return g_strdup(matched->data);
	}

	prefix = g_strdup(matched->data);
	for (cur = matched->next; cur != NULL; cur = cur->next) {
		lastcmp = NULL;
		p1 = prefix;
		p2 = cur->data;
		while (TRUE) {
			u1 = g_utf8_get_char(p1);
			u2 = g_utf8_get_char(p2);

			lastcmp = p1;
			if (u1 != u2) {
				break;
			}

			p1 = g_utf8_next_char(p1);
			p2 = g_utf8_next_char(p2);
			if (p1 == '\0' || p2 == '\0') {
				break;
			}
		}

		if (lastcmp == prefix) {
			g_free(prefix);
			return NULL;
		}

		len = lastcmp - prefix;
		if (len < strlen(prefix)) {
			old_prefix = prefix;
			prefix = g_strndup(old_prefix, len);
			g_free(old_prefix);
		}
	}

	return prefix;
}

static void
remark_entry_complete_nick(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;
	gint position;
	gchar *text_before, *text_after, *text_new, *p;
	GRegex *regex;
	GMatchInfo *match_info;
	GList *matched, *cur;
	int len;
	gchar *word;
	gchar *common_prefix;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	position = gtk_editable_get_position(GTK_EDITABLE(entry->entry));
	text_before = gtk_editable_get_chars(GTK_EDITABLE(entry->entry), 0, position);
	text_after = gtk_editable_get_chars(GTK_EDITABLE(entry->entry), position, -1);

	// TODO: cache?
	regex = g_regex_new("[^\\s]*\\Z", 0, 0, NULL);
	g_regex_match(regex, text_before, 0, &match_info);
	if (!g_match_info_matches(match_info)) {
		g_warning("Failed to match completion regex.");
		g_free(text_before); g_free(text_after);
		return;
	}

	word = g_match_info_fetch(match_info, 0);
	matched = remark_entry_find_completion_matches(entry, word);
	len = g_list_length(matched);

	common_prefix = remark_entry_find_common_prefix(matched);
	if (common_prefix) {
		p = text_before + strlen(text_before) - strlen(word);
		*p = '\0';

		text_new = g_strconcat(text_before, common_prefix, text_after, NULL);
		gtk_entry_set_text(GTK_ENTRY(entry->entry), text_new);
		g_free(text_new);

		gtk_editable_set_position(GTK_EDITABLE(entry->entry),
		           g_utf8_strlen(text_before, -1) + g_utf8_strlen(matched->data, -1));

		g_free(common_prefix);
	}

	// set to statusbar if multiple entries exist.
	if (len > 1) {
		gboolean is_first = TRUE;
		gchar *status_text;
		GString *string = g_string_new(_("Completion: "));

		for (cur = matched; cur != NULL; cur = cur->next) {
			if (!is_first) {
				g_string_append(string, ", ");
			}
			g_string_append(string, cur->data);
			is_first = FALSE;
		}

		status_text = g_string_free(string, FALSE);
		loqui_statusbar_set_completion(LOQUI_STATUSBAR(priv->app->statusbar), status_text);
		g_free(status_text);
	} else {
		loqui_statusbar_set_completion(LOQUI_STATUSBAR(priv->app->statusbar), NULL);
	}

	g_list_foreach(matched, (GFunc) g_free, NULL);
	g_list_free(matched);
      	g_free(word);

	g_match_info_free(match_info);
	g_regex_unref(regex);
	g_free(text_before);
	g_free(text_after);
}

static void
remark_entry_scroll_channel_textview(RemarkEntry *entry, gint pages)
{
	RemarkEntryPrivate *priv;
	GtkWidget *chview;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;
	chview = loqui_app_get_current_channel_text_view(priv->app);

	if (chview)
		loqui_channel_text_view_scroll(LOQUI_CHANNEL_TEXT_VIEW(chview), GTK_MOVEMENT_PAGES, pages);
}
static void
remark_entry_scroll_common_textview(RemarkEntry *entry, gint pages)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	loqui_channel_text_view_scroll(LOQUI_CHANNEL_TEXT_VIEW(priv->app->common_textview), GTK_MOVEMENT_PAGES, pages);
}
static void
remark_entry_send_text(RemarkEntry *remark_entry, gboolean is_notice)
{
	RemarkEntryPrivate *priv;
	gchar *str, *cur;
	LoquiAccount *account;
	LoquiChannel *channel;
	gchar *command_prefix;

        g_return_if_fail(remark_entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(remark_entry));

	priv = remark_entry->priv;

	if (strlen(remark_entry_get_text(remark_entry)) == 0)
		return;

	str = g_strdup(remark_entry_get_text(remark_entry));

	account = loqui_app_get_current_account(priv->app);
	if (!account) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected an account"));
		g_free(str);
		return;
	}

	if(!loqui_account_get_is_connected(account)) {
		gtkutils_msgbox_info(GTK_MESSAGE_WARNING,
				     _("Not connected with this account"));
		g_free(str);
		return;
	}

	cur = str;
	if (remark_entry_get_command_mode(remark_entry)) {
		if (strchr(cur, '\n')) {
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
					     _("Command contains linefeed."));
			return;
		}

		command_prefix = loqui_pref_get_with_default_string(loqui_get_general_pref(),
								    LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "CommandPrefix",
								    LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_COMMAND_PREFIX, NULL);
		if (g_str_has_prefix(cur, command_prefix))
			cur += strlen(command_prefix);
		g_free(command_prefix);

		if (!LOQUI_IS_SENDER_IRC(account->sender)) {
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
					     _("Using not IRC account"));
			return;
		}
		loqui_sender_irc_send_raw(LOQUI_SENDER_IRC(account->sender), cur);
	} else {
		channel = loqui_app_get_current_channel(priv->app);
		if(channel == NULL) {
			gtkutils_msgbox_info(GTK_MESSAGE_WARNING,
					     _("No channel is selected"));
			return;
		}

		if (is_notice)
			loqui_sender_notice(account->sender, channel, str);
		else
			loqui_sender_say(account->sender, channel, str);
	}

	remark_entry_clear_text(remark_entry);

	remark_entry_history_add(remark_entry, str);
	g_free(str);
	priv->current_index = 0;
	LOQUI_G_FREE_UNLESS_NULL(priv->string_list->data);

	remark_entry_set_command_mode(remark_entry, FALSE);
}

static gboolean
remark_entry_textview_or_entry_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, RemarkEntry *remark_entry)
{
        g_return_val_if_fail(remark_entry != NULL, FALSE);
        g_return_val_if_fail(IS_REMARK_ENTRY(remark_entry), FALSE);

	/* Shift + Enter: PRIVMSG, Ctrl + Enter: NOTICE */
	if (event->state & GDK_CONTROL_MASK ||
	    event->state & GDK_SHIFT_MASK) {
		switch (event->keyval) {
		case GDK_Return:
		case GDK_KP_Enter:
			remark_entry_send_text(remark_entry, (event->state & GDK_CONTROL_MASK) != 0);
			return TRUE;
		default:
			break;
		}
	}

	return FALSE;
}

static void
remark_entry_activate_cb(GtkWidget *widget, gpointer data)
{
	remark_entry_send_text(data, FALSE);
}
static void
remark_entry_ok_clicked_cb(GtkWidget *widget, gpointer data)
{
	remark_entry_send_text(data, FALSE);
}
static void
remark_entry_notice_clicked_cb(GtkWidget *widget, gpointer data)
{
	remark_entry_send_text(data, TRUE);
}
static void
remark_entry_entry_multiline_toggled_cb(GtkWidget *widget, gpointer data)
{
        RemarkEntry *remark_entry;
	RemarkEntryPrivate *priv;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(data));

        remark_entry = REMARK_ENTRY(data);
	priv = remark_entry->priv;

	remark_entry_set_multiline(remark_entry,
				   gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}
static void
remark_entry_entry_changed_cb(GtkEntry *widget, RemarkEntry *remark_entry)
{
	RemarkEntryPrivate *priv;
	const gchar *tmp;
	gchar *command_prefix;

        g_return_if_fail(remark_entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(remark_entry));

	priv = remark_entry->priv;

	if (loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
						LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoCommandMode",
						LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_AUTO_COMMAND_MODE, NULL)) {
		command_prefix = loqui_pref_get_with_default_string(loqui_get_general_pref(),
								    LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "CommandPrefix",
								    LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_COMMAND_PREFIX, NULL);
		tmp = gtk_entry_get_text(GTK_ENTRY(remark_entry->entry));
		remark_entry_set_command_mode(remark_entry, g_str_has_prefix(tmp, command_prefix));
		g_free(command_prefix);
	}
}
static void
remark_entry_entry_text_shown_cb(GtkWidget *widget, gpointer data)
{
        RemarkEntry *remark_entry;
	RemarkEntryPrivate *priv;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(data));

        remark_entry = REMARK_ENTRY(data);
	priv = remark_entry->priv;

	if(remark_entry_get_multiline(remark_entry))
		gtk_widget_hide(remark_entry->entry);
	else
		gtk_widget_hide_all(priv->hbox_text);
}
