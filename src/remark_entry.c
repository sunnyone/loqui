/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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
#include "prefs_general.h"
#include "gdk/gdkkeysyms.h"
#include "utils.h"
#include "gtkutils.h"

#include "loqui_stock.h"

#include "main.h"

#include <string.h>


enum {
	ACTIVATE,

	CALL_HISTORY,
	SCROLL_CHANNEL_TEXTVIEW,
	SCROLL_COMMON_TEXTVIEW,

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

	GtkWidget *entry;
	GtkWidget *hbox_text;
	GtkWidget *textview;
	GtkWidget *button_ok;

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

static void remark_entry_entry_text_shown_cb(GtkWidget *widget, gpointer data);
static void remark_entry_entry_multiline_toggled_cb(GtkWidget *widget, gpointer data);
static void remark_entry_activated_cb(GtkWidget *widget, gpointer data);
static void remark_entry_entry_changed_cb(GtkEntry *widget, RemarkEntry *remark_entry);

static void remark_entry_history_add(RemarkEntry *entry, const gchar *str);

static void remark_entry_call_history(RemarkEntry *entry, gint count);
static void remark_entry_scroll_channel_textview(RemarkEntry *entry, gint pages);
static void remark_entry_scroll_common_textview(RemarkEntry *entry, gint pages);

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

        remark_entry_signals[ACTIVATE] = g_signal_new("activate",
						      G_OBJECT_CLASS_TYPE(object_class),
						      G_SIGNAL_RUN_LAST,
						      G_STRUCT_OFFSET(RemarkEntryClass, activate),
						      NULL, NULL,
						      g_cclosure_marshal_VOID__VOID,
						      G_TYPE_NONE, 0);

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

GtkWidget*
remark_entry_new(LoquiApp *app, GtkToggleAction *toggle_command_action)
{
        RemarkEntry *remark_entry;
	RemarkEntryPrivate *priv;
	GtkWidget *hbox;
	GtkWidget *image;

	GtkWidget *scwin;

	remark_entry = g_object_new(remark_entry_get_type(), NULL);
	
	priv = remark_entry->priv;
	priv->app = app;
	
	g_object_ref(toggle_command_action);
	priv->toggle_command_action = toggle_command_action;

	hbox = GTK_WIDGET(remark_entry);

	priv->toggle_command = gtk_toggle_button_new();
	gtk_action_connect_proxy(GTK_ACTION(toggle_command_action), priv->toggle_command);
	gtk_container_remove(GTK_CONTAINER(priv->toggle_command), gtk_bin_get_child(GTK_BIN(priv->toggle_command)));
	
	image = gtk_image_new_from_stock(LOQUI_STOCK_COMMAND, GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(priv->toggle_command), image);

	gtk_button_set_relief(GTK_BUTTON(priv->toggle_command), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_command, FALSE, FALSE, 0);
	
	priv->vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), priv->vbox, TRUE, TRUE, 0);
	
	priv->entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(priv->entry), "activate",
			 G_CALLBACK(remark_entry_activated_cb), remark_entry);
	g_signal_connect(G_OBJECT(priv->entry), "show",
			 G_CALLBACK(remark_entry_entry_text_shown_cb), remark_entry);
	g_signal_connect(G_OBJECT(priv->entry), "changed",
			 G_CALLBACK(remark_entry_entry_changed_cb), remark_entry);
			 
	gtk_box_pack_start(GTK_BOX(priv->vbox), priv->entry, TRUE, TRUE, 0);

	priv->hbox_text = gtk_hbox_new(FALSE, 0);
	g_signal_connect(G_OBJECT(priv->hbox_text), "show",
			 G_CALLBACK(remark_entry_entry_text_shown_cb), remark_entry);
	gtk_box_pack_start(GTK_BOX(priv->vbox), priv->hbox_text, TRUE, TRUE, 2);

	scwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(priv->hbox_text), scwin, TRUE, TRUE, 0);

	priv->textview = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scwin), priv->textview);

	image = gtk_image_new_from_stock(GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON);
	priv->button_ok = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(priv->button_ok), image);
	g_signal_connect(G_OBJECT(priv->button_ok), "clicked",
			 G_CALLBACK(remark_entry_activated_cb), remark_entry);
	g_signal_connect_swapped(G_OBJECT(priv->button_ok), "clicked",
				 G_CALLBACK(remark_entry_grab_focus), remark_entry);
	gtk_box_pack_start(GTK_BOX(priv->hbox_text), priv->button_ok, FALSE, FALSE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_BUTTON);
	priv->toggle_multiline = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(priv->toggle_multiline), image);
	priv->toggle_multiline_toggled_id = g_signal_connect(G_OBJECT(priv->toggle_multiline), "toggled",
						  	     G_CALLBACK(remark_entry_entry_multiline_toggled_cb), remark_entry);
	gtk_button_set_focus_on_click(GTK_BUTTON(priv->toggle_multiline), FALSE);

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
		str = gtk_entry_get_text(GTK_ENTRY(priv->entry));
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

	gtk_entry_set_text(GTK_ENTRY(priv->entry), "");
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
		str = gtk_entry_get_text(GTK_ENTRY(priv->entry));
		gtk_text_buffer_set_text(buffer, str, -1);
		gtk_widget_hide(priv->entry);
		gtk_widget_show_all(priv->hbox_text);
	} else {
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		str = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
		gtk_entry_set_text(GTK_ENTRY(priv->entry), str);
		gtk_widget_hide_all(priv->hbox_text);
		gtk_widget_show(priv->entry);
	}
	
	gtkutils_toggle_button_with_signal_handler_blocked(GTK_TOGGLE_BUTTON(priv->toggle_multiline),
							   priv->toggle_multiline_toggled_id,
							   is_multiline);
	remark_entry_grab_focus(entry);
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

/* FIXME: this should be done with gtk_widget_grab_focus */
void
remark_entry_grab_focus(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	if(remark_entry_get_multiline(entry))
		gtk_widget_grab_focus(priv->textview);
	else {
		gtk_widget_grab_focus(priv->entry);
		gtk_editable_select_region(GTK_EDITABLE(priv->entry), -1, -1);
	}
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
	diff = (gint) g_list_length(priv->string_list) - (gint) prefs_general.remark_history_number - 1;
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
	cur->data = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry)));
	
	priv->current_index = dest;
	new_str = g_list_nth_data(priv->string_list, dest);
	gtk_entry_set_text(GTK_ENTRY(priv->entry), new_str == NULL ? "" : new_str);
}
static void
remark_entry_scroll_channel_textview(RemarkEntry *entry, gint pages)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;
	
	loqui_app_scroll_page_channel_buffer(priv->app, pages);
}
static void
remark_entry_scroll_common_textview(RemarkEntry *entry, gint pages)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;
	
	loqui_app_scroll_page_common_buffer(priv->app, pages);
}
static void
remark_entry_activated_cb(GtkWidget *widget, gpointer data)
{
        RemarkEntry *remark_entry;
	RemarkEntryPrivate *priv;
	gchar *str;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(data));

	remark_entry = REMARK_ENTRY(data);
	priv = remark_entry->priv;

	str = g_strdup(remark_entry_get_text(remark_entry));

	g_signal_emit(remark_entry, remark_entry_signals[ACTIVATE], 0);

	if(strlen(str) > 0)
		remark_entry_history_add(remark_entry, str);
	g_free(str);
	priv->current_index = 0;
	G_FREE_UNLESS_NULL(priv->string_list->data);
	
	remark_entry_set_command_mode(remark_entry, FALSE);
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
	
        g_return_if_fail(remark_entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(remark_entry));

	priv = remark_entry->priv;
	
	if (prefs_general.auto_command_mode) {
		tmp = gtk_entry_get_text(GTK_ENTRY(priv->entry));
		if (strncmp(tmp, prefs_general.command_prefix, strlen(prefs_general.command_prefix)) == 0)
			remark_entry_set_command_mode(remark_entry, TRUE);
		else
			remark_entry_set_command_mode(remark_entry, FALSE);
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
		gtk_widget_hide(priv->entry);
	else
		gtk_widget_hide_all(priv->hbox_text);
}
