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
static void loqui_sender_irc_part(LoquiSender *sender, LoquiChannel *channel);
static void loqui_sender_irc_topic(LoquiSender *sender, LoquiChannel *channel, const gchar *topic);
static void loqui_sender_irc_start_private_talk(LoquiSender *sender, LoquiChannel *channel);

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
}
static void 
loqui_sender_irc_init(LoquiSenderIRC *sender)
{
	LoquiSenderIRCPrivate *priv;

	priv = g_new0(LoquiSenderIRCPrivate, 1);

	sender->priv = priv;
}
LoquiSenderIRC*
loqui_sender_irc_new(Account *account)
{
        LoquiSenderIRC *sender;
	LoquiSenderIRCPrivate *priv;

	sender = g_object_new(loqui_sender_irc_get_type(), NULL);
	
        priv = sender->priv;
	LOQUI_SENDER(sender)->account = account;

        return sender;
}

static void
loqui_sender_irc_say(LoquiSender *sender, LoquiChannel *channel, const gchar *text)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

}
static void
loqui_sender_irc_notice(LoquiSender *sender, LoquiChannel *channel, const gchar *text)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

}
static void
loqui_sender_irc_nick(LoquiSender *sender, const gchar *text)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = account_get_connection(sender->account);
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

	if (!account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = account_get_connection(sender->account);
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
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

}
static void
loqui_sender_irc_join(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

}
static void
loqui_sender_irc_part(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

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
	if (!account_is_connected(sender->account)) {
		g_warning("Not connected");
		return;
	}

	conn = account_get_connection(sender->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandTopic,
				 loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)),
				 topic,
				 NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
static void
loqui_sender_irc_start_private_talk(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

}
void
loqui_sender_irc_pong_raw(LoquiSenderIRC *sender, const gchar *target)
{
	IRCMessage *msg;
	IRCConnection *conn;

        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IRC(sender));

	if (!account_is_connected(LOQUI_SENDER(sender)->account)) {
		g_warning("Not connected");
		return;
	}

	conn = account_get_connection(LOQUI_SENDER(sender)->account);
	g_return_if_fail(conn != NULL);

	msg = irc_message_create(IRCCommandPong, target, NULL);
	irc_connection_push_message(conn, msg);
	g_object_unref(msg);
}
