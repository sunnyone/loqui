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
#include "config.h"

#include "account.h"
#include "loqui_app.h"
#include "irc_connection.h"
#include "irc_handle.h"
#include "gtkutils.h"
#include "utils.h"
#include "main.h"
#include "gtkutils.h"
#include "intl.h"
#include "prefs_general.h"
#include "ctcp_message.h"

#include <string.h>

enum {
	CONNECTED,
	DISCONNECTED,
	NICK_CHANGED,
	AWAY_CHANGED,
	ADD_CHANNEL,
	REMOVE_CHANNEL,
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_NAME,
	PROP_USE,
	PROP_NICK,
	PROP_USERNAME,
	PROP_REALNAME,
	PROP_USERINFO,
	PROP_AUTOJOIN,
	PROP_CODECONV,
	LAST_PROP
};

struct _AccountPrivate
{
	IRCHandle *handle;
	IRCConnection *connection;

	CodeConv *codeconv;
	Server *server_on_connecting;

	gchar *current_nick;
	gboolean is_away;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT
#define IRC_DEFAULT_PORT 6667

static guint account_signals[LAST_SIGNAL] = { 0 };

static void account_class_init(AccountClass *klass);
static void account_init(Account *account);
static void account_finalize(GObject *object);

static void account_get_property(GObject  *object,
				 guint param_id,
                                 GValue *value,
                                 GParamSpec *pspec);
static void account_set_property(GObject *object,
				 guint param_id,
				 const GValue *value,
				 GParamSpec *pspec);

static void account_connection_connected_cb(GObject *object, gboolean is_succes, Account *account);
static void account_connection_disconnected_cb(GObject *object, Account *account);
static void account_connection_terminated_cb(GObject *object, Account *account);
static void account_connection_warn_cb(GObject *object, gchar *str, Account *account);
static void account_connection_info_cb(GObject *object, gchar *str, Account *account);
static void account_connect_internal(Account *account, Server *server);

GType
account_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(AccountClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) account_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(Account),
				0,              /* n_preallocs */
				(GInstanceInitFunc) account_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "Account",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
account_class_init (AccountClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = account_finalize;
	object_class->set_property = account_set_property;
	object_class->get_property = account_get_property;

	g_object_class_install_property(object_class,
					PROP_NAME,
					g_param_spec_string("name",
							    _("Name"),
							    _("Account name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_NICK,
					g_param_spec_string("nick",
							    _("Nick"),
							    _("Nickname at first"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_USE,
					g_param_spec_boolean("use",
							     _("Use account"),
							     _("Whether or not to connect this account by default"),
							     TRUE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_USERNAME,
					g_param_spec_string("username",
							    _("Username"),
							    _("Username"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_REALNAME,
					g_param_spec_string("realname",
							    _("Realname"),
							    _("Realname"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_USERINFO,
					g_param_spec_string("userinfo",
							    _("User information"),
							    _("User information used with CTCP USERINFO"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_AUTOJOIN,
					g_param_spec_string("autojoin",
							    _("Autojoin channels"),
							    _("Channels which are joined automatically"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_CODECONV,
					g_param_spec_object("codeconv",
							    _("Code Converter"),
							    _("Character code converter for IRC server"),
							    TYPE_CODECONV, G_PARAM_READWRITE));


	account_signals[CONNECTED] = g_signal_new("connected",
						  G_OBJECT_CLASS_TYPE(object_class),
						  G_SIGNAL_RUN_FIRST,
						  G_STRUCT_OFFSET(AccountClass, connected),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 0);
	account_signals[DISCONNECTED] = g_signal_new("disconnected",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET(AccountClass, disconnected),
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 0);
	account_signals[NICK_CHANGED] = g_signal_new("nick-changed",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET(AccountClass, nick_changed),
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 0);
	account_signals[AWAY_CHANGED] = g_signal_new("away-changed",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET(AccountClass, away_changed),
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 0);
	account_signals[ADD_CHANNEL] = g_signal_new("add-channel",
						    G_OBJECT_CLASS_TYPE(object_class),
						    G_SIGNAL_RUN_FIRST,
						    G_STRUCT_OFFSET(AccountClass, add_channel),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__OBJECT,
						    G_TYPE_NONE, 1,
						    TYPE_CHANNEL);
	account_signals[REMOVE_CHANNEL] = g_signal_new("remove-channel",
						       G_OBJECT_CLASS_TYPE(object_class),
						       G_SIGNAL_RUN_FIRST,
						       G_STRUCT_OFFSET(AccountClass, remove_channel),
						       NULL, NULL,
						       g_cclosure_marshal_VOID__OBJECT,
						       G_TYPE_NONE, 1,
						       TYPE_CHANNEL);
}
static void 
account_init (Account *account)
{
	AccountPrivate *priv;

	priv = g_new0(AccountPrivate, 1);

	account->priv = priv;
	priv->is_away = FALSE;
	account->channel_hash = g_hash_table_new_full(channel_name_hash, channel_name_equal, g_free, g_object_unref);
	priv->codeconv = codeconv_new();
	codeconv_set_codeset_type(priv->codeconv, CODESET_TYPE_AUTO_DETECTION);
}
static void 
account_finalize (GObject *object)
{
	Account *account;
	AccountPrivate *priv;
	GSList *cur;
	Server *server;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT(object));

        account = ACCOUNT(object);
	priv = account->priv;
	
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);
	G_OBJECT_UNREF_UNLESS_NULL(priv->codeconv);

	G_FREE_UNLESS_NULL(account->name);
	G_FREE_UNLESS_NULL(account->nick);
	G_FREE_UNLESS_NULL(account->username);
	G_FREE_UNLESS_NULL(account->realname);
	G_FREE_UNLESS_NULL(account->userinfo);
	G_FREE_UNLESS_NULL(account->autojoin);

	for(cur = account->server_list; cur != NULL; cur = cur->next) {
		if(cur->data) {
			server = (Server *) cur->data;
			if(server->hostname) g_free(server->hostname);
			if(server->password) g_free(server->password);
			g_free(server);
		}
	}
	if(account->server_list) {
		g_slist_free(account->server_list);
		account->server_list = NULL;
	}
	if(account->channel_hash) {
		g_hash_table_destroy(account->channel_hash);
		account->channel_hash = NULL;
	}

	G_OBJECT_UNREF_UNLESS_NULL(account->console_buffer);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account->priv);
}
static void account_set_property(GObject *object,
				 guint param_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	Account *account;

	account = ACCOUNT(object);

	switch(param_id) {
	case PROP_NAME:
		account_set_name(account, g_value_get_string(value));
		break;
	case PROP_NICK:
		account_set_nick(account, g_value_get_string(value));
		break;
	case PROP_USE:
		account->use = g_value_get_boolean(value);
		break;
	case PROP_USERNAME:
		account_set_username(account, g_value_get_string(value));
		break;
	case PROP_REALNAME:
		account_set_realname(account, g_value_get_string(value));
		break;
	case PROP_USERINFO:
		account_set_userinfo(account, g_value_get_string(value));
		break;
	case PROP_AUTOJOIN:
		account_set_autojoin(account, g_value_get_string(value));
		break;
	case PROP_CODECONV:
		account_set_codeconv(account, g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}

}
static void account_get_property(GObject  *object,
				 guint param_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
	Account *account;

	account = ACCOUNT(object);

	switch(param_id) {
	case PROP_NAME:
		g_value_set_string(value, account_get_name(account));
		break;
	case PROP_NICK:
		g_value_set_string(value, account_get_nick(account));
		break;
	case PROP_USE:
		g_value_set_boolean(value, account->use);
		break;
	case PROP_USERNAME:
		g_value_set_string(value, account_get_username(account));
		break;
	case PROP_REALNAME:
		g_value_set_string(value, account_get_realname(account));
		break;
	case PROP_USERINFO:
		g_value_set_string(value, account_get_userinfo(account));
		break;
	case PROP_AUTOJOIN:
		g_value_set_string(value, account_get_autojoin(account));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}
Account*
account_new (void)
{
        Account *account;

	account = g_object_new(account_get_type(), NULL);
	
	account->console_buffer = channel_buffer_new();
	
	return account;
}

ACCOUNT_ACCESSOR_STRING(name);
ACCOUNT_ACCESSOR_STRING(nick);
ACCOUNT_ACCESSOR_STRING(username);
ACCOUNT_ACCESSOR_STRING(realname);
ACCOUNT_ACCESSOR_STRING(userinfo);
ACCOUNT_ACCESSOR_STRING(autojoin);

void
account_set_codeconv(Account *account, CodeConv *codeconv)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(codeconv != NULL);
	g_return_if_fail(IS_CODECONV(codeconv));
	
	G_OBJECT_UNREF_UNLESS_NULL(account->priv->codeconv);
	g_object_ref(codeconv);
	account->priv->codeconv = codeconv;
}
CodeConv *
account_get_codeconv(Account *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	return account->priv->codeconv;
}
void
account_print(Account *account)
{
	GSList *cur;
	Server *server;
	GParamSpec **param_specs;
	guint n_properties;
	gint i;
	const gchar *param_name;
	gchar *value_str;
	GValue value = { 0, };

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	param_specs = g_object_class_list_properties(G_OBJECT_CLASS(ACCOUNT_GET_CLASS(account)),
						     &n_properties);
	g_print("Account { ");

	g_value_init(&value, G_TYPE_STRING);
	for(i = 0; i < n_properties; i++) {
		param_name = g_param_spec_get_name(param_specs[i]);
		g_object_get_property(G_OBJECT(account), param_name, &value);
		value_str = g_strdup_value_contents(&value);
		g_print("%s = %s; ", param_name, value_str);
		g_free(value_str);
	}
	g_print("\n");

	for(cur = account->server_list; cur != NULL; cur = cur->next) {
		server = (Server *) cur->data;
		g_print("  Server { hostname = '%s', port = %d, password = '%s', use = %d } \n",
			server->hostname, server->port, server->password, server->use);
	}
	g_print("}\n");
}

void
account_add_server(Account *account, const gchar *hostname, gint port,
		   const gchar *password, gboolean use)
{
	Server *server;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	server = g_new0(Server, 1);

	server->hostname = g_strdup(hostname);
	if(port == 0)
		server->port = IRC_DEFAULT_PORT;
	else
		server->port = port;

	if(password && strlen(password) > 0)
		server->password = g_strdup(password);
	server->use = use;

	account->server_list = g_slist_append(account->server_list, server);
}
void
account_remove_all_server(Account *account)
{
	GSList *cur;
	Server *server;

	for(cur = account->server_list; cur != NULL; cur = cur->next) {
		server = (Server *) cur->data;

		if(server->password)
			g_free(server->password);
		if(server->hostname)
			g_free(server->hostname);
		g_free(server);
	}
	g_slist_free(account->server_list);
	account->server_list = NULL;
}
#if 0
static Server*
account_parse_server_string(const gchar *input)
{
	Server *server;
	gchar *str, *s, *t;
	gdouble d;

	g_return_val_if_fail(input != NULL, NULL);
	server = g_new0(Server, 1);
	str = g_strdup(input);

	s = strchr(str, '@');
	if(s != NULL) {
		do {
			t = s;
		} while((s = strchr(++s, '@')) != NULL);
		*t = '\0';
		server->password = g_strdup(str);

		s = ++t;
	} else
		s = str;
 
	t = strchr(s, ':');
	if(t != NULL) {
		*t = '\0';
		d = g_ascii_strtod(t+1, NULL);
		if(d == 0)
			server->port = IRC_DEFAULT_PORT;
		else
			server->port = d;
	}
	server->port = IRC_DEFAULT_PORT;

	if(strlen(s) < 1) {
		g_warning("parse_server_string error: no hostname was found.");
		g_free(server->password);
		g_free(server);
		g_free(str);
		return NULL;
	}
	server->hostname = g_strdup(s);

	g_free(str);
	return server;
}
#endif

void
account_connect_default(Account *account)
{
	AccountPrivate *priv;
	GSList *cur;
	Server *tmp_server = NULL;
		
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	priv = account->priv;
	
	for(cur = account->server_list; cur != NULL; cur = cur->next) {
		if(cur->data && ((Server *) cur->data)->use) {
			tmp_server = (Server *) cur->data;
			break;
		}
	}
	if(!tmp_server) {
		account_console_buffer_append(account, TEXT_TYPE_ERROR, _("No usable servers found."));
	} else {
		account_connect_internal(account, tmp_server);
	}
}	
void
account_connect(Account *account, Server *server)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(server != NULL);
	
	priv = account->priv;

	if(account_is_connected(account)) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
				     _("Account '%s' has already connected."),
				     account_get_name(account));
		return;
	}
	
	account_connect_internal(account, server);
}
static void
account_connect_internal(Account *account, Server *server)
{
	AccountPrivate *priv;
	gchar *str;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(server != NULL);
	g_return_if_fail(!account_is_connected(account));

	priv = account->priv;

	priv->handle = irc_handle_new(account);

	priv->server_on_connecting = server;
	priv->connection = irc_connection_new(server->hostname, server->port);
	irc_connection_set_codeconv(priv->connection, account_get_codeconv(account));
	irc_connection_set_irc_handle(priv->connection, priv->handle);

	str = g_strdup_printf(_("Connecting to %s:%d"), server->hostname, server->port);
	account_console_buffer_append(account, TEXT_TYPE_INFO, str);
	g_free(str);

	irc_connection_connect(priv->connection);

	g_signal_connect(G_OBJECT(priv->connection), "connected",
			 G_CALLBACK(account_connection_connected_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "disconnected",
			 G_CALLBACK(account_connection_disconnected_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "terminated",
			 G_CALLBACK(account_connection_terminated_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "warn",
			 G_CALLBACK(account_connection_warn_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "info",
			 G_CALLBACK(account_connection_info_cb), account);

	g_signal_emit(account, account_signals[CONNECTED], 0);
}

static void
account_connection_connected_cb(GObject *object, gboolean is_success, Account *account)
{
	AccountPrivate *priv;
	IRCMessage *msg;
	Server *server;
	const gchar *tmp;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;
	server = priv->server_on_connecting;

	if(!is_success) {
		account_console_buffer_append(account, TEXT_TYPE_INFO, _("Failed to connect."));
//		if(priv->fallback_current)
//			account_connect_internal(account, server); /* FIXME: not NULL */
		return;
	}

	account_console_buffer_append(account, TEXT_TYPE_INFO, _("Connected. Sending Initial command..."));

	if(server->password && strlen(server->password) > 0) {
		msg = irc_message_create(IRCCommandPass, server->password, NULL);
		if(debug_mode) {
			debug_puts("Sending PASS...");
			irc_message_print(msg);
		}
		irc_connection_push_message(priv->connection, msg);
		g_object_unref(msg);
	}

	msg = irc_message_create(IRCCommandNick, account->nick, NULL);
	if(debug_mode) {
		debug_puts("Sending NICK...");
		irc_message_print(msg);
	}
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);

	account_set_current_nick(account, account->nick);
	
	msg = irc_message_create(IRCCommandUser, 
				 account->username, "*", "*", 
				 account->realname, NULL);
	if(debug_mode) {
		debug_puts("Sending USER...");
		irc_message_print(msg);
	}
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);

	tmp = account_get_autojoin(account);
	if(tmp && strlen(tmp) > 0) {
		msg = irc_message_create(IRCCommandJoin, tmp, NULL);
		if(debug_mode) {
			debug_puts("Sending JOIN for autojoin...");
			irc_message_print(msg);
		}
		irc_connection_push_message(priv->connection, msg);
		g_object_unref(msg);

		account_console_buffer_append(account, TEXT_TYPE_INFO, _("Sent join command for autojoin."));
	}

	account_console_buffer_append(account, TEXT_TYPE_INFO, _("Done."));
}
static void
account_connection_terminated_cb(GObject *object, Account *account)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);

	account_console_buffer_append(account, TEXT_TYPE_INFO, _("Connection terminated."));

	if(prefs_general.auto_reconnect) {
		if(!priv->server_on_connecting) {
			g_warning(_("Can't find the server connecting."));
			return;
		}
		account_console_buffer_append(account, TEXT_TYPE_INFO, _("Trying to reconnect..."));
		account_connect_internal(account, priv->server_on_connecting);
	}
}
static void
account_connection_disconnected_cb(GObject *object, Account *account)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;
	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);

	account_console_buffer_append(account, TEXT_TYPE_INFO, _("Disconnected."));
	account_remove_all_channel(account);

	g_signal_emit(account, account_signals[DISCONNECTED], 0);
}
static void account_connection_warn_cb(GObject *object, gchar *str, Account *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account_console_buffer_append(account, TEXT_TYPE_ERROR, str);
}
static void account_connection_info_cb(GObject *object, gchar *str, Account *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account_console_buffer_append(account, TEXT_TYPE_INFO, str);
}
void
account_disconnect(Account *account)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	priv = account->priv;

	if(priv->connection)
		irc_connection_disconnect(priv->connection);
}
gboolean
account_is_connected(Account *account)
{
        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT(account), FALSE);
	
	return (account->priv->connection != NULL);
}
void
account_add_channel(Account *account, Channel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_hash_table_insert(account->channel_hash, g_strdup(channel_get_name(channel)), channel);

	g_signal_emit(account, account_signals[ADD_CHANNEL], 0, channel);
}
void
account_remove_channel(Account *account, Channel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_signal_emit(account, account_signals[REMOVE_CHANNEL], 0, channel);

	g_hash_table_remove(account->channel_hash, channel_get_name(channel));
}
static gboolean
account_remove_channel_func(gpointer key, gpointer value, Account *account)
{
	g_signal_emit(account, account_signals[REMOVE_CHANNEL], 0, CHANNEL(value));
	return TRUE;
}
void
account_remove_all_channel(Account *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	g_hash_table_foreach_remove(account->channel_hash, (GHRFunc) account_remove_channel_func, account);
}

Channel*
account_get_channel(Account *account, const gchar *name)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return (Channel *) g_hash_table_lookup(account->channel_hash, name);
}
void
account_console_buffer_append(Account *account, TextType type, gchar *str)
{
	MessageText *msgtext;

	g_return_if_fail(account != NULL);
	g_return_if_fail(str != NULL);

	msgtext = message_text_new();
	g_object_set(G_OBJECT(msgtext),
		     "is_remark", FALSE,
		     "account_name", account->name,
		     "text_type", type,
		     "text", str,
		     NULL);

	channel_buffer_append_message_text(account->console_buffer, msgtext, FALSE, FALSE);
	g_object_unref(msgtext);
}
void
account_speak(Account *account, Channel *channel, const gchar *str)
{
	AccountPrivate *priv;
	const gchar *cur;
	gchar *buf;
	IRCMessage *msg;
	gchar **array;
	gint i;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	if(!account_is_connected(account)) {
		gtkutils_msgbox_info(GTK_MESSAGE_WARNING,
				     _("Not connected with this account"));
		return;
	}

	cur = str;
	if(*cur == '/' && !strchr(cur, '\n')) {
		cur++;
		msg = irc_message_parse_line(cur);
		if(debug_mode) {
			buf = irc_message_to_string(msg);
			debug_puts("msg: %s", buf);
			g_free(buf);
		}
		irc_connection_push_message(priv->connection, msg);
		g_object_unref(msg);
	} else {
		if(channel == NULL) {
			gtkutils_msgbox_info(GTK_MESSAGE_WARNING,
					     _("No channel is selected"));
			return;
		}
		buf = g_strdup(str);
		utils_remove_return_code(buf); /* remove last return code */
		array = g_strsplit(buf, "\n", -1);
		g_free(buf);
		for(i = 0; array[i] != NULL; i++) {
			if(strlen(array[i]) == 0)
				continue;

			msg = irc_message_create(IRCCommandPrivmsg, channel_get_name(channel), array[i], NULL);
			irc_connection_push_message(priv->connection, msg);
			g_object_unref(msg);
			channel_append_remark(channel, TEXT_TYPE_NORMAL, TRUE, account_get_current_nick(account), array[i]);
		}
		g_strfreev(array);
	}
}

void
account_set_current_nick(Account *account, const gchar *nick)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	G_FREE_UNLESS_NULL(priv->current_nick);
	priv->current_nick = g_strdup(nick);

	g_signal_emit(account, account_signals[NICK_CHANGED], 0);
}

G_CONST_RETURN gchar *
account_get_current_nick(Account *account)
{
	AccountPrivate *priv;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	priv = account->priv;

	return priv->current_nick;
}

gboolean
account_is_current_nick(Account *account, const gchar *str)
{
	AccountPrivate *priv;

        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT(account), FALSE);

	priv = account->priv;

	g_return_val_if_fail(priv->current_nick != NULL, FALSE);

	if(str == NULL)
		return FALSE;
	
	return (strcmp(priv->current_nick, str) == 0 ? TRUE : FALSE);
}
void
account_set_away_status(Account *account, gboolean is_away)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account->priv->is_away = is_away;
	if(is_away)
		debug_puts("Now away.");
	else
		debug_puts("Now unaway.");

	g_signal_emit(account, account_signals[AWAY_CHANGED], 0);
}
gboolean
account_get_away_status(Account *account)
{
        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT(account), FALSE);

	return account->priv->is_away;
}

void
account_set_away(Account *account, gboolean is_away)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	if(!account_is_connected(account))
		return;

	if(is_away)
		account_set_away_message(account, prefs_general.away_message);
	else
		account_set_away_message(account, NULL);
}
void
account_set_away_message(Account *account, const gchar *away_message)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	if(!account_is_connected(account))
		return;

	if(away_message == NULL)
		msg = irc_message_create(IRCCommandAway, NULL);
	else
		msg = irc_message_create(IRCCommandAway, away_message, NULL);
	
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}

GSList *
account_search_joined_channel(Account *account, gchar *nick)
{
	GList *channel_list, *cur;
	GSList *list = NULL;
	Channel *channel = NULL;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(nick != NULL, NULL);

	channel_list = utils_get_value_list_from_hash(account->channel_hash);
	for(cur = channel_list; cur != NULL; cur = cur->next) {
		channel = CHANNEL(cur->data);
		if(channel_find_user(channel, nick, NULL)) {
			list = g_slist_append(list, channel);
		}
	}
	if(channel_list)
		g_list_free(channel_list);
	
	return list;
}

void account_change_nick(Account *account, const gchar *nick)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	msg = irc_message_create(IRCCommandNick, nick, NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
void account_send_ctcp_request(Account *account, const gchar *target, const gchar *command)
{
	IRCMessage *msg;
	CTCPMessage *ctcp_msg;
	gchar *buf;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	ctcp_msg = ctcp_message_new(command, NULL);
	buf = ctcp_message_to_str(ctcp_msg);
	g_object_unref(ctcp_msg);
	msg = irc_message_create(IRCCommandPrivmsg, target, buf, NULL);
	g_free(buf);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);

	buf = g_strdup_printf(_("Sent CTCP request to %s: %s"), target, command);
	account_console_buffer_append(account, TEXT_TYPE_INFO, buf);
	g_free(buf);
}
void account_whois(Account *account, const gchar *target)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	msg = irc_message_create(IRCCommandWhois, target, target, NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
void
account_join(Account *account, const gchar *target)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
		
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	msg = irc_message_create(IRCCommandJoin, target, NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
void
account_start_private_talk(Account *account, const gchar *target)
{
	Channel *channel;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	if(STRING_IS_CHANNEL(target)) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
				     _("This nick seems to be channel."));
	}

	channel = channel_new(account, target);
	account_add_channel(account, channel);
}
void account_part(Account *account, const gchar *target, const gchar *part_message)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
		
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	if(STRING_IS_CHANNEL(target)) {
		msg = irc_message_create(IRCCommandPart, target, part_message, NULL);
		irc_connection_push_message(priv->connection, msg);
	} else {
		/* FIXME: close private message page? */
	}	
}
void account_set_topic(Account *account, const gchar *target, const gchar *topic)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
		
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	if(STRING_IS_CHANNEL(target)) {
		msg = irc_message_create(IRCCommandTopic, target, topic, NULL);
		irc_connection_push_message(priv->connection, msg);
		g_object_unref(msg);
	}
}
void account_change_channel_user_mode(Account *account, Channel *channel, 
				      gboolean is_give, IRCModeFlag flag, GList *str_list)
{
	IRCMessage *msg;
	GList *cur;
	guint i, p, list_num;
	gchar flag_str[IRC_MESSAGE_PARAMETER_MAX + 10];
	gchar *param_array[IRC_MESSAGE_PARAMETER_MAX + 10];
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	list_num = g_list_length(str_list);
	if(list_num > IRC_MESSAGE_PARAMETER_MAX) {
		g_warning(_("Too many users in change mode request!"));
		return;
	}
	
	p = 0;
	/* MODE #Channel +? user1 user2 user3 */
	param_array[p] = (gchar *) channel_get_name(channel);
	p++;

	if(is_give)
		flag_str[0] = '+';
	else
		flag_str[0] = '-';

	for(i = 0; i < list_num; i++)
		flag_str[i+1] = (gchar) flag;
	flag_str[i+1] = '\0';

	param_array[p] = flag_str;
	p++;

	for(cur = str_list; cur != NULL; cur = cur->next) {
		param_array[p] = cur->data;
		p++;
	}
	param_array[p] = NULL;

	msg = irc_message_createv(IRCCommandMode, param_array);
	debug_puts("Sending MODE command.\n");
	if(show_msg_mode)
		irc_message_print(msg);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
void
account_pong(Account *account, const gchar *target)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	msg = irc_message_create(IRCCommandPong, target, NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);

	debug_puts("put PONG to %s", target);
}
void
account_get_channel_mode(Account *account, const gchar *channel_name)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(channel_name != NULL);

	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	msg = irc_message_create(IRCCommandMode, channel_name, NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
void
account_notice(Account *account, const gchar *target, const gchar *str)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(str != NULL);

	priv = account->priv;

	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	msg = irc_message_create(IRCCommandNotice, target, str, NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
void
account_fetch_away_information(Account *account, Channel *channel)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
        g_return_if_fail(channel != NULL);
        g_return_if_fail(IS_CHANNEL(channel));        

	priv = account->priv;

	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv->handle->prevent_print_who_reply_count++;
	
	msg = irc_message_create(IRCCommandWho, channel_get_name(channel), NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
