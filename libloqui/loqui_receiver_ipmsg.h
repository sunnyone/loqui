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
#ifndef __LOQUI_RECEIVER_IPMSG_H__
#define __LOQUI_RECEIVER_IPMSG_H__

#include <loqui_receiver.h>
#include "loqui_account.h"
#include "ipmsg_packet.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_RECEIVER_IPMSG                 (loqui_receiver_ipmsg_get_type ())
#define LOQUI_RECEIVER_IPMSG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_RECEIVER_IPMSG, LoquiReceiverIPMsg))
#define LOQUI_RECEIVER_IPMSG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_RECEIVER_IPMSG, LoquiReceiverIPMsgClass))
#define LOQUI_IS_RECEIVER_IPMSG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_RECEIVER_IPMSG))
#define LOQUI_IS_RECEIVER_IPMSG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_RECEIVER_IPMSG))
#define LOQUI_RECEIVER_IPMSG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_RECEIVER_IPMSG, LoquiReceiverIPMsgClass))

typedef struct _LoquiReceiverIPMsg            LoquiReceiverIPMsg;
typedef struct _LoquiReceiverIPMsgClass       LoquiReceiverIPMsgClass;

typedef struct _LoquiReceiverIPMsgPrivate     LoquiReceiverIPMsgPrivate;

struct _LoquiReceiverIPMsg
{
        LoquiReceiver parent;
        
        LoquiReceiverIPMsgPrivate *priv;
};

struct _LoquiReceiverIPMsgClass
{
        LoquiReceiverClass parent_class;
};


GType loqui_receiver_ipmsg_get_type(void) G_GNUC_CONST;

LoquiReceiverIPMsg* loqui_receiver_ipmsg_new(LoquiAccount *account);
void loqui_receiver_ipmsg_handle(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet);

G_END_DECLS

#endif /* __LOQUI_RECEIVER_IPMSG_H__ */
