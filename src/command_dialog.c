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
#include "command_dialog.h"
#include "gtkutils.h"
#include "intl.h"
#include <string.h>

static gboolean check_account_connected(Account *account);
static gboolean check_target_valid(const gchar *str);

static gboolean
check_account_connected(Account *account)
{
	if(account == NULL) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Account is not selected."));
		return FALSE;
	}
	if(!account_is_connected(account)) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Account is not connected."));
		return FALSE;
	}
	return TRUE;
}
static gboolean
check_channel_joined(LoquiChannel *channel)
{
	if (channel == NULL) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Channel is not selected"));
		return FALSE;
	}

	if (!loqui_channel_get_is_joined(channel)) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("You are not joined in this channel."));
		return FALSE;
	}
	return TRUE;
}

static gboolean
check_target_valid(const gchar *str)
{
	if(str == NULL || strlen(str) == 0) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Input some characters."));
		return FALSE;
	}

	if(strchr(str, ' ') != NULL) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Error: space contains"));
		return FALSE;
	}
	return TRUE;
}

void
command_dialog_join(LoquiApp *app, Account *account)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry_name;
	GtkWidget *entry_key;
	gchar *buf;
	const gchar *text_name, *text_key;
	gint result;

	if(!check_account_connected(account))
		return;
	
	dialog = gtk_dialog_new_with_buttons(_("Join a channel"), GTK_WINDOW(app),
					     GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	buf = g_strdup_printf(_("Join a channel with the account '%s'"), loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));
	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(GTK_DIALOG(dialog)->vbox, _("Channel name:"), &entry_name, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry_name), TRUE);

	gtkutils_add_label_entry(GTK_DIALOG(dialog)->vbox, _("Channel key (if any):"), &entry_key, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry_key), TRUE);

	gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text_name = gtk_entry_get_text(GTK_ENTRY(entry_name));
		text_key = gtk_entry_get_text(GTK_ENTRY(entry_key));

		if(!check_target_valid(text_name)) {
			gtk_widget_destroy(dialog);
			return;
		}

		if (strlen(text_key) > 0)
			account_join(account, text_name, text_key);
		else
			account_join(account, text_name, NULL);
	}

	gtk_widget_destroy(dialog);
}
void
command_dialog_private_talk(LoquiApp *app, Account *account)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry_name;
	gchar *buf;
	const gchar *text_name;
	gint result;
	LoquiUser *user;

	if(!check_account_connected(account))
		return;
	
	dialog = gtk_dialog_new_with_buttons(_("Start private talk"), GTK_WINDOW(app),
					     GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	buf = g_strdup_printf(_("Start private talk with the user in '%s'"), loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));
	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(GTK_DIALOG(dialog)->vbox, _("Nickname:"), &entry_name, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry_name), TRUE);

	gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text_name = gtk_entry_get_text(GTK_ENTRY(entry_name));

		if (!check_target_valid(text_name)) {
			gtk_widget_destroy(dialog);
			return;
		}

		user = account_fetch_user(account, text_name);
		if (!user) {
			g_warning("Can't fetch user for private talk");
			return;
		}
		loqui_sender_start_private_talk(account_get_sender(account), user);
		g_object_unref(user);
	}
	gtk_widget_destroy(dialog);
}
void
command_dialog_part(LoquiApp *app, Account *account, LoquiChannel *channel)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry;
	gchar *buf;
	const gchar *text;
	gint result;

	if (!check_account_connected(account))
		return;
	if (!check_channel_joined(channel))
		return;

	dialog = gtk_dialog_new_with_buttons(_("Part the channel"), GTK_WINDOW(app),
					     GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	buf = g_strdup_printf(_("Part the channel '%s' (in '%s')"),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));

	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(GTK_DIALOG(dialog)->vbox, _("Part message:"), &entry, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

	gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text = gtk_entry_get_text(GTK_ENTRY(entry));

		account_part(account, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), text);
	}
	gtk_widget_destroy(dialog);
}
void
command_dialog_topic(LoquiApp *app, Account *account, LoquiChannel *channel)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry;
	gchar *buf;
	const gchar *text;
	gint result;

	if (!check_account_connected(account))
		return;
	if (!check_channel_joined(channel))
		return;

	dialog = gtk_dialog_new_with_buttons(_("Set topic"), GTK_WINDOW(app),
					     GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);


	buf = g_strdup_printf(_("Set the topic of the channel '%s' (in '%s')"),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));

	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(GTK_DIALOG(dialog)->vbox, _("Topic:"), &entry, loqui_channel_entry_get_topic(LOQUI_CHANNEL_ENTRY(channel)));
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

	gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text = gtk_entry_get_text(GTK_ENTRY(entry));

		loqui_sender_topic(account_get_sender(account), channel, text);
	}
	gtk_widget_destroy(dialog);
}
void
command_dialog_nick(LoquiApp *app, Account *account)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry;
	gchar *buf;
	const gchar *text;
	gint result;

	if (!check_account_connected(account))
		return;

	dialog = gtk_dialog_new_with_buttons(_("Change nickname"), GTK_WINDOW(app),
					     GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);


	buf = g_strdup_printf(_("New nickname of the account '%s'"), loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));
	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(GTK_DIALOG(dialog)->vbox, _("Nickname:"), &entry, loqui_user_get_nick(account_get_user_self(account)));
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

	gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text = gtk_entry_get_text(GTK_ENTRY(entry));

		if(!check_target_valid(text)) {
			gtk_widget_destroy(dialog);
			return;
		}

		loqui_sender_nick(account_get_sender(account), text);
	}
	gtk_widget_destroy(dialog);
}
