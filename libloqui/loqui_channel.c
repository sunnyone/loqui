/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2002-2004 Yoichi Imai <sunnyone41@gmail.com>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include "loqui-pref.h"
#include "loqui-general-pref-groups.h"
#include "loqui-general-pref-default.h"

#include "loqui_channel.h"
#include <libloqui-intl.h>
#include <string.h>
#include "loqui_sender_irc.h"

#include "loqui-utils.h"
#include "loqui-static-core.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_ACCOUNT,
	PROP_IS_JOINED,
	PROP_IS_PRIVATE_TALK,
	PROP_IDENTIFIER,
        LAST_PROP
};

struct _LoquiChannelPrivate
{
	GList *mode_change_queue;
};

#define MODE_CHANGE_MAX 3

typedef struct {
	gboolean is_give;
	IRCModeFlag flag;
	gchar *nick;
} ModeChange;

static LoquiChannelEntryClass *parent_class = NULL;

/* static guint loqui_channel_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_channel_class_init(LoquiChannelClass *klass);
static void loqui_channel_init(LoquiChannel *channel);
static void loqui_channel_finalize(GObject *object);
static void loqui_channel_dispose(GObject *object);

static void loqui_channel_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_channel_append_message_text(LoquiChannelEntry *chent, LoquiMessageText *msgtext);

GType
loqui_channel_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannel),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_CHANNEL_ENTRY,
					      "LoquiChannel",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_channel_finalize(GObject *object)
{
	LoquiChannel *channel;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL(object));

        channel = LOQUI_CHANNEL(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(channel->priv);
}
static void 
loqui_channel_dispose(GObject *object)
{
	LoquiChannel *channel;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL(object));

        channel = LOQUI_CHANNEL(object);

	LOQUI_G_FREE_UNLESS_NULL(channel->identifier);

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(channel->account);

	loqui_debug_puts("the channel is disposed.");

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_channel_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiChannel *channel;        

        channel = LOQUI_CHANNEL(object);

        switch (param_id) {
	case PROP_ACCOUNT:
		g_value_set_object(value, channel->account);
		break;
	case PROP_IS_JOINED:
		g_value_set_boolean(value, channel->is_joined);
		break;
	case PROP_IS_PRIVATE_TALK:
		g_value_set_boolean(value, channel->is_private_talk);
		break;
	case PROP_IDENTIFIER:
		g_value_set_string(value, channel->identifier);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiChannel *channel;        

        channel = LOQUI_CHANNEL(object);

        switch (param_id) {
	case PROP_ACCOUNT:
		loqui_channel_set_account(channel, g_value_get_object(value));
		break;
	case PROP_IS_JOINED:
		loqui_channel_set_is_joined(channel, g_value_get_boolean(value));
		break;
	case PROP_IS_PRIVATE_TALK:
		loqui_channel_set_is_private_talk(channel, g_value_get_boolean(value));
		break;
	case PROP_IDENTIFIER:
		loqui_channel_set_identifier(channel, g_value_get_string(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_channel_class_init(LoquiChannelClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	LoquiChannelEntryClass *chent_class = LOQUI_CHANNEL_ENTRY_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_finalize;
        object_class->dispose = loqui_channel_dispose;
        object_class->get_property = loqui_channel_get_property;
        object_class->set_property = loqui_channel_set_property;

	chent_class->append_message_text = loqui_channel_append_message_text;

	g_object_class_install_property(object_class,
					PROP_IS_JOINED,
					g_param_spec_boolean("is_joined",
							     _("Joined"),
							     _("Joined or not"),
							     TRUE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IS_PRIVATE_TALK,
					g_param_spec_boolean("is_private_talk",
							     _("PrivateTalk"),
							     _("PrivateTalk or not"),
							     TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class,
					PROP_IDENTIFIER,
					g_param_spec_string("identifier",
							    _("Identifier"),
							    _("The string to identify the channel"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_ACCOUNT,
					g_param_spec_object("account",
							    _("Account"),
							    _("Parent account"),
							    LOQUI_TYPE_ACCOUNT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}
static void 
loqui_channel_init(LoquiChannel *channel)
{
	LoquiChannelPrivate *priv;

	priv = g_new0(LoquiChannelPrivate, 1);

	channel->priv = priv;

	channel->end_names = TRUE;
	channel->channel_mode_manager = loqui_mode_manager_new(NULL);
	g_object_set_data(G_OBJECT(channel->channel_mode_manager), "channel", channel);			  
}
static void
loqui_channel_append_message_text(LoquiChannelEntry *chent, LoquiMessageText *msgtext)
{
	LoquiChannel *channel;

	channel = LOQUI_CHANNEL(chent);
       
        if (LOQUI_CHANNEL_ENTRY_CLASS(parent_class)->append_message_text)
                (* LOQUI_CHANNEL_ENTRY_CLASS(parent_class)->append_message_text)(chent, msgtext);
}
LoquiChannel*
loqui_channel_new(LoquiAccount *account, const gchar *name, const gchar *identifier, gboolean is_joined, gboolean is_private_talk)
{
        LoquiChannel *channel;
	LoquiChannelPrivate *priv;

	channel = g_object_new(loqui_channel_get_type(),
			       "account", account,
			       "name", name,
			       "identifier", identifier,
			       "is_private_talk", is_private_talk,
			       "is_joined", is_joined,
			       NULL);
	
        priv = channel->priv;

        return channel;
}
void
loqui_channel_set_account(LoquiChannel *channel, LoquiAccount *account)
{
        LoquiChannelPrivate *priv;

        g_return_if_fail(channel != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL(channel));

        priv = channel->priv;

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(channel->account);

	g_object_ref(account);
	channel->account = account;
	g_object_notify(G_OBJECT(channel), "account");
}
LoquiAccount *
loqui_channel_get_account(LoquiChannel *channel)
{
        g_return_val_if_fail(channel != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL(channel), 0);

	return channel->account;
}
void
loqui_channel_set_is_private_talk(LoquiChannel *channel, gboolean is_private_talk)
{
	g_return_if_fail(channel != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	if (channel->is_private_talk == is_private_talk)
		return;

	channel->is_private_talk = is_private_talk;
	g_object_notify(G_OBJECT(channel), "is_private_talk");
}
gboolean
loqui_channel_get_is_private_talk(LoquiChannel *channel)
{
        g_return_val_if_fail(channel != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL(channel), 0);

	return channel->is_private_talk;
}
void
loqui_channel_set_is_joined(LoquiChannel *channel, gboolean is_joined)
{
	g_return_if_fail(channel != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	if (channel->is_joined == is_joined)
		return;

	channel->is_joined = is_joined;
	g_object_notify(G_OBJECT(channel), "is_joined");
}
gboolean
loqui_channel_get_is_joined(LoquiChannel *channel)
{
        g_return_val_if_fail(channel != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL(channel), 0);

	return channel->is_joined;
}

LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiChannel, loqui_channel, identifier);

void
loqui_channel_push_user_mode_queue(LoquiChannel *channel, gboolean is_give, IRCModeFlag flag, const gchar *nick)
{
	LoquiChannelPrivate *priv;
	ModeChange *mode_change_old;
	ModeChange *mode_change;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));
	
	priv = channel->priv;

	if(priv->mode_change_queue) {
		mode_change_old = g_list_last(priv->mode_change_queue)->data;
		if(mode_change_old->is_give != is_give || mode_change_old->flag != flag)
			loqui_channel_flush_user_mode_queue(channel);
	}

	if(g_list_length(priv->mode_change_queue) >= MODE_CHANGE_MAX)
		loqui_channel_flush_user_mode_queue(channel);

	mode_change = g_new0(ModeChange, 1);
	mode_change->is_give = is_give;
	mode_change->flag = flag;
	mode_change->nick = g_strdup(nick);

	priv->mode_change_queue = g_list_append(priv->mode_change_queue, mode_change);

}
static void
loqui_channel_mode_change_free(ModeChange *mode_change)
{
	g_return_if_fail(mode_change != NULL);

	g_free(mode_change->nick);
	g_free(mode_change);
}
void
loqui_channel_flush_user_mode_queue(LoquiChannel *channel)
{
	LoquiChannelPrivate *priv;
	GList *tmp_list = NULL, *cur;
	ModeChange *mode_change;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));
	
	priv = channel->priv;
	
	mode_change = NULL;
	for(cur = priv->mode_change_queue; cur != NULL; cur = cur->next) {
		mode_change = (ModeChange *) cur->data;
		tmp_list = g_list_append(tmp_list, mode_change->nick);
	}
	if(mode_change)
		loqui_sender_irc_change_member_mode(LOQUI_SENDER_IRC(loqui_account_get_sender(channel->account)), channel, mode_change->is_give,
						    mode_change->flag, tmp_list);

	g_list_free(tmp_list);

	g_list_foreach(priv->mode_change_queue, (GFunc) loqui_channel_mode_change_free, NULL);
	g_list_free(priv->mode_change_queue);
	priv->mode_change_queue = NULL;
}

/* FIXME: should be more efficient */
static gboolean
loqui_channel_search_patters(const gchar *group, const gchar *key, const gchar *body)
{
	gchar **array;
	int i;

	array = loqui_pref_get_string_list(loqui_get_general_pref(), group, key, NULL, NULL);
	if (!array)
		return FALSE;

	for (i = 0; array[i] != NULL; i++) {
		if (g_pattern_match_simple(array[i], body)) {
			g_strfreev(array);
			return TRUE;
		}
	}

	g_strfreev(array);
	return FALSE;
}

LoquiMessageText *
loqui_channel_create_remark_message_text(LoquiChannel *channel, LoquiTextType type, gboolean is_self, const gchar *nick, const gchar *remark)
{
	LoquiChannelPrivate *priv;
	LoquiMessageText *msgtext;
	gboolean is_transparent = FALSE, is_ignored = FALSE;

	gboolean is_priv = FALSE;

	g_return_val_if_fail(channel != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_CHANNEL(channel), NULL);

	priv = channel->priv;

	is_priv = loqui_channel_get_is_private_talk(channel);

	/* FIXME: should be more efficient */
	if (loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
						LOQUI_GENERAL_PREF_GROUP_IGNORE, "UseTransparentIgnore",
						LOQUI_GENERAL_PREF_DEFAULT_IGNORE_USE_TRANSPARENT_IGNORE, NULL) && nick != NULL) {
		is_transparent = loqui_channel_search_patters(LOQUI_GENERAL_PREF_GROUP_IGNORE, "TransparentIgnoreList", nick);
	}
	if (is_transparent)
		type = LOQUI_TEXT_TYPE_TRANSPARENT;

	if (loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
						LOQUI_GENERAL_PREF_GROUP_IGNORE, "UseNormalIgnore",
						LOQUI_GENERAL_PREF_DEFAULT_IGNORE_USE_NORMAL_IGNORE, NULL) && nick != NULL) {
		is_ignored = loqui_channel_search_patters(LOQUI_GENERAL_PREF_GROUP_IGNORE, "NormalIgnoreList", nick);
	}
	
	msgtext = loqui_message_text_new();
	g_object_set(G_OBJECT(msgtext),
		     "is_remark", TRUE,
		     "text_type", type,
		     "is_priv", is_priv,
		     "is_self", is_self,
		     "is_ignored", is_ignored,
		     "text", remark,
		     "nick", nick,
		     "account_name", loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel->account)),
		     "channel_name", loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
		     NULL);
	
	return msgtext;
}
void
loqui_channel_append_remark(LoquiChannel *channel, LoquiTextType type, gboolean is_self, const gchar *nick, const gchar *remark)
{
	LoquiMessageText *msgtext;

	msgtext = loqui_channel_create_remark_message_text(channel, type, is_self, nick, remark);
	loqui_channel_entry_append_message_text(LOQUI_CHANNEL_ENTRY(channel), msgtext);
	g_object_unref(msgtext);
}
void
loqui_channel_append_text(LoquiChannel *channel, LoquiTextType type, gchar *str)
{
	LoquiMessageText *msgtext;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	msgtext = loqui_message_text_new();
	g_object_set(G_OBJECT(msgtext),
		     "is_remark", FALSE,
		     "text_type", type,
		     "text", str,
		     "account_name", loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel->account)),
		     "channel_name", loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
		     NULL);

	loqui_channel_entry_append_message_text(LOQUI_CHANNEL_ENTRY(channel), msgtext);
	g_object_unref(msgtext);
}
