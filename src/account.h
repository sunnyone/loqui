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
#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include <glib.h>
#include "codeconv.h"
#include "loqui_profile_account.h"

G_BEGIN_DECLS

#define TYPE_ACCOUNT                 (account_get_type ())
#define ACCOUNT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_ACCOUNT, Account))
#define ACCOUNT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_ACCOUNT, AccountClass))
#define IS_ACCOUNT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_ACCOUNT))
#define IS_ACCOUNT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_ACCOUNT))
#define ACCOUNT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ACCOUNT, AccountClass))

typedef struct _Account            Account;
typedef struct _AccountClass       AccountClass;

typedef struct _AccountPrivate     AccountPrivate;

#include "loqui_channel_entry.h"
#include "loqui_channel.h"

struct _Account
{
	LoquiChannelEntry parent;

	GList *channel_list;

	/* key: channel name(gchar *), value: GList * */
	GHashTable *channel_name_hash;

	GHashTable *user_nick_table; /* key: user, value: nick */
	GHashTable *nick_user_table; /* key: nick, value: user */

	LoquiUser *user_self;

        AccountPrivate *priv;
};

struct _AccountClass
{
        LoquiChannelEntryClass parent_class;

	/* signals */
	void (* connected)        (Account *account);
	void (* disconnected)     (Account *account);
	void (* add_channel)      (Account *account,
				   LoquiChannel *channel);
	void (* remove_channel)   (Account *account,
				   LoquiChannel *channel);
};

GType account_get_type(void) G_GNUC_CONST;

Account* account_new(LoquiProfileAccount *profile);

LoquiProfileAccount *account_get_profile(Account *account);
LoquiUser* account_get_user_self(Account *account);

void account_connect(Account *account);
void account_disconnect(Account *account);
gboolean account_is_connected(Account *account);

void account_set_codeconv(Account *account, CodeConv *codeconv);
CodeConv *account_get_codeconv(Account *account);

void account_add_channel(Account *account, LoquiChannel *channel);
void account_remove_channel(Account *account, LoquiChannel *channel);
void account_remove_all_channel(Account *account);

GList *account_get_channel_list(Account *account);
LoquiChannel* account_get_channel_by_name(Account *account, const gchar *name);
GSList *account_search_joined_channel(Account *account, gchar *nick);

void account_console_buffer_append(Account *account, TextType type, gchar *str);
void account_speak(Account *account, LoquiChannel *channel, const gchar *str, gboolean command_mode);

gboolean account_is_current_nick(Account *account, const gchar *str);

void account_set_away(Account *account, gboolean is_away);
void account_set_away_message(Account *account, const gchar *away_message);

void account_change_nick(Account *account, const gchar *nick);
void account_whois(Account *account, const gchar *target);
void account_join(Account *account, const gchar *target, const gchar *key);
void account_start_private_talk(Account *account, const gchar *target);
void account_part(Account *account, const gchar *target, const gchar *part_message);
void account_set_topic(Account *account, const gchar *target, const gchar *topic);
void account_pong(Account *account, const gchar *target);
void account_get_channel_mode(Account *account, const gchar *channel_name);
void account_notice(Account *account, const gchar *target, const gchar *str);

void account_send_ctcp_request(Account *account, const gchar *target, const gchar *command);

void account_change_channel_user_mode(Account *account, LoquiChannel *channel, 
				      gboolean is_give, IRCModeFlag flag, GList *str_list);

void account_fetch_away_information(Account *account, LoquiChannel *channel);

void account_get_updated_number(Account *account, gint *updated_private_talk_number, gint *updated_channel_number); 

LoquiUser* account_fetch_user(Account *account, const gchar *nick);
LoquiUser* account_peek_user(Account *account, const gchar *nick);

G_END_DECLS

#endif /* __ACCOUNT_H__ */
