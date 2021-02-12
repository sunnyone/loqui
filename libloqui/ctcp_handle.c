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
#include "config.h"

#include "ctcp_handle.h"
#include "ctcp_message.h"
#include <libloqui-intl.h>
#include "loqui_profile_account_irc.h"
#include "loqui_sender_irc.h"
#include "loqui_account_irc.h"
#include "loqui_channel.h"

#include <string.h>

#include <libloqui/loqui-core.h>
#include <libloqui/loqui-static-core.h>

#include <gio/gio.h>

struct _CTCPHandlePrivate
{
	LoquiAccount *account;
	LoquiReceiverIRC *receiver;

	GTimer *interval_timer;
};

typedef struct _CTCPHandlerElement {
	gchar *command;
	void (* func) (CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender);
} CTCPHandlerElement;

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

#define CTCP_INTERVAL 0.5

static void ctcp_handle_class_init(CTCPHandleClass *klass);
static void ctcp_handle_init(CTCPHandle *ctcp_handle);
static void ctcp_handle_finalize(GObject *object);

static void ctcp_handle_version(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender);
static void ctcp_handle_ping(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender);
static void ctcp_handle_userinfo(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender);
static void ctcp_handle_clientinfo(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender);
static void ctcp_handle_dcc(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender);
static void ctcp_handle_dcc_send(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender);

#define SUPPORTED_CTCP_REQUEST "CLIENTINFO VERSION PING USERINFO"

static CTCPHandlerElement handler_table[] = {
	{IRCCTCPVersion, ctcp_handle_version},
	{IRCCTCPPing, ctcp_handle_ping},
	{IRCCTCPClientInfo, ctcp_handle_clientinfo},
	{IRCCTCPUserInfo, ctcp_handle_userinfo},
	{IRCCTCPTime, NULL},
	{IRCCTCPFinger, NULL},
	{IRCCTCPDCC, ctcp_handle_dcc},
	{NULL, 0}
};

GType
ctcp_handle_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(CTCPHandleClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) ctcp_handle_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(CTCPHandle),
				0,              /* n_preallocs */
				(GInstanceInitFunc) ctcp_handle_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "CTCPHandle",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
ctcp_handle_class_init(CTCPHandleClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = ctcp_handle_finalize;
}
static void 
ctcp_handle_init(CTCPHandle *ctcp_handle)
{
	CTCPHandlePrivate *priv;

	priv = g_new0(CTCPHandlePrivate, 1);

	priv->interval_timer = g_timer_new();
	g_timer_start(priv->interval_timer);

	ctcp_handle->priv = priv;
}
static void 
ctcp_handle_finalize(GObject *object)
{
	CTCPHandle *ctcp_handle;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(object));

        ctcp_handle = CTCP_HANDLE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(ctcp_handle->priv);
}

CTCPHandle*
ctcp_handle_new(LoquiReceiverIRC *receiver, LoquiAccount *account)
{
        CTCPHandle *ctcp_handle;
	CTCPHandlePrivate *priv;

	ctcp_handle = g_object_new(ctcp_handle_get_type(), NULL);

	priv = ctcp_handle->priv;

	priv->account = account;
	priv->receiver = receiver;

	return ctcp_handle;
}

void ctcp_handle_message(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, gboolean is_request)
{
	gint i;
	CTCPHandlePrivate *priv;
	gchar *buf, *sender, *receiver;
	LoquiAccount *account;
	LoquiChannel *channel;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));
        g_return_if_fail(ctcp_msg != NULL);
        g_return_if_fail(IS_CTCP_MESSAGE(ctcp_msg));
	
	priv = ctcp_handle->priv;
	account = priv->account;

	if(ctcp_msg->command == NULL)
		return;

	sender = g_object_get_data(G_OBJECT(ctcp_msg), "sender");
	if(sender == NULL) {
		g_warning(_("Sender is not set in a CTCP message"));
		return;
	}

	receiver = g_object_get_data(G_OBJECT(ctcp_msg), "receiver");
	if(receiver == NULL) {
		g_warning(_("Receiver is not set in a CTCP message"));
		return;
	}

	if (strcmp(ctcp_msg->command, IRCCTCPAction) == 0) {
		channel = loqui_account_get_channel_by_identifier(account, receiver);
		if (channel) {
			loqui_channel_append_remark(channel, LOQUI_TEXT_TYPE_ACTION,
						    loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), sender),
						    sender,
						    ctcp_msg->argument ? ctcp_msg->argument : "");
			return;
		}
	}

	buf = g_strdup_printf(_("Received CTCP %1$s from %2$s to %3$s: %4$s%5$s%6$s"), 
			      is_request ? "request" : "reply",
			      sender,
			      receiver,
			      ctcp_msg->command,
			      ctcp_msg->argument ? " " : "",
			      ctcp_msg->argument ? ctcp_msg->argument : "");
	loqui_account_append_text(priv->account, NULL, LOQUI_TEXT_TYPE_INFO, buf);
	g_free(buf);

	if(!is_request)
		return;

	g_timer_stop(priv->interval_timer);
	if(g_timer_elapsed(priv->interval_timer, NULL) < CTCP_INTERVAL) {
		loqui_account_append_text(priv->account, NULL, LOQUI_TEXT_TYPE_INFO, _("The CTCP request was ignored."));
		g_timer_start(priv->interval_timer);
		return;
	}

	for(i = 0; handler_table[i].command != NULL; i++) {
		if(strcmp(ctcp_msg->command, handler_table[i].command) == 0 &&
		   handler_table[i].func != NULL) {
			handler_table[i].func(ctcp_handle, ctcp_msg, sender);
		}
	}

	g_timer_reset(priv->interval_timer);
	g_timer_start(priv->interval_timer);
}
static void
ctcp_handle_send_ctcp_reply(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *target)
{
	CTCPHandlePrivate *priv;
	gchar *buf;
	LoquiSender *sender;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));

	priv = ctcp_handle->priv;

	buf = ctcp_message_to_str(ctcp_msg);
	sender = loqui_account_get_sender(priv->account);
	loqui_sender_irc_notice_raw(LOQUI_SENDER_IRC(sender), target, buf);
	g_free(buf);
}
static void
ctcp_handle_version(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender)
{
	CTCPHandlePrivate *priv;
	CTCPMessage *ctcp_reply;
	gchar *buf;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));

	priv = ctcp_handle->priv;

	buf = loqui_core_get_version_info(loqui_get_core());
	ctcp_reply = ctcp_message_new(IRCCTCPVersion, buf);
	g_free(buf);
	ctcp_handle_send_ctcp_reply(ctcp_handle, ctcp_reply, sender);
	g_object_unref(ctcp_reply);
}
static void
ctcp_handle_ping(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender)
{
	CTCPHandlePrivate *priv;
	CTCPMessage *ctcp_reply;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));

	priv = ctcp_handle->priv;

	ctcp_reply = ctcp_message_new(IRCCTCPPing, NULL);
	ctcp_handle_send_ctcp_reply(ctcp_handle, ctcp_reply, sender);
	g_object_unref(ctcp_reply);
}
static void
ctcp_handle_userinfo(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender)
{
	CTCPHandlePrivate *priv;
	CTCPMessage *ctcp_reply;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));

	priv = ctcp_handle->priv;

	/* FIXME: should quote string with ctcp */
	ctcp_reply = ctcp_message_new(IRCCTCPUserInfo,
		loqui_profile_account_irc_get_userinfo(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(priv->account))));
	ctcp_handle_send_ctcp_reply(ctcp_handle, ctcp_reply, sender);
	g_object_unref(ctcp_reply);
}
static void
ctcp_handle_clientinfo(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender)
{
	CTCPHandlePrivate *priv;
	CTCPMessage *ctcp_reply;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));

	priv = ctcp_handle->priv;

	ctcp_reply = ctcp_message_new(IRCCTCPClientInfo, SUPPORTED_CTCP_REQUEST);
	ctcp_handle_send_ctcp_reply(ctcp_handle, ctcp_reply, sender);
	g_object_unref(ctcp_reply);
}

static void
ctcp_handle_dcc(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender)
{
	CTCPHandlePrivate *priv;
	const gchar *subcommand;
	gchar *str;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));
	
	priv = ctcp_handle->priv;
	
	subcommand = ctcp_message_get_param(ctcp_msg, 0);
	if (subcommand == NULL) {
		loqui_account_warning(priv->account, _("This DCC request doesn't have subcommand: %s"), subcommand);
		return;
	}

	if (strcmp(subcommand, IRCDCCSend) == 0) {
		ctcp_handle_dcc_send(ctcp_handle, ctcp_msg, sender);
		return;
	}

	str = ctcp_message_to_str(ctcp_msg);
	loqui_account_warning(priv->account, _("This DCC request has an unsupported subcommand: %s (%s)"), subcommand, str);
	g_free(str);

}

static void
ctcp_handle_dcc_send(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, const gchar *sender)
{
	CTCPHandlePrivate *priv;
	/* LoquiTransferItem *trans_item; */
	const gchar *filename, *address, *port_str, *size_str;
	gchar *str;
	gchar *canon_addr;
	gint port, size;

	GInetAddress *addr;
	guint64 d;
	guint32 network_order;
	gchar *endptr;

	priv = ctcp_handle->priv;

	if (ctcp_message_count_parameters(ctcp_msg) != 5) {
		str = ctcp_message_to_str(ctcp_msg);
		loqui_account_warning(priv->account, _("Invalid DCC request (invalid parameter length: %d (%s))"),
				      ctcp_message_count_parameters(ctcp_msg), str);
		g_free(str);
		return;
	}

	filename = ctcp_message_get_param(ctcp_msg, 1);
	address = ctcp_message_get_param(ctcp_msg, 2);
	port_str = ctcp_message_get_param(ctcp_msg, 3);
	size_str  = ctcp_message_get_param(ctcp_msg, 4);
	
	d = g_ascii_strtoull(port_str, &endptr, 10);
	if (d == 0 || d > G_MAXINT || endptr != (port_str + strlen(port_str))) {
		loqui_account_warning(priv->account, "The port in the DCC request is invalid to be a number: %s\n", port_str);
		return;
	}
	port = (gint32) d;
	
	d = g_ascii_strtoull(size_str, &endptr, 10);
	if (d == 0 || d > G_MAXUINT || endptr != (size_str + strlen(size_str))) {
		loqui_account_warning(priv->account, "The size in the DCC request is invalid to be a number: %s\n", size_str);
		return;
	}
	size = (gint32) d;

	/* FIXME: IPv6 is not supported */
	d = g_ascii_strtoull(address, &endptr, 10);
	if (d == 0 || d > G_MAXUINT || endptr != (address + strlen(address))) {
		loqui_account_warning(priv->account, "The address in the DCC request is invalid to be a number: %s\n", address);
		return;
	}

	network_order = g_htonl((guint32) d);
	addr = g_inet_address_new_from_bytes((guint8 *) &network_order, G_SOCKET_FAMILY_IPV4);
	if (addr == NULL) {
		loqui_account_warning(priv->account, "Address in the DCC request is invalid: %s\n", address);
		return;
	}

	canon_addr = g_inet_address_to_string(addr);
	
	loqui_account_information(priv->account,
				  _("Received DCC SEND request from %s (filename: %s, host: %s, port: %s, size: %s)"),
				  sender, filename, canon_addr, port_str, size_str);

	/* FIXME: create transfer item and register it to the manager */
	/*
	trans_item = LOQUI_TRANSFER_ITEM(loqui_transfer_item_irc_new());
	loqui_transfer_item_set_is_upload(trans_item, FALSE);
	loqui_transfer_item_set_filename(trans_item, filename);
	loqui_transfer_item_set_address(trans_item, canon_addr);
	loqui_transfer_item_set_port(trans_item, port);
	loqui_transfer_item_set_size(trans_item, size);
	loqui_transfer_item_irc_set_inet_addr(LOQUI_TRANSFER_ITEM_IRC(trans_item), addr);
	*/

	/* FIXME: quick hack to receive DCC SEND */
	{
		const gchar *command;
		gchar *buf;

		command = g_getenv("LOQUI_DCC_COMMAND");
		if (command) {
			buf = g_strdup_printf("%s %s %s %d %d %s %s", command, filename, address, port, size, canon_addr, sender);
			if (!g_spawn_command_line_async(buf, NULL))
				g_warning("Failed to execute DCC command");
			g_free(buf);
		}
	}

	g_free(canon_addr);
	g_object_unref(addr);

	/* g_object_unref(trans_item); */
}
