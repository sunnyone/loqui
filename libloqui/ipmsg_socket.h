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
#ifndef __IPMSG_SOCKET_H__
#define __IPMSG_SOCKET_H__

#include <glib-object.h>
#include "ipmsg_packet.h"

G_BEGIN_DECLS

#define TYPE_IPMSG_SOCKET                 (ipmsg_socket_get_type ())
#define IPMSG_SOCKET(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_IPMSG_SOCKET, IPMsgSocket))
#define IPMSG_SOCKET_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_IPMSG_SOCKET, IPMsgSocketClass))
#define IS_IPMSG_SOCKET(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_IPMSG_SOCKET))
#define IS_IPMSG_SOCKET_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_IPMSG_SOCKET))
#define IPMSG_SOCKET_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IPMSG_SOCKET, IPMsgSocketClass))

typedef struct _IPMsgSocket            IPMsgSocket;
typedef struct _IPMsgSocketClass       IPMsgSocketClass;

typedef struct _IPMsgSocketPrivate     IPMsgSocketPrivate;

struct _IPMsgSocket
{
        GObject parent;
        
        IPMsgSocketPrivate *priv;
};

struct _IPMsgSocketClass
{
        GObjectClass parent_class;

	void (* warn) (IPMsgSocket *socket, gchar *str);
	void (* arrive_packet) (IPMsgSocket *socket, IPMsgPacket *packet);
};

GType ipmsg_socket_get_type(void) G_GNUC_CONST;

IPMsgSocket* ipmsg_socket_new(void);
gboolean ipmsg_socket_bind(IPMsgSocket *sock);
void ipmsg_socket_unbind(IPMsgSocket *sock);

void ipmsg_socket_send_packet(IPMsgSocket *sock, IPMsgPacket *packet);

G_END_DECLS

#endif /* __IPMSG_SOCKET_H__ */
