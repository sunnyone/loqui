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

#include "loqui_account.h"
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

struct _LoquiAccountPrivate
{
	IRCHandle *handle;
	IRCConnection *connection;
	LoquiProfileAccount *profile;
	
	CodeConv *codeconv;
};

static LoquiChannelEntryClass *parent_class = NULL;
#define PARENT_TYPE LOQUI_TYPE_CHANNEL_ENTRY

static guint account_signals[LAST_SIGNAL] = { 0 };

static void loqui_account_class_init(LoquiAccountClass *klass);
static void loqui_account_init(LoquiAccount *account);
static void loqui_account_finalize(GObject *object);

static void loqui_account_get_property(GObject  *object,
				 guint param_id,
                                 GValue *value,
                                 GParamSpec *pspec);
static void loqui_account_set_property(GObject *object,
				 guint param_id,
				 const GValue *value,
				 GParamSpec *pspec);

static void loqui_account_set_profile(LoquiAccount *account, LoquiProfileAccount *profile);
static void loqui_account_set_user_self(LoquiAccount *account, LoquiUser *user_self);

static void loqui_account_add_user(LoquiAccount *account, LoquiUser *user);

static void loqui_account_connection_connected_cb(GObject *object, gboolean is_succes, LoquiAccount *account);
static void loqui_account_connection_disconnected_cb(GObject *object, LoquiAccount *account);
static void loqui_account_connection_terminated_cb(GObject *object, LoquiAccount *account);
static void loqui_account_connection_warn_cb(GObject *object, gchar *str, LoquiAccount *account);
static void loqui_account_connection_info_cb(GObject *object, gchar *str, LoquiAccount *account);
static void loqui_account_connection_arrive_message_cb(IRCConnection *connection, IRCMessage *msg, LoquiAccount *account);

static void loqui_account_user_notify_nick_cb(LoquiUser *user, GParamSpec *pspec, LoquiAccount *account);
static void loqui_account_user_self_notify_cb(LoquiUser *user_self, GParamSpec *pspec, LoquiAccount *account);
static void loqui_account_profile_notify_name_cb(LoquiProfileAccount *profile, GParamSpec *pspec, LoquiAccount *account);

static void loqui_account_remove_channel_real(LoquiAccount *account, LoquiChannel *channel);

GType
loqui_account_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccount),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiAccount",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_account_class_init (LoquiAccountClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_finalize;
	object_class->set_property = loqui_account_set_property;
	object_class->get_property = loqui_account_get_property;

	klass->remove_channel = loqui_account_remove_channel_real;

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
						  G_STRUCT_OFFSET(LoquiAccountClass, connected),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 0);
	account_signals[DISCONNECTED] = g_signal_new("disconnected",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET(LoquiAccountClass, disconnected),
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 0);
	account_signals[ADD_CHANNEL] = g_signal_new("add-channel",
						    G_OBJECT_CLASS_TYPE(object_class),
						    G_SIGNAL_RUN_LAST,
						    G_STRUCT_OFFSET(LoquiAccountClass, add_channel),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__OBJECT,
						    G_TYPE_NONE, 1,
						    LOQUI_TYPE_CHANNEL);
	account_signals[REMOVE_CHANNEL] = g_signal_new("remove-channel",
						       G_OBJECT_CLASS_TYPE(object_class),
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET(LoquiAccountClass, remove_channel),
						       NULL, NULL,
						       g_cclosure_marshal_VOID__OBJECT,
						       G_TYPE_NONE, 1,
						       LOQUI_TYPE_CHANNEL);
	account_signals[USER_SELF_CHANGED] = g_signal_new("user-self-changed",
							  G_OBJECT_CLASS_TYPE(object_class),
							  G_SIGNAL_RUN_LAST,
							  G_STRUCT_OFFSET(LoquiAccountClass, user_self_changed),
							  NULL, NULL,
							  g_cclosure_marshal_VOID__VOID,
							  G_TYPE_NONE, 0);
}
static void 
loqui_account_init(LoquiAccount *account)
{
	LoquiAccountPrivate *priv;

	priv = g_new0(LoquiAccountPrivate, 1);

	account->priv = priv;
	
	account->channel_list = NULL;
	account->channel_identifier_table = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, NULL);
	
	account->user_nick_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	account->nick_user_table = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, NULL);

	account->sender = LOQUI_SENDER(loqui_sender_irc_new(account));
}
static void 
loqui_account_finalize (GObject *object)
{
	LoquiAccount *account;
	LoquiAccountPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(object));

        account = LOQUI_ACCOUNT(object);
	priv = account->priv;
	
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);
	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->profile);

	loqui_account_remove_all_channel(account);

	if (account->channel_identifier_table) {
		g_hash_table_destroy(account->channel_identifier_table);
		account->channel_identifier_table = NULL;
	}
	if (account->nick_user_table) {
		g_hash_table_destroy(account->nick_user_table);
		account->nick_user_table = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account->priv);
}
static void loqui_account_set_property(GObject *object,
				 guint param_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	LoquiAccount *account;

	account = LOQUI_ACCOUNT(object);

	switch(param_id) {
	case PROP_USER_SELF:
		loqui_account_set_user_self(account, g_value_get_object(value));
		break;
	case PROP_PROFILE:
		loqui_account_set_profile(account, g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}

}
static void loqui_account_get_property(GObject  *object,
				 guint param_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
	LoquiAccount *account;

	account = LOQUI_ACCOUNT(object);

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
LoquiAccount*
loqui_account_new (LoquiProfileAccount *profile)
{
        LoquiAccount *account;
	LoquiUser *user;

	user = LOQUI_USER(loqui_user_irc_new());
	loqui_user_set_nick(user, loqui_profile_account_get_nick(profile));
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);

	account = g_object_new(loqui_account_get_type(), 
			       "buffer", channel_buffer_new(),
			       "profile", profile,
			       "user_self", user,
			       NULL);

	return account;
}

LoquiProfileAccount *
loqui_account_get_profile(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	return account->priv->profile;
}
static void
loqui_account_set_profile(LoquiAccount *account, LoquiProfileAccount *profile)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	G_OBJECT_UNREF_UNLESS_NULL(account->priv->profile);
	g_object_ref(profile);
	account->priv->profile = profile;

	loqui_channel_entry_set_name(LOQUI_CHANNEL_ENTRY(account), loqui_profile_account_get_name(profile));
	g_signal_connect(G_OBJECT(profile), "notify::name",
			 G_CALLBACK(loqui_account_profile_notify_name_cb), account);

	g_object_notify(G_OBJECT(account), "profile");
}
static void
loqui_account_profile_notify_name_cb(LoquiProfileAccount *profile, GParamSpec *pspec, LoquiAccount *account)
{
	loqui_channel_entry_set_name(LOQUI_CHANNEL_ENTRY(account), loqui_profile_account_get_name(profile));
}
LoquiUser *
loqui_account_get_user_self(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	return account->user_self;
}
static void
loqui_account_set_user_self(LoquiAccount *account, LoquiUser *user_self)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	G_OBJECT_UNREF_UNLESS_NULL(account->user_self);

	g_object_ref(user_self);
	account->user_self = user_self;
	loqui_account_add_user(account, user_self);

	g_signal_connect(user_self, "notify",
			 G_CALLBACK(loqui_account_user_self_notify_cb), account);

	g_object_notify(G_OBJECT(account), "user_self");	
}
static void
loqui_account_user_self_notify_cb(LoquiUser *user_self, GParamSpec *pspec, LoquiAccount *account)
{
	g_signal_emit(G_OBJECT(account), account_signals[USER_SELF_CHANGED], 0);
}
void
loqui_account_connect(LoquiAccount *account)
{
	LoquiAccountPrivate *priv;
	const gchar *servername, *codeset;
	gint port;
	gint codeset_type;
	
	CodeConv *codeconv;
	gchar *str;
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));
	
	priv = account->priv;

	if(loqui_account_is_connected(account)) {
		loqui_account_console_buffer_append(account, TEXT_TYPE_ERROR, _("Already connected."));
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
	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, str);
	g_free(str);

	irc_connection_connect(priv->connection);

	g_signal_connect(G_OBJECT(priv->connection), "connected",
			 G_CALLBACK(loqui_account_connection_connected_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "disconnected",
			 G_CALLBACK(loqui_account_connection_disconnected_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "terminated",
			 G_CALLBACK(loqui_account_connection_terminated_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "warn",
			 G_CALLBACK(loqui_account_connection_warn_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "info",
			 G_CALLBACK(loqui_account_connection_info_cb), account);
	g_signal_connect(G_OBJECT(priv->connection), "arrive_message",
			 G_CALLBACK(loqui_account_connection_arrive_message_cb), account);

	g_signal_emit(account, account_signals[CONNECTED], 0);	
}

static void
loqui_account_connection_connected_cb(GObject *object, gboolean is_success, LoquiAccount *account)
{
	LoquiAccountPrivate *priv;
	IRCMessage *msg;
	const gchar *password, *nick, *username, *realname, *autojoin;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	priv = account->priv;

	if(!is_success) {
		loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, _("Failed to connect."));
		G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
		return;
	}

	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, _("Connected. Sending Initial command..."));

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

		loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, _("Sent join command for autojoin."));
	}

	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, _("Done."));

	loqui_user_set_away(account->user_self, LOQUI_AWAY_TYPE_ONLINE);
}
static void
loqui_account_connection_terminated_cb(GObject *object, LoquiAccount *account)
{
	LoquiAccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	priv = account->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);

	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, _("Connection terminated."));
	loqui_user_set_away(account->user_self, LOQUI_AWAY_TYPE_OFFLINE);

	if(prefs_general.auto_reconnect) {
		loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, _("Trying to reconnect..."));
		loqui_account_connect(account);
	}
}
static void
loqui_account_connection_disconnected_cb(GObject *object, LoquiAccount *account)
{
	LoquiAccountPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	priv = account->priv;
	G_OBJECT_UNREF_UNLESS_NULL(priv->connection);
	G_OBJECT_UNREF_UNLESS_NULL(priv->handle);

	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, _("Disconnected."));
	loqui_account_remove_all_channel(account);

	loqui_user_set_away(account->user_self, LOQUI_AWAY_TYPE_OFFLINE);
	g_signal_emit(account, account_signals[DISCONNECTED], 0);
}
static void
loqui_account_connection_warn_cb(GObject *object, gchar *str, LoquiAccount *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	loqui_account_console_buffer_append(account, TEXT_TYPE_ERROR, str);
}
static void
loqui_account_connection_info_cb(GObject *object, gchar *str, LoquiAccount *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, str);
}
static void
loqui_account_connection_arrive_message_cb(IRCConnection *connection, IRCMessage *msg, LoquiAccount *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	irc_handle_response(account->priv->handle, msg);
}

void
loqui_account_disconnect(LoquiAccount *account)
{
	LoquiAccountPrivate *priv;
	IRCMessage *msg;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));
	
	priv = account->priv;

	if(priv->connection) {
		msg = irc_message_create(IRCCommandQuit, "Loqui", NULL);
		irc_connection_disconnect_after_send(priv->connection, msg);
		g_object_unref(msg);
	}
}
gboolean
loqui_account_is_connected(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), FALSE);
	
	return (account->priv->connection != NULL);
}

IRCHandle *
loqui_account_get_handle(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);
	
	return account->priv->handle;
}
IRCConnection *
loqui_account_get_connection(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);
	
	return account->priv->connection;
}
LoquiSender *
loqui_account_get_sender(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);
	
	return account->sender;
}

void
loqui_account_add_channel(LoquiAccount *account, LoquiChannel *channel)
{
	GList *l;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	g_object_ref(channel);

	l = g_list_alloc();
	l->data = channel;

	account->channel_list = g_list_concat(account->channel_list, l);
	g_hash_table_insert(account->channel_identifier_table, g_strdup(loqui_channel_get_identifier(channel)), l);

	g_signal_emit(account, account_signals[ADD_CHANNEL], 0, channel);
}
static void
loqui_account_remove_channel_real(LoquiAccount *account, LoquiChannel *channel)
{
	g_hash_table_remove(account->channel_identifier_table, loqui_channel_get_identifier(channel));

	account->channel_list = g_list_remove(account->channel_list, channel);
	g_object_unref(channel);
}
void
loqui_account_remove_channel(LoquiAccount *account, LoquiChannel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	g_signal_emit(account, account_signals[REMOVE_CHANNEL], 0, channel);
}
void
loqui_account_remove_all_channel(LoquiAccount *account)
{
	GList *list, *cur;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	list = g_list_copy(account->channel_list);
	for (cur = list; cur != NULL; cur = cur->next) {
		loqui_account_remove_channel(account, cur->data);
	}
	g_list_free(list);
}

GList *
loqui_account_get_channel_list(LoquiAccount *account)
{
       g_return_val_if_fail(account != NULL, NULL);
       g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

       return account->channel_list;
}
LoquiChannel*
loqui_account_get_channel_by_identifier(LoquiAccount *account, const gchar *identifier)
{
	GList *l;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(identifier != NULL, NULL);

	l = g_hash_table_lookup(account->channel_identifier_table, identifier);
	if (l)
		return l->data;

	return NULL;
}
void
loqui_account_console_buffer_append(LoquiAccount *account, TextType type, gchar *str)
{
	MessageText *msgtext;
	ChannelBuffer *buffer;

	g_return_if_fail(account != NULL);
	g_return_if_fail(str != NULL);

	msgtext = message_text_new();
	g_object_set(G_OBJECT(msgtext),
		     "is_remark", FALSE,
		     "account_name", loqui_profile_account_get_name(loqui_account_get_profile(account)),
		     "text_type", type,
		     "text", str,
		     NULL);

	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(account));
	if (buffer)
		channel_buffer_append_message_text(buffer, msgtext, FALSE, FALSE);
	g_object_unref(msgtext);
}
gboolean
loqui_account_is_current_nick(LoquiAccount *account, const gchar *str)
{
	LoquiAccountPrivate *priv;

        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), FALSE);

	priv = account->priv;

	if(str == NULL)
		return FALSE;
	
	return (strcmp(loqui_user_get_nick(account->user_self), str) == 0 ? TRUE : FALSE);
}

GSList *
loqui_account_search_joined_channel(LoquiAccount *account, gchar *nick)
{
	GList *cur;
	GSList *list = NULL;
	LoquiChannelEntry *chent;
	LoquiUser *user;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(nick != NULL, NULL);

	user = loqui_account_peek_user(account, nick);
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
void
loqui_account_get_updated_number(LoquiAccount *account, gint *updated_private_talk_number, gint *updated_channel_number)
{
	LoquiChannel *channel;
	GList *cur;
		
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));
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
loqui_account_user_disposed_cb(LoquiAccount *account, LoquiUser *user)
{
	gchar *nick;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	nick = g_hash_table_lookup(account->user_nick_table, user);
	g_hash_table_remove(account->nick_user_table, nick);
	g_hash_table_remove(account->user_nick_table, user);
}
static void
loqui_account_user_notify_nick_cb(LoquiUser *user, GParamSpec *pspec, LoquiAccount *account)
{
	const gchar *nick_old, *nick_new;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	nick_old = g_hash_table_lookup(account->user_nick_table, user);
	nick_new = loqui_user_get_nick(user);

	g_hash_table_remove(account->nick_user_table, nick_old);
	g_hash_table_insert(account->nick_user_table, g_strdup(nick_new), user);
	g_hash_table_replace(account->user_nick_table, user, g_strdup(nick_new));
}
static void
loqui_account_add_user(LoquiAccount *account, LoquiUser *user)
{
	g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	g_signal_connect(G_OBJECT(user), "notify::nick",
			 G_CALLBACK(loqui_account_user_notify_nick_cb), account);
	g_object_weak_ref(G_OBJECT(user), (GWeakNotify) loqui_account_user_disposed_cb, account);
	g_hash_table_insert(account->nick_user_table, g_strdup(loqui_user_get_nick(user)), user);
	g_hash_table_insert(account->user_nick_table, user, g_strdup(loqui_user_get_nick(user)));	
}
LoquiUser *
loqui_account_fetch_user(LoquiAccount *account, const gchar *nick)
{
	LoquiUser *user;
		
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	if((user = loqui_account_peek_user(account, nick)) == NULL) {
		user = LOQUI_USER(loqui_user_irc_new());
		loqui_user_set_nick(user, nick);
		loqui_account_add_user(account, user);
	} else {
		g_object_ref(user);
	}

	return user;
}
LoquiUser *
loqui_account_peek_user(LoquiAccount *account, const gchar *nick)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	return g_hash_table_lookup(account->nick_user_table, nick);
}
