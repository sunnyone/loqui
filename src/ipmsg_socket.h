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
#ifndef __IPMSG_SOCKET_H__
#define __IPMSG_SOCKET_H__

#include <glib-object.h>

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
};


GType ipmsg_socket_get_type(void) G_GNUC_CONST;

IPMsgSocket* ipmsg_socket_new(void);
gboolean ipmsg_socket_bind(IPMsgSocket *sock);

G_END_DECLS

#endif /* __IPMSG_SOCKET_H__ */
