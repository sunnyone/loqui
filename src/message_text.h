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
#ifndef __MESSAGE_TEXT_H__
#define __MESSAGE_TEXT_H__

#include <glib-object.h>
#include "gobject_utils.h"

G_BEGIN_DECLS

#define TYPE_MESSAGE_TEXT                 (message_text_get_type ())
#define MESSAGE_TEXT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MESSAGE_TEXT, MessageText))
#define MESSAGE_TEXT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MESSAGE_TEXT, MessageTextClass))
#define IS_MESSAGE_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MESSAGE_TEXT))
#define IS_MESSAGE_TEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MESSAGE_TEXT))
#define MESSAGE_TEXT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MESSAGE_TEXT, MessageTextClass))

typedef struct _MessageText            MessageText;
typedef struct _MessageTextClass       MessageTextClass;

typedef struct _MessageTextPrivate     MessageTextPrivate;

typedef enum {
	TEXT_TYPE_NORMAL,
	TEXT_TYPE_NOTICE,
	TEXT_TYPE_INFO,
	TEXT_TYPE_ERROR,
	/* private */
	TEXT_TYPE_TIME,
	TEXT_TYPE_URI,
	TEXT_TYPE_EMPHASIS,
} TextType;

struct _MessageText
{
        GObject parent;
        
	TextType text_type;

	gchar *text;
	gchar *channel_name;
	gchar *account_name;
	gchar *nick;
	
	gboolean is_self;
	gboolean is_priv;
	gboolean is_remark;

	gboolean exec_notification;

        MessageTextPrivate *priv;
};

struct _MessageTextClass
{
        GObjectClass parent_class;
};

GType message_text_get_type (void) G_GNUC_CONST;

ATTR_ACCESSOR_STRING_PROTOTYPE(MessageText, message_text, text);
ATTR_ACCESSOR_STRING_PROTOTYPE(MessageText, message_text, nick);
ATTR_ACCESSOR_STRING_PROTOTYPE(MessageText, message_text, account_name);
ATTR_ACCESSOR_STRING_PROTOTYPE(MessageText, message_text, channel_name);
ATTR_ACCESSOR_BOOLEAN_PROTOTYPE(MessageText, message_text, is_self);
ATTR_ACCESSOR_BOOLEAN_PROTOTYPE(MessageText, message_text, is_priv);
ATTR_ACCESSOR_BOOLEAN_PROTOTYPE(MessageText, message_text, is_remark);
ATTR_ACCESSOR_BOOLEAN_PROTOTYPE(MessageText, message_text, exec_notification);

void message_text_set_text_type(MessageText *msgtext, TextType type);
TextType message_text_get_text_type(MessageText *msgtext);

MessageText* message_text_new(void);

gchar *message_text_get_nick_string(MessageText *msgtext, gboolean with_channel_name);

G_END_DECLS

#endif /* __MESSAGE_TEXT_H__ */
