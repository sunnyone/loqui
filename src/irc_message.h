/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2 <http://loqui.good-day.net/>
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __IRC_MESSAGE_H__
#define __IRC_MESSAGE_H__

#include <gnome.h>
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

/* typedef enum {
	IRC_MESSAGE_COMMAND,
	IRC_MESSAGE_REPLY,
	IRC_MESSAGE_ERROR,
	} IRCMessageType; */
	
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
};


GType irc_message_get_type (void) G_GNUC_CONST;

IRCMessage* irc_message_new (const gchar *prefix, 
			     const gchar *command,
			     gchar **parameter);
gchar* irc_message_get_param(IRCMessage *msg, guint i); /* 1 < i < 15 */ 
IRCMessage* irc_message_parse_line(const gchar *line);
gchar* irc_message_inspect(IRCMessage *msg);
void irc_message_print(IRCMessage *msg);
IRCMessage* irc_message_create(gchar *command, gchar *param, ...);
IRCMessage* irc_message_createv(gchar *command, gchar *param_array[]);
gchar* irc_message_to_string(IRCMessage *msg);

G_END_DECLS

#endif /* __IRC_MESSAGE_H__ */
