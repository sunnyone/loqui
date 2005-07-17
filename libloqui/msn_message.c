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
#include "config.h"

#include "msn_message.h"
#include "loqui_string_tokenizer.h"
#include "loqui-utils.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _MSNMessagePrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint msn_message_signals[LAST_SIGNAL] = { 0 }; */

static GObject* msn_message_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void msn_message_class_init(MSNMessageClass *klass);
static void msn_message_init(MSNMessage *message);
static void msn_message_finalize(GObject *object);
static void msn_message_dispose(GObject *object);

static void msn_message_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void msn_message_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
msn_message_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(MSNMessageClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) msn_message_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(MSNMessage),
				0,              /* n_preallocs */
				(GInstanceInitFunc) msn_message_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "MSNMessage",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
msn_message_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
msn_message_finalize(GObject *object)
{
	MSNMessage *message;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_MSN_MESSAGE(object));

        message = MSN_MESSAGE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(message->priv);
}
static void 
msn_message_dispose(GObject *object)
{
	MSNMessage *message;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_MSN_MESSAGE(object));

        message = MSN_MESSAGE(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
msn_message_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        MSNMessage *message;        

        message = MSN_MESSAGE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
msn_message_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        MSNMessage *message;        

        message = MSN_MESSAGE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
msn_message_class_init(MSNMessageClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = msn_message_constructor; 
        object_class->finalize = msn_message_finalize;
        object_class->dispose = msn_message_dispose;
        object_class->get_property = msn_message_get_property;
        object_class->set_property = msn_message_set_property;
}
static void 
msn_message_init(MSNMessage *message)
{
	MSNMessagePrivate *priv;

	priv = g_new0(MSNMessagePrivate, 1);

	message->priv = priv;
}
/**
   @raw_text if NULL, created from parameters, payload_headers and payload_body
   @parameters used directly, so this must not be freed.
   @payload_headers used directly, so this must not be freed.
*/
MSNMessage*
msn_message_new(const gchar *raw_text, GList *parameters, GList *payload_headers, const gchar *payload_body)
{
        MSNMessage *msg;
	MSNMessagePrivate *priv;

	msg = g_object_new(msn_message_get_type(), NULL);
	
        priv = msg->priv;

	msg->parameters = parameters;
	msg->payload_headers = payload_headers;
	if (payload_body)
		msg->payload_body = g_strdup(payload_body);

	if (raw_text)
		msg->raw_text = g_strdup(raw_text);

        return msg;
}
static void
msn_message_parse_payload(const gchar *text, GList **payload_headers, gchar **payload_body)
{
	LoquiStringTokenizer *st;

	g_return_if_fail(text != NULL);
	
	if (text[0] == '\0')
		return;

	return;

	st = loqui_string_tokenizer_new(text, "");
	if (text[0] != '\r') {
		loqui_string_tokenizer_set_delimiters(st, ":");
	}
}
MSNMessage*
msn_message_parse(const gchar *text)
{
	LoquiStringTokenizer *st;
	MSNMessage *msg;
	GList *list = NULL;
	const gchar *token;
	gchar d;
	gchar *payload_body = NULL;
	GList *payload_headers = NULL;

	st = loqui_string_tokenizer_new(text, " \r");
	loqui_string_tokenizer_set_skip_whitespaces_after_delimiter(st, TRUE);
	
	while ((token = loqui_string_tokenizer_next_token(st, &d))) {
		if (d == '\r') {
			loqui_string_tokenizer_set_delimiters(st, "");
			msn_message_parse_payload(loqui_string_tokenizer_next_token(st, NULL), &payload_headers, &payload_body);
			break;
		} else {
			list = g_list_append(list, loqui_utils_url_decode(token));
		}
	}
	loqui_string_tokenizer_free(st);

	msg = msn_message_new(text, list, payload_headers, payload_body);
	return msg;
}

MSNMessage *
msn_message_create(GList *payload_headers, const gchar *payload_body, const gchar *param, ...)
{
	MSNMessage *msg;
	va_list args;
	/* gint num, i; */
	gchar *arg;
	GList *list = NULL;

	list = g_list_append(list, g_strdup(param));

	va_start(args, param);
	while ((arg = va_arg(args, gchar *)) != NULL) {
		list = g_list_append(list, g_strdup(arg));
	}
	va_end(args);

	msg = msn_message_new(NULL, list, payload_headers, payload_body);

	return msg;
}
