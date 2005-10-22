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
/* Copyright (C) 2002 Marco Pesenti Gritti
   gtkut_menu_position_under_widget() (gul_gui_menu_position_under_widget) is written by him. */
/* Copyright (C) 1999-2005 Hiroyuki Yamamoto
   gtkut_get_default_font_desc() */

#include "config.h"
#include "gtkutils.h"
#include <stdarg.h>
#include <string.h>
#include <glib/gi18n.h>
#include <libloqui/loqui-utils.h>

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
void
gtkutils_add_label_spin_button(GtkWidget *box, const gchar *label_text,
			       GtkWidget **spin, gdouble min, gdouble max, gdouble step)
{
	GtkWidget *hbox;
	GtkWidget *label;

	g_return_if_fail(spin != NULL);
	g_return_if_fail(label_text != NULL);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);

	label = gtk_label_new(label_text);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);

	*spin = gtk_spin_button_new_with_range(min, max, step);
	gtk_box_pack_start(GTK_BOX(hbox), *spin, FALSE, FALSE, 0);
}

void
gtkutils_toggle_button_with_signal_handler_blocked(GtkToggleButton *button, guint signal_handler_id, gboolean bool)
{
	g_signal_handler_block(button, signal_handler_id);
	gtk_toggle_button_set_active(button, bool);
	g_signal_handler_unblock(button, signal_handler_id);
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

gchar *
gtkutils_menu_translate(const gchar *path, gpointer data)
{
	return gettext(path);
}

void
gtkutils_exec_command_argument_with_error_dialog(const gchar *command, const gchar *argument)
{
	gchar *buf, *p;
	gchar *quoted;

	g_return_if_fail(command != NULL);
	g_return_if_fail(argument != NULL);

	if(!( (p = strchr(command, '%')) && *(p + 1) == 's' && 
	      !strchr(p + 2, '%') ))
                gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Illegal command (make sure one %%s exists)"));

	quoted = g_shell_quote(argument);
	loqui_debug_puts("Quoted: %s", quoted);
	buf = g_strdup_printf(command, quoted);
	g_free(quoted);

	gtkutils_exec_command_with_error_dialog(buf);
	g_free(buf);
}
void
gtkutils_exec_command_with_error_dialog(const gchar *command)
{
	GError *error = NULL;

	g_return_if_fail(command != NULL);

	loqui_debug_puts("Execute: %s", command);
	g_spawn_command_line_async(command, &error);
	if(error != NULL) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Error occured when executing command, \"%s\":\n%s"),
				     error->message, command);
		g_error_free(error);
	}
}
void
gtkutils_set_label_color(GtkLabel *label, const gchar *color)
{
	PangoAttrList *pattr_list;
	PangoAttribute *pattr;
	PangoColor pcolor;

	if(!pango_color_parse(&pcolor, color)) {
		g_warning(_("Unable to determine color: %s"), color);
		return;
	}
	
	pattr = pango_attr_foreground_new(pcolor.red,
					  pcolor.green,
					  pcolor.blue);

	pattr->start_index = 0;
	pattr->end_index = G_MAXUINT;
	pattr_list = pango_attr_list_new();
	pango_attr_list_insert(pattr_list, pattr);

	gtk_label_set_use_markup(label, FALSE);
	gtk_label_set_use_underline(label, FALSE);
	gtk_label_set_attributes(label, pattr_list);
	/* pango_attr_list_unref(pattr_list);
	   pango_attribute_destroy(pattr); */
}
void
gtkutils_set_textview_from_string_list(GtkTextView *textview, GList *list)
{
	GtkTextBuffer *buffer;
	gchar *buf;

	g_return_if_fail(textview != NULL);
	g_return_if_fail(GTK_IS_TEXT_VIEW(textview));

	buffer = gtk_text_view_get_buffer(textview);
	if(list) {
		buf = loqui_utils_line_separated_text_from_list(list);
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), buf, -1);
		g_free(buf);
	} else {
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), "", -1);
	}
}
void
gtkutils_set_string_list_from_textview(GList **list_ptr, GtkTextView *textview)
{
	gchar *buf;

	g_return_if_fail(list_ptr != NULL);
	g_return_if_fail(textview != NULL);
	g_return_if_fail(GTK_IS_TEXT_VIEW(textview));

	buf = gtkutils_get_text_from_textview(textview);

	LOQUI_G_LIST_FREE_WITH_ELEMENT_FREE_UNLESS_NULL(*list_ptr);
	*list_ptr = loqui_utils_line_separated_text_to_list(buf);
	g_free(buf);
}
gchar *
gtkutils_get_text_from_textview(GtkTextView *textview)
{
	gchar *buf;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	g_return_val_if_fail(textview != NULL, NULL);

	buffer = gtk_text_view_get_buffer(textview);
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end);
	buf = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE);

	return buf;
}
GtkWidget *
gtkutils_create_framed_textview(GtkWidget **textview_ptr, const gchar *frame_label)
{
	GtkWidget *frame;
	GtkWidget *scrolled_win;
	
	g_return_val_if_fail(textview_ptr != NULL, NULL);

	frame = gtk_frame_new(frame_label);

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(frame), scrolled_win);

	*textview_ptr = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrolled_win), *textview_ptr);

	return frame;
}
void
gtkutils_get_current_font_pixel_size(GtkWidget *widget, gint *width, gint *height)
{
	PangoLayout *layout;

	layout = gtk_widget_create_pango_layout(widget, NULL);
	pango_layout_get_pixel_size(layout, width, height);
	g_object_unref(layout);
}
gboolean
gtkutils_bindings_has_matched_entry(const gchar *class_name, guint modifiers, guint keyval)
{
	GtkBindingSet *bset;
	GtkBindingEntry *bentry;
	gboolean found = FALSE;
	
	bset = gtk_binding_set_find(class_name);
	bentry = bset->entries;
	while (bentry != NULL) {
		if (keyval == bentry->keyval &&
		    modifiers == bentry->modifiers) {
			found = TRUE;
			break;
		}
		bentry = bentry->set_next;
	}
	return found;
}
void
gtkutils_tree_view_popup(GtkTreeView *tree, GdkEventButton *event, GtkMenu *menu)
{
	GtkTreePath *clicked_path;
	GtkTreeSelection *selection;
	GList *row_list = NULL, *search_list;

	if (!gtk_tree_view_get_path_at_pos(tree, event->x, event->y,
					   &clicked_path, NULL, NULL, NULL)) {
		return;
	}
	
	selection = gtk_tree_view_get_selection(tree);
	row_list = gtk_tree_selection_get_selected_rows(selection, NULL);
	
	search_list = g_list_find_custom(row_list,
					 clicked_path,
					 (GCompareFunc) gtk_tree_path_compare);
	if (!search_list) {
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(selection, clicked_path);
	}
	g_list_foreach(row_list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(row_list);
	
	gtk_tree_path_free(clicked_path);

	gtk_menu_popup(menu, NULL, NULL, NULL,
		       tree, event->button, event->time);
}
gboolean
gtkutils_widget_is_iconified(GtkWidget *widget)
{
	return gdk_window_get_state(widget->window) & GDK_WINDOW_STATE_ICONIFIED;
}
void
gtkutils_bin_remove_child_if_exist(GtkBin *bin)
{
	GtkWidget *child;

	child = gtk_bin_get_child(bin);
	if (child)
		gtk_container_remove(GTK_CONTAINER(bin), child);
}

void
gtkutils_menu_position_under_widget(GtkMenu   *menu,
				    gint      *x,
				    gint      *y,
				    gboolean  *push_in,
				    gpointer   user_data)
{
        GtkWidget *w = GTK_WIDGET(user_data);
        const GtkAllocation *allocation = &w->allocation;
        gint screen_width, screen_height;
        GtkRequisition requisition;

        gdk_window_get_origin(w->window, x, y);
        *x += allocation->x;
        *y += allocation->y + allocation->height;

        gtk_widget_size_request(GTK_WIDGET (menu), &requisition);
      
        screen_width = gdk_screen_width();
        screen_height = gdk_screen_height();
          
        *x = CLAMP(*x, 0, MAX(0, screen_width - requisition.width));
        *y = CLAMP(*y, 0, MAX(0, screen_height - requisition.height));
}

/* for notification area */
void
gtkutils_menu_position_under_or_below_widget(GtkMenu   *menu,
					     gint      *x,
					     gint      *y,
					     gboolean  *push_in,
					     gpointer   user_data)
{
        GtkWidget *w = GTK_WIDGET(user_data);
        const GtkAllocation *allocation = &w->allocation;
        gint screen_width, screen_height;
        GtkRequisition requisition;

        gdk_window_get_origin(w->window, x, y);
        *x += allocation->x;
        *y += allocation->y + allocation->height;

        gtk_widget_size_request(GTK_WIDGET (menu), &requisition);
      
        screen_width = gdk_screen_width();
        screen_height = gdk_screen_height();
       
	/* lower area */
	if (*y > screen_height / 2) {
		*x = CLAMP(*x, 0, MAX(0, screen_width - requisition.width));
		*y = CLAMP(MAX(*y - allocation->height, 0), 0, MAX(0, screen_height - requisition.height - allocation->height));
	} else { /* upper area */
		*x = CLAMP(*x, 0, MAX(0, screen_width - requisition.width));
		*y = CLAMP(*y, 0, MAX(0, screen_height - requisition.height));
	}
}

PangoFontDescription *
gtkutils_get_default_font_desc(void)
{
        static PangoFontDescription *font_desc = NULL;

        if (!font_desc) {
                GtkWidget *window;

                window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
                gtk_widget_ensure_style(window);
                font_desc = pango_font_description_copy
                        (window->style->font_desc);
                gtk_object_sink(GTK_OBJECT(window));
        }

        return pango_font_description_copy(font_desc);
}
