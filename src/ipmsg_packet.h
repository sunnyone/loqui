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
#ifndef __IPMSG_PACKET_H__
#define __IPMSG_PACKET_H__

#include <glib-object.h>
#include <gnet.h>

G_BEGIN_DECLS

#define TYPE_IPMSG_PACKET                 (ipmsg_packet_get_type ())
#define IPMSG_PACKET(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_IPMSG_PACKET, IPMsgPacket))
#define IPMSG_PACKET_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_IPMSG_PACKET, IPMsgPacketClass))
#define IS_IPMSG_PACKET(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_IPMSG_PACKET))
#define IS_IPMSG_PACKET_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_IPMSG_PACKET))
#define IPMSG_PACKET_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IPMSG_PACKET, IPMsgPacketClass))

typedef struct _IPMsgPacket            IPMsgPacket;
typedef struct _IPMsgPacketClass       IPMsgPacketClass;

typedef struct _IPMsgPacketPrivate     IPMsgPacketPrivate;

struct _IPMsgPacket
{
        GObject parent;
        
	gint version;
	gint packet_num;
	gchar *username;
	gchar *hostname;
	gint command_num;
	gchar *extra;

	gchar *group_name;

	GInetAddr *inetaddr;

        IPMsgPacketPrivate *priv;
};

struct _IPMsgPacketClass
{
        GObjectClass parent_class;
};


GType ipmsg_packet_get_type(void) G_GNUC_CONST;

IPMsgPacket* ipmsg_packet_new(void);
IPMsgPacket* ipmsg_packet_parse(const gchar *str, gint len);
gchar * ipmsg_packet_inspect(IPMsgPacket *packet);
void ipmsg_packet_print(IPMsgPacket *packet);

void ipmsg_packet_set_inetaddr(IPMsgPacket *packet, GInetAddr *inetaddr);
GInetAddr *ipmsg_packet_get_inetaddr(IPMsgPacket *packet);

G_END_DECLS

#endif /* __IPMSG_PACKET_H__ */
