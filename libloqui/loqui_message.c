/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM library with GNet/GObject <http://loqui.good-day.net/>
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
#include "config.h"

#include "loqui_message.h"
#include <libloqui-intl.h>

#include <string.h>
#include <gobject/gvaluecollector.h>

#include "loqui_user.h"
#include "loqui_channel.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_COMMAND,
        LAST_PROP
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_message_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_message_class_init(LoquiMessageClass *klass);
static void loqui_message_init(LoquiMessage *message);
static void loqui_message_finalize(GObject *object);
static void loqui_message_dispose(GObject *object);

static void loqui_message_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_message_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_message_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiMessageClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_message_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiMessage),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_message_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiMessage",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_message_finalize(GObject *object)
{
	LoquiMessage *message;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE(object));

        message = LOQUI_MESSAGE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);
}
static void 
loqui_message_dispose(GObject *object)
{
	LoquiMessage *message;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE(object));

        message = LOQUI_MESSAGE(object);

	g_datalist_clear(&message->attr_dlist);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_message_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiMessage *message;        

        message = LOQUI_MESSAGE(object);

        switch (param_id) {
	case PROP_COMMAND:
		g_value_set_string(value, g_quark_to_string(message->command_quark));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_message_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
	const gchar *command;
        LoquiMessage *message;        

        message = LOQUI_MESSAGE(object);

        switch (param_id) {
	case PROP_COMMAND:
		command = g_value_get_string(value);
		if (!loqui_message_class_get_command_info(LOQUI_MESSAGE_GET_CLASS(message),
							  command)) {
			g_warning("Unsupported command.");
			return;
		}
		message->command_quark = g_quark_from_string(command);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_message_class_init(LoquiMessageClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_message_finalize;
        object_class->dispose = loqui_message_dispose;
        object_class->get_property = loqui_message_get_property;
        object_class->set_property = loqui_message_set_property;

	g_datalist_init(&klass->command_info_table);

	g_object_class_install_property(object_class,
					PROP_COMMAND,
					g_param_spec_string("command",
							    _("Command"),
							    _("Command"),
							    NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_UNHANDLED,
					    LOQUI_COMMAND_FLAG_RECEIVE,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_ERROR,
					    LOQUI_COMMAND_FLAG_RECEIVE,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_GLOBAL_INFO,
					    LOQUI_COMMAND_FLAG_RECEIVE,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_CHANNEL_INFO,
					    LOQUI_COMMAND_FLAG_RECEIVE,
					    "channel", LOQUI_TYPE_CHANNEL,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_USER_INFO,
					    LOQUI_COMMAND_FLAG_RECEIVE,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);
	
	
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_MESSAGE,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND ,
					    "channel", LOQUI_TYPE_CHANNEL,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    "is-weak", G_TYPE_BOOLEAN,
					    /* "keyword-region-list", LOQUI_TYPE_BOXED_LIST, */
					    NULL);


	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_NICK,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "user", LOQUI_TYPE_USER,
					    "nick-old", G_TYPE_STRING,
					    "nick-new", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_AWAY,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "user", LOQUI_TYPE_USER,
					    "away", G_TYPE_INT, /* LoquiAwayType */
					    "away-message", G_TYPE_BOOLEAN,
					    NULL);
/*	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_WHO,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_WHOIS,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL); */
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_QUIT,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);

	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_TOPIC,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "channel", LOQUI_TYPE_CHANNEL,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_JOIN,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "channel", LOQUI_TYPE_CHANNEL,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_PART,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "channel", LOQUI_TYPE_CHANNEL,
					    "user", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);

	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_INVITE,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "channel", LOQUI_TYPE_CHANNEL,
					    "inviter", LOQUI_TYPE_USER,
					    "invitee", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);
	loqui_message_class_install_command(klass,
					    LOQUI_COMMAND_INVITE,
					    LOQUI_COMMAND_FLAG_RECEIVE | LOQUI_COMMAND_FLAG_SEND,
					    "channel", LOQUI_TYPE_CHANNEL,
					    "kicker", LOQUI_TYPE_USER,
					    "kickee", LOQUI_TYPE_USER,
					    "text", G_TYPE_STRING,
					    NULL);
	
}
static void 
loqui_message_init(LoquiMessage *message)
{
	message->command_quark = 0;
	g_datalist_init(&message->attr_dlist);
}
LoquiMessage*
loqui_message_new(const gchar *command)
{
        LoquiMessage *message;
	LoquiMessagePrivate *priv;

	message = g_object_new(loqui_message_get_type(), "command", command, NULL);

        priv = message->priv;

        return message;
}
static void
destroy_gvalue_and_free(gpointer data)
{
	GValue *value;

	value = data;

	g_value_unset(value);
	g_free(value);
}

void
loqui_message_set_attribute(LoquiMessage *message, const gchar *first_attribute_name, ...)
{
	LoquiMessageCommandInfo *info;
	va_list args;
	const gchar *attr_name;
	GQuark attr_key;
	GValue *value;
	GType type;
	gchar *error;

        g_return_if_fail(message != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE(message));
	g_return_if_fail(message->command_quark != 0);

	info = g_datalist_id_get_data(&LOQUI_MESSAGE_GET_CLASS(message)->command_info_table, message->command_quark);
	g_return_if_fail(info != NULL);

	va_start(args, first_attribute_name);
	attr_name = first_attribute_name;
	while (attr_name) {
		attr_key = g_quark_from_string(attr_name);

		type = (GType) GPOINTER_TO_UINT(g_datalist_id_get_data(&info->attr_def_dlist, attr_key));
		if (type == 0) {
			g_warning("Invalid attribute name.");
			return;
		}
		
		value = g_new0(GValue, 1);
		g_value_init(value, type);
		G_VALUE_COLLECT(value, args, 0, &error);
		if (error) {
			g_warning("%s: %s", G_STRLOC, error);
			g_free(error);
          
			return;
		}

		g_datalist_id_set_data_full(&message->attr_dlist,
					    attr_key,
					    value,
					    (GDestroyNotify) destroy_gvalue_and_free);
		attr_name = va_arg(args, gchar *);
        }
	va_end(args);
}
void
loqui_message_get_attribute(LoquiMessage *message, const gchar *first_attribute_name,...)
{
	GValue *value;
	va_list args;
	const gchar *attr_name;
	GQuark attr_key;
	GType type;
	gchar *error;
	LoquiMessageCommandInfo *info;
	GValue tmp_value = {0, };
        g_return_if_fail(message != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE(message));

	info = g_datalist_id_get_data(&LOQUI_MESSAGE_GET_CLASS(message)->command_info_table, message->command_quark);
	g_return_if_fail(info != NULL);

	va_start(args, first_attribute_name);
	attr_name = first_attribute_name;
	while (attr_name) {
		attr_key = g_quark_from_string(attr_name);

		type = (GType) GPOINTER_TO_UINT(g_datalist_id_get_data(&info->attr_def_dlist, attr_key));
		if (type == 0) {
			g_warning("Invalid attribute name.");
			return;
		}

		value = g_datalist_id_get_data(&message->attr_dlist, attr_key);
		if (value != NULL) {
			G_VALUE_LCOPY(value, args, 0, &error);
			if (error) {
				g_warning("%s: %s", G_STRLOC, error);
				g_free(error);
				
				return;
			}
		} else {
			g_value_unset(&tmp_value);
			g_value_init(&tmp_value, type);
			G_VALUE_LCOPY(value, args, 0, &error);
			if (error) {
				g_warning("%s: %s", G_STRLOC, error);
				g_free(error);
				
				return;
			}
		}

		attr_name = va_arg(args, gchar *);
	}
	va_end(args);
}
void
loqui_message_class_install_command(LoquiMessageClass *message_class,
				    const gchar *name,
				    LoquiMessageCommandFlags flags,
				    const gchar *first_attribute_name,
				    ...)
{
	va_list args;
	const gchar *attr_name;
	LoquiMessageCommandInfo *info;
	GType type;

        g_return_if_fail(message_class != NULL);
        g_return_if_fail(LOQUI_IS_MESSAGE_CLASS(message_class));
	g_return_if_fail(!g_datalist_get_data(&message_class->command_info_table, name));

	info = g_new0(LoquiMessageCommandInfo, 1);
	info->name = g_strdup(name);
	info->flags = flags;
	g_datalist_init(&info->attr_def_dlist);

	va_start(args, first_attribute_name);
	attr_name = first_attribute_name;
	while (attr_name) {
		type = va_arg(args, GType);
		g_datalist_set_data(&info->attr_def_dlist,
				     attr_name,
				     GUINT_TO_POINTER(type));

		attr_name = va_arg(args, gchar *);
	}
	va_end(args);

	g_datalist_set_data(&message_class->command_info_table, name, info);
}
LoquiMessageCommandInfo *
loqui_message_class_get_command_info(LoquiMessageClass *message_class,
				     const gchar *name)
{
        g_return_val_if_fail(message_class != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_MESSAGE_CLASS(message_class), NULL);

	return g_datalist_get_data(&message_class->command_info_table, name);
}
