/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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

#include "loqui_account_irc.h"
#include "irc_handle.h"

#include "loqui_profile_account_irc.h"
#include "loqui_utils_irc.h"
#include "loqui_sender_irc.h"
#include "intl.h"
#include "main.h"
#include "prefs_general.h"

#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiAccountIRCPrivate
{
	IRCHandle *handle;
	IRCConnection *connection;
};

static LoquiAccountClass *parent_class = NULL;

/* static guint loqui_account_irc_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_account_irc_class_init(LoquiAccountIRCClass *klass);
static void loqui_account_irc_init(LoquiAccountIRC *account);
static void loqui_account_irc_finalize(GObject *object);
static void loqui_account_irc_dispose(GObject *object);

static void loqui_account_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_account_irc_connection_connected_cb(GObject *object, gboolean is_succes, LoquiAccountIRC *account);
static void loqui_account_irc_connection_disconnected_cb(GObject *object, LoquiAccountIRC *account);
static void loqui_account_irc_connection_terminated_cb(GObject *object, LoquiAccountIRC *account);
static void loqui_account_irc_connection_warn_cb(GObject *object, gchar *str, LoquiAccountIRC *account);
static void loqui_account_irc_connection_info_cb(GObject *object, gchar *str, LoquiAccountIRC *account);
static void loqui_account_irc_connection_arrive_message_cb(IRCConnection *connection, IRCMessage *msg, LoquiAccountIRC *account);

static void loqui_account_irc_connect(LoquiAccount *account);
static void loqui_account_irc_disconnect(LoquiAccount *account);
static gboolean loqui_account_irc_is_connected(LoquiAccount *account);

GType
loqui_account_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccountIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_ACCOUNT,
					      "LoquiAccountIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_account_irc_finalize(GObject *object)
{
	LoquiAccountIRC *account;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(object));

        account = LOQUI_ACCOUNT_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(account->priv);
}
static void 
loqui_account_irc_dispose(GObject *object)
{
	LoquiAccountIRC *account;
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(object));

        account = LOQUI_ACCOUNT_IRC(object);
	priv = account->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);
	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_account_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiAccountIRC *account;        

        account = LOQUI_ACCOUNT_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_account_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiAccountIRC *account;        

        account = LOQUI_ACCOUNT_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_account_irc_class_init(LoquiAccountIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	LoquiAccountClass *account_class = LOQUI_ACCOUNT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_irc_finalize;
        object_class->dispose = loqui_account_irc_dispose;
        object_class->get_property = loqui_account_irc_get_property;
        object_class->set_property = loqui_account_irc_set_property;
	
	account_class->connect = loqui_account_irc_connect;
	account_class->disconnect = loqui_account_irc_disconnect;
	account_class->is_connected = loqui_account_irc_is_connected;
}
static void 
loqui_account_irc_init(LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;

	priv = g_new0(LoquiAccountIRCPrivate, 1);

	account->priv = priv;

	loqui_account_set_sender(LOQUI_ACCOUNT(account), LOQUI_SENDER(loqui_sender_irc_new(LOQUI_ACCOUNT(account))));
}
LoquiAccountIRC*
loqui_account_irc_new(LoquiProfileAccount *profile)
{
        LoquiAccountIRC *account;
	LoquiAccountIRCPrivate *priv;
	LoquiUser *user;

	user = LOQUI_USER(loqui_user_irc_new());
	loqui_user_set_nick(user, loqui_profile_account_get_nick(profile));
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);

	account = g_object_new(loqui_account_irc_get_type(), 
			       "buffer", channel_buffer_new(),
			       "profile", profile,
			       "user_self", user,
			       NULL);

        priv = account->priv;

        return account;
}
LoquiUserIRC *
loqui_account_irc_fetch_user(LoquiAccountIRC *account, const gchar *nick)
{
	LoquiUser *user;
		
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), NULL);

	if((user = loqui_account_peek_user(LOQUI_ACCOUNT(account), nick)) == NULL) {
		user = LOQUI_USER(loqui_user_irc_new());
		loqui_user_set_nick(user, nick);
		loqui_account_add_user(LOQUI_ACCOUNT(account), user);
	} else {
		g_object_ref(user);
	}

	return LOQUI_USER_IRC(user);
}
static void
loqui_account_irc_connect(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;
	const gchar *servername, *codeset;
	gint port;
	gint codeset_type;
	
	CodeConv *codeconv;
	gchar *str;
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if(loqui_account_is_connected(account)) {
		loqui_account_console_buffer_append(account, TEXT_TYPE_ERROR, _("Already connected."));
		return;
	}
	
	servername = loqui_profile_account_get_servername(loqui_account_get_profile(account));
	port = loqui_profile_account_get_port(loqui_account_get_profile(account));
	codeset_type = loqui_profile_account_irc_get_codeset_type(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account)));
	codeset = loqui_profile_account_irc_get_codeset(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account)));
	
	priv->handle = irc_handle_new(account);

	priv->connection = irc_connection_new(servername, port);

	codeconv = codeconv_new();
	codeconv_set_codeset_type(codeconv, codeset_type);
	if(codeset_type == CODESET_TYPE_CUSTOM)
		codeconv_set_codeset(codeconv, codeset);
	irc_connection_set_codeconv(priv->connection, codeconv);
	
	str = g_strdup_printf(_("Connecting to %s:%d"), servername, port);
	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, str);
	g_free(str);

	irc_connection_connect(priv->connection);

	g_signal_connect(G_OBJECT(priv->connection), "connected",
			 G_CALLBACK(loqui_account_irc_connection_connected_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "disconnected",
			 G_CALLBACK(loqui_account_irc_connection_disconnected_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "terminated",
			 G_CALLBACK(loqui_account_irc_connection_terminated_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "warn",
			 G_CALLBACK(loqui_account_irc_connection_warn_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "info",
			 G_CALLBACK(loqui_account_irc_connection_info_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "arrive_message",
			 G_CALLBACK(loqui_account_irc_connection_arrive_message_cb), account);
}

static void
loqui_account_irc_connection_connected_cb(GObject *object, gboolean is_success, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	IRCMessage *msg;
	const gchar *password, *nick, *username, *realname, *autojoin;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;

	if(!is_success) {
		loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, _("Failed to connect."));
		G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
		return;
	}

	loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, _("Connected. Sending Initial command..."));

	password = loqui_profile_account_get_password(loqui_account_get_profile(LOQUI_ACCOUNT(account)));
	nick = loqui_profile_account_get_nick(loqui_account_get_profile(LOQUI_ACCOUNT(account)));	
	username = loqui_profile_account_get_username(loqui_account_get_profile(LOQUI_ACCOUNT(account)));
	realname = loqui_profile_account_irc_get_realname(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(LOQUI_ACCOUNT(account))));	
	autojoin = loqui_profile_account_irc_get_autojoin(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(LOQUI_ACCOUNT(account))));
	
	if(password && strlen(password) > 0) {
		msg = irc_message_create(IRCCommandPass, password, NULL);
		if(debug_mode) {
			debug_puts("Sending PASS...");
			irc_message_print(msg);
		}
		irc_connection_push_message(priv->connection, msg);
		g_object_unref(msg);
	}

	msg = irc_message_create(IRCCommandNick, nick, NULL);
	if(debug_mode) {
		debug_puts("Sending NICK...");
		irc_message_print(msg);
	}
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);

	loqui_user_set_nick(LOQUI_ACCOUNT(account)->user_self, nick);
	
	msg = irc_message_create(IRCCommandUser, 
				 username, "*", "*", 
				 realname, NULL);
	if(debug_mode) {
		debug_puts("Sending USER...");
		irc_message_print(msg);
	}
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);

	if(autojoin && strlen(autojoin) > 0) {
		msg = irc_message_create(IRCCommandJoin, autojoin, NULL);
		if(debug_mode) {
			debug_puts("Sending JOIN for autojoin...");
			irc_message_print(msg);
		}
		irc_connection_push_message(priv->connection, msg);
		g_object_unref(msg);

		loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, _("Sent join command for autojoin."));
	}

	loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, _("Done."));

	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_ONLINE);
}
static void
loqui_account_irc_connection_terminated_cb(GObject *object, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);

	loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, _("Connection terminated."));
	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_OFFLINE);

	if(prefs_general.auto_reconnect) {
		loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, _("Trying to reconnect..."));
		loqui_account_connect(LOQUI_ACCOUNT(account));
	}
}
static void
loqui_account_irc_connection_disconnected_cb(GObject *object, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);

	loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, _("Disconnected."));
	loqui_account_remove_all_channel(LOQUI_ACCOUNT(account));

	loqui_user_set_away(loqui_account_get_user_self(LOQUI_ACCOUNT(account)), LOQUI_AWAY_TYPE_OFFLINE);
	g_signal_emit_by_name(account, "disconnected", 0);
}
static void
loqui_account_irc_connection_warn_cb(GObject *object, gchar *str, LoquiAccountIRC *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_ERROR, str);
}
static void
loqui_account_irc_connection_info_cb(GObject *object, gchar *str, LoquiAccountIRC *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	loqui_account_console_buffer_append(LOQUI_ACCOUNT(account), TEXT_TYPE_INFO, str);
}
static void
loqui_account_irc_connection_arrive_message_cb(IRCConnection *connection, IRCMessage *msg, LoquiAccountIRC *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	irc_handle_response(account->priv->handle, msg);
}

static void
loqui_account_irc_disconnect(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;
	IRCMessage *msg;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (priv->connection) {
		msg = irc_message_create(IRCCommandQuit, "Loqui", NULL);
		irc_connection_disconnect_after_send(priv->connection, msg);
		g_object_unref(msg);
	}
}
IRCHandle *
loqui_account_irc_get_handle(LoquiAccountIRC *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), NULL);
	
	return account->priv->handle;
}
IRCConnection *
loqui_account_irc_get_connection(LoquiAccountIRC *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), NULL);
	
	return account->priv->connection;
}
static gboolean
loqui_account_irc_is_connected(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), FALSE);
	
	return (LOQUI_ACCOUNT_IRC(account)->priv->connection != NULL);
}
gboolean
loqui_account_irc_is_current_nick(LoquiAccountIRC *account, const gchar *str)
{
	LoquiAccountIRCPrivate *priv;

        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), FALSE);

	priv = account->priv;

	if(str == NULL)
		return FALSE;
	
	return (strcmp(loqui_user_get_nick(loqui_account_get_user_self(LOQUI_ACCOUNT(account))), str) == 0 ? TRUE : FALSE);
}
