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
#include "loqui_profile_account_irc.h"
#include "loqui_user_irc.h"
#include "loqui_utils_irc.h"

#include <string.h>

enum {
	CONNECTED,
	DISCONNECTED,
	ADD_CHANNEL,
	REMOVE_CHANNEL,
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_USER_SELF,
	PROP_PROFILE,
	LAST_PROP
};

struct _AccountPrivate
{
	IRCHandle *handle;
	IRCConnection *connection;
	LoquiProfileAccount *profile;
	
	CodeConv *codeconv;
};

static LoquiChannelEntryClass *parent_class = NULL;
#define PARENT_TYPE LOQUI_TYPE_CHANNEL_ENTRY

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

static void account_set_profile(Account *account, LoquiProfileAccount *profile);
static void account_set_user_self(Account *account, LoquiUser *user_self);

static void account_add_user(Account *account, LoquiUser *user);

static void account_connection_connected_cb(GObject *object, gboolean is_succes, Account *account);
static void account_connection_disconnected_cb(GObject *object, Account *account);
static void account_connection_terminated_cb(GObject *object, Account *account);
static void account_connection_warn_cb(GObject *object, gchar *str, Account *account);
static void account_connection_info_cb(GObject *object, gchar *str, Account *account);

static void account_user_notify_nick_cb(LoquiUser *user, GParamSpec *pspec, Account *account);

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
					PROP_USER_SELF,
					g_param_spec_object("user_self",
							    _("Myself"),
							    _("user self"),
							    LOQUI_TYPE_USER,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class,
					PROP_PROFILE,
					g_param_spec_object("profile",
							    _("Profile"),
							    _("Profile of this account"),
							    LOQUI_TYPE_PROFILE_ACCOUNT,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

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
	account_signals[ADD_CHANNEL] = g_signal_new("add-channel",
						    G_OBJECT_CLASS_TYPE(object_class),
						    G_SIGNAL_RUN_FIRST,
						    G_STRUCT_OFFSET(AccountClass, add_channel),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__OBJECT,
						    G_TYPE_NONE, 1,
						    LOQUI_TYPE_CHANNEL);
	account_signals[REMOVE_CHANNEL] = g_signal_new("remove-channel",
						       G_OBJECT_CLASS_TYPE(object_class),
						       G_SIGNAL_RUN_FIRST,
						       G_STRUCT_OFFSET(AccountClass, remove_channel),
						       NULL, NULL,
						       g_cclosure_marshal_VOID__OBJECT,
						       G_TYPE_NONE, 1,
						       LOQUI_TYPE_CHANNEL);
}
static void 
account_init(Account *account)
{
	AccountPrivate *priv;

	priv = g_new0(AccountPrivate, 1);

	account->priv = priv;
	
	account->channel_hash = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, g_object_unref);
	
	account->user_nick_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	account->nick_user_table = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, NULL);
}
static void 
account_finalize (GObject *object)
{
	Account *account;
	AccountPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT(object));

        account = ACCOUNT(object);
	priv = account->priv;
	
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);
	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->profile);
	
	if(account->channel_hash) {
		g_hash_table_destroy(account->channel_hash);
		account->channel_hash = NULL;
	}
	if (account->nick_user_table) {
		g_hash_table_destroy(account->nick_user_table);
		account->nick_user_table = NULL;
	}

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
	case PROP_USER_SELF:
		account_set_user_self(account, g_value_get_object(value));
		break;
	case PROP_PROFILE:
		account_set_profile(account, g_value_get_object(value));
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
	case PROP_USER_SELF:
		g_value_set_object(value, account->user_self);
		break;
	case PROP_PROFILE:
		g_value_set_object(value, account->priv->profile);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}
Account*
account_new (LoquiProfileAccount *profile)
{
        Account *account;
	LoquiUser *user;

	user = LOQUI_USER(loqui_user_irc_new());
	loqui_user_set_nick(user, loqui_profile_account_get_nick(profile));
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_ONLINE);

	account = g_object_new(account_get_type(), 
			       "buffer", channel_buffer_new(),
			       "profile", profile,
			       "user_self", user,
			       NULL);

	return account;
}

LoquiProfileAccount *
account_get_profile(Account *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	return account->priv->profile;
}
static void
account_set_profile(Account *account, LoquiProfileAccount *profile)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	G_OBJECT_UNREF_UNLESS_NULL(account->priv->profile);
	g_object_ref(profile);
	account->priv->profile = profile;

	g_object_notify(G_OBJECT(account), "profile");
}
LoquiUser *
account_get_user_self(Account *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	return account->user_self;
}
static void
account_set_user_self(Account *account, LoquiUser *user_self)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	G_OBJECT_UNREF_UNLESS_NULL(account->user_self);

	g_object_ref(user_self);
	account->user_self = user_self;
	account_add_user(account, user_self);

	g_object_notify(G_OBJECT(account), "user_self");	
}
void
account_connect(Account *account)
{
	AccountPrivate *priv;
	const gchar *servername, *codeset;
	gint port;
	gint codeset_type;
	
	CodeConv *codeconv;
	gchar *str;
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	priv = account->priv;

	if(account_is_connected(account)) {
		account_console_buffer_append(account, TEXT_TYPE_ERROR, _("Already connected."));
		return;
	}
	
	servername = loqui_profile_account_get_servername(LOQUI_PROFILE_ACCOUNT(priv->profile));
	port = loqui_profile_account_get_port(LOQUI_PROFILE_ACCOUNT(priv->profile));
	codeset_type = loqui_profile_account_irc_get_codeset_type(LOQUI_PROFILE_ACCOUNT_IRC(priv->profile));
	codeset = loqui_profile_account_irc_get_codeset(LOQUI_PROFILE_ACCOUNT_IRC(priv->profile));
	
	priv->handle = irc_handle_new(account);

	priv->connection = irc_connection_new(servername, port);

	codeconv = codeconv_new();
	codeconv_set_codeset_type(codeconv, codeset_type);
	if(codeset_type == CODESET_TYPE_CUSTOM)
		codeconv_set_codeset(codeconv, codeset);
	irc_connection_set_codeconv(priv->connection, codeconv);
	
	irc_connection_set_irc_handle(priv->connection, priv->handle);

	str = g_strdup_printf(_("Connecting to %s:%d"), servername, port);
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
	const gchar *password, *nick, *username, *realname, *autojoin;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	priv = account->priv;

	if(!is_success) {
		account_console_buffer_append(account, TEXT_TYPE_INFO, _("Failed to connect."));
		G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
		return;
	}

	account_console_buffer_append(account, TEXT_TYPE_INFO, _("Connected. Sending Initial command..."));

	password = loqui_profile_account_get_password(LOQUI_PROFILE_ACCOUNT(priv->profile));
	nick = loqui_profile_account_get_nick(LOQUI_PROFILE_ACCOUNT(priv->profile));	
	username = loqui_profile_account_get_username(LOQUI_PROFILE_ACCOUNT(priv->profile));
	realname = loqui_profile_account_irc_get_realname(LOQUI_PROFILE_ACCOUNT_IRC(priv->profile));	
	autojoin = loqui_profile_account_irc_get_autojoin(LOQUI_PROFILE_ACCOUNT_IRC(priv->profile));
	
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

	loqui_user_set_nick(account->user_self, nick);
	
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
		account_console_buffer_append(account, TEXT_TYPE_INFO, _("Trying to reconnect..."));
		account_connect(account);
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
account_add_channel(Account *account, LoquiChannel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_hash_table_insert(account->channel_hash, g_strdup(loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel))), channel);

	g_signal_emit(account, account_signals[ADD_CHANNEL], 0, channel);
}
void
account_remove_channel(Account *account, LoquiChannel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_signal_emit(account, account_signals[REMOVE_CHANNEL], 0, channel);

	g_hash_table_remove(account->channel_hash, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)));
}
static gboolean
account_remove_channel_func(gpointer key, gpointer value, Account *account)
{
	g_signal_emit(account, account_signals[REMOVE_CHANNEL], 0, LOQUI_CHANNEL(value));
	return TRUE;
}
void
account_remove_all_channel(Account *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	g_hash_table_foreach_remove(account->channel_hash, (GHRFunc) account_remove_channel_func, account);
}

LoquiChannel*
account_get_channel(Account *account, const gchar *name)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return (LoquiChannel *) g_hash_table_lookup(account->channel_hash, name);
}
void
account_console_buffer_append(Account *account, TextType type, gchar *str)
{
	MessageText *msgtext;
	ChannelBuffer *buffer;

	g_return_if_fail(account != NULL);
	g_return_if_fail(str != NULL);

	msgtext = message_text_new();
	g_object_set(G_OBJECT(msgtext),
		     "is_remark", FALSE,
		     "account_name", loqui_profile_account_get_name(account_get_profile(account)),
		     "text_type", type,
		     "text", str,
		     NULL);

	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(account));
	if (buffer)
		channel_buffer_append_message_text(buffer, msgtext, FALSE, FALSE);
	g_object_unref(msgtext);
}
void
account_speak(Account *account, LoquiChannel *channel, const gchar *str, gboolean command_mode)
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
	if (command_mode) {
		if (strchr(cur, '\n')) {
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
					     _("Command contains linefeed."));
			return;
		}
		
		if (strncmp(cur, prefs_general.command_prefix, strlen(prefs_general.command_prefix)) == 0)
			cur += strlen(prefs_general.command_prefix);

		msg = irc_message_parse_line(cur);
		if (debug_mode) {
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

			msg = irc_message_create(IRCCommandPrivmsg, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), array[i], NULL);
			irc_connection_push_message(priv->connection, msg);
			g_object_unref(msg);
			loqui_channel_append_remark(channel, TEXT_TYPE_NORMAL, TRUE, loqui_user_get_nick(account->user_self), array[i]);
		}
		g_strfreev(array);
	}
}
gboolean
account_is_current_nick(Account *account, const gchar *str)
{
	AccountPrivate *priv;

        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT(account), FALSE);

	priv = account->priv;

	if(str == NULL)
		return FALSE;
	
	return (strcmp(loqui_user_get_nick(account->user_self), str) == 0 ? TRUE : FALSE);
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
	LoquiChannelEntry *chent;
	LoquiUser *user;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(nick != NULL, NULL);

	user = account_peek_user(account, nick);
	if (!user) {
		return NULL;
	}

	channel_list = utils_get_value_list_from_hash(account->channel_hash);
	for (cur = channel_list; cur != NULL; cur = cur->next) {
		chent = LOQUI_CHANNEL_ENTRY(cur->data);
		if (loqui_channel_entry_get_member_by_user(chent, user))
			list = g_slist_append(list, chent);
	}
	if (channel_list)
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
	LoquiChannel *channel;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(target)) {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR,
				     _("This nick seems to be channel."));
	}

	channel = loqui_channel_new(account, target, TRUE, TRUE);
	account_add_channel(account, channel);
}
void account_part(Account *account, const gchar *target, const gchar *part_message)
{
	IRCMessage *msg;
	AccountPrivate *priv;
	LoquiChannel *channel;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
		
	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv = account->priv;

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(target)) {
		msg = irc_message_create(IRCCommandPart, target, part_message, NULL);
		irc_connection_push_message(priv->connection, msg);
	} else {
		/* close private talk */
		channel = account_get_channel(account, target);
		if (channel)
			account_remove_channel(account, channel);
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

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(target)) {
		msg = irc_message_create(IRCCommandTopic, target, topic, NULL);
		irc_connection_push_message(priv->connection, msg);
		g_object_unref(msg);
	}
}
void account_change_channel_user_mode(Account *account, LoquiChannel *channel, 
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
	param_array[p] = (gchar *) loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel));
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
account_fetch_away_information(Account *account, LoquiChannel *channel)
{
	IRCMessage *msg;
	AccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
        g_return_if_fail(channel != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL(channel));        

	priv = account->priv;

	if(!account_is_connected(account)) {
		g_warning(_("Account is not connected."));
		return;
	}

	priv->handle->prevent_print_who_reply_count++;
	
	msg = irc_message_create(IRCCommandWho, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), NULL);
	irc_connection_push_message(priv->connection, msg);
	g_object_unref(msg);
}
void
account_get_updated_number(Account *account, gint *updated_private_talk_number, gint *updated_channel_number)
{
	LoquiChannel *channel;
	GList *list, *cur;
		
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
        g_return_if_fail(updated_private_talk_number != NULL);
        g_return_if_fail(updated_channel_number != NULL);

	*updated_private_talk_number = 0;
	*updated_channel_number = 0;
	
	list = utils_get_value_list_from_hash(account->channel_hash);
	
	for(cur = list; cur != NULL; cur = cur->next) {
		channel = LOQUI_CHANNEL(cur->data);
		if (loqui_channel_entry_get_is_updated(LOQUI_CHANNEL_ENTRY(channel))) {
			if(loqui_channel_get_is_private_talk(channel)) {
				(*updated_private_talk_number)++;
			} else {
				(*updated_channel_number)++;
			}
		}
	}
	
	g_list_free(list);
}
static void
account_user_disposed_cb(Account *account, LoquiUser *user)
{
	gchar *nick;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	nick = g_hash_table_lookup(account->user_nick_table, user);
	g_hash_table_remove(account->nick_user_table, nick);
	g_hash_table_remove(account->user_nick_table, user);
}
static void
account_user_notify_nick_cb(LoquiUser *user, GParamSpec *pspec, Account *account)
{
	const gchar *nick_old, *nick_new;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	nick_old = g_hash_table_lookup(account->user_nick_table, user);
	nick_new = loqui_user_get_nick(user);

	g_hash_table_remove(account->nick_user_table, nick_old);
	g_hash_table_insert(account->nick_user_table, g_strdup(nick_new), user);
	g_hash_table_replace(account->user_nick_table, user, g_strdup(nick_new));
}
static void
account_add_user(Account *account, LoquiUser *user)
{
	g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_signal_connect(G_OBJECT(user), "notify::nick",
			 G_CALLBACK(account_user_notify_nick_cb), account);
	g_object_weak_ref(G_OBJECT(user), (GWeakNotify) account_user_disposed_cb, account);
	g_hash_table_insert(account->nick_user_table, g_strdup(loqui_user_get_nick(user)), user);
	g_hash_table_insert(account->user_nick_table, user, g_strdup(loqui_user_get_nick(user)));	
}
LoquiUser *
account_fetch_user(Account *account, const gchar *nick)
{
	LoquiUser *user;
		
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	if((user = account_peek_user(account, nick)) == NULL) {
		user = LOQUI_USER(loqui_user_irc_new());
		loqui_user_set_nick(user, nick);
		account_add_user(account, user);
	} else {
		g_object_ref(user);
	}

	return user;
}
LoquiUser *
account_peek_user(Account *account, const gchar *nick)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	return g_hash_table_lookup(account->nick_user_table, nick);
}
