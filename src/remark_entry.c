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
enum {
	ACTIVATE,
        LAST_SIGNAL
};

struct _RemarkEntryPrivate
{
	GtkWidget *label_nick;
	GtkWidget *vbox;
	GtkWidget *combo;
	GtkWidget *textview;
	GtkWidget *button_ok;
	GtkWidget *toggle_multiline;
	GtkWidget *toggle_palette;
};

static GtkHBoxClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_HBOX

static guint remark_entry_signals[LAST_SIGNAL] = { 0 };

static void remark_entry_class_init(RemarkEntryClass *klass);
static void remark_entry_init(RemarkEntry *remark_entry);
static void remark_entry_finalize(GObject *object);
static void remark_entry_destroy(GtkObject *object);
static void remark_entry_combo_activated_cb(GtkWidget *widget, gpointer data);

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

	remark_entry = g_object_new(remark_entry_get_type(), NULL);
	
	priv = remark_entry->priv;

	hbox = GTK_WIDGET(remark_entry);

	priv->label_nick = gtk_label_new("nick");
	gtk_box_pack_start(GTK_BOX(hbox), priv->label_nick, FALSE, FALSE, 0);

	label = gtk_label_new(">");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

	priv->vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), priv->vbox, TRUE, TRUE, 0);

	priv->combo = gtk_combo_new();
	gtk_combo_disable_activate(GTK_COMBO(priv->combo));
	g_signal_connect(G_OBJECT(GTK_COMBO(priv->combo)->entry), "activate",
			 G_CALLBACK(remark_entry_combo_activated_cb), remark_entry);
	gtk_box_pack_start(GTK_BOX(priv->vbox), priv->combo, TRUE, TRUE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_BUTTON);
	priv->toggle_multiline = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(priv->toggle_multiline), image);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_multiline, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(priv->toggle_multiline, FALSE);

	image = gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_BUTTON);
	priv->toggle_palette = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(priv->toggle_palette), image);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_palette, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(priv->toggle_palette, FALSE);

	return GTK_WIDGET(remark_entry);
}

/* FIXME */
G_CONST_RETURN gchar *remark_entry_get_text(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;

        g_return_val_if_fail(entry != NULL, NULL);
        g_return_val_if_fail(IS_REMARK_ENTRY(entry), NULL);

	priv = entry->priv;

	return gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(priv->combo)->entry));
}
void remark_entry_clear_text(RemarkEntry *entry)
{
	RemarkEntryPrivate *priv;

        g_return_if_fail(entry != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(entry));

	priv = entry->priv;

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(priv->combo)->entry), "");
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

	/* FIXME */
	gtk_widget_grab_focus(GTK_COMBO(priv->combo)->entry);
}

static void
remark_entry_combo_activated_cb(GtkWidget *widget, gpointer data)
{
        RemarkEntry *remark_entry;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_REMARK_ENTRY(data));

	remark_entry = REMARK_ENTRY(data);

	g_signal_emit(remark_entry, remark_entry_signals[ACTIVATE], 0);
}

