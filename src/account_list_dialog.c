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

#include "account_list_dialog.h"
#include "account_dialog.h"
#include "account_manager.h"
#include "intl.h"
#include "utils.h"

struct _AccountListDialogPrivate
{
	GtkWidget *treeview;
	GtkListStore *list_store;
};

static GtkDialogClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_DIALOG

enum {
	COLUMN_NAME,
	COLUMN_POINTER,
	COLUMN_NUMBER
};
static void account_list_dialog_class_init(AccountListDialogClass *klass);
static void account_list_dialog_init(AccountListDialog *account_list_dialog);
static void account_list_dialog_finalize(GObject *object);
static void account_list_dialog_destroy(GtkObject *object);

static void account_list_dialog_construct_list(AccountListDialog *dialog);
static Account* account_list_dialog_get_selected_account(AccountListDialog *dialog);

static void account_list_dialog_add_cb(GtkWidget *widget, AccountListDialog *dialog);
static void account_list_dialog_remove_cb(GtkWidget *widget, AccountListDialog *dialog);
static void account_list_dialog_properties_cb(GtkWidget *widget, AccountListDialog *dialog);

GType
account_list_dialog_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(AccountListDialogClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) account_list_dialog_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(AccountListDialog),
				0,              /* n_preallocs */
				(GInstanceInitFunc) account_list_dialog_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "AccountListDialog",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
account_list_dialog_class_init (AccountListDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = account_list_dialog_finalize;
        gtk_object_class->destroy = account_list_dialog_destroy;
}
static void 
account_list_dialog_init (AccountListDialog *account_list_dialog)
{
	AccountListDialogPrivate *priv;

	priv = g_new0(AccountListDialogPrivate, 1);

	account_list_dialog->priv = priv;
}
static void 
account_list_dialog_finalize (GObject *object)
{
	AccountListDialog *account_list_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT_LIST_DIALOG(object));

        account_list_dialog = ACCOUNT_LIST_DIALOG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account_list_dialog->priv);
}
static void 
account_list_dialog_destroy (GtkObject *object)
{
        AccountListDialog *account_list_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT_LIST_DIALOG(object));

        account_list_dialog = ACCOUNT_LIST_DIALOG(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
account_list_dialog_construct_list(AccountListDialog *dialog)
{
	AccountListDialogPrivate *priv;
	GSList *account_list, *cur;
	GtkTreeIter iter;
	Account *account;
	gchar *name;

        g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_ACCOUNT_LIST_DIALOG(dialog));

	priv = dialog->priv;
	
	if(priv->list_store) {
		gtk_list_store_clear(priv->list_store);
	} else {
		priv->list_store = gtk_list_store_new(COLUMN_NUMBER,
						      G_TYPE_STRING,
						      G_TYPE_POINTER);
	}

	account_list = account_manager_get_account_list(account_manager_get());
	for(cur = account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);

		name = account_get_name(account);
		gtk_list_store_append(priv->list_store, &iter);
		gtk_list_store_set(priv->list_store, &iter,
				   COLUMN_NAME, name, 
				   COLUMN_POINTER, account,
				   -1);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(priv->treeview), GTK_TREE_MODEL(priv->list_store));
}
static Account*
account_list_dialog_get_selected_account(AccountListDialog *dialog)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	Account *account = NULL;

        g_return_val_if_fail(dialog != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT_LIST_DIALOG(dialog), NULL);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dialog->priv->treeview));
	if(!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return NULL;
	
	gtk_tree_model_get(GTK_TREE_MODEL(dialog->priv->list_store), &iter,
			   COLUMN_POINTER, &account, -1);

	return account;
}
static void
account_list_dialog_add_cb(GtkWidget *widget, AccountListDialog *dialog)
{
        g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_ACCOUNT_LIST_DIALOG(dialog));

	account_dialog_open_add_dialog(GTK_WINDOW(dialog));
	account_list_dialog_construct_list(dialog);
}
static void
account_list_dialog_remove_cb(GtkWidget *widget, AccountListDialog *dialog)
{
	Account *account;

        g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_ACCOUNT_LIST_DIALOG(dialog));

	account = account_list_dialog_get_selected_account(dialog);
	if(!account)
		return;

	account_dialog_open_remove_dialog(GTK_WINDOW(dialog), account);
	account_list_dialog_construct_list(dialog);
}
static void
account_list_dialog_properties_cb(GtkWidget *widget, AccountListDialog *dialog)
{
	Account *account;

        g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_ACCOUNT_LIST_DIALOG(dialog));

	account = account_list_dialog_get_selected_account(dialog);
	if(!account)
		return;

	account_dialog_open_configure_dialog(GTK_WINDOW(dialog), account);
	account_list_dialog_construct_list(dialog);
}

GtkWidget*
account_list_dialog_new (void)
{
        AccountListDialog *dialog;
	AccountListDialogPrivate *priv;
	GtkWidget *scrolled_win;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	dialog = g_object_new(account_list_dialog_get_type(), NULL);
	
	priv = dialog->priv;

	gtk_window_set_title(GTK_WINDOW(dialog), _("Account List"));
	/* TODO: destroy with parent window */
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), 
			       GTK_STOCK_OK, GTK_RESPONSE_NONE, 
			       NULL);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE, 5);

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), 
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(hbox), scrolled_win, TRUE, TRUE, 0);

	priv->treeview = gtk_tree_view_new();
	account_list_dialog_construct_list(dialog);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Name"),
							  renderer,
							  "text", COLUMN_NAME,
							  NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

	gtk_widget_set_usize(priv->treeview, 200, 100);
	gtk_container_add(GTK_CONTAINER(scrolled_win), priv->treeview);

	vbox = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_ADD);
	g_signal_connect(G_OBJECT(button), "clicked", 
			 G_CALLBACK(account_list_dialog_add_cb), dialog);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 5);

	button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	g_signal_connect(G_OBJECT(button), "clicked", 
			 G_CALLBACK(account_list_dialog_remove_cb), dialog);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 5);

	button = gtk_button_new_from_stock(GTK_STOCK_PROPERTIES);
	g_signal_connect(G_OBJECT(button), "clicked", 
			 G_CALLBACK(account_list_dialog_properties_cb), dialog);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 5);

	gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(dialog)->vbox));

	return GTK_WIDGET(dialog);
}

void 
account_list_dialog_open(GtkWindow *parent)
{
	GtkWidget *dialog;

	dialog = account_list_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}
