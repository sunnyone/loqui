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
#ifndef __LOQUI_ACCOUNT_IRC_H__
#define __LOQUI_ACCOUNT_IRC_H__

#include <glib-object.h>

#include "loqui_account.h"
#include "loqui_user_irc.h"
#include "irc_message.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_ACCOUNT_IRC                 (loqui_account_irc_get_type ())
#define LOQUI_ACCOUNT_IRC(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_ACCOUNT_IRC, LoquiAccountIRC))
#define LOQUI_ACCOUNT_IRC_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_ACCOUNT_IRC, LoquiAccountIRCClass))
#define LOQUI_IS_ACCOUNT_IRC(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_ACCOUNT_IRC))
#define LOQUI_IS_ACCOUNT_IRC_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_ACCOUNT_IRC))
#define LOQUI_ACCOUNT_IRC_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_ACCOUNT_IRC, LoquiAccountIRCClass))

typedef struct _LoquiAccountIRC            LoquiAccountIRC;
typedef struct _LoquiAccountIRCClass       LoquiAccountIRCClass;

typedef struct _LoquiAccountIRCPrivate     LoquiAccountIRCPrivate;

struct _LoquiAccountIRC
{
        LoquiAccount parent;
        
        LoquiAccountIRCPrivate *priv;
};

struct _LoquiAccountIRCClass
{
        LoquiAccountClass parent_class;
};


GType loqui_account_irc_get_type(void) G_GNUC_CONST;

LoquiAccountIRC* loqui_account_irc_new(LoquiProfileAccount *profile);
LoquiUserIRC* loqui_account_irc_fetch_user(LoquiAccountIRC *account, const gchar *nick);

gboolean loqui_account_irc_is_current_nick(LoquiAccountIRC *account, const gchar *str);
void loqui_account_irc_push_message(LoquiAccountIRC *account, IRCMessage *msg);

G_END_DECLS

#endif /* __LOQUI_ACCOUNT_IRC_H__ */
