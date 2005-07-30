/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2002-2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_CHANNEL_H__
#define __LOQUI_CHANNEL_H__

#include <glib-object.h>
#include "loqui_channel_entry.h"
#include "irc_constants.h"
#include <loqui-mode-manager.h>

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

#include "loqui_account.h"

struct _LoquiChannel
{
        LoquiChannelEntry parent;

        LoquiAccount *account;

	gboolean is_joined;
	gboolean is_private_talk;

	gchar *identifier;

	gboolean end_names;

	LoquiModeManager *channel_mode_manager;

        LoquiChannelPrivate *priv;
};

struct _LoquiChannelClass
{
        LoquiChannelEntryClass parent_class;
};

GType loqui_channel_get_type(void) G_GNUC_CONST;

LoquiChannel* loqui_channel_new(LoquiAccount *account,
				const gchar *name,
				const gchar *identifier,
				gboolean is_joined,
				gboolean is_private_talk);

void loqui_channel_set_account(LoquiChannel *channel, LoquiAccount *account);
LoquiAccount *loqui_channel_get_account(LoquiChannel *channel);

void loqui_channel_set_is_private_talk(LoquiChannel *channel, gboolean is_private_talk);
gboolean loqui_channel_get_is_private_talk(LoquiChannel *channel);

void loqui_channel_set_is_joined(LoquiChannel *channel, gboolean is_joined);
gboolean loqui_channel_get_is_joined(LoquiChannel *channel);

LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiChannel, loqui_channel, identifier);

void loqui_channel_push_user_mode_queue(LoquiChannel *channel, gboolean is_give, IRCModeFlag flag, const gchar *nick);
void loqui_channel_flush_user_mode_queue(LoquiChannel *channel);

void loqui_channel_append_remark(LoquiChannel *channel, LoquiTextType type, gboolean is_self, const gchar *nick, const gchar *remark, gboolean is_from_server, gboolean to_set_updated);
void loqui_channel_append_text(LoquiChannel *channel, LoquiTextType type, gchar *str);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_H__ */
