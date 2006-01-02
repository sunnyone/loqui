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

#include "loqui_protocol_msn.h"
#include "loqui_account_msn.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiProtocolMSNPrivate
{
};

static LoquiProtocolClass *parent_class = NULL;

/* static guint loqui_protocol_msn_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_protocol_msn_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_protocol_msn_class_init(LoquiProtocolMSNClass *klass);
static void loqui_protocol_msn_init(LoquiProtocolMSN *protocol);
static void loqui_protocol_msn_finalize(GObject *object);
static void loqui_protocol_msn_dispose(GObject *object);

static void loqui_protocol_msn_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_protocol_msn_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_protocol_msn_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProtocolMSNClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_protocol_msn_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProtocolMSN),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_protocol_msn_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_PROTOCOL,
					      "LoquiProtocolMSN",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_protocol_msn_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_protocol_msn_finalize(GObject *object)
{
	LoquiProtocolMSN *protocol;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL_MSN(object));

        protocol = LOQUI_PROTOCOL_MSN(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(protocol->priv);
}
static void 
loqui_protocol_msn_dispose(GObject *object)
{
	LoquiProtocolMSN *protocol;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL_MSN(object));

        protocol = LOQUI_PROTOCOL_MSN(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_protocol_msn_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiProtocolMSN *protocol;        

        protocol = LOQUI_PROTOCOL_MSN(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_protocol_msn_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiProtocolMSN *protocol;        

        protocol = LOQUI_PROTOCOL_MSN(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_protocol_msn_class_init(LoquiProtocolMSNClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_protocol_msn_constructor; 
        object_class->finalize = loqui_protocol_msn_finalize;
        object_class->dispose = loqui_protocol_msn_dispose;
        object_class->get_property = loqui_protocol_msn_get_property;
        object_class->set_property = loqui_protocol_msn_set_property;
}
static void 
loqui_protocol_msn_init(LoquiProtocolMSN *protocol)
{
	LoquiProtocolMSNPrivate *priv;

	priv = g_new0(LoquiProtocolMSNPrivate, 1);

	protocol->priv = priv;

	LOQUI_PROTOCOL(protocol)->type_account = LOQUI_TYPE_ACCOUNT_MSN;
	LOQUI_PROTOCOL(protocol)->type_user = LOQUI_TYPE_USER;
	LOQUI_PROTOCOL(protocol)->type_channel = LOQUI_TYPE_CHANNEL;
	LOQUI_PROTOCOL(protocol)->type_sender = LOQUI_TYPE_SENDER;
	LOQUI_PROTOCOL(protocol)->type_receiver = LOQUI_TYPE_RECEIVER;
	LOQUI_PROTOCOL(protocol)->type_profile_account = LOQUI_TYPE_PROFILE_ACCOUNT;

	loqui_protocol_set_identifier(LOQUI_PROTOCOL(protocol), "MSN");
}
LoquiProtocolMSN*
loqui_protocol_msn_new(void)
{
        LoquiProtocolMSN *protocol;
	LoquiProtocolMSNPrivate *priv;

	protocol = g_object_new(loqui_protocol_msn_get_type(), NULL);
	
        priv = protocol->priv;

        return protocol;
}
LoquiProtocol*
loqui_protocol_msn_get(void)
{
	static LoquiProtocol *protocol = NULL;

	if (protocol == NULL)
		protocol = LOQUI_PROTOCOL(loqui_protocol_msn_new());

	return protocol;
}
