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
#include "main.h"
#include "gtkutils.h"
#include "intl.h"
#include "prefs_general.h"
#include "ctcp_message.h"
#include "loqui_profile_account_irc.h"
#include "loqui_user_irc.h"
#include "loqui_utils_irc.h"
#include "loqui_sender_irc.h"

#include <string.h>

enum {
	CONNECTED,
	DISCONNECTED,
	ADD_CHANNEL,
	REMOVE_CHANNEL,
	USER_SELF_CHANGED,
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
static void account_connection_arrive_message_cb(IRCConnection *connection, IRCMessage *msg, Account *account);

static void account_user_notify_nick_cb(LoquiUser *user, GParamSpec *pspec, Account *account);
static void account_user_self_notify_cb(LoquiUser *user_self, GParamSpec *pspec, Account *account);
static void account_profile_notify_name_cb(LoquiProfileAccount *profile, GParamSpec *pspec, Account *account);

static void account_remove_channel_real(Account *account, LoquiChannel *channel);

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

	klass->remove_channel = account_remove_channel_real;

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
						    G_SIGNAL_RUN_LAST,
						    G_STRUCT_OFFSET(AccountClass, add_channel),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__OBJECT,
						    G_TYPE_NONE, 1,
						    LOQUI_TYPE_CHANNEL);
	account_signals[REMOVE_CHANNEL] = g_signal_new("remove-channel",
						       G_OBJECT_CLASS_TYPE(object_class),
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET(AccountClass, remove_channel),
						       NULL, NULL,
						       g_cclosure_marshal_VOID__OBJECT,
						       G_TYPE_NONE, 1,
						       LOQUI_TYPE_CHANNEL);
	account_signals[USER_SELF_CHANGED] = g_signal_new("user-self-changed",
							  G_OBJECT_CLASS_TYPE(object_class),
							  G_SIGNAL_RUN_LAST,
							  G_STRUCT_OFFSET(AccountClass, user_self_changed),
							  NULL, NULL,
							  g_cclosure_marshal_VOID__VOID,
							  G_TYPE_NONE, 0);
}
static void 
account_init(Account *account)
{
	AccountPrivate *priv;

	priv = g_new0(AccountPrivate, 1);

	account->priv = priv;
	
	account->channel_list = NULL;
	account->channel_name_hash = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, NULL);
	
	account->user_nick_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	account->nick_user_table = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, NULL);

	account->sender = LOQUI_SENDER(loqui_sender_irc_new(account));
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

	account_remove_all_channel(account);

	if(account->channel_name_hash) {
		g_hash_table_destroy(account->channel_name_hash);
		account->channel_name_hash = NULL;
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
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);

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

	loqui_channel_entry_set_name(LOQUI_CHANNEL_ENTRY(account), loqui_profile_account_get_name(profile));
	g_signal_connect(G_OBJECT(profile), "notify::name",
			 G_CALLBACK(account_profile_notify_name_cb), account);

	g_object_notify(G_OBJECT(account), "profile");
}
static void
account_profile_notify_name_cb(LoquiProfileAccount *profile, GParamSpec *pspec, Account *account)
{
	loqui_channel_entry_set_name(LOQUI_CHANNEL_ENTRY(account), loqui_profile_account_get_name(profile));
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

	g_signal_connect(user_self, "notify",
			 G_CALLBACK(account_user_self_notify_cb), account);

	g_object_notify(G_OBJECT(account), "user_self");	
}
static void
account_user_self_notify_cb(LoquiUser *user_self, GParamSpec *pspec, Account *account)
{
	g_signal_emit(G_OBJECT(account), account_signals[USER_SELF_CHANGED], 0);
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
	g_signal_connect(G_OBJECT(priv->connection), "arrive_message",
			 G_CALLBACK(account_connection_arrive_message_cb), account);

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

	loqui_user_set_away(account->user_self, LOQUI_AWAY_TYPE_ONLINE);
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
	loqui_user_set_away(account->user_self, LOQUI_AWAY_TYPE_OFFLINE);

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

	loqui_user_set_away(account->user_self, LOQUI_AWAY_TYPE_OFFLINE);
	g_signal_emit(account, account_signals[DISCONNECTED], 0);
}
static void
account_connection_warn_cb(GObject *object, gchar *str, Account *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account_console_buffer_append(account, TEXT_TYPE_ERROR, str);
}
static void
account_connection_info_cb(GObject *object, gchar *str, Account *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	account_console_buffer_append(account, TEXT_TYPE_INFO, str);
}
static void
account_connection_arrive_message_cb(IRCConnection *connection, IRCMessage *msg, Account *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	irc_handle_response(account->priv->handle, msg);
}

void
account_disconnect(Account *account)
{
	AccountPrivate *priv;
	IRCMessage *msg;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	priv = account->priv;

	if(priv->connection) {
		msg = irc_message_create(IRCCommandQuit, "Loqui", NULL);
		irc_connection_disconnect_after_send(priv->connection, msg);
		g_object_unref(msg);
	}
}
gboolean
account_is_connected(Account *account)
{
        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT(account), FALSE);
	
	return (account->priv->connection != NULL);
}

IRCHandle *
account_get_handle(Account *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	
	return account->priv->handle;
}
IRCConnection *
account_get_connection(Account *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	
	return account->priv->connection;
}
LoquiSender *
account_get_sender(Account *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	
	return account->sender;
}

void
account_add_channel(Account *account, LoquiChannel *channel)
{
	GList *l;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_object_ref(channel);

	l = g_list_alloc();
	l->data = channel;

	account->channel_list = g_list_concat(account->channel_list, l);
	g_hash_table_insert(account->channel_name_hash, g_strdup(loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel))), l);

	g_signal_emit(account, account_signals[ADD_CHANNEL], 0, channel);
}
static void
account_remove_channel_real(Account *account, LoquiChannel *channel)
{
	g_hash_table_remove(account->channel_name_hash, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)));

	account->channel_list = g_list_remove(account->channel_list, channel);
	g_object_unref(channel);
}
void
account_remove_channel(Account *account, LoquiChannel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	g_signal_emit(account, account_signals[REMOVE_CHANNEL], 0, channel);
}
void
account_remove_all_channel(Account *account)
{
	GList *list, *cur;

        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));

	list = g_list_copy(account->channel_list);
	for (cur = list; cur != NULL; cur = cur->next) {
		account_remove_channel(account, cur->data);
	}
	g_list_free(list);
}

GList *
account_get_channel_list(Account *account)
{
       g_return_val_if_fail(account != NULL, NULL);
       g_return_val_if_fail(IS_ACCOUNT(account), NULL);

       return account->channel_list;
}
LoquiChannel*
account_get_channel_by_name(Account *account, const gchar *name)
{
	GList *l;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	l = g_hash_table_lookup(account->channel_name_hash, name);
	if (l)
		return l->data;

	return NULL;
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

GSList *
account_search_joined_channel(Account *account, gchar *nick)
{
	GList *cur;
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

	for (cur = account->channel_list; cur != NULL; cur = cur->next) {
		chent = LOQUI_CHANNEL_ENTRY(cur->data);
		if (loqui_channel_entry_get_member_by_user(chent, user))
			list = g_slist_append(list, chent);
	}
	
	return list;
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
account_get_updated_number(Account *account, gint *updated_private_talk_number, gint *updated_channel_number)
{
	LoquiChannel *channel;
	GList *cur;
		
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
        g_return_if_fail(updated_private_talk_number != NULL);
        g_return_if_fail(updated_channel_number != NULL);

	*updated_private_talk_number = 0;
	*updated_channel_number = 0;
	
	for(cur = account->channel_list; cur != NULL; cur = cur->next) {
		channel = LOQUI_CHANNEL(cur->data);
		if (loqui_channel_entry_get_is_updated(LOQUI_CHANNEL_ENTRY(channel))) {
			if(loqui_channel_get_is_private_talk(channel)) {
				(*updated_private_talk_number)++;
			} else {
				(*updated_channel_number)++;
			}
		}
	}
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
