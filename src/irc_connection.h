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
#ifndef __IRC_CONNECTION_H__
#define __IRC_CONNECTION_H__

#include "irc_handle.h"
#include "irc_message.h"
#include "codeconv.h"

G_BEGIN_DECLS

#define TYPE_IRC_CONNECTION                 (irc_connection_get_type ())
#define IRC_CONNECTION(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_IRC_CONNECTION, IRCConnection))
#define IRC_CONNECTION_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_IRC_CONNECTION, IRCConnectionClass))
#define IS_IRC_CONNECTION(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_IRC_CONNECTION))
#define IS_IRC_CONNECTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_IRC_CONNECTION))
#define IRC_CONNECTION_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IRC_CONNECTION, IRCConnectionClass))

typedef struct _IRCConnection            IRCConnection;
typedef struct _IRCConnectionClass       IRCConnectionClass;

typedef struct _IRCConnectionPrivate     IRCConnectionPrivate;

struct _IRCConnection
{
        GObject parent;
        
        IRCConnectionPrivate *priv;
};

struct _IRCConnectionClass
{
        GObjectClass parent_class;

	void (* connected) (IRCHandle *handle, gboolean is_success);
	void (* disconnected) (IRCHandle *handle);
	/* when connection is terminated by force, 
	   "terminated" signal is called instead of "disconnected". */
	void (* terminated) (IRCHandle *handle);

	void (* warn) (IRCHandle *handle, gchar *str);
	void (* info) (IRCHandle *handle, gchar *str);
};


GType irc_connection_get_type (void) G_GNUC_CONST;

IRCConnection* irc_connection_new(const gchar *hostname, guint port);

void irc_connection_set_codeconv(IRCConnection *connection, CodeConv *codeconv);
void irc_connection_set_irc_handle(IRCConnection *connection, IRCHandle *handle);

void irc_connection_push_message(IRCConnection *connection, IRCMessage *msg);

void irc_connection_connect(IRCConnection *connection);
void irc_connection_disconnect(IRCConnection *connection);

G_END_DECLS

#endif /* __IRC_CONNECTION_H__ */
