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

#include "loqui_channel.h"
#include "intl.h"
#include "loqui_utils_irc.h"
#include "prefs_general.h"
#include <string.h>
#include "loqui_sender_irc.h"

enum {
	MODE_CHANGED,
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_ACCOUNT,
	PROP_IS_JOINED,
	PROP_IS_PRIVATE_TALK,
        LAST_PROP
};

struct _LoquiChannelPrivate
{
	GSList *mode_list;
	GList *mode_change_queue;
};

#define MODE_CHANGE_MAX 3

typedef struct {
	IRCModeFlag flag;
	gchar *argument;
} LoquiChannelMode;

typedef struct {
	gboolean is_give;
	IRCModeFlag flag;
	gchar *nick;
} ModeChange;

static LoquiChannelEntryClass *parent_class = NULL;

static guint loqui_channel_signals[LAST_SIGNAL] = { 0 };

static void loqui_channel_class_init(LoquiChannelClass *klass);
static void loqui_channel_init(LoquiChannel *channel);
static void loqui_channel_finalize(GObject *object);
static void loqui_channel_dispose(GObject *object);

static void loqui_channel_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

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

	G_OBJECT_UNREF_UNLESS_NULL(channel->account);

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
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_channel_class_init(LoquiChannelClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_finalize;
        object_class->dispose = loqui_channel_dispose;
        object_class->get_property = loqui_channel_get_property;
        object_class->set_property = loqui_channel_set_property;

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
					PROP_ACCOUNT,
					g_param_spec_object("account",
							    _("Account"),
							    _("Parent account"),
							    LOQUI_TYPE_ACCOUNT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

        loqui_channel_signals[MODE_CHANGED] = g_signal_new("mode-changed",
							   G_OBJECT_CLASS_TYPE(object_class),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET(LoquiChannelClass, mode_changed),
							   NULL, NULL,
							   g_cclosure_marshal_VOID__VOID,
							   G_TYPE_NONE, 0);
}
static void 
loqui_channel_init(LoquiChannel *channel)
{
	LoquiChannelPrivate *priv;

	priv = g_new0(LoquiChannelPrivate, 1);

	channel->priv = priv;

	channel->end_names = TRUE;
}
LoquiChannel*
loqui_channel_new(LoquiAccount *account, const gchar *name, gboolean is_joined, gboolean is_private_talk)
{
        LoquiChannel *channel;
	LoquiChannelPrivate *priv;

	channel = g_object_new(loqui_channel_get_type(),
			       "account", account,
			       "name", name,
			       "is_private_talk", is_private_talk,
			       "is_joined", is_joined,
			       "buffer", channel_buffer_new(),
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

	G_OBJECT_UNREF_UNLESS_NULL(channel->account);

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
static void
loqui_channel_mode_free(LoquiChannelMode *mode)
{
	g_return_if_fail(mode);

	if(mode->argument)
		g_free(mode->argument);
	g_free(mode);
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
void
loqui_channel_change_mode(LoquiChannel *channel, gboolean is_add, IRCModeFlag flag, gchar *argument)
{
	LoquiChannelPrivate *priv;
	GSList *cur;
	LoquiChannelMode *matched = NULL;
	LoquiChannelMode *mode;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));
	
	priv = channel->priv;

	for(cur = priv->mode_list; cur != NULL; cur = cur->next) {
		mode = (LoquiChannelMode *) cur->data;
		if(mode->flag == flag) {
			matched = mode;
			break;
		}
	}

	if(is_add) {
		if(matched)
			return;
		
		mode = g_new0(LoquiChannelMode, 1);
		mode->flag = flag;
		mode->argument = g_strdup(argument);
		priv->mode_list = g_slist_append(priv->mode_list, mode);
	} else {
		if(!matched)
			return;

		priv->mode_list = g_slist_remove(priv->mode_list, matched);
		loqui_channel_mode_free(matched);
	}

	debug_puts("LoquiChannel mode changed: %s %c%c %s", loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), is_add ? '+' : '-', flag, argument ? argument : "");

	g_signal_emit(channel, loqui_channel_signals[MODE_CHANGED], 0);
}
void
loqui_channel_clear_mode(LoquiChannel *channel)
{
	LoquiChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));
	
	priv = channel->priv;

	if(!priv->mode_list)
		return;

	g_slist_foreach(priv->mode_list, (GFunc) loqui_channel_mode_free, NULL);
	g_slist_free(priv->mode_list);

	priv->mode_list = NULL;

	g_signal_emit(channel, loqui_channel_signals[MODE_CHANGED], 0);
}
gchar *
loqui_channel_get_mode(LoquiChannel *channel)
{
	LoquiChannelPrivate *priv;
	GString *flag_string;
	GString *argument_string;
	gchar *str;
	GSList *cur;
	LoquiChannelMode *mode;

	g_return_val_if_fail(channel != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_CHANNEL(channel), NULL);
	
	priv = channel->priv;

	if(!priv->mode_list)
		return g_strdup("");

	flag_string = g_string_sized_new(20);
	argument_string = g_string_new(NULL);

	flag_string = g_string_append_c(flag_string, '+');

	for(cur = priv->mode_list; cur != NULL; cur = cur->next) {
		mode = (LoquiChannelMode *) cur->data;

		flag_string = g_string_append_c(flag_string, mode->flag);
		if(mode->argument)
			g_string_append_printf(argument_string, " %s", mode->argument);
	}
	if(argument_string->len > 1) {
		g_string_append(flag_string, argument_string->str);
	}
	g_string_free(argument_string, TRUE);

	str = flag_string->str;
	g_string_free(flag_string, FALSE);

	return str;
}
LoquiMember *
loqui_channel_add_member_by_nick(LoquiChannel *channel, const gchar *nick, gboolean parse_power, gboolean is_channel_operator, gboolean speakable)
{
	gchar *tmp_nick;
	LoquiMember *member;
	LoquiUser *user;
	gboolean is_o, is_v;

        g_return_val_if_fail(channel != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CHANNEL(channel), NULL);
        g_return_val_if_fail(nick != NULL, NULL);
        g_return_val_if_fail(*nick != '\0', NULL);
	
	if (parse_power) {
		loqui_utils_irc_parse_nick(nick, &is_o, &is_v, &tmp_nick);
		nick = tmp_nick;
		is_channel_operator = is_o;
		speakable = is_v;
	}

	user = loqui_account_fetch_user(channel->account, nick);
	member = loqui_member_new(user);
	g_object_unref(user); /* member has reference count */
	if (is_channel_operator)
		loqui_member_set_is_channel_operator(member, TRUE);
	else if (speakable)
		loqui_member_set_speakable(member, TRUE);
	loqui_channel_entry_add_member(LOQUI_CHANNEL_ENTRY(channel), member);
	g_object_unref(member); /* channel entry has reference count */

	return member;
}
void
loqui_channel_append_remark(LoquiChannel *channel, TextType type, gboolean is_self, const gchar *nick, const gchar *remark, gboolean is_from_server)
{
	ChannelBuffer *buffer;
	LoquiChannelPrivate *priv;
	MessageText *msgtext;
	GList *cur;
	gchar *word;

	gboolean is_priv = FALSE;
	gboolean exec_notification = TRUE && !is_from_server && !is_self;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	priv = channel->priv;
	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(channel));

	is_priv = loqui_channel_get_is_private_talk(channel);

	msgtext = message_text_new();
	g_object_set(G_OBJECT(msgtext),
		     "is_remark", TRUE,
		     "text_type", type,
		     "is_priv", is_priv,
		     "is_self", is_self,
		     "text", remark,
		     "nick", nick,
		     "channel_name", loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), NULL);

	if (type == TEXT_TYPE_NOTICE || is_from_server)
		loqui_channel_entry_set_is_updated_weak(LOQUI_CHANNEL_ENTRY(channel), TRUE);
	else
		loqui_channel_entry_set_is_updated(LOQUI_CHANNEL_ENTRY(channel), TRUE);
	
	if (!is_self && !is_from_server) {
		for (cur = prefs_general.highlight_list; cur != NULL; cur = cur->next) {
			word = (gchar *) cur->data;
			if (strstr(remark, word) != NULL) {
				loqui_channel_entry_set_has_unread_keyword(LOQUI_CHANNEL_ENTRY(channel), TRUE);
			}
		}
	}

	channel_buffer_append_message_text(buffer, msgtext, FALSE, exec_notification);
	g_object_unref(msgtext);
}
void
loqui_channel_append_text(LoquiChannel *channel, TextType type, gchar *str)
{
	ChannelBuffer *buffer;
	MessageText *msgtext;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(channel));

	msgtext = message_text_new();
	g_object_set(G_OBJECT(msgtext),
		     "is_remark", FALSE,
		     "text_type", type,
		     "text", str,
		     NULL);

	channel_buffer_append_message_text(buffer, msgtext, FALSE, FALSE);
	g_object_unref(msgtext);
}
