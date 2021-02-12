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
#ifndef __CTCP_MESSAGE_H__
#define __CTCP_MESSAGE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_CTCP_MESSAGE                 (ctcp_message_get_type ())
#define CTCP_MESSAGE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CTCP_MESSAGE, CTCPMessage))
#define CTCP_MESSAGE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CTCP_MESSAGE, CTCPMessageClass))
#define IS_CTCP_MESSAGE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CTCP_MESSAGE))
#define IS_CTCP_MESSAGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CTCP_MESSAGE))
#define CTCP_MESSAGE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CTCP_MESSAGE, CTCPMessageClass))

typedef struct _CTCPMessage            CTCPMessage;
typedef struct _CTCPMessageClass       CTCPMessageClass;

typedef struct _CTCPMessagePrivate     CTCPMessagePrivate;

struct _CTCPMessage
{
        GObject parent;

        gchar *command;
	gchar *argument;
	gchar **parameters; /* split argument with ' '; null terminated */

        CTCPMessagePrivate *priv;
};

struct _CTCPMessageClass
{
        GObjectClass parent_class;
};


GType ctcp_message_get_type (void) G_GNUC_CONST;

CTCPMessage* ctcp_message_new(const gchar *command, const gchar *argument);
gboolean ctcp_message_parse_line(const gchar *line, CTCPMessage **ctcp_msg);
gchar *ctcp_message_to_str(CTCPMessage *ctcp_msg);

gint ctcp_message_count_parameters(CTCPMessage *ctcp_msg);
G_CONST_RETURN gchar *ctcp_message_get_param(CTCPMessage *ctcp_msg, gint i); /* zero origin */

G_END_DECLS

#endif /* __CTCP_MESSAGE_H__ */
