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
#ifndef __GTKUTILS_H__
#define __GTKUTILS_H__
#include <gtk/gtk.h>

#define GTK_EVENTS_FLUSH() \
{ \
        while (gtk_events_pending()) \
                gtk_main_iteration(); \
}

void gtkutils_msgbox_info(GtkMessageType icon, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void gtkutils_add_label_entry(GtkWidget *box, const gchar *label_text, 
			      GtkWidget **entry, const gchar *default_string);
void gtkutils_add_label_spin_button(GtkWidget *box, const gchar *label_text,
			            GtkWidget **spin, gdouble min, gdouble max, gdouble step);

void gtkutils_toggle_button_with_signal_handler_blocked(GtkToggleButton *button, guint signal_handler_id, gboolean bool);

gchar *gtkutils_menu_translate(const gchar *path, gpointer data);

void gtkutils_exec_command_argument_with_error_dialog(const gchar *command, const gchar *argument);
void gtkutils_exec_command_with_error_dialog(const gchar *command);

void gtkutils_set_label_color(GtkLabel *label, const gchar *color);

void gtkutils_set_textview_from_string_list(GtkTextView *textview, GList *list);
void gtkutils_set_string_list_from_textview(GList **list, GtkTextView *textview);
GtkWidget *gtkutils_create_framed_textview(GtkWidget **textview_ptr, const gchar *frame_label);

gboolean gtk_tree_model_find_by_column_data(GtkTreeModel * model, GtkTreeIter * iter,
					    GtkTreeIter * start, gint col,
					    gpointer data);

void gtkutils_get_current_font_pixel_size(GtkWidget *widget, gint *width, gint *height);

void gtkutils_menu_position_under_widget(GtkMenu   *menu,
					 gint      *x,
					 gint      *y,
					 gboolean  *push_in,
					 gpointer   user_data);

#endif /* __GTKUTILS_H__ */
