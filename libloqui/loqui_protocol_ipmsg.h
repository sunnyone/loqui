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
#ifndef __LOQUI_PROTOCOL_IPMSG_H__
#define __LOQUI_PROTOCOL_IPMSG_H__

#include <glib-object.h>
#include <libloqui/loqui-protocol.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_PROTOCOL_IPMSG                 (loqui_protocol_ipmsg_get_type ())
#define LOQUI_PROTOCOL_IPMSG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROTOCOL_IPMSG, LoquiProtocolIPMSG))
#define LOQUI_PROTOCOL_IPMSG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROTOCOL_IPMSG, LoquiProtocolIPMSGClass))
#define LOQUI_IS_PROTOCOL_IPMSG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROTOCOL_IPMSG))
#define LOQUI_IS_PROTOCOL_IPMSG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROTOCOL_IPMSG))
#define LOQUI_PROTOCOL_IPMSG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROTOCOL_IPMSG, LoquiProtocolIPMSGClass))

typedef struct _LoquiProtocolIPMsg            LoquiProtocolIPMsg;
typedef struct _LoquiProtocolIPMsgClass       LoquiProtocolIPMsgClass;

struct _LoquiProtocolIPMsg
{
        LoquiProtocol parent;
};

struct _LoquiProtocolIPMsgClass
{
        LoquiProtocolClass parent_class;
};


GType loqui_protocol_ipmsg_get_type(void) G_GNUC_CONST;

LoquiProtocol* loqui_protocol_ipmsg_get(void);

G_END_DECLS

#endif /* __LOQUI_PROTOCOL_IPMSG_H__ */
