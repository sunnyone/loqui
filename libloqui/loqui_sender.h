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
#ifndef __LOQUI_SENDER_H__
#define __LOQUI_SENDER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_SENDER                 (loqui_sender_get_type ())
#define LOQUI_SENDER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_SENDER, LoquiSender))
#define LOQUI_SENDER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_SENDER, LoquiSenderClass))
#define LOQUI_IS_SENDER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_SENDER))
#define LOQUI_IS_SENDER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_SENDER))
#define LOQUI_SENDER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_SENDER, LoquiSenderClass))

typedef struct _LoquiSender            LoquiSender;
typedef struct _LoquiSenderClass       LoquiSenderClass;

typedef struct _LoquiSenderPrivate     LoquiSenderPrivate;

#include "loqui_account.h"
#include "loqui_channel.h"

struct _LoquiSender
{
        GObject parent;
        
	/* < protected > */
	LoquiAccount *account;

        LoquiSenderPrivate *priv;
};

struct _LoquiSenderClass
{
        GObjectClass parent_class;

	void (*say) (LoquiSender *sender, LoquiChannel *channel, const gchar *text);
	void (*notice) (LoquiSender *sender, LoquiChannel *channel, const gchar *text);
	void (*nick) (LoquiSender *sender, const gchar *text);
	void (*away) (LoquiSender *sender, LoquiAwayType away_type, const gchar *away_message);
	void (*whois) (LoquiSender *sender, LoquiUser *user);
	void (*join) (LoquiSender *sender, LoquiChannel *channel);
	void (*part) (LoquiSender *sender, LoquiChannel *channel, const gchar *part_message);
	void (*topic) (LoquiSender *sender, LoquiChannel *channel, const gchar *topic);
	void (*start_private_talk) (LoquiSender *sender, LoquiUser *user);
	void (*end_private_talk) (LoquiSender *sender, LoquiChannel *channel);
	void (*refresh) (LoquiSender *sender, LoquiChannel *channel);
	void (*quit) (LoquiSender *sender, const gchar *quit_message);

	void (*join_raw) (LoquiSender *sender, const gchar *target, const gchar *key);
	void (*start_private_talk_raw) (LoquiSender *sender, const gchar *target);
};

GType loqui_sender_get_type(void) G_GNUC_CONST;

LoquiSender* loqui_sender_new(LoquiAccount *account);
LoquiAccount* loqui_sender_get_account(LoquiSender *sender);

/* if functions not using Loqui(Channel|User) are required, create a new function (has _raw suffix) for each protocol.
   For example, loqui_sender_irc_say_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *text); */

void loqui_sender_say(LoquiSender *sender, LoquiChannel *channel, const gchar *text);
void loqui_sender_notice(LoquiSender *sender, LoquiChannel *channel, const gchar *text);
void loqui_sender_nick(LoquiSender *sender, const gchar *text);
void loqui_sender_away(LoquiSender *sender, LoquiAwayType away_type, const gchar *away_message);
void loqui_sender_whois(LoquiSender *sender, LoquiUser *user);
void loqui_sender_join(LoquiSender *sender, LoquiChannel *channel);
void loqui_sender_part(LoquiSender *sender, LoquiChannel *channel, const gchar *part_message);
void loqui_sender_topic(LoquiSender *sender, LoquiChannel *channel, const gchar *topic);
void loqui_sender_start_private_talk(LoquiSender *sender, LoquiUser *user);
void loqui_sender_end_private_talk(LoquiSender *sender, LoquiChannel *channel);
void loqui_sender_refresh(LoquiSender *sender, LoquiChannel *channel);
void loqui_sender_quit(LoquiSender *sender, const gchar *quit_message);

void loqui_sender_join_raw(LoquiSender *sender, const gchar *target, const gchar *key);
void loqui_sender_start_private_talk_raw(LoquiSender *sender, const gchar *target);

G_END_DECLS

#endif /* __LOQUI_SENDER_H__ */
