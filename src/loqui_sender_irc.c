/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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

#include "loqui_sender_irc.h"
#include "prefs_general.h"

#include "main.h"
#include "ctcp_message.h"
#include "intl.h"
#include "loqui_utils_irc.h"

#include <string.h>

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
static void loqui_sender_irc_join_raw(LoquiSender *sender, const gchar *target, const gchar *key);

/* helper */
static void loqui_sender_irc_speak(LoquiSenderIRC *sender, LoquiChannel *channel, const gchar *text, gboolean is_notice);

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

	sender_class->join_raw = loqui_sender_irc_join_raw;
}
static void 
loqui_sender_irc_init(LoquiSenderIRC *sender)
{
	LoquiSenderIRCPrivate *priv;

	priv = g_new0(LoquiSenderIRCPrivate, 1);

	sender->priv = priv;
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

/* helper */
static void
loqui_sender_irc_speak(LoquiSenderIRC *sender, LoquiChannel *channel, const gchar *text, gboolean is_notice)
{
	gchar *buf;
	gchar **array;
	int i;
	LoquiMember *member;
	
	buf = g_strdup(text);
	utils_remove_return_code(buf); /* remove last return code */
	array = g_strsplit(buf, "\n", -1);
	g_free(buf);

	for(i = 0; array[i] != NULL; i++) {
		if(strlen(array[i]) == 0)
			continue;
		
		if (is_notice)
			loqui_sender_irc_notice_raw(sender, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), array[i]);
		else
			loqui_sender_irc_say_raw(sender, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), array[i]);

		loqui_channel_append_remark(channel, is_notice ? TEXT_TYPE_NOTICE : TEXT_TYPE_NORMAL,
					    TRUE, loqui_user_get_nick(LOQUI_SENDER(sender)->account->user_self), array[i], FALSE);
	}
	g_strfreev(array);
	
	member = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), loqui_account_get_user_self(LOQUI_SENDER(sender)->account));
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
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandNick, text, NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_away(LoquiSender *sender, LoquiAwayType away_type, const gchar *away_message)
{
	const gchar *text;
	IRCMessage *msg = NULL;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	if (away_type == LOQUI_AWAY_TYPE_AWAY) {
		text = away_message == NULL ? prefs_general.away_message : away_message;
		msg = irc_message_create(IRCCommandAway, text, NULL);
	} else if (away_type == LOQUI_AWAY_TYPE_ONLINE) {
		msg = irc_message_create(IRCCommandAway, NULL);
	}

	if (msg) {
		irc_connection_push_message(conn, msg);
		g_object_unref(msg);
	}
}
static void
loqui_sender_irc_whois(LoquiSender *sender, LoquiUser *user)
{
	IRCMessage *msg = NULL;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandWhois, loqui_user_get_nick(user), loqui_user_get_nick(user), NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_join(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	/* FIXME: handle key */
	loqui_sender_irc_join_raw(sender, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), NULL);
}
static void
loqui_sender_irc_part(LoquiSender *sender, LoquiChannel *channel, const gchar *part_message)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (loqui_channel_get_is_private_talk(channel)) {
		g_warning("This is a private talk");
		return;
	}
	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandPart,
				 loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
				 part_message,
				 NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_topic(LoquiSender *sender, LoquiChannel *channel, const gchar *topic)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (loqui_channel_get_is_private_talk(channel)) {
		g_warning("This is a private talk");
		return;
	}
	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandTopic,
				 loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
				 topic,
				 NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_start_private_talk(LoquiSender *sender, LoquiUser *user)
{
	LoquiMember *member;
	LoquiChannel *channel;
	LoquiUser *user_self;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	if ((channel = loqui_account_get_channel_by_name(sender->account, loqui_user_get_nick(user))) == NULL) {
		channel = loqui_channel_new(sender->account, loqui_user_get_nick(user), TRUE, TRUE);
		
		user_self = loqui_account_get_user_self(sender->account);
		
		if (user_self != user) {
			member = loqui_member_new(user);
			loqui_channel_entry_add_member(LOQUI_CHANNEL_ENTRY(channel), member);
			g_object_unref(member);
		}

		member = loqui_member_new(user_self);
		loqui_channel_entry_add_member(LOQUI_CHANNEL_ENTRY(channel), member);
		g_object_unref(member);

		loqui_account_add_channel(sender->account, channel);
		g_object_unref(channel);
	}
}
static void
loqui_sender_irc_end_private_talk(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	loqui_account_remove_channel(sender->account, channel);
}
static void
loqui_sender_irc_refresh(LoquiSender *sender, LoquiChannel *channel)
{
	IRCMessage *msg;
	IRCConnection *conn;
	IRCHandle *handle;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	handle = loqui_account_get_handle(sender->account);
	handle->prevent_print_who_reply_count++;
	
	msg = irc_message_create(IRCCommandWho, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_join_raw(LoquiSender *sender, const gchar *target, const gchar *key)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	if (!LOQUI_UTILS_IRC_STRING_IS_CHANNEL(target)) {
		g_warning("This name seems not to be a channel.");
		return;
	}

	conn = loqui_account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);

	if (key == NULL || strlen(key) == 0)
		key = NULL;

	msg = irc_message_create(IRCCommandJoin, target, key, NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_pong_raw(LoquiSenderIRC *sender, const gchar *target)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandPong, target, NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_say_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *text)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);
	
	msg = irc_message_create(IRCCommandPrivmsg, 
				 target, text, NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_notice_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *text)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);
	
	msg = irc_message_create(IRCCommandNotice,
				 target, text, NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}

void loqui_sender_irc_send_raw(LoquiSenderIRC *sender, const gchar *str)
{
	IRCMessage *msg;
	IRCConnection *conn;
	gchar *buf;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);
	
	msg = irc_message_parse_line(str);
	if (debug_mode) {
		buf = irc_message_to_string(msg);
		debug_puts("send_raw: %s", buf);
		g_free(buf);
	}
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
void
loqui_sender_irc_get_channel_mode(LoquiSender *sender, LoquiChannel *channel)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandMode, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)), NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}

void
loqui_sender_irc_ctcp_request_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *command)
{
	IRCMessage *msg;
	CTCPMessage *ctcp_msg;
	IRCConnection *conn;
	gchar *buf;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);
	
	ctcp_msg = ctcp_message_new(command, NULL);
	buf = ctcp_message_to_str(ctcp_msg);
	g_object_unref(ctcp_msg);

	msg = irc_message_create(IRCCommandPrivmsg, target, buf, NULL);
	g_free(buf);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);

	buf = g_strdup_printf(_("Sent CTCP request to %s: %s"), target, command);
	loqui_account_console_buffer_append(LOQUI_SENDER(sender)->account, TEXT_TYPE_INFO, buf);
	g_free(buf);
}
void
loqui_sender_irc_change_member_mode(LoquiSenderIRC *sender, LoquiChannel *channel,
				    gboolean is_give, IRCModeFlag flag, GList *str_list)
{
	IRCMessage *msg;
	IRCConnection *conn;
	guint i, p, list_num;
	gchar flag_str[IRC_MESSAGE_PARAMETER_MAX + 10];
	gchar *param_array[IRC_MESSAGE_PARAMETER_MAX + 10];
	GList *cur;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!loqui_account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	conn = loqui_account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);
	
	list_num = g_list_length(str_list);
	if (list_num > IRC_MESSAGE_PARAMETER_MAX) {
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
	debug_puts("Sending MODE command.\n");
	if (show_msg_mode)
		irc_message_print(msg);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}