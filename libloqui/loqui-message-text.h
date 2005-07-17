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
#ifndef __LOQUI_MESSAGE_TEXT_H__
#define __LOQUI_MESSAGE_TEXT_H__

#include <glib-object.h>
#include <libloqui/loqui-gobject-utils.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_MESSAGE_TEXT                 (loqui_message_text_get_type ())
#define LOQUI_MESSAGE_TEXT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_MESSAGE_TEXT, LoquiMessageText))
#define LOQUI_MESSAGE_TEXT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_MESSAGE_TEXT, LoquiMessageTextClass))
#define LOQUI_IS_MESSAGE_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_MESSAGE_TEXT))
#define LOQUI_IS_MESSAGE_TEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_MESSAGE_TEXT))
#define LOQUI_MESSAGE_TEXT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_MESSAGE_TEXT, LoquiMessageTextClass))

typedef struct _LoquiMessageText            LoquiMessageText;
typedef struct _LoquiMessageTextClass       LoquiMessageTextClass;

typedef struct _LoquiMessageTextPrivate     LoquiMessageTextPrivate;

typedef enum {
	LOQUI_TEXT_TYPE_NORMAL,
	LOQUI_TEXT_TYPE_NOTICE,
	LOQUI_TEXT_TYPE_ACTION,
	LOQUI_TEXT_TYPE_INFO,
	LOQUI_TEXT_TYPE_ERROR,
	/* private */
	LOQUI_TEXT_TYPE_TIME,
	LOQUI_TEXT_TYPE_URI,
	LOQUI_TEXT_TYPE_EMPHASIS,
	LOQUI_TEXT_TYPE_TRANSPARENT,
} LoquiTextType;

/* in bytes */
typedef struct _LoquiTextRegion {
	gsize start; /* relative offset */
	gsize len;
} LoquiTextRegion;

struct _LoquiMessageText
{
        GObject parent;
        
	LoquiTextType text_type;

	gchar *text;
	gchar *channel_name;
	gchar *account_name;
	gchar *nick;
	
	gboolean is_self;
	gboolean is_priv;
	gboolean is_remark;
	gboolean exec_notification;

	GList *highlight_region_list;
	GList *uri_region_list;

        LoquiMessageTextPrivate *priv;
};

struct _LoquiMessageTextClass
{
        GObjectClass parent_class;
};

GType loqui_message_text_get_type (void) G_GNUC_CONST;

LoquiMessageText* loqui_message_text_new(void);

LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiMessageText, loqui_message_text, text);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiMessageText, loqui_message_text, nick);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiMessageText, loqui_message_text, account_name);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiMessageText, loqui_message_text, channel_name);

LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiMessageText, loqui_message_text, is_self, gboolean);
LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiMessageText, loqui_message_text, is_priv, gboolean);
LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiMessageText, loqui_message_text, is_remark, gboolean);
LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiMessageText, loqui_message_text, exec_notification, gboolean);

void loqui_message_text_set_text_type(LoquiMessageText *msgtext, LoquiTextType type);
LoquiTextType loqui_message_text_get_text_type(LoquiMessageText *msgtext);

gchar *loqui_message_text_get_nick_string(LoquiMessageText *msgtext, gboolean with_channel_name);

G_END_DECLS

#endif /* __LOQUI_MESSAGE_TEXT_H__ */
