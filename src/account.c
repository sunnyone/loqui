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
#include "irc_handle.h"
#include "gtkutils.h"
#include "utils.h"
#include "account_manager.h"
#include "main.h"
#include "gtkutils.h"
#include "intl.h"
#include "prefs_general.h"

#include <string.h>

enum {
	NICK_CHANGED,
	AWAY_CHANGED,
	ADD_CHANNEL,
	REMOVE_CHANNEL,
	LAST_SIGNAL
};

struct _AccountPrivate
{
	IRCHandle *handle;

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

	account_signals[NICK_CHANGED] = g_signal_new("nick-changed",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_FIRST,
						     0,
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 0);
	account_signals[AWAY_CHANGED] = g_signal_new("away-changed",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_FIRST,
						     0,
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

	if(priv->handle) {
		g_object_unref(priv->handle);
		priv->handle = NULL;
	}

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
	if(account->channel_list) {
		for(cur = account->channel_list; cur != NULL; cur = cur->next) {
			if(cur->data) {
				g_object_unref(cur->data);
			}
		}
		g_slist_free(account->channel_list);
		account->channel_list = NULL;
	}

	if(account->console_buffer) {
		g_object_unref(account->console_buffer);
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account->priv);
}

Account*
account_new (void)
{
        Account *account;

	account = g_object_new(account_get_type(), NULL);
	
	account->console_buffer = channel_buffer_new();
	
	return account;
}

#define DEFINE_STRING_ACCESSOR(n) \
void account_set_##n(Account *account, const gchar *str) \
{ \
 	g_return_if_fail(account != NULL); \
        g_return_if_fail(IS_ACCOUNT(account)); \
\
	G_FREE_UNLESS_NULL(account->n); \
	if(str) \
		account->n = g_strdup(str); \
} \
gchar *account_get_##n(Account *account) \
{ \
	g_return_val_if_fail(account != NULL, NULL); \
        g_return_val_if_fail(IS_ACCOUNT(account), NULL); \
\
        return account->n; \
}

DEFINE_STRING_ACCESSOR(name);
DEFINE_STRING_ACCESSOR(nick);
DEFINE_STRING_ACCESSOR(username);
DEFINE_STRING_ACCESSOR(realname);
DEFINE_STRING_ACCESSOR(userinfo);
DEFINE_STRING_ACCESSOR(autojoin);

void
account_set(Account *account,
	    gboolean use,
	    const gchar *name,
	    const gchar *nick,
	    const gchar *username,
	    const gchar *realname,
	    const gchar *userinfo,
	    const gchar *autojoin)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account->use = use;
	account_set_name(account, name);
	account_set_nick(account, nick);
	account_set_username(account, username);
	account_set_realname(account, realname);
	account_set_userinfo(account, userinfo);
	account_set_autojoin(account, autojoin);
}
void
account_print(Account *account)
{
	GSList *cur;
	Server *server;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_print("ACCOUNT[%s] {\n", account_get_name(account));
	g_print(" use: %s\n", account->use ? "TRUE" : "FALSE");
	g_print(" nick: %s\n", account_get_nick(account));
	g_print(" username: %s\n", account_get_username(account));
	g_print(" realname: %s\n", account_get_realname(account));
	g_print(" userinfo: %s\n", account_get_userinfo(account));
	g_print(" autojoin: %s\n", account_get_autojoin(account));
	
	for(cur = account->server_list; cur != NULL; cur = cur->next) {
		server = (Server *) cur->data;
		g_print(" server: hostname => %s, port => %d, password => %s, use => %d\n",
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
account_connect(Account *account, Server *server)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	if(account_is_connected(account)) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
				     _("Account '%s' has already connected."),
				     account_get_name(account));
		return;
	}

	account_manager_set_current_account(account_manager_get(), account); 
	priv->handle = irc_handle_new(account, server);
}
void
account_disconnect(Account *account)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	priv = account->priv;

	if(!priv->handle)
		return;

	/* TODO: remove channels from channel tree */
	account_manager_remove_channels_of_account(account_manager_get(), account);

	irc_handle_disconnect(priv->handle);
	priv->handle = NULL;
}
gboolean account_is_connected(Account *account)
{
        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT(account), FALSE);
	
	return (account->priv->handle != NULL);
}

void
account_add_channel(Account *account, Channel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account->channel_list = g_slist_append(account->channel_list, channel);

	g_signal_emit(account, account_signals[ADD_CHANNEL], 0, channel);
}
void account_remove_channel(Account *account, Channel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account->channel_list = g_slist_remove(account->channel_list, channel);

	g_signal_emit(account, account_signals[REMOVE_CHANNEL], 0, channel);
}
Channel*
account_search_channel_by_name(Account *account, gchar *name)
{
	GSList *cur;
	Channel *channel;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	g_return_val_if_fail(name != NULL, NULL);

	for(cur = account->channel_list; cur != NULL; cur = cur->next) {
		channel = CHANNEL(cur->data);
		if(g_ascii_strcasecmp(channel_get_name(channel), name) == 0)
			return channel;
	}
	return NULL;
}
gboolean
account_has_channel(Account *account, Channel *channel)
{
	GSList *cur;
	Channel *tmp;

        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT(account), FALSE);
	g_return_val_if_fail(channel != NULL, FALSE);

	for(cur = account->channel_list; cur != NULL; cur = cur->next) {
		tmp = CHANNEL(cur->data);
		if(channel == tmp)
			return TRUE;
	}
	return FALSE;
}

void
account_console_buffer_append(Account *account, gboolean with_common_buffer, TextType type, gchar *str)
{
	gchar *buf;

	g_return_if_fail(account != NULL);
	g_return_if_fail(str != NULL);

	channel_buffer_append_line(account->console_buffer, type, str);
	if(with_common_buffer &&
	   !account_manager_is_current_channel_buffer(account_manager_get(), account->console_buffer)) {
		buf = g_strdup_printf("[%s] %s", account->name, str);
		account_manager_common_buffer_append(account_manager_get(), type, buf);
		g_free(buf);
	}
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

	if(priv->handle == NULL) {
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
		irc_handle_push_message(priv->handle, msg);
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
			irc_handle_push_message(priv->handle, msg);
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
	
	irc_handle_push_message(priv->handle, msg);
}

GSList *
account_search_joined_channel(Account *account, gchar *nick)
{
	GSList *list = NULL, *cur;
	Channel *channel = NULL;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(nick != NULL, NULL);

	for(cur = account->channel_list; cur != NULL; cur = cur->next) {
		channel = (Channel *) cur->data;
		if(!channel) continue;
		
		if(channel_find_user(channel, nick, NULL)) {
			list = g_slist_append(list, channel);
		}
	}

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
	irc_handle_push_message(priv->handle, msg);
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

	msg = irc_message_create(IRCCommandWhois, target, NULL);
	irc_handle_push_message(priv->handle, msg);
}
void account_join(Account *account, const gchar *target)
{
	IRCMessage *msg;
	AccountPrivate *priv;
	Channel *channel;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
		
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	if(STRING_IS_CHANNEL(target)) {
		msg = irc_message_create(IRCCommandJoin, target, NULL);
		irc_handle_push_message(priv->handle, msg);
	} else {
		channel = channel_new(account, target);
		account_add_channel(account, channel);
	}
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
		irc_handle_push_message(priv->handle, msg);
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
		irc_handle_push_message(priv->handle, msg);
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
	param_array[p] = channel_get_name(channel);
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
	irc_handle_push_message(priv->handle, msg);
}
