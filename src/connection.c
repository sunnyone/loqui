/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
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
/* most functions of this class were called in a thread.
   gdk_threads_enter() must be called before gdk calls are called.
*/

#include "config.h"

#include "connection.h"
#include "codeconv.h"
#include "utils.h"
#include "intl.h"

#include <gnet.h>
#include <string.h>

struct _ConnectionPrivate
{
	Server *server;

	GTcpSocket *sock;
	GIOChannel *io;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT
#define FAILED_CODECONV_COMMAND IRCMessageFailedCodeconv " codeconv"

static void connection_class_init(ConnectionClass *klass);
static void connection_init(Connection *connection);
static void connection_finalize(GObject *object);


/* FIXME: ERROR_CONNECTION_* are required */

GType
connection_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ConnectionClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) connection_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(Connection),
				0,              /* n_preallocs */
				(GInstanceInitFunc) connection_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "Connection",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
connection_class_init (ConnectionClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = connection_finalize;
}
static void 
connection_init (Connection *connection)
{
	ConnectionPrivate *priv;

	priv = g_new0(ConnectionPrivate, 1);

	priv->sock = NULL;
	priv->io = NULL;

	connection->priv = priv;
}
static void 
connection_finalize (GObject *object)
{
	Connection *connection;

        g_return_if_fail (object != NULL);
        g_return_if_fail (IS_CONNECTION (object));

        connection = CONNECTION(object);

	if (connection->priv->sock) {
		connection_disconnect(connection);
		connection->priv->sock = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(connection->priv);
}

Connection*
connection_new (Server *server)
{
        Connection *connection;
	ConnectionPrivate *priv;
	GIOFlags flags;

	connection = g_object_new(connection_get_type(), NULL);
	
	priv = connection->priv;

	priv->server = server;

	if((priv->sock = gnet_tcp_socket_connect(server->hostname, server->port)) == NULL) {
                debug_puts(_("Connection error"));
		return NULL;
	}
	priv->io = gnet_tcp_socket_get_iochannel(priv->sock);

	flags = g_io_channel_get_flags(connection->priv->io);
	g_io_channel_set_flags(connection->priv->io, flags | G_IO_FLAG_NONBLOCK, NULL);

	return connection;
}

gchar *connection_gets(Connection *connection, GError **error)
{
	ConnectionPrivate *priv;
	GIOStatus status;
	gchar *local;
	GString *string;
	gchar c;

        g_return_val_if_fail(connection != NULL, NULL);
        g_return_val_if_fail(IS_CONNECTION(connection), NULL);

	priv = connection->priv;
	if(priv->io == NULL)
		return NULL;

	string = g_string_sized_new(512);
	while(priv->io) {
		status = g_io_channel_read_chars(priv->io, &c, 1, NULL, error);
		if(status == G_IO_STATUS_EOF) {
			break;
		} else if(status == G_IO_STATUS_ERROR) {
			if(error && *error) {
				g_warning(_("connection_gets error: %s"), (*error)->message);
				return NULL;
			} else {
				return NULL;
			}
		} else if (status == G_IO_STATUS_AGAIN) {
			g_usleep(500);
			continue;
		}
		string = g_string_append_c(string, c);
		if(c == '\n')
			break;
	}

	if(priv->io == NULL)
		return NULL;

	local = codeconv_to_local(string->str);
	if(string->len > 0 && local == NULL)
		local = g_strdup(FAILED_CODECONV_COMMAND);
	g_string_free(string, TRUE);
	
	return local;
}

IRCMessage *connection_get_irc_message(Connection *connection, GError **error)
{
	IRCMessage *msg;
	gchar *str;

        g_return_val_if_fail(connection != NULL, NULL);
        g_return_val_if_fail(IS_CONNECTION(connection), NULL);

	str = connection_gets(connection, error);
	if(str == NULL)
		return NULL;

	msg = irc_message_parse_line(str);
	g_free(str);

	return msg;
}

gboolean connection_puts(Connection *connection, gchar *in_str, GError **error)
{
	ConnectionPrivate *priv;
	GIOStatus status;
	gchar *str;
	gchar *serv_str;
	gint len;

	priv = connection->priv;

	serv_str = codeconv_to_server(in_str);
	str = g_strdup_printf("%s\r\n", serv_str);
	g_free(serv_str);

	status = g_io_channel_write_chars(priv->io, str, -1, &len, error);
	g_free(str);

	if(status == G_IO_STATUS_ERROR)
		return FALSE;
	return TRUE;
}
gboolean connection_put_irc_message(Connection *connection, IRCMessage *msg, GError **error)
{
	gchar *str;
	gboolean success;

	if((str = irc_message_to_string(msg)) == NULL)
		return FALSE;

	success = connection_puts(connection, str, error);

	g_free(str);
	
	return success;
}
void connection_disconnect(Connection *connection)
{
        g_return_if_fail(connection != NULL);
        g_return_if_fail(IS_CONNECTION(connection));


	debug_puts("Disconnecting...");
	g_io_channel_shutdown(connection->priv->io, TRUE, NULL);
	connection->priv->io = NULL;
	gnet_tcp_socket_delete(connection->priv->sock);
	debug_puts("Done.");
}
