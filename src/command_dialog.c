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
#include "channel_input_dialog.h"
#include "gtkutils.h"
#include "intl.h"
#include <string.h>


static void command_dialog_private_talk_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data);
static void command_dialog_join_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data);
static void command_dialog_part_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data);
static void command_dialog_topic_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data);
static void command_dialog_nick_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data);

static gboolean check_account_connected(Account *account);
static gboolean check_target_valid(const gchar *str);

static gboolean check_account_connected(Account *account)
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
static gboolean check_target_valid(const gchar *str)
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

static void command_dialog_join_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data)
{
	if(!check_account_connected(account) ||
	   !check_target_valid(channel_text))
		return;
	
	account_join(account, channel_text, text);
}

static void command_dialog_private_talk_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data)
{
	if(!check_account_connected(account) ||
	   !check_target_valid(text))
		return;
	
	account_start_private_talk(account, text);
}

static void command_dialog_part_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data)
{
	if(!check_account_connected(account) ||
	   !check_target_valid(channel_text))
		return;
	
	account_part(account, channel_text, text);
}
static void command_dialog_topic_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data)
{
	if(!check_account_connected(account) ||
	   !check_target_valid(channel_text))
		return;
	
	account_set_topic(account, channel_text, text);
}
static void command_dialog_nick_cb(Account *account, const gchar *channel_text, const gchar *text, gpointer data)
{
	if(!check_account_connected(account) ||
	   !check_target_valid(text))
		return;
	
	account_change_nick(account, text);
}

void command_dialog_join(LoquiApp *app, Account *account)
{
	channel_input_dialog_open(app, 
				  _("Join channel"),
				  _("Type channel name to join (and channel key, if any)."),
				  CHANNEL_HISTORY_SAVED,
				  command_dialog_join_cb, NULL,
				  TRUE, account, TRUE, NULL, TRUE, NULL);
}
void command_dialog_private_talk(LoquiApp *app, Account *account)
{
	channel_input_dialog_open(app, 
				  _("Start private talk"),
				  _("Type nick to start private talk."),
				  CHANNEL_HISTORY_NONE,
				  command_dialog_private_talk_cb, NULL,
				  TRUE, account, FALSE, NULL, TRUE, NULL);
}
void command_dialog_part(LoquiApp *app, Account *account, LoquiChannel *channel)
{
	channel_input_dialog_open(app, 
				  _("Part channel"),
				  _("Type channel name to part and the part message."),
				  CHANNEL_HISTORY_JOINED,
				  command_dialog_part_cb, NULL,
				  TRUE, account, TRUE, channel ? loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)) : NULL, TRUE, NULL);
}
void command_dialog_topic(LoquiApp *app, Account *account, LoquiChannel *channel)
{
	channel_input_dialog_open(app, 
				  _("Set the topic of the channel"),
				  _("Type channel name and its topic."),
				  CHANNEL_HISTORY_JOINED,
				  command_dialog_topic_cb, NULL,
				  TRUE, account,
				  TRUE, channel ? loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)) : NULL,
				  TRUE, channel ? loqui_channel_entry_get_topic(LOQUI_CHANNEL_ENTRY(channel)) : NULL);
}
void command_dialog_nick(LoquiApp *app, Account *account)
{
	channel_input_dialog_open(app, 
				  _("Change nickname"),
				  _("Type nickname."),
				  CHANNEL_HISTORY_NONE,
				  command_dialog_nick_cb, NULL,
				  TRUE, account, FALSE, NULL, TRUE, loqui_user_get_nick(account_get_user_self(account)));
}
