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

#include "irc_connection.h"
#include <gnet.h>

#include "intl.h"
#include "utils.h"
#include "main.h"

struct _IRCConnectionPrivate
{
	GTcpSocket *socket;
	GTcpSocketConnectAsyncID connect_id;
	guint in_watch;
	guint out_watch;
	
	gchar *hostname;
	guint port;

	IRCHandle *handle;
	CodeConv *codeconv;

	GQueue *msg_queue;
};

enum {
	CONNECTED,
	DISCONNECTED,
	TERMINATED,
	WARN,
	INFO,
	LAST_SIGNAL
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT
static guint signals[LAST_SIGNAL] = { 0 };

static void irc_connection_class_init(IRCConnectionClass *klass);
static void irc_connection_init(IRCConnection *irc_connection);
static void irc_connection_finalize(GObject *object);

static void irc_connection_print_warning(IRCConnection *connection, gchar *str);
static void irc_connection_print_info(IRCConnection *connection, gchar *str);

static void irc_connection_connect_cb(GTcpSocket *socket,
				      GTcpSocketConnectAsyncStatus status,
				      gpointer data);
static gboolean irc_connection_watch_in_cb(GIOChannel *ioch, 
				       GIOCondition condition, gpointer data);
static gboolean irc_connection_watch_out_cb(GIOChannel *ioch, 
					GIOCondition condition, gpointer data);
static void irc_connection_disconnect_internal(IRCConnection *connection);

GType
irc_connection_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(IRCConnectionClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) irc_connection_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(IRCConnection),
				0,              /* n_preallocs */
				(GInstanceInitFunc) irc_connection_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "IRCConnection",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
irc_connection_class_init(IRCConnectionClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = irc_connection_finalize;

	signals[CONNECTED] = g_signal_new("connected",
					  G_OBJECT_CLASS_TYPE(object_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET(IRCConnectionClass, connected),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__BOOLEAN,
					  G_TYPE_NONE, 1,
					  G_TYPE_BOOLEAN);
	signals[DISCONNECTED] = g_signal_new("disconnected",
					     G_OBJECT_CLASS_TYPE(object_class),
					     G_SIGNAL_RUN_FIRST,
					     G_STRUCT_OFFSET(IRCConnectionClass, disconnected),
					     NULL, NULL,
					     g_cclosure_marshal_VOID__VOID,
					     G_TYPE_NONE, 0);
	signals[TERMINATED] = g_signal_new("terminated",
					   G_OBJECT_CLASS_TYPE(object_class),
					   G_SIGNAL_RUN_FIRST,
					   G_STRUCT_OFFSET(IRCConnectionClass, terminated),
					   NULL, NULL,
					   g_cclosure_marshal_VOID__VOID,
					   G_TYPE_NONE, 0);
	signals[WARN] = g_signal_new("warn",
				     G_OBJECT_CLASS_TYPE(object_class),
				     G_SIGNAL_RUN_FIRST,
				     G_STRUCT_OFFSET(IRCConnectionClass, connected),
				     NULL, NULL,
				     g_cclosure_marshal_VOID__STRING,
				     G_TYPE_NONE, 1,
				     G_TYPE_BOOLEAN);
	signals[INFO] = g_signal_new("info",
				     G_OBJECT_CLASS_TYPE(object_class),
				     G_SIGNAL_RUN_FIRST,
				     G_STRUCT_OFFSET(IRCConnectionClass, connected),
				     NULL, NULL,
				     g_cclosure_marshal_VOID__STRING,
				     G_TYPE_NONE, 1,
				     G_TYPE_STRING);
}
static void 
irc_connection_init(IRCConnection *connection)
{
	IRCConnectionPrivate *priv;

	priv = g_new0(IRCConnectionPrivate, 1);

	connection->priv = priv;

	priv->msg_queue = NULL;
	priv->codeconv = NULL;
	priv->handle = NULL;

}
static void 
irc_connection_finalize(GObject *object)
{
	IRCConnection *connection;
	IRCConnectionPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(object));

        connection = IRC_CONNECTION(object);
	priv = connection->priv;

	irc_connection_disconnect_internal(connection);

	G_FREE_UNLESS_NULL(priv->hostname);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(connection->priv);
}
static void
irc_connection_connect_cb(GTcpSocket *socket,
		      GTcpSocketConnectAsyncStatus status,
		      gpointer data)
{
	IRCConnection *connection;
	IRCConnectionPrivate *priv;
	GIOChannel *ioch;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(data));

	connection = IRC_CONNECTION(data);

	priv = connection->priv;

	priv->connect_id = 0;

	if(status != GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK || socket == NULL) {
		g_signal_emit(connection, signals[CONNECTED], 0, FALSE);
		return;
	}

	priv->socket = socket;

	ioch = gnet_tcp_socket_get_io_channel(socket);
	priv->in_watch = g_io_add_watch(ioch, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					(GIOFunc) irc_connection_watch_in_cb, connection);

	g_signal_emit(connection, signals[CONNECTED], 0, TRUE);
}
static void
irc_connection_disconnect_internal(IRCConnection *connection)
{
	IRCConnectionPrivate *priv;	

        g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(connection));

	priv = connection->priv;

	if(priv->connect_id != 0) {
		gnet_tcp_socket_connect_async_cancel(priv->connect_id);
		priv->connect_id = 0;
	}
	if(priv->in_watch != 0) {
		g_source_remove(priv->in_watch);
		priv->in_watch = 0;
	}
	if(priv->out_watch != 0) {
		g_source_remove(priv->out_watch);
		priv->out_watch = 0;
	}
	if(priv->socket) {
		gnet_tcp_socket_delete(priv->socket);
		priv->socket = NULL;
	}

}

static gboolean
irc_connection_watch_out_cb(GIOChannel *ioch, GIOCondition condition, gpointer data)
{
	IRCConnection *connection;
	IRCConnectionPrivate *priv;
	IRCMessage *msg;
	GIOStatus status;
	GError *error;
	gchar *buf, *tmp, *serv_str;

        g_return_val_if_fail(data != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_CONNECTION(data), FALSE);

	connection = IRC_CONNECTION(data);

	priv = connection->priv;

	if(g_queue_is_empty(priv->msg_queue)) {
		priv->out_watch = 0;
		return FALSE;
	}

	msg = IRC_MESSAGE(g_queue_pop_head(priv->msg_queue));
	buf = irc_message_to_string(msg);
	g_object_unref(msg);

	if(priv->codeconv) {
		serv_str = codeconv_to_server(priv->codeconv, buf);
	} else {
		g_warning("Code converter is not set.");
		serv_str = g_strdup(buf);
	}
	g_free(buf);

	tmp = g_strdup_printf("%s\r\n", serv_str);
	g_free(serv_str);

	error = NULL;
	status = g_io_channel_write_chars(ioch, tmp, -1, NULL, &error);
	g_free(tmp);
	
	if(status == G_IO_STATUS_ERROR) {
		tmp = g_strdup_printf(_("IOChannel write error: %s"), error->message);
		irc_connection_print_warning(connection, tmp);
		g_free(tmp);
		g_error_free(error);
		priv->out_watch = 0;
		return FALSE;
	}

	return TRUE;
}

static gboolean
irc_connection_watch_in_cb(GIOChannel *ioch, GIOCondition condition, gpointer data)
{
	IRCConnection *connection;
	IRCConnectionPrivate *priv;
	IRCMessage *msg;
	GIOError io_error;
	gchar *buf, *local;
	gsize len;

        g_return_val_if_fail(data != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_CONNECTION(data), FALSE);

	connection = IRC_CONNECTION(data);

	priv = connection->priv;

	if(condition == G_IO_HUP || condition == G_IO_ERR) {
		priv->in_watch = 0;
		irc_connection_disconnect_internal(connection);
		g_signal_emit(connection, signals[TERMINATED], 0);
		return FALSE;
	}

	io_error = gnet_io_channel_readline_strdup(ioch, &buf, &len);

	if(io_error != G_IO_ERROR_NONE) {
		priv->in_watch = 0;
		irc_connection_disconnect_internal(connection);
		g_signal_emit(connection, signals[TERMINATED], 0);
		return FALSE;
	}
	
	if(len == 0) {
		if(buf)
			g_free(buf);
		priv->in_watch = 0;
		irc_connection_disconnect(connection);
		return FALSE;
	}

	if(priv->codeconv) {
		local = codeconv_to_local(priv->codeconv, buf);
	} else {
		g_warning("Code converter is not set.");
		local = g_strdup(buf);
	}

	g_free(buf);

	if(len > 0 && local == NULL) {
		irc_connection_print_warning(connection, _("*** Failed to convert codeset."));
		return TRUE;
	}

	msg = irc_message_parse_line(local);
	g_free(local);

	if(msg == NULL) {
		irc_connection_print_warning(connection, _("*** Failed to parse a line"));
		return TRUE;
	}

	if(priv->handle)
		irc_handle_response(priv->handle, msg);
	else
		g_warning("IRCHandle is not set.");
	
	g_object_unref(msg);

	return TRUE;
}	

IRCConnection*
irc_connection_new(const gchar *hostname, guint port)
{
        IRCConnection *connection;
	IRCConnectionPrivate *priv;

	g_return_val_if_fail(hostname != NULL, NULL);

	connection = g_object_new(irc_connection_get_type(), NULL);
	
	priv = connection->priv;

	priv->msg_queue = g_queue_new();
	priv->hostname = g_strdup(hostname);
	priv->port = port;
	
	return connection;
}

void
irc_connection_set_codeconv(IRCConnection *connection, CodeConv *codeconv)
{
	IRCConnectionPrivate *priv;

	g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(connection));
	g_return_if_fail(codeconv != NULL);
	g_return_if_fail(IS_CODECONV(codeconv));

	priv = connection->priv;

	priv->codeconv = codeconv;
}
void
irc_connection_set_irc_handle(IRCConnection *connection, IRCHandle *handle)
{
	IRCConnectionPrivate *priv;

	g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(connection));
	g_return_if_fail(handle != NULL);
	g_return_if_fail(IS_IRC_HANDLE(handle));

	priv = connection->priv;

	priv->handle = handle;	
}
void
irc_connection_connect(IRCConnection *connection)
{
	IRCConnectionPrivate *priv;

	g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(connection));

	priv = connection->priv;

	priv->connect_id = gnet_tcp_socket_connect_async(priv->hostname, priv->port,
							 irc_connection_connect_cb, connection);
}

static void
irc_connection_print_warning(IRCConnection *connection, gchar *str)
{
	g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(connection));

	g_signal_emit(connection, signals[WARN], 0, str);

	if(debug_mode)
		g_warning("CodeConv: %s", str);
}
static void
irc_connection_print_info(IRCConnection *connection, gchar *str)
{
	g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(connection));

	g_signal_emit(connection, signals[INFO], 0, str);

	if(debug_mode)
		g_print("CodeConv(Info): %s", str);
}

void
irc_connection_disconnect(IRCConnection *connection)
{
	irc_connection_disconnect_internal(connection);
	g_signal_emit(connection, signals[DISCONNECTED], 0);
}
void
irc_connection_push_message(IRCConnection *connection, IRCMessage *msg)
{
	IRCConnectionPrivate *priv;
	GIOChannel *ioch;

        g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_IRC_CONNECTION(connection));

	priv = connection->priv;

	if(!priv->msg_queue) {
		g_warning("Message queue is not created.");
		return;
	}
	if(!priv->socket) {
		g_warning("Not connected.");
		return;
	}
	g_object_ref(msg);
	g_queue_push_tail(priv->msg_queue, msg);
	
	if(priv->out_watch == 0) {
		ioch = gnet_tcp_socket_get_io_channel(priv->socket);
		priv->out_watch = g_io_add_watch(ioch, G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
						 (GIOFunc) irc_connection_watch_out_cb, connection);
	}
}
