/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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

#include "ctcp_handle.h"
#include "ctcp_message.h"
#include "intl.h"

#include <string.h>

struct _CTCPHandlePrivate
{
	Account *account;
	IRCHandle *handle;

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

#define SUPPORTED_CTCP_REQUEST "CLIENTINFO VERSION PING USERINFO"

static CTCPHandlerElement handler_table[] = {
	{IRCCTCPVersion, ctcp_handle_version},
	{IRCCTCPPing, ctcp_handle_ping},
	{IRCCTCPClientInfo, ctcp_handle_clientinfo},
	{IRCCTCPUserInfo, ctcp_handle_userinfo},
	{IRCCTCPTime, NULL},
	{IRCCTCPFinger, NULL},
	{IRCCTCPDCC, NULL},
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
ctcp_handle_new(IRCHandle *handle, Account *account)
{
        CTCPHandle *ctcp_handle;
	CTCPHandlePrivate *priv;

	ctcp_handle = g_object_new(ctcp_handle_get_type(), NULL);

	priv = ctcp_handle->priv;

	priv->account = account;
	priv->handle = handle;

	return ctcp_handle;
}

void ctcp_handle_message(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, gboolean is_request)
{
	gint i;
	CTCPHandlePrivate *priv;
	gchar *buf, *sender, *receiver;
	
        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));
        g_return_if_fail(ctcp_msg != NULL);
        g_return_if_fail(IS_CTCP_MESSAGE(ctcp_msg));
	
	priv = ctcp_handle->priv;

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

	buf = g_strdup_printf(_("Received CTCP %s from %s to %s: %s%s%s"), 
			      is_request ? "request" : "reply",
			      sender,
			      receiver,
			      ctcp_msg->command,
			      ctcp_msg->argument ? " " : "",
			      ctcp_msg->argument ? ctcp_msg->argument : "");
	account_console_buffer_append(priv->account, TEXT_TYPE_INFO, buf);

	if(!is_request)
		return;

	g_timer_stop(priv->interval_timer);
	if(g_timer_elapsed(priv->interval_timer, NULL) < CTCP_INTERVAL) {
		account_console_buffer_append(priv->account, TEXT_TYPE_INFO, _("The CTCP request was ignored."));
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
	IRCMessage *msg;
	gchar *buf, *tmp;

        g_return_if_fail(ctcp_handle != NULL);
        g_return_if_fail(IS_CTCP_HANDLE(ctcp_handle));

	priv = ctcp_handle->priv;

	buf = ctcp_message_to_str(ctcp_msg);
	msg = irc_message_create(IRCCommandNotice, target, buf, NULL);
	g_free(buf);
	irc_handle_push_message(priv->handle, msg);
	g_object_unref(msg);

	if(ctcp_msg->argument)
		tmp = g_strdup_printf("%s %s", ctcp_msg->command, ctcp_msg->argument);
	else
		tmp = g_strdup(ctcp_msg->command);

	buf = g_strdup_printf(_("Sent CTCP reply to %s: %s"), target, tmp);
	account_console_buffer_append(priv->account, TEXT_TYPE_INFO, buf);
	g_free(tmp);
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

	buf = g_strdup_printf("Loqui version %s", VERSION);
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
	ctcp_reply = ctcp_message_new(IRCCTCPUserInfo, account_get_userinfo(priv->account));
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
