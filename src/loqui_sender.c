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

#include "loqui_sender.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiSenderPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_sender_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_sender_class_init(LoquiSenderClass *klass);
static void loqui_sender_init(LoquiSender *sender);
static void loqui_sender_finalize(GObject *object);
static void loqui_sender_dispose(GObject *object);

static void loqui_sender_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_sender_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_sender_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiSenderClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_sender_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiSender),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_sender_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiSender",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_sender_finalize(GObject *object)
{
	LoquiSender *sender;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(object));

        sender = LOQUI_SENDER(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(sender->priv);
}
static void 
loqui_sender_dispose(GObject *object)
{
	LoquiSender *sender;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(object));

        sender = LOQUI_SENDER(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_sender_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiSender *sender;        

        sender = LOQUI_SENDER(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_sender_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiSender *sender;        

        sender = LOQUI_SENDER(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_sender_class_init(LoquiSenderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_sender_finalize;
        object_class->dispose = loqui_sender_dispose;
        object_class->get_property = loqui_sender_get_property;
        object_class->set_property = loqui_sender_set_property;
}
static void 
loqui_sender_init(LoquiSender *sender)
{
	LoquiSenderPrivate *priv;

	priv = g_new0(LoquiSenderPrivate, 1);

	sender->priv = priv;
}
LoquiSender*
loqui_sender_new(LoquiAccount *account)
{
        LoquiSender *sender;
	LoquiSenderPrivate *priv;

	sender = g_object_new(loqui_sender_get_type(), NULL);
	
        priv = sender->priv;
	sender->account = account;

        return sender;
}
LoquiAccount*
loqui_sender_get_account(LoquiSender *sender)
{
        g_return_val_if_fail(sender != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_SENDER(sender), NULL);

	/* check account is valid */
	g_return_val_if_fail(sender->account != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_ACCOUNT(sender->account), NULL);

	return sender->account;
}

#define CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, name) { \
	if (!LOQUI_SENDER_GET_CLASS(sender)->name) { \
		g_warning("Not defined %s in the class %s", # name, G_OBJECT_TYPE_NAME(sender)); \
		return; \
	} \
}

void
loqui_sender_say(LoquiSender *sender, LoquiChannel *channel, const gchar *text)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, say);

	(* LOQUI_SENDER_GET_CLASS(sender)->say) (sender, channel, text);
}
void
loqui_sender_notice(LoquiSender *sender, LoquiChannel *channel, const gchar *text)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, notice);

	(* LOQUI_SENDER_GET_CLASS(sender)->notice) (sender, channel, text);
}
void
loqui_sender_nick(LoquiSender *sender, const gchar *text)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, nick);

	(* LOQUI_SENDER_GET_CLASS(sender)->nick) (sender, text);
}
void
loqui_sender_away(LoquiSender *sender, LoquiAwayType away_type, const gchar *away_message)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, away);

	(* LOQUI_SENDER_GET_CLASS(sender)->away) (sender, away_type, away_message);
}
void
loqui_sender_whois(LoquiSender *sender, LoquiUser *user)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, whois);

	(* LOQUI_SENDER_GET_CLASS(sender)->whois) (sender, user);
}
void
loqui_sender_join(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, join);

	(* LOQUI_SENDER_GET_CLASS(sender)->join) (sender, channel);
}
void
loqui_sender_part(LoquiSender *sender, LoquiChannel *channel, const gchar *part_message)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, part);

	(* LOQUI_SENDER_GET_CLASS(sender)->part) (sender, channel, part_message);
}
void
loqui_sender_topic(LoquiSender *sender, LoquiChannel *channel, const gchar *topic)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, topic);

	(* LOQUI_SENDER_GET_CLASS(sender)->topic) (sender, channel, topic);
}
void
loqui_sender_start_private_talk(LoquiSender *sender, LoquiUser *user)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, start_private_talk);

	(* LOQUI_SENDER_GET_CLASS(sender)->start_private_talk) (sender, user);
}
void
loqui_sender_end_private_talk(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, end_private_talk);

	(* LOQUI_SENDER_GET_CLASS(sender)->end_private_talk) (sender, channel);
}
void
loqui_sender_refresh(LoquiSender *sender, LoquiChannel *channel)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, refresh);

	(* LOQUI_SENDER_GET_CLASS(sender)->refresh) (sender, channel);
}
void
loqui_sender_join_raw(LoquiSender *sender, const gchar *target, const gchar *key)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, join_raw);

	(* LOQUI_SENDER_GET_CLASS(sender)->join_raw) (sender, target, key);
}
void
loqui_sender_start_private_talk_raw(LoquiSender *sender, const gchar *target)
{
        g_return_if_fail(sender != NULL);
        g_return_if_fail(LOQUI_IS_SENDER(sender));

	CHECK_FUNCTION_IS_DEFINED_AND_RETURN_IF_FAIL(sender, start_private_talk_raw);

	(* LOQUI_SENDER_GET_CLASS(sender)->start_private_talk_raw) (sender, target);
}
