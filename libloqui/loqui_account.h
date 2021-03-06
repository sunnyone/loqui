/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2002-2004 Yoichi Imai <sunnyone41@gmail.com>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __LOQUI_ACCOUNT_H__
#define __LOQUI_ACCOUNT_H__

#include <glib.h>

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
#include "loqui_receiver.h"
#include "loqui_profile_account.h"
#include "loqui_codeconv.h"

#define LOQUI_ACCOUNT_USER_SELF_ACCOUNT_KEY "parent-account"
#define LOQUI_ACCOUNT_RECONNECT_COUNT_MAX 5
#define LOQUI_ACCOUNT_RECONNECT_INTERVAL 30000

struct _LoquiAccount
{
	LoquiChannelEntry parent;

	GList *channel_list;

	GHashTable *identifier_channel_table; /* key: channel identifier(gchar *), value: GList * */

	GHashTable *user_identifier_table; /* key: user, value: identifier */
	GHashTable *identifier_user_table; /* key: identifier, value: user */

	/* You can get account with get_user_data and LOQUI_ACCOUNT_USER_SELF_ACCOUNT_KEY */
	LoquiUser *user_self;

	LoquiSender *sender;
	LoquiReceiver *receiver;

	gboolean is_connected;
	gboolean is_pending_reconnecting;

	gint reconnect_try_count;

        LoquiAccountPrivate *priv;
};

struct _LoquiAccountClass
{
        LoquiChannelEntryClass parent_class;

	/* must implement start */
	void (* connect)          (LoquiAccount *account);
	void (* disconnect)       (LoquiAccount *account);
	void (* terminate)        (LoquiAccount *account);
	/* must implement end */

	void (* warn) (LoquiAccount *account, const gchar *str);
	void (* info) (LoquiAccount *account, const gchar *str);

	/* Whenever child class detected the connection is closed , this method is called. 
	   Default handler will try to reconnect if the setting enabled and is_success is false. */
	void (* closed)           (LoquiAccount *account, gboolean is_success);

	void (* add_channel)      (LoquiAccount *account,
				   LoquiChannel *channel);
	void (* remove_channel)   (LoquiAccount *account,
				   LoquiChannel *channel);
};

GType loqui_account_get_type(void) G_GNUC_CONST;

LoquiAccount* loqui_account_new(LoquiProfileAccount *profile);

LoquiProfileAccount *loqui_account_get_profile(LoquiAccount *account);

void loqui_account_set_user_self(LoquiAccount *account, LoquiUser *user_self);
LoquiUser* loqui_account_get_user_self(LoquiAccount *account);

void loqui_account_connect(LoquiAccount *account);
void loqui_account_disconnect(LoquiAccount *account);
void loqui_account_terminate(LoquiAccount *account);
void loqui_account_force_reconnect(LoquiAccount *account);

void loqui_account_cancel_pending_reconnecting(LoquiAccount *account);

/* send signal */
void loqui_account_closed(LoquiAccount *account, gboolean is_success);

LoquiSender *loqui_account_get_sender(LoquiAccount *account);
void loqui_account_set_sender(LoquiAccount *account, LoquiSender *sender);

LoquiReceiver *loqui_account_get_receiver(LoquiAccount *account);
void loqui_account_set_receiver(LoquiAccount *account, LoquiReceiver *receiver);

void loqui_account_add_channel(LoquiAccount *account, LoquiChannel *channel);
void loqui_account_remove_channel(LoquiAccount *account, LoquiChannel *channel);
void loqui_account_remove_all_channel(LoquiAccount *account);

GList *loqui_account_get_channel_list(LoquiAccount *account);
LoquiChannel* loqui_account_get_channel_by_identifier(LoquiAccount *account, const gchar *channel_identifier);
GList *loqui_account_search_joined_channel(LoquiAccount *account, LoquiUser *user);
GList *loqui_account_search_joined_channel_by_identifier(LoquiAccount *account, gchar *user_identifier);

void loqui_account_append_text(LoquiAccount *account,
			       GList *channel_list,
			       LoquiTextType text_type,
			       gchar *text);
void loqui_account_append_text_printf(LoquiAccount *account,
				      GList *channel_list,
				      LoquiTextType text_type,
				      const gchar *format, ...);

void loqui_account_append_text_to_joined_channels(LoquiAccount *account,
						  LoquiUser *user,
						  gboolean fallback_console,
						  LoquiTextType text_type,
						  gchar *text);


void loqui_account_get_updated_number(LoquiAccount *account, gint *updated_private_talk_number, gint *updated_channel_number); 

void loqui_account_add_user(LoquiAccount *account, LoquiUser *user);
LoquiUser* loqui_account_peek_user(LoquiAccount *account, const gchar *identifier);
void loqui_account_remove_user_from_all(LoquiAccount *account, LoquiUser *user, gboolean include_account_channel_entry, GList **removed_channel_entry_list);
void loqui_account_remove_all_user(LoquiAccount *account);

LoquiChannel *loqui_account_open_private_talk(LoquiAccount *account, const gchar *identifier, LoquiUser *user);

void loqui_account_warning(LoquiAccount *account, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void loqui_account_information(LoquiAccount *account, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

void loqui_account_set_is_connected(LoquiAccount *account, gboolean is_connected);
gboolean loqui_account_get_is_connected(LoquiAccount *account);

void loqui_account_set_is_pending_reconnecting(LoquiAccount *account, gboolean is_pending_reconnecting);
gboolean loqui_account_get_is_pending_reconnecting(LoquiAccount *account);

void loqui_account_set_all_channel_unjoined(LoquiAccount *account);

void loqui_account_print_communication(LoquiAccount *account, gboolean receive, gchar *str);

G_END_DECLS

#endif /* __LOQUI_ACCOUNT_H__ */
