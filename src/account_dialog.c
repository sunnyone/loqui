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

#include <loqui_account_manager.h>
#include <utils.h>
#include <loqui_profile_account.h>
#include <loqui_profile_account_irc.h>
#include <loqui_codeconv.h>

#include "loqui_account_irc.h"
#include <string.h>

struct _AccountDialogPrivate
{
	LoquiProfileAccount *profile;

	GtkWidget *entry_name;
	GtkWidget *check_use;

	GtkWidget *entry_nick;
	GtkWidget *entry_username;
	GtkWidget *entry_password;
	
	GtkWidget *entry_servername;
	GtkWidget *spin_port;
	
	GtkWidget *entry_realname;
	GtkWidget *entry_userinfo;
	GtkWidget *entry_autojoin;
	GtkWidget *entry_quit_message;

	GtkWidget *radio_automatic;
	GtkWidget *radio_noconv;
	GtkWidget *radio_bytable;
	GtkWidget *combobox_table;
	GtkWidget *radio_codeset;
	GtkWidget *entry_codeset;
	
	GtkTreeModel *model_codeconv_table;

	GtkWidget *textview_nicklist;
};

enum {
	CODECONV_COLUMN_TITLE,
	CODECONV_COLUMN_NAME,
	CODECONV_COLUMN_NUMBER,
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
account_dialog_set_codeconv_mode(AccountDialog *dialog, LoquiCodeConvMode mode)
{
	AccountDialogPrivate *priv;

	priv = dialog->priv;

	switch (mode) {
	case LOQUI_CODECONV_MODE_AUTOMATIC:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->radio_automatic), TRUE);
		break;
	case LOQUI_CODECONV_MODE_NO_CONV:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->radio_noconv), TRUE);
		break;
	case LOQUI_CODECONV_MODE_BY_TABLE:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->radio_bytable), TRUE);
		break;
	case LOQUI_CODECONV_MODE_CODESET:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->radio_codeset), TRUE);
		break;
	}
}
static LoquiCodeConvMode
account_dialog_get_codeconv_mode(AccountDialog *dialog)
{
	AccountDialogPrivate *priv;

	priv = dialog->priv;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->radio_automatic)))
		return LOQUI_CODECONV_MODE_AUTOMATIC;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->radio_noconv)))
		return LOQUI_CODECONV_MODE_NO_CONV;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->radio_bytable)))
		return LOQUI_CODECONV_MODE_BY_TABLE;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->radio_codeset)))
		return LOQUI_CODECONV_MODE_CODESET;

	return 0;
}
static void
account_dialog_response_cb(GtkWidget *widget, gint response, gpointer data)
{
	AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GList *nick_list;
	GtkTreeIter iter;
	const gchar *codeconv_item_name;

	dialog = ACCOUNT_DIALOG(data);

	priv = dialog->priv;

	switch(response) {
	case GTK_RESPONSE_OK:
		if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(priv->combobox_table), &iter))
			gtk_tree_model_get(priv->model_codeconv_table, &iter, CODECONV_COLUMN_NAME, &codeconv_item_name, -1);
		else
			codeconv_item_name = NULL;

		g_object_set(priv->profile,
			     "use", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_use)),
			     "name", gtk_entry_get_text(GTK_ENTRY(priv->entry_name)),
			     "nick", gtk_entry_get_text(GTK_ENTRY(priv->entry_nick)),
			     "username", gtk_entry_get_text(GTK_ENTRY(priv->entry_username)),
			     "password", gtk_entry_get_text(GTK_ENTRY(priv->entry_password)),
			     "realname", gtk_entry_get_text(GTK_ENTRY(priv->entry_realname)),
			     "userinfo", gtk_entry_get_text(GTK_ENTRY(priv->entry_userinfo)),
			     "autojoin", gtk_entry_get_text(GTK_ENTRY(priv->entry_autojoin)),
			     "quit_message", gtk_entry_get_text(GTK_ENTRY(priv->entry_quit_message)),
			     "servername", gtk_entry_get_text(GTK_ENTRY(priv->entry_servername)),
			     "port", (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(priv->spin_port)),
			     "codeconv_mode", account_dialog_get_codeconv_mode(dialog),
			     "codeconv_item_name", codeconv_item_name,
			     "codeset", gtk_entry_get_text(GTK_ENTRY(priv->entry_codeset)),
			     NULL);
		nick_list = NULL;
		gtkutils_set_string_list_from_textview(&nick_list, GTK_TEXT_VIEW(priv->textview_nicklist));
		loqui_profile_account_set_nick_list(priv->profile, nick_list);
		break;
	default:
		break;
	}
}
static void
account_dialog_update_codeconv_sensitivity(AccountDialog *dialog)
{
	AccountDialogPrivate *priv;
	LoquiCodeConvMode mode;

	priv = dialog->priv;
	mode = account_dialog_get_codeconv_mode(dialog);

	gtk_widget_set_sensitive(priv->entry_codeset, (mode == LOQUI_CODECONV_MODE_CODESET));
	gtk_widget_set_sensitive(priv->combobox_table, (mode == LOQUI_CODECONV_MODE_BY_TABLE));
}
static void
account_dialog_radio_toggled_cb(GtkWidget *radio, AccountDialog *dialog)
{
	account_dialog_update_codeconv_sensitivity(dialog);
}
GtkWidget*
account_dialog_new(LoquiProfileAccount *profile)
{
        AccountDialog *dialog;
	AccountDialogPrivate *priv;
	GtkWidget *notebook;
	GtkWidget *vbox, *hbox, *vbox_c;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *frame;
	GtkWidget *label;
        GSList *group_codeconv = NULL;
       
	gint i;
	const gchar *codeset, *name;
	LoquiCodeConvTableItem *table;
	GtkTreeIter iter;
	GtkCellRenderer *cell;

	dialog = g_object_new(account_dialog_get_type(), NULL);
	
	priv = dialog->priv;
	priv->profile = profile;

	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_OK, GTK_RESPONSE_OK,
			       NULL);

	g_signal_connect(G_OBJECT(dialog),
			 "response",
			 G_CALLBACK(account_dialog_response_cb),
			 dialog);

	vbox = GTK_DIALOG(dialog)->vbox;
	
	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Profile")));
	
	hbox = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	
	gtkutils_add_label_entry(hbox, _("Account name:"), &priv->entry_name, loqui_profile_account_get_name(profile));
	
	priv->check_use = gtk_check_button_new_with_label(_("Connect by default"));
	gtk_box_pack_start(GTK_BOX(hbox), priv->check_use, TRUE, TRUE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_use), loqui_profile_account_get_use(profile));

	frame = gtk_frame_new(_("User"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	
	vbox_c = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox_c);
	
	gtkutils_add_label_entry(vbox_c, _("Nickname:"), &priv->entry_nick, loqui_profile_account_get_nick(profile));
	gtkutils_add_label_entry(vbox_c, _("User name:"), &priv->entry_username, loqui_profile_account_get_username(profile));
	gtkutils_add_label_entry(vbox_c, _("Password:"), &priv->entry_password, loqui_profile_account_get_password(profile));
	gtk_entry_set_visibility(GTK_ENTRY(priv->entry_password), FALSE);

	frame = gtk_frame_new(_("Server"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	
	vbox_c = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox_c);
	
	gtkutils_add_label_entry(vbox_c, _("Hostname:"), &priv->entry_servername, loqui_profile_account_get_servername(profile));
	gtkutils_add_label_spin_button(vbox_c, _("Port:"), &priv->spin_port, 1, 65535, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->spin_port), loqui_profile_account_get_port(profile));

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("IRC")));
	
	gtkutils_add_label_entry(vbox, _("Realname:"), &priv->entry_realname,
				 loqui_profile_account_irc_get_realname(LOQUI_PROFILE_ACCOUNT_IRC(profile)));
	gtkutils_add_label_entry(vbox, _("User information:"), &priv->entry_userinfo,
				 loqui_profile_account_irc_get_userinfo(LOQUI_PROFILE_ACCOUNT_IRC(profile)));
	gtkutils_add_label_entry(vbox, _("Auto join channels:"), &priv->entry_autojoin,
				 loqui_profile_account_irc_get_autojoin(LOQUI_PROFILE_ACCOUNT_IRC(profile)));
	gtkutils_add_label_entry(vbox, _("Quit message:"), &priv->entry_quit_message,
				 loqui_profile_account_irc_get_quit_message(LOQUI_PROFILE_ACCOUNT_IRC(profile)));

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Code")));

	frame = gtk_frame_new(_("Code convertion"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 2);
	
        vbox1 = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), vbox1);

        priv->radio_automatic = gtk_radio_button_new_with_mnemonic(NULL, _("Automatic detection by locale"));
        gtk_box_pack_start(GTK_BOX(vbox1), priv->radio_automatic, FALSE, FALSE, 0);
        group_codeconv = gtk_radio_button_get_group(GTK_RADIO_BUTTON(priv->radio_automatic));
	g_signal_connect(G_OBJECT(priv->radio_automatic), "toggled",
			 G_CALLBACK(account_dialog_radio_toggled_cb), dialog);

        priv->radio_noconv = gtk_radio_button_new_with_mnemonic(NULL, _("No convertion"));
        gtk_box_pack_start(GTK_BOX(vbox1), priv->radio_noconv, FALSE, FALSE, 0);
        gtk_radio_button_set_group(GTK_RADIO_BUTTON(priv->radio_noconv), group_codeconv);
        group_codeconv = gtk_radio_button_get_group(GTK_RADIO_BUTTON(priv->radio_noconv));
	g_signal_connect(G_OBJECT(priv->radio_noconv), "toggled",
			 G_CALLBACK(account_dialog_radio_toggled_cb), dialog);

        priv->radio_bytable = gtk_radio_button_new_with_mnemonic(NULL, _("Select from the table"));
        gtk_box_pack_start(GTK_BOX(vbox1), priv->radio_bytable, FALSE, FALSE, 0);
        gtk_radio_button_set_group(GTK_RADIO_BUTTON(priv->radio_bytable), group_codeconv);
        group_codeconv = gtk_radio_button_get_group(GTK_RADIO_BUTTON(priv->radio_bytable));
	g_signal_connect(G_OBJECT(priv->radio_bytable), "toggled",
			 G_CALLBACK(account_dialog_radio_toggled_cb), dialog);

	priv->model_codeconv_table = GTK_TREE_MODEL(gtk_list_store_new(CODECONV_COLUMN_NUMBER,
								       G_TYPE_STRING,
								       G_TYPE_STRING));
	
        priv->combobox_table = gtk_combo_box_new_with_model(priv->model_codeconv_table);
        gtk_box_pack_start(GTK_BOX(vbox1), priv->combobox_table, FALSE, FALSE, 0);

	cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(priv->combobox_table), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(priv->combobox_table), cell,
				       "text", 0,
				       NULL);

	table = loqui_protocol_get_codeconv_table(loqui_profile_account_get_protocol(profile));
	name = loqui_profile_account_get_codeconv_item_name(profile);

	for (i = 0; table[i].name != NULL; i++) {
		gtk_list_store_append(GTK_LIST_STORE(priv->model_codeconv_table), &iter);
		gtk_list_store_set(GTK_LIST_STORE(priv->model_codeconv_table),
				   &iter,
				   CODECONV_COLUMN_TITLE, loqui_codeconv_translate(table[i].title),
				   CODECONV_COLUMN_NAME, table[i].name,
				   -1);

		if (name != NULL && strcmp(table[i].name, name) == 0) {
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(priv->combobox_table), &iter);
		}
	}

        priv->radio_codeset = gtk_radio_button_new_with_mnemonic(NULL, _("Specify codeset"));
        gtk_box_pack_start(GTK_BOX(vbox1), priv->radio_codeset, FALSE, FALSE, 0);
        gtk_radio_button_set_group(GTK_RADIO_BUTTON(priv->radio_codeset), group_codeconv);
        group_codeconv = gtk_radio_button_get_group(GTK_RADIO_BUTTON(priv->radio_codeset));
	g_signal_connect(G_OBJECT(priv->radio_codeset), "toggled",
			 G_CALLBACK(account_dialog_radio_toggled_cb), dialog);

        hbox1 = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);

        label = gtk_label_new(_("Codeset: "));
        gtk_box_pack_start(GTK_BOX(hbox1), label, FALSE, FALSE, 0);

        priv->entry_codeset = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(hbox1), priv->entry_codeset, TRUE, TRUE, 0);
	codeset = loqui_profile_account_get_codeset(LOQUI_PROFILE_ACCOUNT(profile));
	if (codeset)
		gtk_entry_set_text(GTK_ENTRY(priv->entry_codeset), codeset);
	account_dialog_set_codeconv_mode(dialog, loqui_profile_account_get_codeconv_mode(LOQUI_PROFILE_ACCOUNT(profile)));
	account_dialog_update_codeconv_sensitivity(dialog);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Nick")));

	frame = gtkutils_create_framed_textview(&priv->textview_nicklist,
						_("List of often used nick(Separate each words with linefeeds)"));
	gtkutils_set_textview_from_string_list(GTK_TEXT_VIEW(priv->textview_nicklist),
					       loqui_profile_account_get_nick_list(profile));

	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(GTK_WIDGET(dialog));

	return GTK_WIDGET(dialog);
}

void
account_dialog_open_add_dialog(GtkWindow *parent, LoquiAccountManager *manager)
{
	AccountDialog *dialog;
	LoquiProfileAccount *profile;
	LoquiAccount *account;
	gint response;

	/* FIXME */
	profile = LOQUI_PROFILE_ACCOUNT(loqui_profile_account_irc_new());
	dialog = ACCOUNT_DIALOG(account_dialog_new(profile));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));

	if (response == GTK_RESPONSE_OK) {
		account = LOQUI_ACCOUNT(loqui_account_irc_new(profile));
		loqui_account_manager_add_account(manager, account);
		g_object_unref(account);
		loqui_account_manager_save_accounts(manager);
	} else {
		g_object_unref(profile);
	}
}

void
account_dialog_open_configure_dialog(GtkWindow *parent, LoquiAccountManager *manager, LoquiAccount *account)
{
	GtkWidget *dialog;
	
	dialog = account_dialog_new(loqui_account_get_profile(account));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	/* FIXME: should do
	   account_manager_update_account(manager, account); */
	loqui_account_manager_save_accounts(manager);
}

void
account_dialog_open_remove_dialog(GtkWindow *parent, LoquiAccountManager *manager, LoquiAccount *account)
{
	GtkWidget *dialog;
	gint response;

	dialog = gtk_message_dialog_new(parent,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_YES_NO,
					_("This account's configuration and connection will be removed.\n"
					  "Do you really want to remove this account?"));
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if (response == GTK_RESPONSE_YES) {
		loqui_account_manager_remove_account(manager, account);
		loqui_account_manager_save_accounts(manager);
		debug_puts("Removed account.");
	}
}
