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
#ifndef __LOQUI_CHANNEL_H__
#define __LOQUI_CHANNEL_H__

#include <glib-object.h>
#include "loqui_channel_entry.h"
#include "irc_constants.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL                 (loqui_channel_get_type ())
#define LOQUI_CHANNEL(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL, LoquiChannel))
#define LOQUI_CHANNEL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNEL, LoquiChannelClass))
#define LOQUI_IS_CHANNEL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL))
#define LOQUI_IS_CHANNEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNEL))
#define LOQUI_CHANNEL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNEL, LoquiChannelClass))

typedef struct _LoquiChannel            LoquiChannel;
typedef struct _LoquiChannelClass       LoquiChannelClass;

typedef struct _LoquiChannelPrivate     LoquiChannelPrivate;

#include "account.h"

struct _LoquiChannel
{
        LoquiChannelEntry parent;

        Account *account;

	gboolean is_joined;
	gboolean is_private_talk;

	gboolean end_names;

        LoquiChannelPrivate *priv;
};

struct _LoquiChannelClass
{
        LoquiChannelEntryClass parent_class;

	void (* mode_changed)        (LoquiChannel *channel);
};

GType loqui_channel_get_type(void) G_GNUC_CONST;

LoquiChannel* loqui_channel_new(Account *account,
				const gchar *name,
				gboolean is_joined,
				gboolean is_private_talk);

void loqui_channel_set_account(LoquiChannel *channel, Account *account);
Account *loqui_channel_get_account(LoquiChannel *channel);

void loqui_channel_set_is_private_talk(LoquiChannel *channel, gboolean is_private_talk);
gboolean loqui_channel_get_is_private_talk(LoquiChannel *channel);

void loqui_channel_set_is_joined(LoquiChannel *channel, gboolean is_joined);
gboolean loqui_channel_get_is_joined(LoquiChannel *channel);

void loqui_channel_change_mode(LoquiChannel *channel, gboolean is_add, IRCModeFlag flag, gchar *argument);
void loqui_channel_clear_mode(LoquiChannel *channel);
gchar *loqui_channel_get_mode(LoquiChannel *channel);

void loqui_channel_push_user_mode_queue(LoquiChannel *channel, gboolean is_give, IRCModeFlag flag, const gchar *nick);
void loqui_channel_flush_user_mode_queue(LoquiChannel *channel);

LoquiMember *loqui_channel_add_member_by_nick(LoquiChannel *channel, 
					      const gchar *nick,
					      gboolean parse_power,
					      gboolean is_channel_operator,
					      gboolean speakable);

void loqui_channel_append_remark(LoquiChannel *channel, TextType type, gboolean is_self, const gchar *nick, const gchar *remark, gboolean is_from_server);
void loqui_channel_append_text(LoquiChannel *channel, TextType type, gchar *str);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_H__ */
