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
#ifndef __LOQUI_CHANNEL_IRC_H__
#define __LOQUI_CHANNEL_IRC_H__

#include <glib-object.h>

#include "loqui_channel.h"
#include "loqui_member.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL_IRC                 (loqui_channel_irc_get_type ())
#define LOQUI_CHANNEL_IRC(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL_IRC, LoquiChannelIRC))
#define LOQUI_CHANNEL_IRC_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNEL_IRC, LoquiChannelIRCClass))
#define LOQUI_IS_CHANNEL_IRC(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL_IRC))
#define LOQUI_IS_CHANNEL_IRC_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNEL_IRC))
#define LOQUI_CHANNEL_IRC_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNEL_IRC, LoquiChannelIRCClass))

typedef struct _LoquiChannelIRC            LoquiChannelIRC;
typedef struct _LoquiChannelIRCClass       LoquiChannelIRCClass;

typedef struct _LoquiChannelIRCPrivate     LoquiChannelIRCPrivate;

struct _LoquiChannelIRC
{
        LoquiChannel parent;
        
        LoquiChannelIRCPrivate *priv;
};

struct _LoquiChannelIRCClass
{
        LoquiChannelClass parent_class;
};


GType loqui_channel_irc_get_type(void) G_GNUC_CONST;

LoquiChannelIRC* loqui_channel_irc_new(LoquiAccount *account, const gchar *name, gboolean is_joined, gboolean is_private_talk);

LoquiMember* loqui_channel_irc_add_member_by_nick(LoquiChannelIRC *channel_irc,
						  const gchar *nick,
						  gboolean parse_power,
						  gboolean is_channel_operator,
						  gboolean speakable);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_IRC_H__ */
