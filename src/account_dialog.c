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

#include "account_dialog.h"
#include "intl.h"
#include "gtkutils.h"
#include "account_manager.h"

struct _AccountDialogPrivate
{
	Account *account;

	GtkWidget *entry_name;
	GtkWidget *check_use;

	GtkWidget *entry_nick;
	GtkWidget *entry_username;
	GtkWidget *entry_realname;
	GtkWidget *entry_userinfo;
	GtkWidget *entry_autojoin;

	GtkWidget *treeview;
	GtkListStore *list_store;
};

enum {
	COLUMN_USE,
	COLUMN_HOSTNAME,
	COLUMN_PORT,
	COLUMN_PASSWORD,
	COLUMN_EDITABLE,
	COLUMN_NUMBER
};

static GtkDialogClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_DIALOG

static void account_dialog_class_init(AccountDialogClass *klass);
static void account_dialog_init(AccountDialog *account_dialog);
static void account_dialog_finalize(GObject *object);
static void account_dialog_destroy(GtkObject *object);

static void account_dialog_response_cb(GtkWidget *widget, gint response, gpointer data);

static void account_dialog_list_use_toggled_cb(GtkCellRendererToggle *cell,
					       gchar *path_str, gpointer data);
static void account_dialog_list_column_edited_cb(GtkCellRendererText *cell,
						 const gchar *path_str,
						 const gchar *new_text,
						 gpointer data);
static void account_dialog_add_cb(GtkWidget *widget, gpointer data);
static void account_dialog_remove_cb(GtkWidget *widget, gpointer data);
static void account_dialog_up_cb(GtkWidget *widget, gpointer data);
static void account_dialog_down_cb(GtkWidget *widget, gpointer data);

GType
account_dialog_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(AccountDialogClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) account_dialog_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(AccountDialog),
				0,              /* n_preallocs */
				(GInstanceInitFunc) account_dialog_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "AccountDialog",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
account_dialog_class_init(AccountDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = account_dialog_finalize;
        gtk_object_class->destroy = account_dialog_destroy;
}
static void 
account_dialog_init(AccountDialog *account_dialog)
{
	AccountDialogPrivate *priv;

	priv = g_new0(AccountDialogPrivate, 1);

	account_dialog->priv = priv;
}
static void 
account_dialog_finalize(GObject *object)
{
	AccountDialog *account_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT_DIALOG(object));

        account_dialog = ACCOUNT_DIALOG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account_dialog->priv);
}
static void 
account_dialog_destroy(GtkObject *object)
{
        AccountDialog *account_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT_DIALOG(object));

        account_dialog = ACCOUNT_DIALOG(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
account_dialog_response_cb(GtkWidget *widget, gint response, gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkTreeIter iter;
	guint port;
	gchar *hostname, *password;
	gboolean use;

	dialog = ACCOUNT_DIALOG(data);

	priv = dialog->priv;

	switch(response) {
	case GTK_RESPONSE_OK:
		account_set(priv->account,
			    gtk_entry_get_text(GTK_ENTRY(priv->entry_name)),
			    gtk_entry_get_text(GTK_ENTRY(priv->entry_nick)),
			    gtk_entry_get_text(GTK_ENTRY(priv->entry_username)),
			    gtk_entry_get_text(GTK_ENTRY(priv->entry_realname)),
			    gtk_entry_get_text(GTK_ENTRY(priv->entry_userinfo)),
			    gtk_entry_get_text(GTK_ENTRY(priv->entry_autojoin)));
		account_remove_all_server(priv->account);

		if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(priv->list_store), &iter))
			break;
		do {
			gtk_tree_model_get(GTK_TREE_MODEL(priv->list_store), &iter,
					   COLUMN_HOSTNAME, &hostname,
					   COLUMN_PORT, &port,
					   COLUMN_PASSWORD, &password,
					   COLUMN_USE, &use, -1);
			account_add_server(priv->account, hostname, port, password, use);
		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->list_store), &iter));
		break;
	default:
		break;
	}
}
static void account_dialog_list_use_toggled_cb(GtkCellRendererToggle *cell,
					       gchar *path_str, gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkTreeIter  iter;
	gboolean is_active;

	dialog = ACCOUNT_DIALOG(data);
	priv = dialog->priv;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(priv->list_store), &iter, path_str);
	gtk_tree_model_get(GTK_TREE_MODEL(priv->list_store), &iter, COLUMN_USE, &is_active, -1);
	
	is_active ^= 1;

	gtk_list_store_set(priv->list_store, &iter, COLUMN_USE, is_active, -1);
}
static void account_dialog_list_column_edited_cb(GtkCellRendererText *cell,
						 const gchar *path_str,
						 const gchar *new_text,
						 gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkTreeIter  iter;
	gint column;
	guint u;

	dialog = ACCOUNT_DIALOG(data);
	priv = dialog->priv;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(priv->list_store), &iter, path_str);

	column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));
	switch(column) {
	case COLUMN_HOSTNAME:
	case COLUMN_PASSWORD:
		gtk_list_store_set(priv->list_store, &iter, column, new_text, -1);
		break;
	case COLUMN_PORT:
		u = (guint) g_ascii_strtoull(new_text, NULL, 10);
		gtk_list_store_set(priv->list_store, &iter, column, u, -1);
		break;
	default:
		break;
	}
}
static void account_dialog_add_cb(GtkWidget *widget, gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkTreeIter iter;

	dialog = ACCOUNT_DIALOG(data);
	priv = dialog->priv;

	gtk_list_store_append(priv->list_store, &iter);
	gtk_list_store_set(priv->list_store, &iter,
			   COLUMN_USE, TRUE,
			   COLUMN_HOSTNAME, "irc.example.com",
			   COLUMN_PORT, 6667,
			   COLUMN_EDITABLE, TRUE,
			   -1);
}
static void account_dialog_remove_cb(GtkWidget *widget, gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkTreeIter iter, iter_next;
	GtkTreeSelection *selection;

	dialog = ACCOUNT_DIALOG(data);
	priv = dialog->priv;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->treeview));
	if(!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return;
	iter_next = iter;
	if(gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->list_store), &iter_next))
		gtk_tree_selection_select_iter(selection, &iter_next);
	gtk_list_store_remove(priv->list_store, &iter);
}
static void account_dialog_up_cb(GtkWidget *widget, gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkTreePath *path;
	GtkTreeIter iter, iter_prev;
	GtkTreeSelection *selection;

	dialog = ACCOUNT_DIALOG(data);
	priv = dialog->priv;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->treeview));
	if(!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return;
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(priv->list_store), &iter);
	if(!gtk_tree_path_prev(path))
		goto error;
	if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->list_store), &iter_prev, path))
		goto error;
	
	gtk_list_store_swap(priv->list_store, &iter, &iter_prev);

 error:
	gtk_tree_path_free(path);
	return;
}
static void account_dialog_down_cb(GtkWidget *widget, gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkTreePath *path;
	GtkTreeIter iter, iter_next;
	GtkTreeSelection *selection;

	dialog = ACCOUNT_DIALOG(data);
	priv = dialog->priv;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->treeview));
	if(!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return;

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(priv->list_store), &iter);
	gtk_tree_path_next(path);
	if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->list_store), &iter_next, path))
		goto error;
	
	gtk_list_store_swap(priv->list_store, &iter, &iter_next);

 error:
	gtk_tree_path_free(path);
	return;
}

GtkWidget*
account_dialog_new(Account *account)
{
        AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkWidget *vbox, *hbox, *vbox_c;
	GtkWidget *scrolled_win;
	GtkWidget *frame;
	GtkWidget *button;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	GSList *cur;
	Server *server;

	g_return_val_if_fail(account != NULL, NULL);

	dialog = g_object_new(account_dialog_get_type(), NULL);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_OK, GTK_RESPONSE_OK,
			       NULL);

	g_signal_connect(G_OBJECT(dialog),
			 "response",
			 G_CALLBACK(account_dialog_response_cb),
			 dialog);

	priv = dialog->priv;
	vbox = GTK_DIALOG(dialog)->vbox;
	
	hbox = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	gtkutils_add_label_entry(hbox, _("Account:"), &priv->entry_name, account_get_name(account));

/*	priv->check_use = gtk_check_button_new(_("Use"));
	gtk_box_pack_start(GTK_BOX(hbox), priv->check_use, TRUE, TRUE, 0); */

	gtkutils_add_label_entry(vbox, _("Nickname:"), &priv->entry_nick, account_get_nick(account));
	gtkutils_add_label_entry(vbox, _("Username:"), &priv->entry_username, account_get_username(account));
	gtkutils_add_label_entry(vbox, _("Realname:"), &priv->entry_realname, account_get_realname(account));
	gtkutils_add_label_entry(vbox, _("User information:"), &priv->entry_userinfo, account_get_userinfo(account));
	gtkutils_add_label_entry(vbox, _("Auto join channels:"), &priv->entry_autojoin, account_get_autojoin(account));

	frame = gtk_frame_new(_("Servers (Cells are editable.)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX (hbox), scrolled_win, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	priv->list_store = gtk_list_store_new(COLUMN_NUMBER,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING,
					      G_TYPE_UINT,
					      G_TYPE_STRING,
					      G_TYPE_BOOLEAN);

	for(cur = account->server_list; cur != NULL; cur = cur->next) {
		server = (Server *) cur->data;

		gtk_list_store_append(priv->list_store, &iter);
		gtk_list_store_set(priv->list_store, &iter,
				   COLUMN_USE, server->use,
				   COLUMN_HOSTNAME, server->hostname,
				   COLUMN_PORT, server->port,
				   COLUMN_PASSWORD, server->password, 
				   COLUMN_EDITABLE, TRUE,
				   -1);
	}

	priv->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(priv->list_store));

	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(G_OBJECT(renderer), "toggled",
	G_CALLBACK(account_dialog_list_use_toggled_cb), dialog);
	column = gtk_tree_view_column_new_with_attributes("Use",
							  renderer,
							  "active", COLUMN_USE,
							  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);
	
	renderer = gtk_cell_renderer_text_new();
	g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_HOSTNAME));
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(account_dialog_list_column_edited_cb), dialog);
	column = gtk_tree_view_column_new_with_attributes("Hostname",
							  renderer,
							  "text", COLUMN_HOSTNAME,
							  "editable", COLUMN_EDITABLE,
							  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

	renderer = gtk_cell_renderer_text_new();
	g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_PORT));
	g_signal_connect(G_OBJECT(renderer), "edited",
			  G_CALLBACK(account_dialog_list_column_edited_cb), dialog);
	column = gtk_tree_view_column_new_with_attributes("Port",
							  renderer,
							  "text", COLUMN_PORT,
							  "editable", COLUMN_EDITABLE,
							  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

	renderer = gtk_cell_renderer_text_new();
	g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_PASSWORD));
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(account_dialog_list_column_edited_cb), dialog);
	column = gtk_tree_view_column_new_with_attributes("Password",
							  renderer,
							  "text", COLUMN_PASSWORD,
							  "editable", COLUMN_EDITABLE,
							  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

	gtk_container_add(GTK_CONTAINER(scrolled_win), priv->treeview);

	vbox_c = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox_c, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_ADD);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(account_dialog_add_cb), dialog);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(account_dialog_up_cb), dialog);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(account_dialog_down_cb), dialog);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(account_dialog_remove_cb), dialog);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	gtk_widget_show_all(GTK_WIDGET(dialog));

	priv->account = account;

	return GTK_WIDGET(dialog);
}
