/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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

#include "channel_tree.h"
#include "gtkutils.h"
#include "utils.h"
#include "account_manager.h"

struct _ChannelTreePrivate
{
};

static GtkTreeViewClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TREE_VIEW

static void channel_tree_class_init(ChannelTreeClass *klass);
static void channel_tree_init(ChannelTree *channel_tree);
static void channel_tree_finalize(GObject *object);
static void channel_tree_destroy(GtkObject *object);

static void channel_tree_row_activated_cb(ChannelTree *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data);
static void channel_tree_row_selected_cb(GtkTreeSelection *selection, GtkTreeModel *model);

enum {
	COLUMN_TEXT,
	COLUMN_ACCOUNT,
	COLUMN_CHANNEL,
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
static void
channel_tree_row_activated_cb(ChannelTree *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data)
{
	debug_puts("Double clicked!");
}
static void
channel_tree_row_selected_cb(GtkTreeSelection *selection, GtkTreeModel *model)
{
	GtkTreeIter iter;
	Account *account;
	Channel *channel;

	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return;

	gtk_tree_model_get(model, &iter,
			   COLUMN_ACCOUNT, &account,
			   COLUMN_CHANNEL, &channel,
			   -1);
	account_manager_set_current(account_manager_get(), account, channel);
		
	debug_puts("Selected!");
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

        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

	model = gtk_tree_store_new(COLUMN_NUMBER, 
				   G_TYPE_STRING, 
				   G_TYPE_POINTER,
				   G_TYPE_POINTER);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(model));

        renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes ("Text",
							   renderer,
							   "text", COLUMN_TEXT,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

        renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes ("Account",
							   renderer,
							   "text", COLUMN_ACCOUNT,
							   NULL);
        gtk_tree_view_column_set_visible(column, FALSE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

        renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes ("Channel",
							   renderer,
							   "text", COLUMN_CHANNEL,
							   NULL);
        gtk_tree_view_column_set_visible(column, FALSE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	g_signal_connect(G_OBJECT(tree), "row_activated",
			 G_CALLBACK(channel_tree_row_activated_cb), NULL);
	g_signal_connect(G_OBJECT(selection), "changed",
			 G_CALLBACK(channel_tree_row_selected_cb), model);

	return GTK_WIDGET(tree);
}
void
channel_tree_add_account(ChannelTree *tree, Account *account)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	g_return_if_fail(tree != NULL);
	g_return_if_fail(account != NULL);
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);	
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   COLUMN_TEXT, account->name,
			   COLUMN_ACCOUNT, account,
			   COLUMN_CHANNEL, NULL,
			   -1);
}
void
channel_tree_add_channel(ChannelTree *tree, Account *account, Channel *channel)
{
	GtkTreeIter iter, parent;
	GtkTreeModel *model;
	gboolean is_found;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	is_found = gtk_tree_model_find_by_column_data(model, &parent, NULL, COLUMN_ACCOUNT, account);

	if(is_found) {
		gtk_tree_store_append (GTK_TREE_STORE(model), &iter, &parent);
		gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
				    COLUMN_TEXT, channel->name,
				    COLUMN_ACCOUNT, NULL,
				    COLUMN_CHANNEL, channel, -1);
	} else {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Failed to add channel"));
	}
	gtk_tree_view_expand_row(GTK_TREE_VIEW(tree), gtk_tree_model_get_path(model, &parent), TRUE);
}
	
