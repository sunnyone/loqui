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

#include <utils.h>
#include <loqui_account_manager.h>

#include "loqui-core-gtk.h"
#include "channel_tree.h"
#include "gtkutils.h"

#include <glib/gi18n.h>
#include "loqui_account_manager_store.h"
#include "loqui_stock.h"

struct _ChannelTreePrivate
{
	LoquiApp *app;
	GtkMenu *popup_menu_account;
	GtkMenu *popup_menu_channel;
	GtkMenu *popup_menu_private_talk;

	guint selection_changed_signal_id;
};

static GtkTreeViewClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TREE_VIEW

static void channel_tree_class_init(ChannelTreeClass *klass);
static void channel_tree_init(ChannelTree *channel_tree);
static void channel_tree_finalize(GObject *object);
static void channel_tree_destroy(GtkObject *object);

static void channel_tree_row_activated_cb(ChannelTree *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data);
static void channel_tree_row_selected_cb(GtkTreeSelection *selection, gpointer data);
static gboolean channel_tree_key_press_event(GtkWidget *widget,
					     GdkEventKey *event);
static void channel_tree_cell_data_func_basic_away(GtkTreeViewColumn *tree_column,
						   GtkCellRenderer *cell,
						   GtkTreeModel *tree_model,
						   GtkTreeIter *iter,
						   gpointer data);
static gboolean
channel_tree_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, gpointer data);

GType
channel_tree_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelTreeClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_tree_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ChannelTree),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_tree_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ChannelTree",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_tree_class_init(ChannelTreeClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_tree_finalize;
        gtk_object_class->destroy = channel_tree_destroy;

	widget_class->key_press_event = channel_tree_key_press_event;
}
static void 
channel_tree_init(ChannelTree *channel_tree)
{
	ChannelTreePrivate *priv;

	priv = g_new0(ChannelTreePrivate, 1);

	channel_tree->priv = priv;
}
static void 
channel_tree_finalize(GObject *object)
{
	ChannelTree *channel_tree;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(object));

        channel_tree = CHANNEL_TREE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_tree->priv);
}
static void 
channel_tree_destroy(GtkObject *object)
{
        ChannelTree *channel_tree;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(object));

        channel_tree = CHANNEL_TREE(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void /* double click */
channel_tree_row_activated_cb(ChannelTree *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	LoquiChannelEntry *chent;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter,
			   LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_CHANNEL_ENTRY, &chent, -1);
	if (chent && LOQUI_IS_ACCOUNT(chent) && !loqui_account_get_is_connected(LOQUI_ACCOUNT(chent))) {
		loqui_account_connect(LOQUI_ACCOUNT(chent));
	}
	if (chent)
		g_object_unref(chent);

	gtk_widget_grab_focus(tree->priv->app->remark_entry);
}
static void
channel_tree_row_selected_cb(GtkTreeSelection *selection, gpointer data)
{
	ChannelTree *tree;
	ChannelTreePrivate *priv;
	GtkTreeIter iter;
	GtkTreeModel *model;
	LoquiChannelEntry *chent;
	
        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(data));

	tree = CHANNEL_TREE(data);
	priv = tree->priv;

	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
		return;

	gtk_tree_model_get(model, &iter,
			   LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_CHANNEL_ENTRY, &chent,
			   -1);

	loqui_app_set_current_channel_entry(priv->app, chent);
	g_object_unref(chent);

	gtk_widget_grab_focus(priv->app->remark_entry);
}
static gboolean
channel_tree_key_press_event(GtkWidget *widget,
			     GdkEventKey *event)
{
	loqui_app_grab_focus_if_key_unused(CHANNEL_TREE(widget)->priv->app,
					   "GtkTreeView", event);

	if (* GTK_WIDGET_CLASS(parent_class)->key_press_event)
		return (* GTK_WIDGET_CLASS(parent_class)->key_press_event)(widget, event);

	return FALSE;
}
static void
channel_tree_cell_data_func_basic_away(GtkTreeViewColumn *tree_column,
				       GtkCellRenderer *cell,
				       GtkTreeModel *tree_model,
				       GtkTreeIter *iter,
				       gpointer data)
{
	const gchar *stock_id;
	ChannelTree *tree;
	ChannelTreePrivate *priv;
	GdkPixbuf *pixbuf;

	tree = CHANNEL_TREE(data);
	priv = tree->priv;
	
	gtk_tree_model_get(tree_model, iter, LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY_STOCK_ID, &stock_id, -1);

	if (stock_id == NULL)
		g_object_set(G_OBJECT(cell), "pixbuf", NULL, NULL);
	else {
		pixbuf = gtk_widget_render_icon(GTK_WIDGET(tree), stock_id, LOQUI_ICON_SIZE_FONT, NULL);
		g_object_set(G_OBJECT(cell), "pixbuf", pixbuf, NULL);
		g_object_unref(pixbuf);
	}
}

static gboolean
channel_tree_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
        ChannelTree *tree;
	ChannelTreePrivate *priv;
	GtkTreePath *clicked_path;
	GtkTreeIter iter;
	GtkMenu *menu;
	GtkTreeModel *model;
	LoquiChannelEntry *chent;
	GtkTreeSelection *selection;

	tree = CHANNEL_TREE(widget);
	priv = tree->priv;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tree), event->x, event->y,
						   &clicked_path, NULL, NULL, NULL)) {
			return FALSE;
		}

		if (!gtk_tree_selection_path_is_selected(selection, clicked_path))
			gtk_tree_selection_select_path(selection, clicked_path);

		gtk_tree_model_get_iter(model, &iter, clicked_path);
		gtk_tree_model_get(model, &iter, LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_CHANNEL_ENTRY, &chent, -1);
		g_return_val_if_fail(chent != NULL, FALSE);
		
		if (LOQUI_IS_ACCOUNT(chent)) {
			menu = GTK_MENU(priv->popup_menu_account);
		} else if (LOQUI_IS_CHANNEL(chent)) {
			if (loqui_channel_get_is_private_talk(LOQUI_CHANNEL(chent)))
				menu = GTK_MENU(priv->popup_menu_private_talk);
			else
				menu = GTK_MENU(priv->popup_menu_channel);
		} else {
			g_warning("Unknown channel entry type");
			return FALSE;
		}
		g_object_unref(chent);

		gtk_tree_path_free(clicked_path);
		gtk_menu_popup(menu, NULL, NULL, NULL,
			       tree, event->button, event->time);
		return TRUE;
	}

	return FALSE;
}	
GtkWidget*
channel_tree_new(LoquiApp *app, GtkMenu *menu_account, GtkMenu *menu_channel, GtkMenu *menu_private_talk)
{
        ChannelTree *tree;
	ChannelTreePrivate *priv;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkCellRenderer *renderer_pb;
	GtkTreeSelection *selection;

	tree = g_object_new(channel_tree_get_type(), NULL);

	priv = tree->priv;
	priv->app = app;
	priv->popup_menu_account = menu_account;
	priv->popup_menu_channel = menu_channel;
	priv->popup_menu_private_talk = menu_private_talk;

        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("E"), renderer, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 15);
	gtk_tree_view_append_column(GTK_TREE_VIEW (tree), column);
	gtk_tree_view_set_expander_column(GTK_TREE_VIEW(tree), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("#",
							   renderer,
							   "text", LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_POSITION,
							   NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer),  "ypad", 0, NULL);
	renderer_pb = gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(renderer_pb), "stock-size", GTK_ICON_SIZE_MENU,
		     NULL);
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Name"));
	gtk_tree_view_column_pack_start(column, renderer_pb, FALSE);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_NAME);
	gtk_tree_view_column_add_attribute(column, renderer, "foreground", LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_COLOR);
	gtk_tree_view_column_set_cell_data_func(column, renderer_pb,
						channel_tree_cell_data_func_basic_away,
						tree, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Users",
							   renderer,
							   "text", LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_USERS, 
							   NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Op",
							   renderer,
							   "text", LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_OP_USERS,
							   NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	g_signal_connect(G_OBJECT(tree), "row_activated",
			 G_CALLBACK(channel_tree_row_activated_cb), NULL);
	g_signal_connect(G_OBJECT(tree), "button_press_event",
			 G_CALLBACK(channel_tree_button_press_event_cb), NULL);
	priv->selection_changed_signal_id = g_signal_connect(G_OBJECT(selection), "changed",
							     G_CALLBACK(channel_tree_row_selected_cb), tree);

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);

	return GTK_WIDGET(tree);
}

void
channel_tree_select_channel_entry(ChannelTree *tree, LoquiChannelEntry *chent)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	ChannelTreePrivate *priv;
	LoquiAccountManagerStore *store;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	priv = tree->priv;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	store = LOQUI_ACCOUNT_MANAGER_STORE(model);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	
	g_signal_handler_block(selection, priv->selection_changed_signal_id);

	if (LOQUI_IS_CHANNEL(chent)) {
		loqui_account_manager_store_get_iter_by_channel_entry(store, &iter,
								      LOQUI_CHANNEL_ENTRY(loqui_channel_get_account(LOQUI_CHANNEL(chent))));
		gtk_tree_view_expand_row(GTK_TREE_VIEW(tree), gtk_tree_model_get_path(model, &iter), TRUE);
	}

	loqui_account_manager_store_get_iter_by_channel_entry(store, &iter, chent);
	gtk_tree_selection_select_iter(selection, &iter);

	path = gtk_tree_model_get_path(model, &iter);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree), path, NULL, FALSE, 0, 0);
	gtk_tree_path_free(path);

	g_signal_handler_unblock(selection, priv->selection_changed_signal_id);
}
void
channel_tree_expand_to_channel_entry(ChannelTree *tree, LoquiChannelEntry *chent)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	ChannelTreePrivate *priv;
	LoquiAccountManagerStore *store;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	priv = tree->priv;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	store = LOQUI_ACCOUNT_MANAGER_STORE(model);
	
	loqui_account_manager_store_get_iter_by_channel_entry(store, &iter, chent);
	path = gtk_tree_model_get_path(model, &iter);
	gtk_tree_view_expand_to_path(GTK_TREE_VIEW(tree), path);
	gtk_tree_path_free(path);
}
