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
#include "gobject_utils.h"
#include "codeconv.h"

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
typedef struct _Server Server;

#include "channel.h"

struct _Account
{
        GObject parent;

	gchar *name;

	gboolean use;

	GSList *server_list; /* list of Server */
	GList *nick_list; /* list of gchar * */

	/* key: channel name(gchar *), value: channel(Channel) */
	GHashTable *channel_hash;

	gchar *nick;
	gchar *username;
	gchar *realname;
	gchar *userinfo;
	gchar *autojoin;

	ChannelBuffer *console_buffer;

        AccountPrivate *priv;
};

struct _AccountClass
{
        GObjectClass parent_class;

	/* signals */
	void (* connected)        (Account *account);
	void (* disconnected)     (Account *account);
	void (* nick_changed)     (Account *account);
	void (* away_changed)     (Account *account);
	void (* add_channel)      (Account *account,
				   Channel *channel);
	void (* remove_channel)   (Account *account,
				   Channel *channel);
};
struct _Server
{
	gchar *hostname;
	guint port;
	gchar *password;
	gboolean use;
};

typedef enum {
	AWAY_STATE_NONE,
	AWAY_STATE_ONLINE,
	AWAY_STATE_AWAY,
	AWAY_STATE_BUSY,
	AWAY_STATE_AWAY_WITH_MESSAGE,
	AWAY_STATE_CONFIGURE,
	AWAY_STATE_QUIT,
	AWAY_STATE_OFFLINE,
	AWAY_STATE_DISCONNECT
} AwayState;

GType account_get_type(void) G_GNUC_CONST;

Account* account_new(void);
void account_print(Account *account);

#define ACCOUNT_ACCESSOR_STRING(attr_name) \
  ATTR_ACCESSOR_STRING(Account, account, attr_name)
#define ACCOUNT_ACCESSOR_STRING_PROTOTYPE(attr_name) \
  ATTR_ACCESSOR_STRING_PROTOTYPE(Account, account, attr_name)

ACCOUNT_ACCESSOR_STRING_PROTOTYPE(name);
ACCOUNT_ACCESSOR_STRING_PROTOTYPE(nick);
ACCOUNT_ACCESSOR_STRING_PROTOTYPE(username);
ACCOUNT_ACCESSOR_STRING_PROTOTYPE(realname);
ACCOUNT_ACCESSOR_STRING_PROTOTYPE(userinfo);
ACCOUNT_ACCESSOR_STRING_PROTOTYPE(autojoin);

void account_add_server(Account *account, const gchar *hostname,
			gint port, const gchar *password,
			gboolean use);
void account_remove_all_server(Account *account);

/* if server is null, account will be connected with fallback */
void account_connect(Account *account, Server *server);
void account_disconnect(Account *account);
gboolean account_is_connected(Account *account);

void account_set_codeconv(Account *account, CodeConv *codeconv);
CodeConv *account_get_codeconv(Account *account);

void account_add_channel(Account *account, Channel *channel);
void account_remove_channel(Account *account, Channel *channel);
void account_remove_all_channel(Account *account);

Channel* account_get_channel(Account *account, const gchar *name);
GSList *account_search_joined_channel(Account *account, gchar *nick);

void account_console_buffer_append(Account *account, TextType type, gchar *str);
void account_speak(Account *account, Channel *channel, const gchar *str);

void account_set_current_nick(Account *account, const gchar *nick);
G_CONST_RETURN gchar* account_get_current_nick(Account *account);
gboolean account_is_current_nick(Account *account, const gchar *str);

void account_set_away_status(Account *account, gboolean is_away);
gboolean account_get_away_status(Account *account);

void account_set_away(Account *account, gboolean is_away);
void account_set_away_message(Account *account, const gchar *away_message);

void account_change_nick(Account *account, const gchar *nick);
void account_whois(Account *account, const gchar *target);
void account_join(Account *account, const gchar *target);
void account_start_private_talk(Account *account, const gchar *target);
void account_part(Account *account, const gchar *target, const gchar *part_message);
void account_set_topic(Account *account, const gchar *target, const gchar *topic);
void account_pong(Account *account, const gchar *target);
void account_get_channel_mode(Account *account, const gchar *channel_name);
void account_notice(Account *account, const gchar *target, const gchar *str);

void account_send_ctcp_request(Account *account, const gchar *target, const gchar *command);

void account_change_channel_user_mode(Account *account, Channel *channel, 
				      gboolean is_give, IRCModeFlag flag, GList *str_list);
				      
G_END_DECLS

#endif /* __ACCOUNT_H__ */
