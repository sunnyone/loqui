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

#include "connect_dialog.h"
#include "account_manager.h"
#include "account.h"
#include "intl.h"

struct _ConnectDialogPrivate
{
	GtkTreeStore *tree_store;
	GtkWidget *treeview;
};

enum {
	COLUMN_USE,
	COLUMN_NAME,
	COLUMN_ACCOUNT,
	COLUMN_SERVER,
	COLUMN_ACTIVATABLE,
	COLUMN_NUMBER
};

static GtkDialogClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_DIALOG

static void connect_dialog_class_init(ConnectDialogClass *klass);
static void connect_dialog_init(ConnectDialog *connect_dialog);
static void connect_dialog_finalize(GObject *object);
static void connect_dialog_destroy(GtkObject *object);

static void connect_dialog_response_cb(GtkWidget *widget, gint response, gpointer data);

static void connect_dialog_cell_data_func_use(GtkTreeViewColumn *tree_column,
					      GtkCellRenderer *cell,
					      GtkTreeModel *tree_model,
					      GtkTreeIter *iter,
					      gpointer data);
static void connect_dialog_use_toggled_cb(GtkCellRendererToggle *cell,
					  gchar *path_str, gpointer data);

GType
connect_dialog_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ConnectDialogClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) connect_dialog_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ConnectDialog),
				0,              /* n_preallocs */
				(GInstanceInitFunc) connect_dialog_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ConnectDialog",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
connect_dialog_class_init (ConnectDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = connect_dialog_finalize;
        gtk_object_class->destroy = connect_dialog_destroy;
}
static void 
connect_dialog_init (ConnectDialog *connect_dialog)
{
	ConnectDialogPrivate *priv;

	priv = g_new0(ConnectDialogPrivate, 1);

	connect_dialog->priv = priv;
}
static void 
connect_dialog_finalize (GObject *object)
{
	ConnectDialog *connect_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CONNECT_DIALOG(object));

        connect_dialog = CONNECT_DIALOG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(connect_dialog->priv);
}
static void 
connect_dialog_destroy (GtkObject *object)
{
        ConnectDialog *connect_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CONNECT_DIALOG(object));

        connect_dialog = CONNECT_DIALOG(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

static void connect_dialog_cell_data_func_use(GtkTreeViewColumn *tree_column,
					      GtkCellRenderer *cell,
					      GtkTreeModel *tree_model,
					      GtkTreeIter *iter,
					      gpointer data)
{
	ConnectDialog *dialog;
	ConnectDialogPrivate *priv;
	Account *account;

	dialog = CONNECT_DIALOG(data);
	priv = dialog->priv;

	gtk_tree_model_get(tree_model, iter, COLUMN_ACCOUNT, &account, -1);

	if(account)
		gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(cell), FALSE);
	else
		gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(cell), TRUE);
}
static void connect_dialog_use_toggled_cb(GtkCellRendererToggle *cell,
					  gchar *path_str, gpointer data)
{
	ConnectDialog *dialog;
	ConnectDialogPrivate *priv;
	GtkTreeIter parent, iter, tmp_iter;
	gboolean is_active;

	dialog = CONNECT_DIALOG(data);
	priv = dialog->priv;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(priv->tree_store), &iter, path_str);
	gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store), &iter, COLUMN_USE, &is_active, -1);

	if(gtk_tree_model_iter_parent(GTK_TREE_MODEL(priv->tree_store), &parent, &iter)) { /* server */
		if(!gtk_tree_model_iter_children(GTK_TREE_MODEL(priv->tree_store), &tmp_iter, &parent)) {
			return;
		}
		do {
			gtk_tree_store_set(priv->tree_store, &tmp_iter, COLUMN_USE, FALSE, -1);
		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->tree_store), &tmp_iter));

		gtk_tree_store_set(priv->tree_store, &iter, COLUMN_USE, TRUE, -1);
	} else { /* account */
		gtk_tree_store_set(priv->tree_store, &iter, COLUMN_USE, !is_active, -1);
	}
}
static void
connect_dialog_response_cb(GtkWidget *widget, gint response, gpointer data)
{
	ConnectDialog *dialog;
	ConnectDialogPrivate *priv;
	GtkTreeIter parent, iter;
	Server *server = NULL;
	Account *account;
	gboolean is_use;

	dialog = CONNECT_DIALOG(widget);
	priv = dialog->priv;

	if(response != GTK_RESPONSE_OK)
		return;

	if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(priv->tree_store), &parent))
		return;

	do {
		gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store), &parent, COLUMN_USE, &is_use, -1);
		if(!is_use)
			continue;
		gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store), &parent, COLUMN_ACCOUNT, &account, -1);

		if(!gtk_tree_model_iter_children(GTK_TREE_MODEL(priv->tree_store), &iter, &parent))
			continue;
		do {
			gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store), &iter, COLUMN_USE, &is_use, -1);

			if(!is_use)
				continue;

			gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store), &iter, COLUMN_SERVER, &server, -1);
			break;
		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->tree_store), &iter));

		if(server)
			account_connect(account, server);
		else
			account_connect_default(account);
		
	} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->tree_store), &parent));
}
GtkWidget*
connect_dialog_new(void)
{
        ConnectDialog *dialog;
	ConnectDialogPrivate *priv;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeIter parent, iter;
	GtkTreeSelection *selection;
	GtkWidget *scrolled_win;
	GSList *account_list;
	GSList *cur, *tmp;
	Account *account;
	Server *server;
	gboolean is_connected;

	dialog = g_object_new(connect_dialog_get_type(), NULL);
	
	priv = dialog->priv;

	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_OK, GTK_RESPONSE_OK,
			       NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Select accounts/server to connect"));
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	g_signal_connect(G_OBJECT(dialog),
			 "response",
			 G_CALLBACK(connect_dialog_response_cb),
			 dialog);

	priv->tree_store = gtk_tree_store_new(COLUMN_NUMBER,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING,
					      G_TYPE_OBJECT,
					      G_TYPE_POINTER,
					      G_TYPE_BOOLEAN);

	
	priv->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(priv->tree_store));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->treeview));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(priv->treeview), TRUE);

	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(G_OBJECT(renderer), "toggled",
			 G_CALLBACK(connect_dialog_use_toggled_cb), dialog);
	column = gtk_tree_view_column_new_with_attributes("Use",
							  renderer,
							  "active", COLUMN_USE,
							  "activatable", COLUMN_ACTIVATABLE,
							  NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer,
						connect_dialog_cell_data_func_use,
						dialog, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name",
							  renderer,
							  "text", COLUMN_NAME,
							  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

	account_list = account_manager_get_account_list(account_manager_get());

	for(cur = account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);
		is_connected = account_is_connected(account);

		gtk_tree_store_append(priv->tree_store, &parent, NULL);
		gtk_tree_store_set(priv->tree_store, &parent,
				   COLUMN_USE, is_connected ? FALSE : account->use,
				   COLUMN_NAME, account_get_name(account),
				   COLUMN_ACCOUNT, account,
				   COLUMN_ACTIVATABLE, !is_connected,
				   -1);

		gtk_tree_store_append(priv->tree_store, &iter, &parent);
		gtk_tree_store_set(priv->tree_store, &iter,
				   COLUMN_USE, TRUE,
				   COLUMN_NAME, _("Default"),
				   COLUMN_ACTIVATABLE, TRUE,
				   -1);

		for(tmp = account->server_list; tmp != NULL; tmp = tmp->next) {
			server = (Server *) tmp->data;

			gtk_tree_store_append(priv->tree_store, &iter, &parent);
			gtk_tree_store_set(priv->tree_store, &iter,
					   COLUMN_USE, FALSE,
					   COLUMN_NAME, server->hostname,
					   COLUMN_SERVER, server,
					   COLUMN_ACTIVATABLE, TRUE,
					   -1);
		}
	}

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), scrolled_win, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(scrolled_win), priv->treeview);

	gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(dialog)->vbox));

	gtk_widget_set_usize(GTK_WIDGET(dialog), 300, 200);

	return GTK_WIDGET(dialog);
}

void
connect_dialog_open(GtkWindow *parent_window)
{
	GtkWidget *dialog;

	dialog = connect_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent_window);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}
