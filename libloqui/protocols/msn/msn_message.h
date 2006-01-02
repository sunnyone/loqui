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
#ifndef __MSN_MESSAGE_H__
#define __MSN_MESSAGE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_MSN_MESSAGE                 (msn_message_get_type ())
#define MSN_MESSAGE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MSN_MESSAGE, MSNMessage))
#define MSN_MESSAGE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MSN_MESSAGE, MSNMessageClass))
#define IS_MSN_MESSAGE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MSN_MESSAGE))
#define IS_MSN_MESSAGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MSN_MESSAGE))
#define MSN_MESSAGE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MSN_MESSAGE, MSNMessageClass))

typedef struct _MSNMessage            MSNMessage;
typedef struct _MSNMessageClass       MSNMessageClass;

typedef struct _MSNMessagePrivate     MSNMessagePrivate;

struct _MSNMessage
{
        GObject parent;

	gchar *raw_text;
        GList *parameters;
	GList *payload_headers;
	gchar *payload_body;

        MSNMessagePrivate *priv;
};

struct _MSNMessageClass
{
        GObjectClass parent_class;
};


GType msn_message_get_type(void) G_GNUC_CONST;

MSNMessage* msn_message_new(const gchar *raw_text, GList *parameters, GList *payload_headers, const gchar *payload_body);
MSNMessage* msn_message_parse(const gchar *text);
MSNMessage* msn_message_create(GList *payload_headers, const gchar *payload_body, const gchar *param, ...);

G_END_DECLS

#endif /* __MSN_MESSAGE_H__ */
