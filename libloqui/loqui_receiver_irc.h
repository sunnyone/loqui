/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2002-2004 Yoichi Imai <sunnyone41@gmail.com>
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
#ifndef __LOQUI_RECEIVER_IRC_H__
#define __LOQUI_RECEIVER_IRC_H__

#include "irc_message.h"
#include "loqui_receiver.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_RECEIVER_IRC                 (loqui_receiver_irc_get_type ())
#define LOQUI_RECEIVER_IRC(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_RECEIVER_IRC, LoquiReceiverIRC))
#define LOQUI_RECEIVER_IRC_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_RECEIVER_IRC, LoquiReceiverIRCClass))
#define LOQUI_IS_RECEIVER_IRC(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_RECEIVER_IRC))
#define LOQUI_IS_RECEIVER_IRC_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_RECEIVER_IRC))
#define LOQUI_RECEIVER_IRC_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_RECEIVER_IRC, LoquiReceiverIRCClass))

typedef struct _LoquiReceiverIRC            LoquiReceiverIRC;
typedef struct _LoquiReceiverIRCClass       LoquiReceiverIRCClass;

typedef struct _LoquiReceiverIRCPrivate     LoquiReceiverIRCPrivate;

#include "loqui_account.h"

struct _LoquiReceiverIRC
{
        LoquiReceiver parent;
        
	guint prevent_print_who_reply_count;
	gboolean passed_welcome;

        LoquiReceiverIRCPrivate *priv;
};

struct _LoquiReceiverIRCClass
{
        LoquiReceiverClass parent_class;
};

GType loqui_receiver_irc_get_type(void) G_GNUC_CONST;

LoquiReceiverIRC* loqui_receiver_irc_new(LoquiAccount *account);
void loqui_receiver_irc_response(LoquiReceiverIRC *receiver, IRCMessage *msg);

void loqui_receiver_irc_reset(LoquiReceiverIRC *receiver);

G_END_DECLS

#endif /* __LOQUI_RECEIVER_IRC_H__ */
