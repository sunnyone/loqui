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

#include "loqui_receiver_ipmsg.h"
#include "intl.h"
#include "loqui_user_ipmsg.h"
#include "ipmsg.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiReceiverIPMsgPrivate
{
};

static LoquiReceiverClass *parent_class = NULL;

/* static guint loqui_receiver_ipmsg_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_receiver_ipmsg_class_init(LoquiReceiverIPMsgClass *klass);
static void loqui_receiver_ipmsg_init(LoquiReceiverIPMsg *receiver);
static void loqui_receiver_ipmsg_finalize(GObject *object);
static void loqui_receiver_ipmsg_dispose(GObject *object);

static void loqui_receiver_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_receiver_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_receiver_ipmsg_command_br_entry(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet);
static void loqui_receiver_ipmsg_command_br_exit(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet);
static void loqui_receiver_ipmsg_command_sendmsg(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet);

GType
loqui_receiver_ipmsg_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiReceiverIPMsgClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_receiver_ipmsg_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiReceiverIPMsg),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_receiver_ipmsg_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_RECEIVER,
					      "LoquiReceiverIPMsg",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_receiver_ipmsg_finalize(GObject *object)
{
	LoquiReceiverIPMsg *receiver;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IPMSG(object));

        receiver = LOQUI_RECEIVER_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(receiver->priv);
}
static void 
loqui_receiver_ipmsg_dispose(GObject *object)
{
	LoquiReceiverIPMsg *receiver;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IPMSG(object));

        receiver = LOQUI_RECEIVER_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_receiver_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiReceiverIPMsg *receiver;        

        receiver = LOQUI_RECEIVER_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_receiver_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiReceiverIPMsg *receiver;        

        receiver = LOQUI_RECEIVER_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_receiver_ipmsg_class_init(LoquiReceiverIPMsgClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_receiver_ipmsg_finalize;
        object_class->dispose = loqui_receiver_ipmsg_dispose;
        object_class->get_property = loqui_receiver_ipmsg_get_property;
        object_class->set_property = loqui_receiver_ipmsg_set_property;
}
static void 
loqui_receiver_ipmsg_init(LoquiReceiverIPMsg *receiver)
{
	LoquiReceiverIPMsgPrivate *priv;

	priv = g_new0(LoquiReceiverIPMsgPrivate, 1);

	receiver->priv = priv;
}
#define VALIDATE_PACKET_AND_RETURN_IF_FAIL(identifier_p, account, packet) { \
	*identifier_p = ipmsg_packet_get_identifier(packet); \
	if (!identifier) { \
		loqui_account_warning(account, _("Invalid IP address or port")); \
		return; \
	} \
	if (!packet->username) { \
		loqui_account_warning(account, _("Username is not set")); \
                g_free(*identifier_p); \
		return; \
	} \
	if (!packet->hostname) { \
		loqui_account_warning(account, _("Hostname is not set")); \
                g_free(*identifier_p); \
		return; \
	} \
}
static void
loqui_receiver_ipmsg_command_br_entry(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet)
{
	LoquiAccount *account;
	LoquiUser *user;
	LoquiMember *member;
	gchar *identifier;
	gchar *str;
	gchar *ip_addr;

	account = LOQUI_RECEIVER(receiver)->account;

	VALIDATE_PACKET_AND_RETURN_IF_FAIL(&identifier, account, packet);

	if ((user = loqui_account_peek_user(account, identifier)) == NULL) {
		user = LOQUI_USER(loqui_user_ipmsg_new());

		ip_addr = ipmsg_packet_get_ip_addr(packet);
		loqui_user_ipmsg_set_ip_addr(LOQUI_USER_IPMSG(user), ip_addr);
		g_free(ip_addr);
		loqui_user_ipmsg_set_port(LOQUI_USER_IPMSG(user), ipmsg_packet_get_port(packet));

		loqui_account_add_user(account, user);
	} else {
		g_object_ref(user);
	}

	loqui_user_set_nick(user, packet->username);
	loqui_user_set_hostname(user, packet->hostname);
	loqui_user_ipmsg_set_group_name(LOQUI_USER_IPMSG(user), packet->group_name);

	if (!loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(account), user)) {
		member = loqui_member_new(user);
		loqui_channel_entry_add_member(LOQUI_CHANNEL_ENTRY(account), member);
		g_object_unref(member);
		
		str = g_strdup_printf("*** Appeared %s@%s (%s)",
				      packet->username, packet->hostname, packet->group_name ? packet->group_name : "");
		loqui_account_append_text(account, NULL, LOQUI_TEXT_TYPE_INFO, str);
		g_free(str);
	}

	g_object_unref(user);
}
static void
loqui_receiver_ipmsg_command_br_exit(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet)
{
	LoquiAccount *account;
	LoquiUser *user;
	gchar *identifier;
	gchar *str;

	account = LOQUI_RECEIVER(receiver)->account;

	VALIDATE_PACKET_AND_RETURN_IF_FAIL(&identifier, account, packet);

	if ((user = loqui_account_peek_user(account, identifier)) == NULL) {
		/* two BR_EXIT packet come from original IPMsg, so just ignore the packet */
		/* loqui_account_warning(account, _("The user '%s' exit, but he/she is not registered."), identifier); */
		return;
	}

	str = g_strdup_printf("*** Disappeared %s@%s (%s)",
			      packet->username, packet->hostname, packet->group_name ? packet->group_name : "");
	loqui_account_append_text(account, NULL, LOQUI_TEXT_TYPE_INFO, str);
	g_free(str);
	
	/* TODO: loqui_account_remove_user(user); */
	loqui_channel_entry_remove_member_by_user(LOQUI_CHANNEL_ENTRY(account), user);
}
static void
loqui_receiver_ipmsg_command_sendmsg(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet)
{
}
LoquiReceiverIPMsg*
loqui_receiver_ipmsg_new(LoquiAccount *account)
{
        LoquiReceiverIPMsg *receiver;
	LoquiReceiverIPMsgPrivate *priv;

	receiver = g_object_new(loqui_receiver_ipmsg_get_type(), NULL);
	
        priv = receiver->priv;
	LOQUI_RECEIVER(receiver)->account = account;

        return receiver;
}
void
loqui_receiver_ipmsg_handle(LoquiReceiverIPMsg *receiver, IPMsgPacket *packet)
{
	gchar *str;
	LoquiAccount *account;

	g_return_if_fail(receiver != NULL);
	g_return_if_fail(LOQUI_IS_RECEIVER_IPMSG(receiver));
	g_return_if_fail(packet != NULL);
	g_return_if_fail(IS_IPMSG_PACKET(packet));

	account = LOQUI_RECEIVER(receiver)->account;

	if (packet->version != 1) {
		loqui_account_warning(account, _("Only supports IPMessenger Protocol Version 1 (the message is version '%d')"), packet->version);
		return;
	}

	switch (IPMSG_GET_MODE(packet->command_num)) {
	case IPMSG_NOOPERATION:
		return;
	case IPMSG_BR_ENTRY:
		loqui_receiver_ipmsg_command_br_entry(receiver, packet);
		return;
	case IPMSG_BR_EXIT:
		loqui_receiver_ipmsg_command_br_exit(receiver, packet);
		return;
	case IPMSG_SENDMSG:
//		loqui_receiver_ipmsg_command_sendmsg(receiver, packet);
//		return;
	default:
		break;
	}

	str = ipmsg_packet_inspect(packet);
	loqui_account_append_text(LOQUI_ACCOUNT(account), NULL, LOQUI_TEXT_TYPE_NORMAL, str);
	g_free(str);
}
