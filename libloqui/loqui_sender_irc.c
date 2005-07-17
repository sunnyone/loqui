/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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

#include "loqui_sender_irc.h"

#include "ctcp_message.h"
#include <libloqui-intl.h>
#include "loqui_utils_irc.h"
#include "loqui_account_irc.h"
#include "loqui_channel_irc.h"
#include "loqui_receiver_irc.h"
#include "loqui-static-core.h"

#include <string.h>
#include "loqui-general-pref-default.h"
#include "loqui-general-pref-groups.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiSenderIRCPrivate
{
};

static LoquiSenderClass *parent_class = NULL;

/* static guint loqui_sender_irc_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_sender_irc_class_init(LoquiSenderIRCClass *klass);
static void loqui_sender_irc_init(LoquiSenderIRC *sender);
static void loqui_sender_irc_finalize(GObject *object);
static void loqui_sender_irc_dispose(GObject *object);

static void loqui_sender_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_sender_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_sender_irc_say(LoquiSender *sender, LoquiChannel *channel, const gchar *text);
static void loqui_sender_irc_notice(LoquiSender *sender, LoquiChannel *channel, const gchar *text);
static void loqui_sender_irc_nick(LoquiSender *sender, const gchar *text);
static void loqui_sender_irc_away(LoquiSender *sender, LoquiAwayType away_type, const gchar *away_message);
static void loqui_sender_irc_whois(LoquiSender *sender, LoquiUser *user);
static void loqui_sender_irc_join(LoquiSender *sender, LoquiChannel *channel);
static void loqui_sender_irc_part(LoquiSender *sender, LoquiChannel *channel, const gchar *part_message);
static void loqui_sender_irc_topic(LoquiSender *sender, LoquiChannel *channel, const gchar *topic);
static void loqui_sender_irc_start_private_talk(LoquiSender *sender, LoquiUser *user);
static void loqui_sender_irc_end_private_talk(LoquiSender *sender, LoquiChannel *channel);
static void loqui_sender_irc_refresh(LoquiSender *sender, LoquiChannel *channel);
static void loqui_sender_irc_quit(LoquiSender *sender, const gchar *quit_message);
static void loqui_sender_irc_join_raw(LoquiSender *sender, const gchar *target, const gchar *key);
static void loqui_sender_irc_start_private_talk_raw(LoquiSender *sender, const gchar *target);

static void loqui_sender_sent_quit(LoquiSenderIRC *sender, IRCMessage *msg);
static void loqui_sender_sent_privmsg_notice(LoquiSenderIRC *sender, IRCMessage *msg);

/* helper */
static void loqui_sender_irc_send_irc_message(LoquiSenderIRC *sender, IRCMessage *msg);
static void loqui_sender_irc_speak(LoquiSenderIRC *sender, LoquiChannel *channel, const gchar *text, gboolean is_notice);
static gboolean check_target_valid(LoquiAccount *account, const gchar *str);

#define WARN_AND_RETURN_UNLESS_CONNECTED(sender) { \
	LoquiAccount *ac; \
	ac = loqui_sender_get_account(LOQUI_SENDER(sender)); \
	if (!loqui_account_get_is_connected(ac)) { \
		loqui_account_warning(ac, _("Not connected")); \
		return; \
	} \
}
static gboolean
check_target_valid(LoquiAccount *account, const gchar *str)
{
	if(str == NULL || strlen(str) == 0) {
		loqui_account_warning(account, _("No characters exist."));
		return FALSE;
	}

	if(strchr(str, ' ') != NULL) {
		loqui_account_warning(account, _("Error: space contains"));
		return FALSE;
	}
	return TRUE;
}


GType
loqui_sender_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiSenderIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_sender_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiSenderIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_sender_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_SENDER,
					      "LoquiSenderIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_sender_irc_finalize(GObject *object)
{
	LoquiSenderIRC *sender;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(object));

        sender = LOQUI_SENDER_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(sender->priv);
}
static void 
loqui_sender_irc_dispose(GObject *object)
{
	LoquiSenderIRC *sender;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(object));

        sender = LOQUI_SENDER_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_sender_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiSenderIRC *sender;        

        sender = LOQUI_SENDER_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_sender_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiSenderIRC *sender;        

        sender = LOQUI_SENDER_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_sender_irc_class_init(LoquiSenderIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	LoquiSenderClass *sender_class = LOQUI_SENDER_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_sender_irc_finalize;
        object_class->dispose = loqui_sender_irc_dispose;
        object_class->get_property = loqui_sender_irc_get_property;
        object_class->set_property = loqui_sender_irc_set_property;

	sender_class->say = loqui_sender_irc_say;
	sender_class->notice = loqui_sender_irc_notice;
	sender_class->nick = loqui_sender_irc_nick;
	sender_class->away = loqui_sender_irc_away;
	sender_class->whois = loqui_sender_irc_whois;
	sender_class->join = loqui_sender_irc_join;
	sender_class->part = loqui_sender_irc_part;
	sender_class->topic = loqui_sender_irc_topic;
	sender_class->start_private_talk = loqui_sender_irc_start_private_talk;
	sender_class->end_private_talk = loqui_sender_irc_end_private_talk;
	sender_class->refresh = loqui_sender_irc_refresh;
	sender_class->quit = loqui_sender_irc_quit;

	sender_class->join_raw = loqui_sender_irc_join_raw;
	sender_class->start_private_talk_raw = loqui_sender_irc_start_private_talk_raw;
}
static void 
loqui_sender_irc_init(LoquiSenderIRC *sender)
{
	LoquiSenderIRCPrivate *priv;

	priv = g_new0(LoquiSenderIRCPrivate, 1);

	sender->priv = priv;

	sender->sent_quit = FALSE;
}
LoquiSenderIRC*
loqui_sender_irc_new(LoquiAccount *account)
{
        LoquiSenderIRC *sender;
	LoquiSenderIRCPrivate *priv;

	sender = g_object_new(loqui_sender_irc_get_type(), NULL);
	
        priv = sender->priv;
	LOQUI_SENDER(sender)->account = account;

        return sender;
}
void
loqui_sender_irc_reset(LoquiSenderIRC *sender)
{
	sender->sent_quit = FALSE;
}

/* helper */
static void
loqui_sender_irc_send_irc_message(LoquiSenderIRC *sender, IRCMessage *msg)
{
	loqui_account_irc_push_message(LOQUI_ACCOUNT_IRC(loqui_sender_get_account(LOQUI_SENDER(sender))), msg);
}
/* helper */
static void
loqui_sender_irc_speak(LoquiSenderIRC *sender, LoquiChannel *channel, const gchar *text, gboolean is_notice)
{
	gchar *buf;
	gchar **array;
	int i;
	LoquiMember *member;
	LoquiUser *user_self;
	
	buf = g_strdup(text);
	utils_remove_return_code(buf); /* remove last return code */
	array = g_strsplit(buf, "\n", -1);
	g_free(buf);

	user_self = loqui_account_get_user_self(loqui_sender_get_account(LOQUI_SENDER(sender)));

	for(i = 0; array[i] != NULL; i++) {
		if(strlen(array[i]) == 0)
			continue;
		
		if (is_notice)
			loqui_sender_irc_notice_raw(sender, loqui_channel_get_identifier(channel), array[i]);
		else
			loqui_sender_irc_say_raw(sender, loqui_channel_get_identifier(channel), array[i]);

		loqui_channel_append_remark(channel, is_notice ? LOQUI_TEXT_TYPE_NOTICE : LOQUI_TEXT_TYPE_NORMAL,
					    TRUE, loqui_user_get_nick(user_self), array[i], FALSE);
	}
	g_strfreev(array);
	
	member = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user_self);
	loqui_member_set_last_message_time(member, time(NULL));
}
static void
loqui_sender_irc_say(LoquiSender *sender, LoquiChannel *channel, const gchar *text)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	loqui_sender_irc_speak(LOQUI_SENDER_IRC(sender), channel, text, FALSE);
}
static void
loqui_sender_irc_notice(LoquiSender *sender, LoquiChannel *channel, const gchar *text)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	loqui_sender_irc_speak(LOQUI_SENDER_IRC(sender), channel, text, TRUE);
}
static void
loqui_sender_irc_nick(LoquiSender *sender, const gchar *text)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);
	if (!check_target_valid(loqui_sender_get_account(sender), text))
		return;

	msg = irc_message_create(IRCCommandNick, text, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_away(LoquiSender *sender, LoquiAwayType away_type, const gchar *away_message)
{
	const gchar *text;
	IRCMessage *msg = NULL;
	gchar *away_message_pref;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	away_message_pref = loqui_pref_get_with_default_string(loqui_get_general_pref(),
							       LOQUI_GENERAL_PREF_GROUP_MESSAGES, "AwayMessage",
							       LOQUI_GENERAL_PREF_DEFAULT_MESSAGES_AWAY_MESSAGE, NULL);
	if (away_type == LOQUI_AWAY_TYPE_AWAY) {
		text = away_message == NULL ? away_message_pref : away_message;
		msg = irc_message_create(IRCCommandAway, text, NULL);
	} else if (away_type == LOQUI_AWAY_TYPE_ONLINE) {
		msg = irc_message_create(IRCCommandAway, NULL);
	} else {
		g_warning("Invalid away_type is specified.");
		g_free(away_message_pref);
		return;
	}
	g_free(away_message_pref);

	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_whois(LoquiSender *sender, LoquiUser *user)
{
	IRCMessage *msg = NULL;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_create(IRCCommandWhois, loqui_user_get_nick(user), loqui_user_get_nick(user), NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_join(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	/* FIXME: handle key */
	loqui_sender_irc_join_raw(sender, loqui_channel_get_identifier(channel), NULL);
}
static void
loqui_sender_irc_part(LoquiSender *sender, LoquiChannel *channel, const gchar *part_message)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (loqui_channel_get_is_private_talk(channel)) {
		loqui_account_warning(loqui_sender_get_account(sender), _("This is a private talk"));
		return;
	}

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_create(IRCCommandPart,
				 loqui_channel_get_identifier(channel),
				 part_message,
				 NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_topic(LoquiSender *sender, LoquiChannel *channel, const gchar *topic)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (loqui_channel_get_is_private_talk(channel)) {
		loqui_account_warning(loqui_sender_get_account(sender), "This is a private talk");
		return;
	}
	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_create(IRCCommandTopic,
				 loqui_channel_get_identifier(channel),
				 topic,
				 NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_start_private_talk(LoquiSender *sender, LoquiUser *user)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	loqui_account_open_private_talk(loqui_sender_get_account(sender), loqui_user_get_nick(user), user);
}
static void
loqui_sender_irc_end_private_talk(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	loqui_account_remove_channel(loqui_sender_get_account(sender), channel);
}
static void
loqui_sender_irc_refresh(LoquiSender *sender, LoquiChannel *channel)
{
	IRCMessage *msg;
	LoquiReceiverIRC *receiver;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	receiver = LOQUI_RECEIVER_IRC(loqui_account_get_receiver(loqui_sender_get_account(sender)));
	receiver->prevent_print_who_reply_count++;
	
	msg = irc_message_create(IRCCommandWho, loqui_channel_get_identifier(channel), NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_quit(LoquiSender *sender, const gchar *quit_message)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_create(IRCCommandQuit, quit_message, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_join_raw(LoquiSender *sender, const gchar *target, const gchar *key)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	if (!check_target_valid(loqui_sender_get_account(sender), target))
		return;

	if (!LOQUI_UTILS_IRC_STRING_IS_CHANNEL(target)) {
		loqui_account_warning(loqui_sender_get_account(sender), _("This name seems not to be a channel."));
		return;
	}

	if (key == NULL || strlen(key) == 0)
		key = NULL;

	msg = irc_message_create(IRCCommandJoin, target, key, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_start_private_talk_raw(LoquiSender *sender, const gchar *target)
{
	LoquiUser *user;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	if (!check_target_valid(loqui_sender_get_account(sender), target))
		return;

	if (LOQUI_UTILS_IRC_STRING_IS_CHANNEL(target)) {
		loqui_account_warning(loqui_sender_get_account(sender), _("This name seems not to be a nick."));
		return;
	}
	
	user = LOQUI_USER(loqui_account_irc_fetch_user(LOQUI_ACCOUNT_IRC(loqui_sender_get_account(sender)), target));
	if (!user) {
		g_warning("Can't fetch user for private talk");
		return;
	}
	loqui_sender_start_private_talk(sender, user);
	g_object_unref(user);
}
void
loqui_sender_irc_pong_raw(LoquiSenderIRC *sender, const gchar *target)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_create(IRCCommandPong, target, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_say_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *text)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_create(IRCCommandPrivmsg, 
				 target, text, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_notice_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *text)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);
	
	msg = irc_message_create(IRCCommandNotice,
				 target, text, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_send_raw(LoquiSenderIRC *sender, const gchar *str)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_parse_line(str);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_get_channel_mode(LoquiSender *sender, LoquiChannel *channel)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	msg = irc_message_create(IRCCommandMode, loqui_channel_get_identifier(channel), NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}

void
loqui_sender_irc_ctcp_request_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *command)
{
	IRCMessage *msg;
	CTCPMessage *ctcp_msg;
	gchar *buf;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	if (!check_target_valid(loqui_sender_get_account(LOQUI_SENDER(sender)), target))
		return;

	ctcp_msg = ctcp_message_new(command, NULL);
	buf = ctcp_message_to_str(ctcp_msg);
	g_object_unref(ctcp_msg);

	msg = irc_message_create(IRCCommandPrivmsg, target, buf, NULL);
	g_free(buf);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);

	buf = g_strdup_printf(_("Sent CTCP request to %s: %s"), target, command);
	loqui_account_append_text(loqui_sender_get_account(LOQUI_SENDER(sender)), NULL, LOQUI_TEXT_TYPE_INFO, buf);
	g_free(buf);
}
void
loqui_sender_irc_change_member_mode(LoquiSenderIRC *sender, LoquiChannel *channel,
				    gboolean is_give, IRCModeFlag flag, GList *str_list)
{
	IRCMessage *msg;
	guint i, p, list_num;
	gchar flag_str[IRC_MESSAGE_PARAMETER_MAX + 10];
	gchar *param_array[IRC_MESSAGE_PARAMETER_MAX + 10];
	GList *cur;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	list_num = g_list_length(str_list);
	if (list_num > IRC_MESSAGE_PARAMETER_MAX) {
		g_warning(_("Too many users in change mode request!"));
		return;
	}
	
	p = 0;
	/* MODE #Channel +? user1 user2 user3 */
	param_array[p] = (gchar *) loqui_channel_get_identifier(channel);
	p++;

	if(is_give)
		flag_str[0] = '+';
	else
		flag_str[0] = '-';

	for (i = 0; i < list_num; i++)
		flag_str[i+1] = (gchar) flag;
	flag_str[i+1] = '\0';

	param_array[p] = flag_str;
	p++;
	
	for (cur = str_list; cur != NULL; cur = cur->next) {
		param_array[p] = cur->data;
		p++;
	}
	param_array[p] = NULL;

	msg = irc_message_createv(IRCCommandMode, param_array);
	loqui_debug_puts("Sending MODE command.\n");
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_user_raw(LoquiSenderIRC *sender, const gchar *username, const gchar *realname)
{
	IRCMessage *msg;
	LoquiAccount *account;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	account = loqui_sender_get_account(LOQUI_SENDER(sender));

	if (username == NULL || strlen(username) == 0) {
		loqui_account_warning(account, _("Username is not specified."));
		return;
	}
	if (strchr(username, ' ')) {
		loqui_account_warning(account, _("Username must not include space(s)."));
		return;
	}
	if (realname == NULL || strlen(realname) == 0) {
		loqui_account_warning(account, _("Realname is not specified."));
		return;
	}

	msg = irc_message_create(IRCCommandUser,
				 username, "0", "*", realname, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_pass(LoquiSenderIRC *sender, const gchar *password)
{
	IRCMessage *msg;
	LoquiAccount *account;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	account = loqui_sender_get_account(LOQUI_SENDER(sender));

	if (password == NULL || strlen(password) == 0) {
		loqui_account_warning(account, _("Password is not specified."));
		return;
	}
	if (strchr(password, ' ')) {
		loqui_account_warning(account, _("Password must not contain space(s)."));
		return;
	}

	msg = irc_message_create(IRCCommandPass,
				 password, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_ping_raw(LoquiSenderIRC *sender, const gchar *target)
{
	IRCMessage *msg;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	msg = irc_message_create(IRCCommandPing, target, NULL);
	loqui_sender_irc_send_irc_message(LOQUI_SENDER_IRC(sender), msg);
	g_object_unref(msg);
}
/* the function receives the message sent by AccountIRC */
void
loqui_sender_irc_message_sent(LoquiSenderIRC *sender, IRCMessage *msg)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	WARN_AND_RETURN_UNLESS_CONNECTED(sender);

	switch (msg->response) {
	case IRC_COMMAND_QUIT:
		loqui_sender_sent_quit(sender, msg);
		break;
	case IRC_COMMAND_PRIVMSG:
	case IRC_COMMAND_NOTICE:
		loqui_sender_sent_privmsg_notice(sender, msg);
		break;
	default:
		break;
	}
}

static void
loqui_sender_sent_quit(LoquiSenderIRC *sender, IRCMessage *msg)
{
	LoquiAccount *account;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	account = loqui_sender_get_account(LOQUI_SENDER(sender));

	LOQUI_SENDER_IRC(sender)->sent_quit = TRUE;
}

static void
loqui_sender_sent_privmsg_notice(LoquiSenderIRC *sender, IRCMessage *msg)
{
	LoquiAccount *account;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	account = loqui_sender_get_account(LOQUI_SENDER(sender));

	/* TODO: append to buffer */
}
