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

enum {
	ACTIVATE,
        LAST_SIGNAL
};

struct _RemarkEntryPrivate
{
	GList *string_list;

	gboolean is_multiline;

	GtkWidget *label_nick;
	GtkWidget *vbox;
	GtkWidget *combo;
	GtkWidget *hbox_text;
	GtkWidget *textview;
	GtkWidget *button_ok;

	GtkWidget *toggle_multiline;
	guint toggled_id;

	GtkWidget *toggle_palette;
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

static void remark_entry_history_add(RemarkEntry *entry, const gchar *str);

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

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = remark_entry_finalize;
        gtk_object_class->destroy = remark_entry_destroy;

        remark_entry_signals[ACTIVATE] = g_signal_new("activate",
						      G_OBJECT_CLASS_TYPE(object_class),
						      G_SIGNAL_RUN_FIRST,
						      0,
						      NULL, NULL,
						      g_cclosure_marshal_VOID__VOID,
						      G_TYPE_NONE, 0);

}
static void 
remark_entry_init(RemarkEntry *remark_entry)
{
	RemarkEntryPrivate *priv;

	priv = g_new0(RemarkEntryPrivate, 1);

	priv->is_multiline = FALSE;

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
remark_entry_new(void)
{
        RemarkEntry *remark_entry;
	RemarkEntryPrivate *priv;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *image;

	GtkWidget *scwin;

	remark_entry = g_object_new(remark_entry_get_type(), NULL);
	
	priv = remark_entry->priv;

	hbox = GTK_WIDGET(remark_entry);

	priv->label_nick = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), priv->label_nick, FALSE, FALSE, 0);

	label = gtk_label_new(">");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

	priv->vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), priv->vbox, TRUE, TRUE, 0);

	priv->combo = gtk_combo_new();
	gtk_combo_disable_activate(GTK_COMBO(priv->combo));
	g_signal_connect(G_OBJECT(GTK_COMBO(priv->combo)->entry), "activate",
			 G_CALLBACK(remark_entry_activated_cb), remark_entry);
	g_signal_connect(G_OBJECT(priv->combo), "show",
			 G_CALLBACK(remark_entry_entry_text_shown_cb), remark_entry);
	gtk_box_pack_start(GTK_BOX(priv->vbox), priv->combo, TRUE, TRUE, 0);

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
	priv->toggled_id = g_signal_connect(G_OBJECT(priv->toggle_multiline), "toggled",
					    G_CALLBACK(remark_entry_entry_multiline_toggled_cb), remark_entry);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_multiline, FALSE, FALSE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_BUTTON);
	priv->toggle_palette = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(priv->toggle_palette), image);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_palette, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(priv->toggle_palette, FALSE);

	priv->string_list = g_list_append(priv->string_list, NULL);

	return GTK_WIDGET(remark_entry);
}

G_CONST_RETURN gchar *remark_entry_get_text(RemarkEntry *entry)
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
		str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(priv->combo)->entry));
	}

	return str;
}
void remark_entry_clear_text(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(priv->combo)->entry), "");
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
		str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(priv->combo)->entry));
		gtk_text_buffer_set_text(buffer, str, -1);
		gtk_widget_hide(priv->combo);
		gtk_widget_show_all(priv->hbox_text);
	} else {
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		str = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(priv->combo)->entry), str);
		gtk_widget_hide_all(priv->hbox_text);
		gtk_widget_show(priv->combo);
	}

	g_signal_handler_block(priv->toggle_multiline, priv->toggled_id);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->toggle_multiline), is_multiline);
	g_signal_handler_unblock(priv->toggle_multiline, priv->toggled_id);

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
remark_entry_set_nick(RemarkEntry *entry, const gchar *nick)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;
	
	gtk_label_set_text(GTK_LABEL(priv->label_nick), nick);
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
		gtk_widget_grab_focus(GTK_COMBO(priv->combo)->entry);
		gtk_editable_select_region(GTK_EDITABLE(GTK_COMBO(priv->combo)->entry), -1, -1);
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

	gtk_combo_set_popdown_strings(GTK_COMBO(priv->combo), priv->string_list);
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

	remark_entry_history_add(remark_entry, str);
	g_free(str);
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
remark_entry_entry_text_shown_cb(GtkWidget *widget, gpointer data)
{
        RemarkEntry *remark_entry;
	RemarkEntryPrivate *priv;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(data));

        remark_entry = REMARK_ENTRY(data);
	priv = remark_entry->priv;

	if(remark_entry_get_multiline(remark_entry))
		gtk_widget_hide(priv->combo);
	else
		gtk_widget_hide_all(priv->hbox_text);
}
