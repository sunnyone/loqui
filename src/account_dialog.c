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

struct _AccountDialogPrivate
{
	GtkWidget *entry_name;
	GtkWidget *check_use;

	GtkWidget *entry_nick;
	GtkWidget *entry_username;
	GtkWidget *entry_realname;
	GtkWidget *entry_autojoin;
};

static GtkDialogClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_DIALOG

static void account_dialog_class_init(AccountDialogClass *klass);
static void account_dialog_init(AccountDialog *account_dialog);
static void account_dialog_finalize(GObject *object);
static void account_dialog_destroy(GtkObject *object);

static void account_dialog_response_cb(GtkWidget *widget, gint response, gpointer data);

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

	dialog = ACCOUNT_DIALOG(data);

}
GtkWidget*
account_dialog_new(Account *account)
{
        AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkWidget *vbox, *hbox, *vbox_c;
	GtkWidget *scrolled_win;
	GtkWidget *treeview;
	GtkWidget *frame;
	GtkWidget *button;

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

	priv->check_use = gtk_check_button_new_with_mnemonic (_("Use"));
	gtk_box_pack_start(GTK_BOX(hbox), priv->check_use, TRUE, TRUE, 0);

	gtkutils_add_label_entry(vbox, _("Nickname:"), &priv->entry_nick, account_get_nick(account));
	gtkutils_add_label_entry(vbox, _("Username:"), &priv->entry_username, account_get_username(account));
	gtkutils_add_label_entry(vbox, _("Realname:"), &priv->entry_realname, account_get_realname(account));
	gtkutils_add_label_entry(vbox, _("Auto join channels:"), &priv->entry_autojoin, account_get_autojoin(account));

	frame = gtk_frame_new(_("Servers (Cells are editable.)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX (hbox), scrolled_win, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(scrolled_win), treeview);

	vbox_c = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox_c, FALSE, TRUE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock(GTK_STOCK_PROPERTIES);
	gtk_box_pack_start(GTK_BOX(vbox_c), button, FALSE, FALSE, 0);

	gtk_widget_show_all(GTK_WIDGET(dialog));

	return GTK_WIDGET(dialog);
}
