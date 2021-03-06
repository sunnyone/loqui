/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk
 * Copyright (C) 2003 Yoichi Imai <sunnyone41@gmail.com>
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
#include <glib/gi18n.h>
#include <string.h>

static gboolean check_account_connected(LoquiAccount *account);
static gboolean check_channel_joined(LoquiChannel *channel);

static gboolean
check_account_connected(LoquiAccount *account)
{
       if(account == NULL) {
               gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Account is not selected."));
               return FALSE;
       }
       if(!loqui_account_get_is_connected(account)) {
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

void
command_dialog_join(LoquiApp *app, LoquiAccount *account)
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

	buf = g_strdup_printf(_("Join a channel with the account %s"), loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));
	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(gtk_dialog_get_content_area(GTK_DIALOG(dialog)), _("Channel name:"), &entry_name, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry_name), TRUE);

	gtkutils_add_label_entry(gtk_dialog_get_content_area(GTK_DIALOG(dialog)), _("Channel key (if any):"), &entry_key, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry_key), TRUE);

	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text_name = gtk_entry_get_text(GTK_ENTRY(entry_name));
		text_key = gtk_entry_get_text(GTK_ENTRY(entry_key));

		loqui_sender_join_raw(loqui_account_get_sender(account), text_name, text_key);
	}

	gtk_widget_destroy(dialog);
}
void
command_dialog_private_talk(LoquiApp *app, LoquiAccount *account)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry_name;
	gchar *buf;
	const gchar *text_name;
	gint result;

	if(!check_account_connected(account))
		return;
	
	dialog = gtk_dialog_new_with_buttons(_("Start private talk"), GTK_WINDOW(app),
					     GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	buf = g_strdup_printf(_("Start private talk with the user in %s"), loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));
	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(gtk_dialog_get_content_area(GTK_DIALOG(dialog)), _("Nickname:"), &entry_name, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry_name), TRUE);

	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text_name = gtk_entry_get_text(GTK_ENTRY(entry_name));

		loqui_sender_start_private_talk_raw(loqui_account_get_sender(account), text_name);
	}
	gtk_widget_destroy(dialog);
}
void
command_dialog_part(LoquiApp *app, LoquiAccount *account, LoquiChannel *channel)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry;
	gchar *buf;
	const gchar *text;
	gint result;

	if (!check_account_connected(account))
		return;

	dialog = gtk_dialog_new_with_buttons(_("Part the channel"), GTK_WINDOW(app),
					     GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	buf = g_strdup_printf(_("Part the channel %s (in %s)"),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));

	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(gtk_dialog_get_content_area(GTK_DIALOG(dialog)), _("Part message:"), &entry, NULL);
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text = gtk_entry_get_text(GTK_ENTRY(entry));

		loqui_sender_part(loqui_account_get_sender(account), channel, text);
	}
	gtk_widget_destroy(dialog);
}
void
command_dialog_topic(LoquiApp *app, LoquiAccount *account, LoquiChannel *channel)
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


	buf = g_strdup_printf(_("Set the topic of the channel %s (in %s)"),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
			      loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));

	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(gtk_dialog_get_content_area(GTK_DIALOG(dialog)), _("Topic:"), &entry, loqui_channel_entry_get_topic(LOQUI_CHANNEL_ENTRY(channel)));
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text = gtk_entry_get_text(GTK_ENTRY(entry));

		loqui_sender_topic(loqui_account_get_sender(account), channel, text);
	}
	gtk_widget_destroy(dialog);
}
void
command_dialog_nick(LoquiApp *app, LoquiAccount *account)
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


	buf = g_strdup_printf(_("New nickname of the account %s"), loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)));
	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, FALSE, FALSE, 0);
	g_free(buf);

	gtkutils_add_label_entry(gtk_dialog_get_content_area(GTK_DIALOG(dialog)), _("Nickname:"), &entry, loqui_user_get_nick(loqui_account_get_user_self(account)));
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_OK) {
		text = gtk_entry_get_text(GTK_ENTRY(entry));

		loqui_sender_nick(loqui_account_get_sender(account), text);
	}
	gtk_widget_destroy(dialog);
}
