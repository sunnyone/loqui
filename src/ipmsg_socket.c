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
#include "config.h"

#include "ipmsg_socket.h"
#include "ipmsg_packet.h"
#include <gnet.h>
#include "ipmsg.h"
#include <stdlib.h>
#include "main.h"

enum {
	SIGNAL_WARN,
	SIGNAL_ARRIVE_PACKET,
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _IPMsgSocketPrivate
{
	GUdpSocket *udpsock;
	GInetAddr *inetaddr;

	guint in_watch;
	guint out_watch;
};

static GObjectClass *parent_class = NULL;

static guint signals[LAST_SIGNAL] = { 0 };

static void ipmsg_socket_class_init(IPMsgSocketClass *klass);
static void ipmsg_socket_init(IPMsgSocket *sock);
static void ipmsg_socket_finalize(GObject *object);
static void ipmsg_socket_dispose(GObject *object);

static void ipmsg_socket_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void ipmsg_socket_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static gboolean ipmsg_socket_watch_in_cb(GIOChannel *ioch, GIOCondition condition, gpointer data);
static void ipmsg_socket_warning(IPMsgSocket *sock, const gchar *str);

GType
ipmsg_socket_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(IPMsgSocketClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) ipmsg_socket_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(IPMsgSocket),
				0,              /* n_preallocs */
				(GInstanceInitFunc) ipmsg_socket_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "IPMsgSocket",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
ipmsg_socket_finalize(GObject *object)
{
	IPMsgSocket *sock;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IPMSG_SOCKET(object));

        sock = IPMSG_SOCKET(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(sock->priv);
}
static void 
ipmsg_socket_dispose(GObject *object)
{
	IPMsgSocket *sock;
	IPMsgSocketPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IPMSG_SOCKET(object));

        sock = IPMSG_SOCKET(object);
	priv = sock->priv;

	ipmsg_socket_unbind(sock);
	if (priv->inetaddr) {
		gnet_inetaddr_unref(priv->inetaddr);
		priv->inetaddr = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
ipmsg_socket_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        IPMsgSocket *sock;        

        sock = IPMSG_SOCKET(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
ipmsg_socket_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        IPMsgSocket *sock;        

        sock = IPMSG_SOCKET(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
ipmsg_socket_class_init(IPMsgSocketClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = ipmsg_socket_finalize;
        object_class->dispose = ipmsg_socket_dispose;
        object_class->get_property = ipmsg_socket_get_property;
        object_class->set_property = ipmsg_socket_set_property;

	signals[SIGNAL_WARN] = g_signal_new("warn",
				     G_OBJECT_CLASS_TYPE(object_class),
				     G_SIGNAL_RUN_FIRST,
				     G_STRUCT_OFFSET(IPMsgSocketClass, warn),
				     NULL, NULL,
				     g_cclosure_marshal_VOID__STRING,
				     G_TYPE_NONE, 1,
				     G_TYPE_STRING);
	signals[SIGNAL_ARRIVE_PACKET] = g_signal_new("arrive_packet",
						     G_OBJECT_CLASS_TYPE(object_class),
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET(IPMsgSocketClass, arrive_packet),
						     NULL, NULL,
						     g_cclosure_marshal_VOID__OBJECT,
						     G_TYPE_NONE, 1,
						     TYPE_IPMSG_PACKET);
}
static void 
ipmsg_socket_init(IPMsgSocket *sock)
{
	IPMsgSocketPrivate *priv;

	priv = g_new0(IPMsgSocketPrivate, 1);

	sock->priv = priv;
}
static gboolean
ipmsg_socket_watch_in_cb(GIOChannel *ioch, GIOCondition condition, gpointer data)
{
#define MAXBUF 65536
	gchar buf[MAXBUF+1];
	gsize len;
        IPMsgSocket *sock;
	IPMsgSocketPrivate *priv;
	IPMsgPacket *packet;
	GInetAddr *addr;
	gchar *str;

	sock = IPMSG_SOCKET(data);

	priv = sock->priv;

	len = gnet_udp_socket_receive(priv->udpsock, buf, MAXBUF, &addr);
	if (len < 0) {
		ipmsg_socket_warning(sock, "Error: receiving.");
		priv->in_watch = 0;
		return FALSE;
	} else if (len == 0) {
		ipmsg_socket_warning(sock, "No characters is arrived.");
		return TRUE;
	}

	buf[len] = '\0';

	packet = ipmsg_packet_parse(buf, len);
	if (!packet) {
		str = g_strdup_printf("Invalid packet: '%s'", buf);
		ipmsg_socket_warning(sock, str);
		g_free(str);
		return TRUE;
	}
	ipmsg_packet_set_inetaddr(packet, addr);
	gnet_inetaddr_unref(addr);
	if (show_msg_mode)
		ipmsg_packet_print(packet);
	g_signal_emit(G_OBJECT(sock), signals[SIGNAL_ARRIVE_PACKET], 0, packet);
	g_object_unref(packet);

	return TRUE;
}
static void
ipmsg_socket_warning(IPMsgSocket *sock, const gchar *str)
{
	g_signal_emit(G_OBJECT(sock), signals[SIGNAL_WARN], 0, str);
}
IPMsgSocket*
ipmsg_socket_new(void)
{
        IPMsgSocket *sock;
	IPMsgSocketPrivate *priv;

	sock = g_object_new(ipmsg_socket_get_type(), NULL);
	
        priv = sock->priv;
	
        return sock;
}
gboolean
ipmsg_socket_bind(IPMsgSocket *sock)
{
	IPMsgSocketPrivate *priv;
	GIOChannel *ioch;

	priv = sock->priv;

	priv->udpsock = gnet_udp_socket_new_with_port(IPMSG_DEFAULT_PORT);

	if (!priv->udpsock)
		return FALSE;

	ioch = gnet_udp_socket_get_io_channel(priv->udpsock);
	priv->in_watch = g_io_add_watch(ioch, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					(GIOFunc) ipmsg_socket_watch_in_cb, sock);

	priv->inetaddr = gnet_inetaddr_new_nonblock("255.255.255.255", IPMSG_DEFAULT_PORT);

	return TRUE;
}
void
ipmsg_socket_unbind(IPMsgSocket *sock)
{
	IPMsgSocketPrivate *priv;

	priv = sock->priv;

	if (!priv->udpsock)
		return;

	if (priv->in_watch) {
		g_source_remove(priv->in_watch);
		priv->in_watch = 0;
	}

	gnet_udp_socket_delete(priv->udpsock);
	priv->udpsock = NULL;
}
void
ipmsg_socket_send_packet(IPMsgSocket *sock, IPMsgPacket *packet)
{
	IPMsgSocketPrivate *priv;
	gchar *buf;
	gint len;

	priv = sock->priv;

	g_return_if_fail(priv->inetaddr);
	if (!priv->udpsock)
		return;

	if ((buf = ipmsg_packet_to_string(packet, &len)) == NULL) {
		g_warning("Failed to send a packet.");
		return;
	}

	gnet_udp_socket_send(priv->udpsock, buf, len, priv->inetaddr);

	if (show_msg_mode) {
		g_print("Sent:");
		ipmsg_packet_print(packet);
	}

	g_free(buf);
}
