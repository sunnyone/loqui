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

#include "main.h"
#include "channel_tree.h"
#include "gtkutils.h"
#include "utils.h"
#include "account_manager.h"
#include "intl.h"

struct _ChannelTreePrivate
{
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

static void channel_tree_update_buffer_number(ChannelTree *tree);

enum {
	COLUMN_TEXT,
	COLUMN_BUFFER_NUMBER,
	COLUMN_ACCOUNT,
	COLUMN_CHANNEL,
	COLUMN_COLOR,
	COLUMN_USERS,
	COLUMN_OP_USERS,
	COLUMN_NUMBER
};

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

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_tree_finalize;
        gtk_object_class->destroy = channel_tree_destroy;
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
}
static void
channel_tree_row_selected_cb(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	Account *account;
	Channel *channel;
	
	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
		return;

	gtk_tree_model_get(model, &iter,
			   COLUMN_ACCOUNT, &account,
			   COLUMN_CHANNEL, &channel,
			   -1);

	if(account)
		account_manager_set_current_account(account_manager_get(), account);
	else if(channel)
		account_manager_set_current_channel(account_manager_get(), channel);
}
GtkWidget*
channel_tree_new(void)
{
        ChannelTree *tree;
	ChannelTreePrivate *priv;
	GtkTreeStore *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	tree = g_object_new(channel_tree_get_type(), NULL);

	priv = tree->priv;

        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

	model = gtk_tree_store_new(COLUMN_NUMBER, 
				   G_TYPE_STRING,
				   G_TYPE_UINT,
				   G_TYPE_POINTER,
				   G_TYPE_POINTER,
				   G_TYPE_STRING,
				   G_TYPE_UINT,
				   G_TYPE_UINT);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(model));


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
							   "text", COLUMN_BUFFER_NUMBER,
							   NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Name"),
							   renderer,
							   "text", COLUMN_TEXT,
							   "foreground", COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Users",
							   renderer,
							   "text", COLUMN_USERS,
							   NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Op",
							   renderer,
							   "text", COLUMN_OP_USERS,
							   NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	g_signal_connect(G_OBJECT(tree), "row_activated",
			 G_CALLBACK(channel_tree_row_activated_cb), NULL);
	priv->selection_changed_signal_id = g_signal_connect(G_OBJECT(selection), "changed",
							     G_CALLBACK(channel_tree_row_selected_cb), tree);

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);

	return GTK_WIDGET(tree);
}
void
channel_tree_add_account(ChannelTree *tree, Account *account)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);	
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   COLUMN_TEXT, loqui_profile_account_get_name(account_get_profile(account)), 
			   COLUMN_ACCOUNT, account,
			   COLUMN_CHANNEL, NULL,
			   -1);

	channel_tree_update_buffer_number(tree);
}
void
channel_tree_update_account(ChannelTree *tree, Account *account)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	if(!gtk_tree_model_find_by_column_data(model, &iter, NULL, COLUMN_ACCOUNT, account)) {
		return;
	}
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   COLUMN_TEXT, loqui_profile_account_get_name(account_get_profile(account)),
			   -1);
}

void
channel_tree_remove_account(ChannelTree *tree, Account *account)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	if(!gtk_tree_model_find_by_column_data(model, &iter, NULL, COLUMN_ACCOUNT, account)) {
		return;
	}
	gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

	channel_tree_update_buffer_number(tree);
}
void
channel_tree_add_channel(ChannelTree *tree, Account *account, Channel *channel)
{
	GtkTreeIter iter, parent;
	GtkTreeModel *model;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

	if(!gtk_tree_model_find_by_column_data(model, &parent, NULL, COLUMN_ACCOUNT, account)) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Failed to add channel"));
		return;
	}
	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, &parent);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   COLUMN_TEXT, channel_get_name(channel),
			   COLUMN_ACCOUNT, NULL,
			   COLUMN_CHANNEL, channel, -1);

	gtk_tree_view_expand_row(GTK_TREE_VIEW(tree), gtk_tree_model_get_path(model, &parent), TRUE);

	channel_tree_update_buffer_number(tree);
}
void
channel_tree_remove_channel(ChannelTree *tree, Channel *channel)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

	if(gtk_tree_model_find_by_column_data(model, &iter, NULL, COLUMN_CHANNEL, channel))
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

	channel_tree_update_buffer_number(tree);
}
void
channel_tree_select_channel(ChannelTree *tree, Channel *channel)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	ChannelTreePrivate *priv;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
        g_return_if_fail(channel != NULL);
        g_return_if_fail(IS_CHANNEL(channel));
	
	priv = tree->priv;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	
	g_signal_handler_block(selection, priv->selection_changed_signal_id);
	if(gtk_tree_model_find_by_column_data(model, &iter, NULL, COLUMN_CHANNEL, channel))
		gtk_tree_selection_select_iter(selection, &iter);
	g_signal_handler_unblock(selection, priv->selection_changed_signal_id);
}
void
channel_tree_select_account(ChannelTree *tree, Account *account)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	ChannelTreePrivate *priv;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = tree->priv;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

	g_signal_handler_block(selection, priv->selection_changed_signal_id);
	if(gtk_tree_model_find_by_column_data(model, &iter, NULL, COLUMN_ACCOUNT, account))
		gtk_tree_selection_select_iter(selection, &iter);
	g_signal_handler_unblock(selection, priv->selection_changed_signal_id);

}
void
channel_tree_update_user_number(ChannelTree *tree, Channel *channel)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	guint users;
	guint op_users;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
        g_return_if_fail(channel != NULL);
        g_return_if_fail(IS_CHANNEL(channel));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

	if(!gtk_tree_model_find_by_column_data(model, &iter, NULL, COLUMN_CHANNEL, channel))
		return;
	
	channel_get_user_number(channel, &users, &op_users);

	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   COLUMN_USERS, users,
			   COLUMN_OP_USERS, op_users,
			   -1);
}
void
channel_tree_set_updated(ChannelTree *tree, Account *account, Channel *channel)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *color;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

	/* FIXME: handling account */
	if(account) {
		return;
	}

	if(channel) {
		if(!gtk_tree_model_find_by_column_data(model, &iter, NULL, COLUMN_CHANNEL, channel))
			return;

		if(channel_get_updated(channel))
			color = FRESH_COLOR;
		else
			color = NONFRESH_COLOR;

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				   COLUMN_COLOR, color, -1);
	}
}

void
channel_tree_select_prev_channel(ChannelTree *tree, gboolean require_updated)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter, parent;
	GtkTreePath *path;
	Channel *channel, *selected_channel;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_tree_model_get_iter_first(model, &iter);

	gtk_tree_model_get(model, &iter, COLUMN_CHANNEL, &selected_channel, -1);
	path = gtk_tree_model_get_path(model, &iter);

	if(!selected_channel) {
	retry:
		if(!gtk_tree_path_prev(path)) /* select previous account */
			return;
		if(!gtk_tree_model_get_iter(model, &parent, path))
			return;
		gtk_tree_path_free(path);

		do {
			if(gtk_tree_model_iter_children(model, &iter, &parent))
				break;
		} while(gtk_tree_model_iter_next(model, &parent));
		while(gtk_tree_model_iter_next(model, &iter));

		path = gtk_tree_model_get_path(model, &iter);
	}

	do {
		if(!gtk_tree_model_get_iter(model, &iter, path))
			break;
		
		gtk_tree_model_get(model, &iter, COLUMN_CHANNEL, &channel, -1);
		if(!channel)
			continue;
		if(channel == selected_channel)
			continue;
		if(require_updated && !channel_get_updated(channel))
			continue;

		gtk_tree_selection_select_iter(selection, &iter);
		gtk_tree_path_free(path);
		return;
	} while(gtk_tree_path_prev(path));

	if(gtk_tree_path_up(path))
		goto retry;

	gtk_tree_path_free(path);
}

void
channel_tree_select_next_channel(ChannelTree *tree, gboolean require_updated)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter parent, iter;
	Channel *channel, *selected_channel;
	gboolean is_found;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_tree_model_get_iter_first(model, &iter);

	gtk_tree_model_get(model, &iter, COLUMN_CHANNEL, &selected_channel, -1);

	if(!selected_channel) {
		parent = iter;
	retry:
		do {
			if((is_found = gtk_tree_model_iter_children(model, &iter, &parent)))
				break;
		} while(gtk_tree_model_iter_next(model, &parent));
		if(!is_found)
			return;
	}

	do {
		gtk_tree_model_get(model, &iter, COLUMN_CHANNEL, &channel, -1);
		if(!channel)
			continue;
		if(selected_channel == channel)
			continue;
		if(require_updated && !channel_get_updated(channel))
			continue;

		gtk_tree_selection_select_iter(selection, &iter);
		return;
	} while(gtk_tree_model_iter_next(model, &iter));

	if(gtk_tree_model_iter_parent(model, &parent, &iter)) {
		if(!gtk_tree_model_iter_next(model, &parent))
			return;
		goto retry;
	}
}
static void
channel_tree_update_buffer_number(ChannelTree *tree)
{
	GtkTreeModel *model;
	GtkTreeIter tmp, iter;
	gboolean is_enter_child = FALSE;
	int i;

        g_return_if_fail(tree != NULL);
        g_return_if_fail(IS_CHANNEL_TREE(tree));
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	i = 0;
	do {
		do {
			is_enter_child = FALSE;
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COLUMN_BUFFER_NUMBER, i, -1);
			if(gtk_tree_model_iter_children(model, &tmp, &iter)) {
				iter = tmp;
				is_enter_child = TRUE;
			}
			tmp = iter;
			i++;
		} while(is_enter_child || gtk_tree_model_iter_next(model, &iter));
		if(!gtk_tree_model_iter_parent(model, &iter, &tmp))
			break;
	} while(gtk_tree_model_iter_next(model, &iter));
}
