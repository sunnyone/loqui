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
#include "gtkutils.h"
#include "utils.h"
#include "main.h"
#include "gtkutils.h"
#include "intl.h"
#include "loqui_user.h"
#include "loqui_sender.h"

enum {
	SIGNAL_CONNECT,
	SIGNAL_DISCONNECT,
	DISCONNECTED,
	SIGNAL_ADD_CHANNEL,
	SIGNAL_REMOVE_CHANNEL,
	SIGNAL_USER_SELF_CHANGED,
	SIGNAL_WARN,
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_USER_SELF,
	PROP_PROFILE,
	PROP_SENDER,
	PROP_RECEIVER,
	PROP_IS_CONNECTED,
	LAST_PROP
};

struct _LoquiAccountPrivate
{
	LoquiProfileAccount *profile;
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
static void loqui_account_warn_real(LoquiAccount *account, const gchar *str);

static void loqui_account_set_profile(LoquiAccount *account, LoquiProfileAccount *profile);
static void loqui_account_set_user_self(LoquiAccount *account, LoquiUser *user_self);

static void loqui_account_user_notify_identifier_cb(LoquiUser *user, GParamSpec *pspec, LoquiAccount *account);
static void loqui_account_user_self_notify_cb(LoquiUser *user_self, GParamSpec *pspec, LoquiAccount *account);
static void loqui_account_profile_notify_name_cb(LoquiProfileAccount *profile, GParamSpec *pspec, LoquiAccount *account);
static void loqui_account_channel_notify_identifier_cb(LoquiChannel *channel, GParamSpec *pspec, LoquiAccount *account);

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
loqui_account_class_init(LoquiAccountClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_finalize;
	object_class->set_property = loqui_account_set_property;
	object_class->get_property = loqui_account_get_property;

	klass->remove_channel = loqui_account_remove_channel_real;
	klass->warn = loqui_account_warn_real;

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
	g_object_class_install_property(object_class,
					PROP_SENDER,
					g_param_spec_object("sender",
							    _("Sender"),
							    _("Command sender"),
							    LOQUI_TYPE_SENDER,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_RECEIVER,
					g_param_spec_object("receiver",
							    _("Receiver"),
							    _("Command receiver"),
							    LOQUI_TYPE_RECEIVER,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IS_CONNECTED,
					g_param_spec_boolean("is_connected",
							     _("IsConnected"),
							     _("Connected or not"),
							     FALSE, G_PARAM_READWRITE));

	account_signals[SIGNAL_CONNECT] = g_signal_new("connect",
						       G_OBJECT_CLASS_TYPE(object_class),
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET(LoquiAccountClass, connect),
						       NULL, NULL,
						       g_cclosure_marshal_VOID__VOID,
						       G_TYPE_NONE, 0);
	account_signals[SIGNAL_DISCONNECT] = g_signal_new("disconnect",
						       G_OBJECT_CLASS_TYPE(object_class),
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET(LoquiAccountClass, disconnect),
						       NULL, NULL,
						       g_cclosure_marshal_VOID__VOID,
						       G_TYPE_NONE, 0);
	account_signals[SIGNAL_ADD_CHANNEL] = g_signal_new("add-channel",
						    G_OBJECT_CLASS_TYPE(object_class),
						    G_SIGNAL_RUN_LAST,
						    G_STRUCT_OFFSET(LoquiAccountClass, add_channel),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__OBJECT,
						    G_TYPE_NONE, 1,
						    LOQUI_TYPE_CHANNEL);
	account_signals[SIGNAL_REMOVE_CHANNEL] = g_signal_new("remove-channel",
						       G_OBJECT_CLASS_TYPE(object_class),
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET(LoquiAccountClass, remove_channel),
						       NULL, NULL,
						       g_cclosure_marshal_VOID__OBJECT,
						       G_TYPE_NONE, 1,
						       LOQUI_TYPE_CHANNEL);
	account_signals[SIGNAL_USER_SELF_CHANGED] = g_signal_new("user-self-changed",
							  G_OBJECT_CLASS_TYPE(object_class),
							  G_SIGNAL_RUN_LAST,
							  G_STRUCT_OFFSET(LoquiAccountClass, user_self_changed),
							  NULL, NULL,
							  g_cclosure_marshal_VOID__VOID,
							  G_TYPE_NONE, 0);
	account_signals[SIGNAL_WARN] = g_signal_new("warn",
						    G_OBJECT_CLASS_TYPE(object_class),
						    G_SIGNAL_RUN_LAST,
						    G_STRUCT_OFFSET(LoquiAccountClass, warn),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__OBJECT,
						    G_TYPE_NONE, 1,
						    G_TYPE_STRING);

	account_signals[DISCONNECTED] = g_signal_new("disconnected",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_LAST,
						     G_STRUCT_OFFSET(LoquiAccountClass, disconnected),
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
	
	/* FIXME: identifier should be case sensitive */
	account->channel_list = NULL;
	account->identifier_channel_table = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, NULL);

	account->user_identifier_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	account->identifier_user_table = g_hash_table_new_full(utils_strcase_hash, utils_strcase_equal, g_free, NULL);

	account->is_connected = FALSE;
}
static void 
loqui_account_finalize(GObject *object)
{
	LoquiAccount *account;
	LoquiAccountPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(object));

        account = LOQUI_ACCOUNT(object);
	priv = account->priv;
	
	G_OBJECT_UNREF_UNLESS_NULL(priv->profile);

	loqui_account_remove_all_channel(account);

#define DESTROY_HASH_TABLE(table) { \
  if (table) { \
	  g_hash_table_destroy(table); \
	  table = NULL; \
  } \
}

	G_OBJECT_UNREF_UNLESS_NULL(account->sender);
	G_OBJECT_UNREF_UNLESS_NULL(account->receiver);

	DESTROY_HASH_TABLE(account->identifier_channel_table);
	DESTROY_HASH_TABLE(account->user_identifier_table);
	DESTROY_HASH_TABLE(account->identifier_user_table);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account->priv);
}
static void
loqui_account_set_property(GObject *object,
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
	case PROP_SENDER:
		loqui_account_set_sender(account, g_value_get_object(value));
		break;
	case PROP_RECEIVER:
		loqui_account_set_receiver(account, g_value_get_object(value));
		break;
	case PROP_IS_CONNECTED:
		loqui_account_set_is_connected(account, g_value_get_boolean(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}

}
static void
loqui_account_get_property(GObject  *object,
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
	case PROP_SENDER:
		g_value_set_object(value, account->sender);
		break;
	case PROP_RECEIVER:
		g_value_set_object(value, account->receiver);
		break;
	case PROP_IS_CONNECTED:
		g_value_set_boolean(value, account->is_connected);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}
static void
loqui_account_warn_real(LoquiAccount *account, const gchar *str)
{
	gchar *tmp;

	tmp = g_strconcat("+++ ", str, NULL);
	loqui_account_console_buffer_append(account, TEXT_TYPE_ERROR, tmp);
	g_free(tmp);
}
LoquiAccount*
loqui_account_new(LoquiProfileAccount *profile)
{
        LoquiAccount *account;
	LoquiUser *user;

	user = loqui_user_new();

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
	g_signal_emit(G_OBJECT(account), account_signals[SIGNAL_USER_SELF_CHANGED], 0);
}
LoquiSender *
loqui_account_get_sender(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	return account->sender;
}
void
loqui_account_set_sender(LoquiAccount *account, LoquiSender *sender)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	G_OBJECT_UNREF_UNLESS_NULL(account->sender);

	g_object_ref(sender);
	account->sender = sender;
}
LoquiReceiver *
loqui_account_get_receiver(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	return account->receiver;
}
void
loqui_account_set_receiver(LoquiAccount *account, LoquiReceiver *receiver)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	G_OBJECT_UNREF_UNLESS_NULL(account->receiver);

	g_object_ref(receiver);
	account->receiver = receiver;
}
void
loqui_account_connect(LoquiAccount *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));
	
	if (!LOQUI_ACCOUNT_GET_CLASS(account)->connect) {
		g_warning("LoquiAccount#connect is not implemented.");
		return;
	}

	g_signal_emit(G_OBJECT(account), account_signals[SIGNAL_CONNECT], 0);
}
void
loqui_account_disconnect(LoquiAccount *account)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	g_signal_emit(G_OBJECT(account), account_signals[SIGNAL_DISCONNECT], 0);
}
static void
loqui_account_channel_notify_identifier_cb(LoquiChannel *channel, GParamSpec *pspec, LoquiAccount *account)
{
	GList *l;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));
	
	g_hash_table_foreach_remove(account->identifier_channel_table,
				    utils_return_true_if_data_of_list_equals_data,
				    channel);

	l = g_list_find(account->channel_list, channel);
	g_hash_table_insert(account->identifier_channel_table, g_strdup(loqui_channel_get_identifier(channel)), l);
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
	g_hash_table_insert(account->identifier_channel_table, g_strdup(loqui_channel_get_identifier(channel)), l);
	g_signal_connect_object(G_OBJECT(channel), "notify::identifier",
				G_CALLBACK(loqui_account_channel_notify_identifier_cb), account, 0);

	g_signal_emit(account, account_signals[SIGNAL_ADD_CHANNEL], 0, channel);
}
static void
loqui_account_remove_channel_real(LoquiAccount *account, LoquiChannel *channel)
{
	g_hash_table_remove(account->identifier_channel_table, loqui_channel_get_identifier(channel));

	account->channel_list = g_list_remove(account->channel_list, channel);
	g_object_unref(channel);
}
void
loqui_account_remove_channel(LoquiAccount *account, LoquiChannel *channel)
{
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	g_signal_emit(account, account_signals[SIGNAL_REMOVE_CHANNEL], 0, channel);
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

	l = g_hash_table_lookup(account->identifier_channel_table, identifier);
	if (l)
		return l->data;

	return NULL;
}
void
loqui_account_console_buffer_append(LoquiAccount *account, TextType type, const gchar *str)
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

GList *
loqui_account_search_joined_channel(LoquiAccount *account, LoquiUser *user)
{
	GList *cur;
	GList *joined_list = NULL;
	LoquiChannelEntry *chent;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	if (user == NULL)
		return NULL;

	for (cur = account->channel_list; cur != NULL; cur = cur->next) {
		chent = LOQUI_CHANNEL_ENTRY(cur->data);
		if (loqui_channel_entry_get_member_by_user(chent, user))
			joined_list = g_list_prepend(joined_list, chent);
	}
	joined_list = g_list_reverse(joined_list);

	return joined_list;
}
GList *
loqui_account_search_joined_channel_by_identifier(LoquiAccount *account, gchar *user_identifier)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);
	
	if (!user_identifier)
		return NULL;

	return loqui_account_search_joined_channel(account, loqui_account_peek_user(account, user_identifier));
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
	gchar *identifier;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	identifier = g_hash_table_lookup(account->user_identifier_table, user);
	g_hash_table_remove(account->identifier_user_table, identifier);
	g_hash_table_remove(account->user_identifier_table, user);
}
static void
loqui_account_user_notify_identifier_cb(LoquiUser *user, GParamSpec *pspec, LoquiAccount *account)
{
	const gchar *identifier_old, *identifier_new;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	identifier_old = g_hash_table_lookup(account->user_identifier_table, user);
	identifier_new = loqui_user_get_identifier(user);

	g_hash_table_remove(account->identifier_user_table, identifier_old);
	g_hash_table_insert(account->identifier_user_table, g_strdup(identifier_new), user);
	g_hash_table_replace(account->user_identifier_table, user, g_strdup(identifier_new));
}
void
loqui_account_add_user(LoquiAccount *account, LoquiUser *user)
{
	g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	g_signal_connect(G_OBJECT(user), "notify::identifier",
			 G_CALLBACK(loqui_account_user_notify_identifier_cb), account);
	g_object_weak_ref(G_OBJECT(user), (GWeakNotify) loqui_account_user_disposed_cb, account);
	g_hash_table_insert(account->identifier_user_table, g_strdup(loqui_user_get_identifier(user)), user);
	g_hash_table_insert(account->user_identifier_table, user, g_strdup(loqui_user_get_identifier(user)));	
}
LoquiUser *
loqui_account_peek_user(LoquiAccount *account, const gchar *identifier)
{
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), NULL);

	return g_hash_table_lookup(account->identifier_user_table, identifier);
}
void
loqui_account_warning(LoquiAccount *account, const gchar *format, ...)
{
	va_list args;
	gchar *str;
	
	va_start(args, format);
	str = g_strdup_vprintf(format, args);
	va_end(args);

	g_signal_emit(G_OBJECT(account), account_signals[SIGNAL_WARN], 0, str);

	g_free(str);	
}
void
loqui_account_set_is_connected(LoquiAccount *account, gboolean is_connected)
{
	g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	if (account->is_connected == is_connected)
		return;

	account->is_connected = is_connected;

	g_object_notify(G_OBJECT(account), "is_connected");
}
gboolean
loqui_account_get_is_connected(LoquiAccount *account)
{
        g_return_val_if_fail(account != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT(account), 0);

	return account->is_connected;
}
