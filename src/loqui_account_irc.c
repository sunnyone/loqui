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

#include "loqui_profile_account_irc.h"
#include "loqui_sender_irc.h"
#include "loqui_receiver_irc.h"

#include "loqui_utils_irc.h"
#include "intl.h"
#include "main.h"
#include "prefs_general.h"

#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
        LAST_PROP
};

struct _LoquiAccountIRCPrivate
{
	IRCConnection *connection;
};

static LoquiAccountClass *parent_class = NULL;

/* static guint loqui_account_irc_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_account_irc_constructor(GType type, guint n_props, GObjectConstructParam *props);

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
	object_class->constructor = loqui_account_irc_constructor;

	account_class->connect = loqui_account_irc_connect;
	account_class->disconnect = loqui_account_irc_disconnect;
}
static void 
loqui_account_irc_init(LoquiAccountIRC *account_irc)
{
	LoquiAccountIRCPrivate *priv;
	LoquiAccount *account;

	priv = g_new0(LoquiAccountIRCPrivate, 1);

	account_irc->priv = priv;

	account = LOQUI_ACCOUNT(account_irc);
}
static GObject*
loqui_account_irc_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
	GObject *object;
	GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	LoquiAccount *account;
	LoquiUser *user;

	object = object_class->constructor(type, n_props, props);
	
	account = LOQUI_ACCOUNT(object);

	loqui_account_set_sender(account, LOQUI_SENDER(loqui_sender_irc_new(account)));
	loqui_account_set_receiver(account, LOQUI_RECEIVER(loqui_receiver_irc_new(account)));

	user = LOQUI_USER(loqui_user_irc_new());
	loqui_user_set_nick(user, loqui_profile_account_get_nick(loqui_account_get_profile(account)));
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);
	loqui_account_set_user_self(account, user);

	return object;
}
LoquiAccountIRC*
loqui_account_irc_new(LoquiProfileAccount *profile)
{
        LoquiAccountIRC *account;
	LoquiAccountIRCPrivate *priv;

	account = g_object_new(loqui_account_irc_get_type(), 
			       "buffer", channel_buffer_new(),
			       "profile", profile,
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
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (loqui_account_get_is_connected(account)) {
		loqui_account_warning(account, _("Already connected."));
		return;
	}
	
	servername = loqui_profile_account_get_servername(loqui_account_get_profile(account));
	port = loqui_profile_account_get_port(loqui_account_get_profile(account));
	codeset_type = loqui_profile_account_irc_get_codeset_type(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account)));
	codeset = loqui_profile_account_irc_get_codeset(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account)));
	
	loqui_account_set_is_connected(account, TRUE);
	priv->connection = irc_connection_new(servername, port);
	
	codeconv = codeconv_new();
	codeconv_set_codeset_type(codeconv, codeset_type);
	if(codeset_type == CODESET_TYPE_CUSTOM)
		codeconv_set_codeset(codeconv, codeset);
	irc_connection_set_codeconv(priv->connection, codeconv);
	
	loqui_account_information(account, _("Connecting to %s:%d"), servername, port);

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
	LoquiSenderIRC *sender;
	const gchar *password, *nick, *username, *realname;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;

	if(!is_success) {
		loqui_account_information(LOQUI_ACCOUNT(account), _("Failed to connect."));
		loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);
		G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
		return;
	}

	loqui_account_information(LOQUI_ACCOUNT(account), _("Connected. Sending Initial command..."));

	password = loqui_profile_account_get_password(loqui_account_get_profile(LOQUI_ACCOUNT(account)));
	nick = loqui_profile_account_get_nick(loqui_account_get_profile(LOQUI_ACCOUNT(account)));	
	username = loqui_profile_account_get_username(loqui_account_get_profile(LOQUI_ACCOUNT(account)));
	realname = loqui_profile_account_irc_get_realname(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(LOQUI_ACCOUNT(account))));	
	
	sender = LOQUI_SENDER_IRC(loqui_account_get_sender(LOQUI_ACCOUNT(account)));

	if(password && strlen(password) > 0) {
		loqui_sender_irc_pass(sender, password);
		debug_puts("Sending PASS...");
	}

	loqui_sender_nick(LOQUI_SENDER(sender), nick);
	debug_puts("Sending NICK...");
	loqui_user_set_nick(LOQUI_ACCOUNT(account)->user_self, nick);

	loqui_sender_irc_user_raw(sender, username, realname);
	debug_puts("Sending USER...");

	loqui_account_information(LOQUI_ACCOUNT(account), _("Done."));

	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_ONLINE);
}
static void
loqui_account_irc_connection_terminated_cb(GObject *object, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	GList *cur;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);

	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);
	loqui_receiver_irc_reset(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver));

	loqui_account_information(LOQUI_ACCOUNT(account), _("Connection terminated."));
	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_OFFLINE);

	for (cur = LOQUI_ACCOUNT(account)->channel_list; cur != NULL; cur = cur->next)
		loqui_channel_set_is_joined(LOQUI_CHANNEL(cur->data), FALSE);

	if(prefs_general.auto_reconnect) {
		loqui_account_information(LOQUI_ACCOUNT(account), _("Trying to reconnect..."));
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

	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);
	loqui_receiver_irc_reset(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver));

	loqui_account_information(LOQUI_ACCOUNT(account), _("Disconnected."));
	loqui_account_remove_all_channel(LOQUI_ACCOUNT(account));

	loqui_user_set_away(loqui_account_get_user_self(LOQUI_ACCOUNT(account)), LOQUI_AWAY_TYPE_OFFLINE);

	loqui_account_disconnected(LOQUI_ACCOUNT(account));
}
static void
loqui_account_irc_connection_warn_cb(GObject *object, gchar *str, LoquiAccountIRC *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	loqui_account_warning(LOQUI_ACCOUNT(account), "%s", str);
}
static void
loqui_account_irc_connection_info_cb(GObject *object, gchar *str, LoquiAccountIRC *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	loqui_account_information(LOQUI_ACCOUNT(account), "%s", str);
}
static void
loqui_account_irc_connection_arrive_message_cb(IRCConnection *connection, IRCMessage *msg, LoquiAccountIRC *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	loqui_receiver_irc_response(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver), msg);
}
static void
loqui_account_irc_disconnect(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (priv->connection)
		irc_connection_disconnect(priv->connection);
}
IRCConnection *
loqui_account_irc_get_connection(LoquiAccountIRC *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), NULL);
	
	return account->priv->connection;
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
