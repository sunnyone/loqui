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
#include "prefs_general.h"
#include "loqui.h"

#include <string.h>
#include <gnet.h>
#include "codeconv.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
        LAST_PROP
};

struct _LoquiAccountIRCPrivate
{
	GConn *conn;
	gboolean is_conn_connected;

	CodeConv *codeconv;
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

static void loqui_account_irc_conn_error_cb(GConn *conn, LoquiAccountIRC *account);
static void loqui_account_irc_conn_connected_cb(GConn *conn, LoquiAccountIRC *account);
static void loqui_account_irc_conn_closed_cb(GConn *conn, LoquiAccountIRC *account);
static void loqui_account_irc_conn_readline_cb(GConn *conn, const gchar *buffer, LoquiAccountIRC *account);

static void loqui_account_irc_connect(LoquiAccount *account);
static void loqui_account_irc_disconnect(LoquiAccount *account);
static void loqui_account_irc_terminate(LoquiAccount *account);

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

	if (priv->conn) {
		gnet_conn_unref(priv->conn);
		priv->conn = NULL;
	}
	G_OBJECT_UNREF_UNLESS_NULL(priv->codeconv);

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
	account_class->terminate = loqui_account_irc_terminate;
	
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
loqui_account_irc_conn_cb(GConn *conn, GConnEvent *event, LoquiAccountIRC *account)
{
	switch (event->type) {
	case GNET_CONN_ERROR:
		loqui_account_irc_conn_error_cb(conn, account);
		break;
	case GNET_CONN_CONNECT:
		loqui_account_irc_conn_connected_cb(conn, account);
		break;
	case GNET_CONN_CLOSE:
		loqui_account_irc_conn_closed_cb(conn, account);
		break;
	case GNET_CONN_READ:
		loqui_account_irc_conn_readline_cb(conn, event->buffer, account);
	default:
		break;
	} 
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
	priv->conn = gnet_conn_new(servername, port, (GConnFunc) loqui_account_irc_conn_cb, account);

	codeconv = codeconv_new();
	codeconv_set_codeset_type(codeconv, codeset_type);
	if(codeset_type == CODESET_TYPE_CUSTOM)
		codeconv_set_codeset(codeconv, codeset);
	priv->codeconv = codeconv;
	
	loqui_account_information(account, _("Connecting to %s:%d"), servername, port);

	priv->is_conn_connected = FALSE;
	gnet_conn_connect(priv->conn);
}

static void
loqui_account_irc_conn_error_cb(GConn *conn, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;

	if (!priv->is_conn_connected) {
		loqui_account_warning(LOQUI_ACCOUNT(account), _("Failed to connect."));
	} else {
		loqui_account_warning(LOQUI_ACCOUNT(account), _("An error occured on connection."));
	}

	priv->is_conn_connected = FALSE;
	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);

	if (priv->conn) {
		gnet_conn_unref(priv->conn);
		priv->conn = NULL;
	}

	loqui_account_closed(LOQUI_ACCOUNT(account));
}
static void
loqui_account_irc_conn_connected_cb(GConn *conn, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	LoquiSenderIRC *sender;
	const gchar *password, *nick, *username, *realname;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;

	priv->is_conn_connected = TRUE;
	loqui_account_information(LOQUI_ACCOUNT(account), _("Connected."));

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

	loqui_account_information(LOQUI_ACCOUNT(account), _("Sent initial commands."));

	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_ONLINE);

	gnet_conn_readline(priv->conn);
}
static void
loqui_account_irc_conn_closed_cb(GConn *conn, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	LoquiSenderIRC *sender;
	GList *cur;
	gboolean sent_quit;
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;
		
	if (priv->conn) {
		gnet_conn_unref(priv->conn);
		priv->conn = NULL;
	}
	
	priv->is_conn_connected = FALSE;
	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);
	loqui_receiver_irc_reset(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver));
	
	sender = LOQUI_SENDER_IRC(LOQUI_ACCOUNT(account)->sender);
	sent_quit = sender->sent_quit;
	loqui_sender_irc_reset(sender);

	loqui_account_information(LOQUI_ACCOUNT(account), _("Connection closed."));
	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_OFFLINE);

	for (cur = LOQUI_ACCOUNT(account)->channel_list; cur != NULL; cur = cur->next)
		loqui_channel_set_is_joined(LOQUI_CHANNEL(cur->data), FALSE);
	
	if (!sent_quit)
		loqui_account_closed(LOQUI_ACCOUNT(account));
}
static void
loqui_account_irc_conn_readline_cb(GConn *conn, const gchar *buffer, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	IRCMessage *msg;
	gchar *local;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	local = codeconv_to_local(priv->codeconv, buffer);
	if (local == NULL) {
		loqui_account_warning(LOQUI_ACCOUNT(account), "Failed to convert codeset.");
		return;
	}

	msg = irc_message_parse_line(local);
	g_free(local);

	if (msg == NULL) {
		loqui_account_warning(LOQUI_ACCOUNT(account), "Failed to parse a line");
		return;
	}
	if (loqui_get_show_msg_mode())
		irc_message_print(msg);

	loqui_receiver_irc_response(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver), msg);
	g_object_unref(msg);

	gnet_conn_readline(conn);
}
static void
loqui_account_irc_disconnect(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (LOQUI_ACCOUNT_CLASS(parent_class)->disconnect)
		(* LOQUI_ACCOUNT_CLASS(parent_class)->disconnect) (account);

	if (priv->conn && LOQUI_RECEIVER_IRC(loqui_account_get_receiver(account))->passed_welcome) {
		loqui_sender_quit(loqui_account_get_sender(account),
				  loqui_profile_account_irc_get_quit_message(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account))));
	} else {
		loqui_account_irc_terminate(account);
	}
}
static void
loqui_account_irc_terminate(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (LOQUI_ACCOUNT_CLASS(parent_class)->terminate)
		(* LOQUI_ACCOUNT_CLASS(parent_class)->terminate) (account);

	if (priv->conn) {
		gnet_conn_delete(priv->conn);
		priv->conn = NULL;
		
		loqui_account_information(LOQUI_ACCOUNT(account), _("Terminated."));
		loqui_account_remove_all_channel(LOQUI_ACCOUNT(account));
		
		loqui_user_set_away(loqui_account_get_user_self(LOQUI_ACCOUNT(account)), LOQUI_AWAY_TYPE_OFFLINE);

		loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);
		loqui_receiver_irc_reset(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver));
		loqui_sender_irc_reset(LOQUI_SENDER_IRC(LOQUI_ACCOUNT(account)->sender));
	}
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
void
loqui_account_irc_push_message(LoquiAccountIRC *account, IRCMessage *msg)
{
	LoquiAccountIRCPrivate *priv;
	gchar *serv_str, *buf, *tmp;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;
	
	if (!priv->conn) {
		loqui_account_warning(LOQUI_ACCOUNT(account), _("The account is not connected."));
		return;
	}

	buf = irc_message_to_string(msg);
	serv_str = codeconv_to_server(priv->codeconv, buf);
	g_free(buf);

	tmp = g_strdup_printf("%s\r\n", serv_str);
	g_free(serv_str);

	gnet_conn_write(priv->conn, tmp, strlen(tmp));
	g_free(tmp);
}
