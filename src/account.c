/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2 <http://loqui.good-day.net/>
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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
#include "loqui_gconf.h"
#include "loqui_app.h"
#include "irc_handle.h"
#include "gtkutils.h"
#include "utils.h"
#include "account_manager.h"
#include <string.h>

struct _AccountPrivate
{
	IRCHandle *handle;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT
#define IRC_DEFAULT_PORT 6667

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
}
static void 
account_init (Account *account)
{
	AccountPrivate *priv;

	priv = g_new0(AccountPrivate, 1);

	account->priv = priv;
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

	if(account->name)
		g_free(account->name);
	if(account->nick)
		g_free(account->nick);
	if(account->username)
		g_free(account->username);
	if(account->realname)
		g_free(account->realname);
	if(account->userinfo)
		g_free(account->userinfo);
	if(account->autojoin)
		g_free(account->autojoin);

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
	if(account->use_server_list) {
		g_slist_free(account->use_server_list);
		account->use_server_list = NULL;
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

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account->priv);
}

Account*
account_new (void)
{
        Account *account;

	account = g_object_new(account_get_type(), NULL);
	
	account->console_text = CHANNEL_TEXT(channel_text_new());
	account_manager_add_channel_text(account_manager_get(), account->console_text);
	
	return account;
}

void
account_set(Account *account,
	    const gchar *name,
	    const gchar *nick,
	    const gchar *username,
	    const gchar *realname,
	    const gchar *userinfo,
	    const gchar *autojoin)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_return_if_fail(account != NULL);

	priv = account->priv;

	account->name = g_strdup(name);
	account->nick = g_strdup(nick);
	account->username = g_strdup(username);
	account->realname = g_strdup(realname);
	account->userinfo = g_strdup(userinfo);
	account->autojoin = g_strdup(autojoin);
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

	server->password = g_strdup(password);
	server->use = use;

	account->server_list = g_slist_append(account->server_list, server);
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

/* FIXME: configuration handlling should be more sensitive. 
   broken values may stop the program on the current implementation. */
void 
account_restore(Account *account, const gchar *name)
{
	AccountPrivate *priv;
	gchar *key;
	GSList *list;
	GSList *cur;
	gchar *hostname;
	gint port;
	gchar *password;
	gboolean use;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_return_if_fail(name != NULL);
	g_return_if_fail(strchr(name, '/') == NULL);

	priv = account->priv;

	account->name = g_strdup(name);

#define ACCOUNT_GCONF_GET_STRING(str, subkey) \
{ \
	key = g_strdup_printf("%s/%s/%s", LOQUI_GCONF_ACCOUNT, name, subkey); \
	str = eel_gconf_get_string(key); \
	g_free(key); \
}
	ACCOUNT_GCONF_GET_STRING(account->nick, "nick");
	ACCOUNT_GCONF_GET_STRING(account->username, "username");
	ACCOUNT_GCONF_GET_STRING(account->realname, "realname");
	ACCOUNT_GCONF_GET_STRING(account->userinfo, "userinfo");
	ACCOUNT_GCONF_GET_STRING(account->autojoin, "autojoin");

	key = g_strdup_printf("%s/%s/%s", LOQUI_GCONF_ACCOUNT, name, "server");
	list = eel_gconf_get_dirs(key);
	g_free(key);

#undef ACCOUNT_GCONF_GET_STRING

	for(cur = list; cur != NULL; cur = cur->next) {
		hostname = utils_gconf_get_basename((gchar *) cur->data);
		if(!hostname) {
			g_free(cur->data);
			continue;
		}
#define CONF_SERVER_GET_VALUE(type, ret, subkey) { \
     key = g_strdup_printf("%s/%s", (gchar *) cur->data, subkey); \
     ret = eel_gconf_get_##type(key); \
     g_free(key); \
}
		CONF_SERVER_GET_VALUE(integer, port, "port");
		CONF_SERVER_GET_VALUE(string, password, "password");
		CONF_SERVER_GET_VALUE(boolean, use, "use");

		account_add_server(account, hostname, port, password, use);

		g_free(cur->data);
#undef CONF_SERVER_GET_VALUE
	}
}

void
account_connect(Account *account, gint server_num, gboolean fallback)
{
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	g_return_if_fail(account != NULL);

	if(priv->handle) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
				     _("Account '%s' has already connected."),
				     account->name);
		return;
	}
	account_manager_set_current(account_manager_get(), account, NULL); /* FIXME: one of channel_tree's row should be selected */
	priv->handle = irc_handle_new(account, server_num, fallback);
}
void
account_add_channel(Account *account, Channel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account->channel_list = g_slist_append(account->channel_list, channel);
	account_manager_add_channel(account_manager_get(), account, channel);
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
		if(g_ascii_strcasecmp(channel->name, name) == 0)
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
account_console_text_append(Account *account, gboolean with_common_text, TextType type, gchar *str)
{
	gchar *buf;

	g_return_if_fail(account != NULL);
	g_return_if_fail(str != NULL);

	channel_text_append(account->console_text, type, str);
	if(with_common_text &&
	   !account_manager_is_current_account(account_manager_get(), account)) {
		buf = g_strdup_printf("[%s] %s", account->name, str);
		account_manager_common_text_append(account_manager_get(), type, buf);
		g_free(buf);
	}
}
void
account_speak(Account *account, Channel *channel, const gchar *str)
{
	AccountPrivate *priv;
	const gchar *cur;
	IRCMessage *msg;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	if(priv->handle == NULL) {
		g_warning(_("Not connected with this account"));
		return;
	}

	cur = str;
	if(*cur == '/') {
		g_warning("Not supported");
	} else {
		if(channel == NULL) {
			g_warning(_("No channel is selected"));
			return;
		}

		if(account == NULL) {
			g_warning(_("Can't find account"));
			return;
		}
		msg = irc_message_create(IRCCommandPrivmsg, channel->name, str, NULL);
		irc_handle_push_message(priv->handle, msg);
	}
}
