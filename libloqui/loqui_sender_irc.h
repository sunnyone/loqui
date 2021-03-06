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
#ifndef __LOQUI_SENDER_IRC_H__
#define __LOQUI_SENDER_IRC_H__

#include <glib-object.h>
#include "loqui_sender.h"
#include <libloqui/irc_message.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_SENDER_IRC                 (loqui_sender_irc_get_type ())
#define LOQUI_SENDER_IRC(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_SENDER_IRC, LoquiSenderIRC))
#define LOQUI_SENDER_IRC_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_SENDER_IRC, LoquiSenderIRCClass))
#define LOQUI_IS_SENDER_IRC(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_SENDER_IRC))
#define LOQUI_IS_SENDER_IRC_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_SENDER_IRC))
#define LOQUI_SENDER_IRC_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_SENDER_IRC, LoquiSenderIRCClass))

typedef struct _LoquiSenderIRC            LoquiSenderIRC;
typedef struct _LoquiSenderIRCClass       LoquiSenderIRCClass;

typedef struct _LoquiSenderIRCPrivate     LoquiSenderIRCPrivate;

struct _LoquiSenderIRC
{
        LoquiSender parent;
        
	gboolean sent_quit;

        LoquiSenderIRCPrivate *priv;
};

struct _LoquiSenderIRCClass
{
        LoquiSenderClass parent_class;
};


GType loqui_sender_irc_get_type(void) G_GNUC_CONST;

LoquiSenderIRC* loqui_sender_irc_new(LoquiAccount *account);
void loqui_sender_irc_reset(LoquiSenderIRC *sender);

/* (say|notice)_raw not echos */
void loqui_sender_irc_say_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *text);
void loqui_sender_irc_notice_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *text);

void loqui_sender_irc_pong_raw(LoquiSenderIRC *sender, const gchar *target);
void loqui_sender_irc_ctcp_request_raw(LoquiSenderIRC *sender, const gchar *target, const gchar *command);
void loqui_sender_irc_get_channel_mode(LoquiSender *sender, LoquiChannel *channel);

void loqui_sender_irc_send_raw(LoquiSenderIRC *sender, const gchar *str);
void loqui_sender_irc_change_member_mode(LoquiSenderIRC *sender, LoquiChannel *channel,
					 gboolean is_give, IRCModeFlag flag, GList *str_list);
void loqui_sender_irc_user_raw(LoquiSenderIRC *sender, const gchar *username, const gchar *realname);
void loqui_sender_irc_pass(LoquiSenderIRC *sender, const gchar *password);
void loqui_sender_irc_ping_raw(LoquiSenderIRC *sender, const gchar *target);


void loqui_sender_irc_message_sent(LoquiSenderIRC *sender, IRCMessage *msg);

G_END_DECLS

#endif /* __LOQUI_SENDER_IRC_H__ */
