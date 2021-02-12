/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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

#include "loqui_channel_irc.h"
#include "loqui_utils_irc.h"
#include "loqui_account_irc.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiChannelIRCPrivate
{
};

static LoquiChannelClass *parent_class = NULL;

/* static guint loqui_channel_irc_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_channel_irc_class_init(LoquiChannelIRCClass *klass);
static void loqui_channel_irc_init(LoquiChannelIRC *channel);
static void loqui_channel_irc_finalize(GObject *object);
static void loqui_channel_irc_dispose(GObject *object);

static void loqui_channel_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_channel_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_CHANNEL,
					      "LoquiChannelIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_channel_irc_finalize(GObject *object)
{
	LoquiChannelIRC *channel;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_IRC(object));

        channel = LOQUI_CHANNEL_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(channel->priv);
}
static void 
loqui_channel_irc_dispose(GObject *object)
{
	LoquiChannelIRC *channel;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_IRC(object));

        channel = LOQUI_CHANNEL_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_channel_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiChannelIRC *channel;        

        channel = LOQUI_CHANNEL_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiChannelIRC *channel;        

        channel = LOQUI_CHANNEL_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_channel_irc_class_init(LoquiChannelIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_irc_finalize;
        object_class->dispose = loqui_channel_irc_dispose;
        object_class->get_property = loqui_channel_irc_get_property;
        object_class->set_property = loqui_channel_irc_set_property;
}
static void 
loqui_channel_irc_init(LoquiChannelIRC *channel)
{
	LoquiChannelIRCPrivate *priv;

	priv = g_new0(LoquiChannelIRCPrivate, 1);

	channel->priv = priv;
}
LoquiChannelIRC*
loqui_channel_irc_new(LoquiAccount *account, const gchar *name, gboolean is_joined, gboolean is_private_talk)
{
        LoquiChannelIRC *channel;
	LoquiChannelIRCPrivate *priv;

	channel = g_object_new(loqui_channel_irc_get_type(), 
			       "account", account,
			       "name", name,
			       "identifier", name,
			       "is_private_talk", is_private_talk,
			       "is_joined", is_joined,
			       NULL);
	
        priv = channel->priv;

        return channel;
}
LoquiMember *
loqui_channel_irc_add_member_by_nick(LoquiChannelIRC *channel_irc, const gchar *nick, gboolean parse_power, gboolean is_channel_operator, gboolean speakable)
{
	gchar *tmp_nick;
	LoquiMember *member;
	LoquiUser *user;
	gboolean is_o, is_v;
	LoquiChannel *channel;

        g_return_val_if_fail(channel_irc != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_IRC(channel_irc), NULL);
        g_return_val_if_fail(nick != NULL, NULL);
        g_return_val_if_fail(*nick != '\0', NULL);

	channel = LOQUI_CHANNEL(channel_irc);

	if (parse_power) {
		loqui_utils_irc_parse_nick(nick, &is_o, &is_v, &tmp_nick);
		nick = tmp_nick;
		is_channel_operator = is_o;
		speakable = is_v;
	}

	user = LOQUI_USER(loqui_account_irc_fetch_user(LOQUI_ACCOUNT_IRC(channel->account), nick));
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
