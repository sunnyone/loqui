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
#ifndef __LOQUI_SENDER_IPMSG_H__
#define __LOQUI_SENDER_IPMSG_H__

#include <glib-object.h>
#include "loqui_sender.h"
#include "loqui_account.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_SENDER_IPMSG                 (loqui_sender_ipmsg_get_type ())
#define LOQUI_SENDER_IPMSG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_SENDER_IPMSG, LoquiSenderIPMsg))
#define LOQUI_SENDER_IPMSG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_SENDER_IPMSG, LoquiSenderIPMsgClass))
#define LOQUI_IS_SENDER_IPMSG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_SENDER_IPMSG))
#define LOQUI_IS_SENDER_IPMSG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_SENDER_IPMSG))
#define LOQUI_SENDER_IPMSG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_SENDER_IPMSG, LoquiSenderIPMsgClass))

typedef struct _LoquiSenderIPMsg            LoquiSenderIPMsg;
typedef struct _LoquiSenderIPMsgClass       LoquiSenderIPMsgClass;

typedef struct _LoquiSenderIPMsgPrivate     LoquiSenderIPMsgPrivate;

struct _LoquiSenderIPMsg
{
        LoquiSender parent;
        
        LoquiSenderIPMsgPrivate *priv;
};

struct _LoquiSenderIPMsgClass
{
        LoquiSenderClass parent_class;
};


GType loqui_sender_ipmsg_get_type(void) G_GNUC_CONST;

LoquiSenderIPMsg* loqui_sender_ipmsg_new(LoquiAccount *account);

G_END_DECLS

#endif /* __LOQUI_SENDER_IPMSG_H__ */
