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
#ifndef __LOQUI_ACCOUNT_H__
#define __LOQUI_ACCOUNT_H__

#include <glib.h>
#include "codeconv.h"
#include "loqui_profile_account.h"
#include "irc_connection.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_ACCOUNT                 (loqui_account_get_type ())
#define LOQUI_ACCOUNT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_ACCOUNT, LoquiAccount))
#define LOQUI_ACCOUNT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_ACCOUNT, LoquiAccountClass))
#define LOQUI_IS_ACCOUNT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_ACCOUNT))
#define LOQUI_IS_ACCOUNT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_ACCOUNT))
#define LOQUI_ACCOUNT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_ACCOUNT, LoquiAccountClass))

typedef struct _LoquiAccount            LoquiAccount;
typedef struct _LoquiAccountClass       LoquiAccountClass;

typedef struct _LoquiAccountPrivate     LoquiAccountPrivate;

#include "loqui_channel_entry.h"
#include "loqui_channel.h"
#include "loqui_sender.h"
#include "irc_handle.h"

struct _LoquiAccount
{
	LoquiChannelEntry parent;

	GList *channel_list;

	/* key: channel name(gchar *), value: GList * */
	GHashTable *channel_name_hash;

	GHashTable *user_nick_table; /* key: user, value: nick */
	GHashTable *nick_user_table; /* key: nick, value: user */

	LoquiUser *user_self;

	LoquiSender *sender;

        LoquiAccountPrivate *priv;
};

struct _LoquiAccountClass
{
        LoquiChannelEntryClass parent_class;

	/* signals */
	void (* connected)        (LoquiAccount *account);
	void (* disconnected)     (LoquiAccount *account);
	void (* add_channel)      (LoquiAccount *account,
				   LoquiChannel *channel);
	void (* remove_channel)   (LoquiAccount *account,
				   LoquiChannel *channel);
	void (* user_self_changed) (LoquiAccount *account);
};

GType loqui_account_get_type(void) G_GNUC_CONST;

LoquiAccount* loqui_account_new(LoquiProfileAccount *profile);

LoquiProfileAccount *loqui_account_get_profile(LoquiAccount *account);
LoquiUser* loqui_account_get_user_self(LoquiAccount *account);

void loqui_account_connect(LoquiAccount *account);
void loqui_account_disconnect(LoquiAccount *account);
gboolean loqui_account_is_connected(LoquiAccount *account);

IRCConnection *loqui_account_get_connection(LoquiAccount *account);
IRCHandle *loqui_account_get_handle(LoquiAccount *account);
LoquiSender *loqui_account_get_sender(LoquiAccount *account);

void loqui_account_set_codeconv(LoquiAccount *account, CodeConv *codeconv);
CodeConv *loqui_account_get_codeconv(LoquiAccount *account);

void loqui_account_add_channel(LoquiAccount *account, LoquiChannel *channel);
void loqui_account_remove_channel(LoquiAccount *account, LoquiChannel *channel);
void loqui_account_remove_all_channel(LoquiAccount *account);

GList *loqui_account_get_channel_list(LoquiAccount *account);
LoquiChannel* loqui_account_get_channel_by_name(LoquiAccount *account, const gchar *name);
GSList *loqui_account_search_joined_channel(LoquiAccount *account, gchar *nick);

void loqui_account_console_buffer_append(LoquiAccount *account, TextType type, gchar *str);

gboolean loqui_account_is_current_nick(LoquiAccount *account, const gchar *str);

void loqui_account_get_updated_number(LoquiAccount *account, gint *updated_private_talk_number, gint *updated_channel_number); 

LoquiUser* loqui_account_fetch_user(LoquiAccount *account, const gchar *nick);
LoquiUser* loqui_account_peek_user(LoquiAccount *account, const gchar *nick);

G_END_DECLS

#endif /* __LOQUI_ACCOUNT_H__ */
