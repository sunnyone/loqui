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

/* Copyright (C) 2002-2003  Takuo KITAME
   gtk_tree_model_find_by_column_data() is written by him. */

#include "config.h"
#include "gtkutils.h"
#include <stdarg.h>

void
gtkutils_msgbox_info(GtkMessageType icon, const gchar *format, ...)
{
	GtkWidget *dialog;
	va_list args;
	gchar *buf;
	
	va_start(args, format);
	buf = g_strdup_vprintf(format, args);
	va_end(args);

	dialog = gtk_message_dialog_new(NULL,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					icon,
					GTK_BUTTONS_CLOSE,
					"%s", buf);
	
	g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  GTK_OBJECT (dialog));
	
	gtk_widget_show_all(dialog);

	g_free(buf);
}

void
gtkutils_add_label_entry(GtkWidget *box, const gchar *label_text,
			      GtkWidget **entry, const gchar *default_string)
{
	GtkWidget *hbox;
	GtkWidget *label;

	g_return_if_fail(entry != NULL);
	g_return_if_fail(label_text != NULL);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);

	label = gtk_label_new(label_text);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);

	*entry = gtk_entry_new();
	if(default_string != NULL)
		gtk_entry_set_text(GTK_ENTRY(*entry), default_string);
	gtk_box_pack_start(GTK_BOX(hbox), *entry, TRUE, TRUE, 0);
}

gboolean
gtk_tree_model_find_by_column_data (GtkTreeModel * model, GtkTreeIter * iter,
				    GtkTreeIter * start, gint col,
				    gpointer data)
{
	gpointer stock;
	gboolean valid = TRUE;
	
	if (start == NULL) {
		valid = gtk_tree_model_get_iter_first(model, iter);
	} else {
		valid = gtk_tree_model_iter_children(model, iter, start);
	}
	
	while (valid) {
		gtk_tree_model_get(model, iter, col, &stock, -1);
		if (stock && stock == data) {
			return valid;
		}
		if (gtk_tree_model_iter_has_child(model, iter)) {
			GtkTreeIter toget;
			valid = gtk_tree_model_find_by_column_data (model, &toget, iter,
								    col, data);
			if (valid) {
				*iter = toget;
				return valid;
			}
		}
		valid = gtk_tree_model_iter_next (model, iter);
	}
	
	return FALSE;
}
