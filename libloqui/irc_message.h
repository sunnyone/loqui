/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2002-2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __IRC_MESSAGE_H__
#define __IRC_MESSAGE_H__

#include <glib.h>
#include <glib-object.h>

#include "irc_constants.h"

G_BEGIN_DECLS

#define TYPE_IRC_MESSAGE                 (irc_message_get_type ())
#define IRC_MESSAGE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_IRC_MESSAGE, IRCMessage))
#define IRC_MESSAGE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_IRC_MESSAGE, IRCMessageClass))
#define IS_IRC_MESSAGE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_IRC_MESSAGE))
#define IS_IRC_MESSAGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_IRC_MESSAGE))
#define IRC_MESSAGE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IRC_MESSAGE, IRCMessageClass))

typedef struct _IRCMessage            IRCMessage;
typedef struct _IRCMessageClass       IRCMessageClass;

typedef struct _IRCMessagePrivate     IRCMessagePrivate;

#define IRC_MESSAGE_IS_COMMAND(msg) (msg->response > 1000)
#define IRC_MESSAGE_IS_REPLY(msg) (msg->response < 100 || (200 < msg->response && msg->response < 400))
#define IRC_MESSAGE_IS_ERROR(msg) ((400 < msg->response && msg->response < 1000))

#define IRC_MESSAGE_PARAM_INDEX_TARGET 0

struct _IRCMessage
{
        GObject parent;

	gchar *prefix;

	/* if prefix != NULL or prefix != server, these variables are set. */
	gchar *nick;
	gchar *user;
	gchar *host;

	IRCResponse response;
	gchar *command;

	gchar **parameter;

        IRCMessagePrivate *priv;
};

struct _IRCMessageClass
{
        GObjectClass parent_class;

	GHashTable *command_hash;
};


GType irc_message_get_type (void) G_GNUC_CONST;

IRCMessage* irc_message_new(const gchar *prefix, 
			    const gchar *command,
			    gchar **parameter);
gchar* irc_message_get_param(IRCMessage *msg, guint i); /* 0 <= i < IRC_MESSAGE_MAX */ 
gchar* irc_message_get_target(IRCMessage *msg);
gchar* irc_message_get_last_param(IRCMessage *msg);
gint irc_message_count_parameters(IRCMessage *msg);

IRCMessage* irc_message_parse_line(const gchar *line);
gchar* irc_message_inspect(IRCMessage *msg);
void irc_message_print(IRCMessage *msg);
IRCMessage* irc_message_create(const gchar *command, const gchar *param, ...);
IRCMessage* irc_message_createv(const gchar *command, gchar *param_array[]);
gchar* irc_message_to_string(IRCMessage *msg);
gchar* irc_message_format(IRCMessage *msg, const gchar *format);

G_END_DECLS

#endif /* __IRC_MESSAGE_H__ */
