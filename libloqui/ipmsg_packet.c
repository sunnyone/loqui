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
#include "config.h"

#include "ipmsg_packet.h"
#include "ipmsg.h"
#include "gobject_utils.h"
#include "utils.h"

#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _IPMsgPacketPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint ipmsg_packet_signals[LAST_SIGNAL] = { 0 }; */

static void ipmsg_packet_class_init(IPMsgPacketClass *klass);
static void ipmsg_packet_init(IPMsgPacket *packet);
static void ipmsg_packet_finalize(GObject *object);
static void ipmsg_packet_dispose(GObject *object);

static void ipmsg_packet_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void ipmsg_packet_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
ipmsg_packet_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(IPMsgPacketClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) ipmsg_packet_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(IPMsgPacket),
				0,              /* n_preallocs */
				(GInstanceInitFunc) ipmsg_packet_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "IPMsgPacket",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
ipmsg_packet_finalize(GObject *object)
{
	IPMsgPacket *packet;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IPMSG_PACKET(object));

        packet = IPMSG_PACKET(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(packet->priv);
}
static void 
ipmsg_packet_dispose(GObject *object)
{
	IPMsgPacket *packet;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IPMSG_PACKET(object));

        packet = IPMSG_PACKET(object);

	G_FREE_UNLESS_NULL(packet->username);
	G_FREE_UNLESS_NULL(packet->hostname);
	G_FREE_UNLESS_NULL(packet->extra);
	G_FREE_UNLESS_NULL(packet->group_name);
	
	if (packet->inetaddr) {
		gnet_inetaddr_unref(packet->inetaddr);
		packet->inetaddr = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
ipmsg_packet_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        IPMsgPacket *packet;        

        packet = IPMSG_PACKET(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
ipmsg_packet_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        IPMsgPacket *packet;        

        packet = IPMSG_PACKET(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
ipmsg_packet_class_init(IPMsgPacketClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = ipmsg_packet_finalize;
        object_class->dispose = ipmsg_packet_dispose;
        object_class->get_property = ipmsg_packet_get_property;
        object_class->set_property = ipmsg_packet_set_property;
}
static void 
ipmsg_packet_init(IPMsgPacket *packet)
{
	IPMsgPacketPrivate *priv;

	priv = g_new0(IPMsgPacketPrivate, 1);

	packet->priv = priv;

	packet->version = 0;
	packet->packet_num = 0;
	packet->username = NULL;
	packet->hostname = NULL;
	packet->command_num = 0;
	packet->extra = NULL;
	packet->group_name = NULL;
}
IPMsgPacket*
ipmsg_packet_new(void)
{
        IPMsgPacket *packet;
	IPMsgPacketPrivate *priv;

	packet = g_object_new(ipmsg_packet_get_type(), NULL);
	
        priv = packet->priv;

        return packet;
}
IPMsgPacket *
ipmsg_packet_parse(const gchar *str, gint len)
{
	gchar *buf;
	gchar *cur;
	gchar *array[IPMSG_COMMAND_NUMBER];
	gchar *group_name = NULL;
	guint real_len;
	int i, pos;

	if (len < 0)
		real_len = strlen(str);
	else if (len == 0)
		real_len = 0;
	else
		real_len = len;
	
	for (i = 0; i < IPMSG_COMMAND_NUMBER; i++)
		array[i] = NULL;

	buf = g_memdup(str, real_len + 1);
	pos = 0;
	array[0] = buf;
	cur = buf;
	while (cur < buf + real_len && pos < IPMSG_COMMAND_NUMBER) {
		if (*cur == ':') {
			*cur = '\0';
			array[++pos] = ++cur;
			continue;
		}

		cur++;
	}

	if (pos != IPMSG_COMMAND_POSITION_EXTRA) {
		g_free(buf);
		return NULL;
	}

	/* group extension */
	while (cur < buf + real_len) {
		if (*cur == '\0') {
			group_name = cur+1;
			break;
		}
		cur++;
	}

	return ipmsg_packet_create((gint) g_ascii_strtoull(array[IPMSG_COMMAND_POSITION_VERSION], NULL, 10),
				   (gint) g_ascii_strtoull(array[IPMSG_COMMAND_POSITION_PACKET_NUMBER], NULL, 10),
				   array[IPMSG_COMMAND_POSITION_USERNAME],
				   array[IPMSG_COMMAND_POSITION_HOSTNAME],
				   (gint) g_ascii_strtoull(array[IPMSG_COMMAND_POSITION_COMMAND_NUMBER], NULL, 10),
				   array[IPMSG_COMMAND_POSITION_EXTRA],
				   group_name);
}
IPMsgPacket *
ipmsg_packet_create(gint version, gint packet_num, const gchar *username, const gchar *hostname, gint command_num, const gchar *extra, const gchar *group_name)
{
	IPMsgPacket *packet;

	packet = ipmsg_packet_new();

	packet->version = version;
	packet->packet_num = packet_num;
	if (username)
		packet->username = g_strdup(username);
	if (hostname)
		packet->hostname = g_strdup(hostname);
	packet->command_num = command_num;
	if (extra)
		packet->extra = g_strdup(extra);
	if (group_name)
		packet->group_name = g_strdup(group_name);

	return packet;
}
gchar *
ipmsg_packet_to_string(IPMsgPacket *packet, gint *len)
{
	GString *string;

	string = g_string_new(NULL);

#define LENZERO_IF_NULL(str) (str ? str : "")
	g_string_printf(string, "%d:%d:%s:%s:%d:%s",
			packet->version,
			packet->packet_num,
			LENZERO_IF_NULL(packet->username),
			LENZERO_IF_NULL(packet->hostname),
			packet->command_num,
			LENZERO_IF_NULL(packet->extra));
#undef LENZERO_IF_NULL

	if (packet->group_name) {
		g_string_append_c(string, '\0');
		g_string_append(string, packet->group_name);
	}

	if (len != NULL)
		*len = string->len;
	return g_string_free(string, FALSE);
}
gchar *
ipmsg_packet_inspect(IPMsgPacket *packet)
{
	GInetAddr *addr;
	gchar *addr_str;
	GString *string;

	string = g_string_new(NULL);

	addr = ipmsg_packet_get_inetaddr(packet);
	if (addr) {
		addr_str = gnet_inetaddr_get_canonical_name(addr);
		g_string_append_printf(string, "From: %s:%d\n", utils_remove_ipv6_prefix_ffff(addr_str), gnet_inetaddr_get_port(addr));
		g_free(addr_str);
	}

	g_string_append_printf(string, "Version: %d, PacketNumber: %d\n", packet->version, packet->packet_num);
	g_string_append_printf(string, "Username: %s, Hostname: %s, Group: %s\n",
		packet->username, packet->hostname, packet->group_name ? packet->group_name : "(not set)");
	g_string_append_printf(string, "Command Number: 0x%x\n", packet->command_num);
	g_string_append_printf(string, "Extra: %s", packet->extra);

	return g_string_free(string, FALSE);
}
void
ipmsg_packet_print(IPMsgPacket *packet)
{
	gchar *str;
	
	g_print("-- \n");
	str = ipmsg_packet_inspect(packet);
	g_print("%s\n", str);
	g_free(str);
}

void
ipmsg_packet_set_inetaddr(IPMsgPacket *packet, GInetAddr *inetaddr)
{
        g_return_if_fail(packet != NULL);
        g_return_if_fail(IS_IPMSG_PACKET(packet));
	
	if (packet->inetaddr) {
		gnet_inetaddr_unref(inetaddr);
		packet->inetaddr = NULL;
	}

	if (inetaddr) {
		gnet_inetaddr_ref(inetaddr);
		packet->inetaddr = inetaddr;
	}
}
GInetAddr *
ipmsg_packet_get_inetaddr(IPMsgPacket *packet)
{
        g_return_val_if_fail(packet != NULL, NULL);
        g_return_val_if_fail(IS_IPMSG_PACKET(packet), NULL);
	
	return packet->inetaddr;
}
gchar *
ipmsg_packet_get_ip_addr(IPMsgPacket *packet)
{
	gchar *addr_str;
	gchar *str;
        g_return_val_if_fail(packet != NULL, NULL);
        g_return_val_if_fail(IS_IPMSG_PACKET(packet), NULL);

	if (packet->inetaddr == NULL)
		return NULL;

	addr_str = gnet_inetaddr_get_canonical_name(packet->inetaddr);
	str = g_strdup(utils_remove_ipv6_prefix_ffff(addr_str));
	g_free(addr_str);

	return str;
}
gint
ipmsg_packet_get_port(IPMsgPacket *packet)
{
        g_return_val_if_fail(packet != NULL, 0);
        g_return_val_if_fail(IS_IPMSG_PACKET(packet), 0);

	if (packet->inetaddr == NULL)
		return 0;

	return gnet_inetaddr_get_port(packet->inetaddr);
}
gchar *
ipmsg_packet_get_identifier(IPMsgPacket *packet)
{
	gchar *str;
	gchar *addr_str;
        g_return_val_if_fail(packet != NULL, NULL);
        g_return_val_if_fail(IS_IPMSG_PACKET(packet), NULL);

	if (packet->inetaddr == NULL)
		return NULL;

	addr_str = ipmsg_packet_get_ip_addr(packet);
	str = g_strdup_printf("%s:%d", addr_str, ipmsg_packet_get_port(packet));
	g_free(addr_str);

	return str;
}
