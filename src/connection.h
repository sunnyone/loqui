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
#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "account.h"
#include "irc_message.h"

G_BEGIN_DECLS

#define TYPE_CONNECTION                 (connection_get_type ())
#define CONNECTION(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CONNECTION, Connection))
#define CONNECTION_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CONNECTION, ConnectionClass))
#define IS_CONNECTION(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CONNECTION))
#define IS_CONNECTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CONNECTION))
#define CONNECTION_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CONNECTION, ConnectionClass))

typedef struct _Connection            Connection;
typedef struct _ConnectionClass       ConnectionClass;

typedef struct _ConnectionPrivate     ConnectionPrivate;

struct _Connection
{
        GObject parent;
       	
        ConnectionPrivate *priv;
};

struct _ConnectionClass
{
        GObjectClass parent_class;
};


GType connection_get_type (void) G_GNUC_CONST;

Connection* connection_new(Server *server);
gchar *connection_gets(Connection *connection, GError **error);
gboolean connection_puts(Connection *connection, gchar *in_str, GError **error);

IRCMessage *connection_get_irc_message(Connection *connection, GError **error);
gboolean connection_put_irc_message(Connection *connection, IRCMessage *msg, GError **error);

void connection_disconnect(Connection *connection);

G_END_DECLS

#endif /* __CONNECTION_H__ */
