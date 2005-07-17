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
#include "config.h"

#include "loqui-message-text.h"
#include <libloqui-intl.h>

enum {
	PROP_0,
	PROP_TEXT,
	PROP_CHANNEL_NAME,
	PROP_ACCOUNT_NAME,
	PROP_NICK,
	PROP_IS_SELF,
	PROP_IS_PRIV,
	PROP_IS_REMARK,
	PROP_TEXT_TYPE,
	PROP_EXEC_NOTIFICATION,
	LAST_PROP
};

struct _LoquiMessageTextPrivate
{
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void loqui_message_text_class_init(LoquiMessageTextClass *klass);
static void loqui_message_text_init(LoquiMessageText *message_text);
static void loqui_message_text_finalize(GObject *object);

static void loqui_message_text_get_property(GObject  *object,
				      guint param_id,
				      GValue *value,
				      GParamSpec *pspec);
static void loqui_message_text_set_property(GObject *object,
				      guint param_id,
				      const GValue *value,
				      GParamSpec *pspec);
GType
loqui_message_text_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiMessageTextClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_message_text_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiMessageText),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_message_text_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiMessageText",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_message_text_class_init(LoquiMessageTextClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
	object_class->set_property = loqui_message_text_set_property;
	object_class->get_property = loqui_message_text_get_property;	
        object_class->finalize = loqui_message_text_finalize;

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
					PROP_IS_SELF,
					g_param_spec_boolean("is_self",
							    _("Self?"),
							    _("Whether or not the message is by myself"),
							    FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IS_PRIV,
					g_param_spec_boolean("is_priv",
							    _("Priv?"),
							    _("Whether or not private message"),
							    FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IS_REMARK,
					g_param_spec_boolean("is_remark",
							    _("Remark?"),
							    _("Remark?"),
							    FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_EXEC_NOTIFICATION,
					g_param_spec_boolean("exec_notification",
							    _("Execute notification"),
							    _("Execute notification or not"),
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
loqui_message_text_init(LoquiMessageText *message_text)
{
	LoquiMessageTextPrivate *priv;

	priv = g_new0(LoquiMessageTextPrivate, 1);

	message_text->priv = priv;
}
static void 
loqui_message_text_finalize(GObject *object)
{
	LoquiMessageText *msgtext;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE_TEXT(object));

        msgtext = LOQUI_MESSAGE_TEXT(object);

	LOQUI_G_FREE_UNLESS_NULL(msgtext->text);
	LOQUI_G_FREE_UNLESS_NULL(msgtext->channel_name);
	LOQUI_G_FREE_UNLESS_NULL(msgtext->nick);
	LOQUI_G_FREE_UNLESS_NULL(msgtext->account_name);

	if (msgtext->uri_region_list) {
		g_list_foreach(msgtext->uri_region_list, (GFunc) g_free, NULL);
		g_list_free(msgtext->uri_region_list);
		msgtext->uri_region_list = NULL;
	}
	if (msgtext->highlight_region_list) {
		g_list_foreach(msgtext->highlight_region_list, (GFunc) g_free, NULL);
		g_list_free(msgtext->highlight_region_list);
		msgtext->highlight_region_list = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(msgtext->priv);
}
static void
loqui_message_text_set_property(GObject *object,
			  guint param_id,
			  const GValue *value,
			  GParamSpec *pspec)
{
	LoquiMessageText *msgtext;

	msgtext = LOQUI_MESSAGE_TEXT(object);

	switch(param_id) {
	case PROP_TEXT:
		loqui_message_text_set_text(msgtext, g_value_get_string(value));
		break;
	case PROP_NICK:
		loqui_message_text_set_nick(msgtext, g_value_get_string(value));
		break;
	case PROP_CHANNEL_NAME:
		loqui_message_text_set_channel_name(msgtext, g_value_get_string(value));
		break;
	case PROP_ACCOUNT_NAME:
		loqui_message_text_set_account_name(msgtext, g_value_get_string(value));
		break;
	case PROP_IS_SELF:
		msgtext->is_self = g_value_get_boolean(value);
		break;
	case PROP_IS_PRIV:
		msgtext->is_priv = g_value_get_boolean(value);
		break;
	case PROP_IS_REMARK:
		msgtext->is_remark = g_value_get_boolean(value);
		break;
	case PROP_EXEC_NOTIFICATION:
		msgtext->exec_notification = g_value_get_boolean(value);
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
loqui_message_text_get_property(GObject  *object,
			  guint param_id,
			  GValue *value,
			  GParamSpec *pspec)
{
	LoquiMessageText *msgtext;

	msgtext = LOQUI_MESSAGE_TEXT(object);

	switch(param_id) {
	case PROP_TEXT:
		g_value_set_string(value, loqui_message_text_get_text(msgtext));
		break;
	case PROP_NICK:
		g_value_set_string(value, loqui_message_text_get_nick(msgtext));
		break;
	case PROP_CHANNEL_NAME:
		g_value_set_string(value, loqui_message_text_get_channel_name(msgtext));
		break;
	case PROP_ACCOUNT_NAME:
		g_value_set_string(value, loqui_message_text_get_account_name(msgtext));
		break;
	case PROP_IS_SELF:
		g_value_set_boolean(value, msgtext->is_self);
		break;
	case PROP_IS_PRIV:
		g_value_set_boolean(value, msgtext->is_priv);
		break;
	case PROP_IS_REMARK:
		g_value_set_boolean(value, msgtext->is_remark);
		break;
	case PROP_EXEC_NOTIFICATION:
		g_value_set_boolean(value, msgtext->exec_notification);
		break;
	case PROP_TEXT_TYPE:
		g_value_set_uint(value, msgtext->text_type);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}
LoquiMessageText*
loqui_message_text_new(void)
{
        LoquiMessageText *message_text;

	message_text = g_object_new(loqui_message_text_get_type(), NULL);
	
	return message_text;
}

LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiMessageText, loqui_message_text, text);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiMessageText, loqui_message_text, nick);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiMessageText, loqui_message_text, account_name);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiMessageText, loqui_message_text, channel_name);

LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiMessageText, loqui_message_text, is_priv, gboolean);
LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiMessageText, loqui_message_text, is_self, gboolean);
LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiMessageText, loqui_message_text, is_remark, gboolean);
LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiMessageText, loqui_message_text, exec_notification, gboolean);

void
loqui_message_text_set_text_type(LoquiMessageText *msgtext, LoquiTextType type)
{
        g_return_if_fail(msgtext != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE_TEXT(msgtext));

	msgtext->text_type = type;
}

LoquiTextType
loqui_message_text_get_text_type(LoquiMessageText *msgtext)
{
        g_return_val_if_fail(msgtext != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_MESSAGE_TEXT(msgtext), 0);
	
	return msgtext->text_type;
}
gchar *
loqui_message_text_get_nick_string(LoquiMessageText *msgtext, gboolean with_channel_name)
{
	gchar *nick_str;

        g_return_val_if_fail(msgtext != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_MESSAGE_TEXT(msgtext), NULL);

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
