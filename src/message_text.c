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

#include "message_text.h"
#include "intl.h"

enum {
	PROP_0,
	PROP_TEXT,
	PROP_CHANNEL_NAME,
	PROP_ACCOUNT_NAME,
	PROP_NICK,
	PROP_SELF,
	PROP_PRIV,
	PROP_REMARK,
	PROP_TEXT_TYPE,
	LAST_PROP
};

struct _MessageTextPrivate
{
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void message_text_class_init(MessageTextClass *klass);
static void message_text_init(MessageText *message_text);
static void message_text_finalize(GObject *object);

static void message_text_get_property(GObject  *object,
				      guint param_id,
				      GValue *value,
				      GParamSpec *pspec);
static void message_text_set_property(GObject *object,
				      guint param_id,
				      const GValue *value,
				      GParamSpec *pspec);
GType
message_text_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(MessageTextClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) message_text_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(MessageText),
				0,              /* n_preallocs */
				(GInstanceInitFunc) message_text_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "MessageText",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
message_text_class_init(MessageTextClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
	object_class->set_property = message_text_set_property;
	object_class->get_property = message_text_get_property;	
        object_class->finalize = message_text_finalize;

	g_object_class_install_property(object_class,
					PROP_TEXT,
					g_param_spec_string("text",
							    _("Text"),
							    _("Text"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_CHANNEL_NAME,
					g_param_spec_string("channel_name",
							    _("Channel name"),
							    _("Channel name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_ACCOUNT_NAME,
					g_param_spec_string("account_name",
							    _("Account name"),
							    _("Account name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_NICK,
					g_param_spec_string("nick",
							    _("Nick"),
							    _("Nickname"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_SELF,
					g_param_spec_boolean("is_self",
							    _("Self?"),
							    _("Whether or not the message is by myself"),
							    FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_PRIV,
					g_param_spec_boolean("is_priv",
							    _("Priv?"),
							    _("Whether or not private message"),
							    FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_REMARK,
					g_param_spec_boolean("is_remark",
							    _("Remark?"),
							    _("Remark?"),
							    FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_TEXT_TYPE,
					g_param_spec_uint("text_type", /* should use g_param_spec_enum */
							  _("Text type"),
							  _("Text type"),
							  0, INT_MAX,
							  0, G_PARAM_READWRITE));
}
static void 
message_text_init(MessageText *message_text)
{
	MessageTextPrivate *priv;

	priv = g_new0(MessageTextPrivate, 1);

	message_text->priv = priv;
}
static void 
message_text_finalize(GObject *object)
{
	MessageText *msgtext;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_MESSAGE_TEXT(object));

        msgtext = MESSAGE_TEXT(object);

	G_FREE_UNLESS_NULL(msgtext->text);
	G_FREE_UNLESS_NULL(msgtext->channel_name);
	G_FREE_UNLESS_NULL(msgtext->nick);
	G_FREE_UNLESS_NULL(msgtext->account_name);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(msgtext->priv);
}
static void
message_text_set_property(GObject *object,
			  guint param_id,
			  const GValue *value,
			  GParamSpec *pspec)
{
	MessageText *msgtext;

	msgtext = MESSAGE_TEXT(object);

	switch(param_id) {
	case PROP_TEXT:
		message_text_set_text(msgtext, g_value_get_string(value));
		break;
	case PROP_NICK:
		message_text_set_nick(msgtext, g_value_get_string(value));
		break;
	case PROP_CHANNEL_NAME:
		message_text_set_channel_name(msgtext, g_value_get_string(value));
		break;
	case PROP_ACCOUNT_NAME:
		message_text_set_account_name(msgtext, g_value_get_string(value));
		break;
	case PROP_SELF:
		msgtext->is_self = g_value_get_boolean(value);
		break;
	case PROP_PRIV:
		msgtext->is_priv = g_value_get_boolean(value);
		break;
	case PROP_REMARK:
		msgtext->is_remark = g_value_get_boolean(value);
		break;
	case PROP_TEXT_TYPE:
		msgtext->text_type = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}

}
static void
message_text_get_property(GObject  *object,
			  guint param_id,
			  GValue *value,
			  GParamSpec *pspec)
{
	MessageText *msgtext;

	msgtext = MESSAGE_TEXT(object);

	switch(param_id) {
	case PROP_TEXT:
		g_value_set_string(value, message_text_get_text(msgtext));
		break;
	case PROP_NICK:
		g_value_set_string(value, message_text_get_nick(msgtext));
		break;
	case PROP_CHANNEL_NAME:
		g_value_set_string(value, message_text_get_channel_name(msgtext));
		break;
	case PROP_ACCOUNT_NAME:
		g_value_set_string(value, message_text_get_account_name(msgtext));
		break;
	case PROP_SELF:
		g_value_set_boolean(value, msgtext->is_self);
		break;
	case PROP_PRIV:
		g_value_set_boolean(value, msgtext->is_priv);
		break;
	case PROP_REMARK:
		g_value_set_boolean(value, msgtext->is_remark);
		break;
	case PROP_TEXT_TYPE:
		g_value_set_uint(value, msgtext->text_type);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}
MessageText*
message_text_new(void)
{
        MessageText *message_text;

	message_text = g_object_new(message_text_get_type(), NULL);
	
	return message_text;
}

MESSAGE_TEXT_ACCESSOR_STRING(text);
MESSAGE_TEXT_ACCESSOR_STRING(nick);
MESSAGE_TEXT_ACCESSOR_STRING(account_name);
MESSAGE_TEXT_ACCESSOR_STRING(channel_name);

ATTR_ACCESSOR_GENERIC(gboolean, FALSE, MessageText, message_text, is_priv);
ATTR_ACCESSOR_GENERIC(gboolean, FALSE, MessageText, message_text, is_self);
ATTR_ACCESSOR_GENERIC(gboolean, FALSE, MessageText, message_text, is_remark);

void
message_text_set_text_type(MessageText *msgtext, TextType type)
{
        g_return_if_fail(msgtext != NULL);
        g_return_if_fail(IS_MESSAGE_TEXT(msgtext));

	msgtext->text_type = type;
}

TextType
message_text_get_text_type(MessageText *msgtext)
{
        g_return_val_if_fail(msgtext != NULL, 0);
        g_return_val_if_fail(IS_MESSAGE_TEXT(msgtext), 0);
	
	return msgtext->text_type;
}
gchar *
message_text_get_nick_string(MessageText *msgtext, gboolean with_channel_name)
{
	gchar *nick_str;

        g_return_val_if_fail(msgtext != NULL, NULL);
        g_return_val_if_fail(IS_MESSAGE_TEXT(msgtext), NULL);

	if(msgtext->is_priv) {
		if(msgtext->is_self)
			nick_str = g_strdup_printf(">%s< ", msgtext->nick);
		else
			nick_str = g_strdup_printf("=%s= ", msgtext->nick);
	} else if (with_channel_name && msgtext->channel_name) {
		if(msgtext->is_self)
			nick_str = g_strdup_printf(">%s:%s< ", msgtext->channel_name, msgtext->nick);
		else
			nick_str = g_strdup_printf("<%s:%s> ", msgtext->channel_name, msgtext->nick);
	} else {
		if(msgtext->is_self)
			nick_str = g_strdup_printf(">%s< ", msgtext->nick);
		else
			nick_str = g_strdup_printf("<%s> ", msgtext->nick);
	}

	return nick_str;
}
