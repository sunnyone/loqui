/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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

#include "loqui_protocol.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiProtocolPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_protocol_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_protocol_class_init(LoquiProtocolClass *klass);
static void loqui_protocol_init(LoquiProtocol *protocol);
static void loqui_protocol_finalize(GObject *object);
static void loqui_protocol_dispose(GObject *object);

static void loqui_protocol_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_protocol_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_protocol_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProtocolClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_protocol_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProtocol),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_protocol_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiProtocol",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_protocol_finalize(GObject *object)
{
	LoquiProtocol *protocol;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL(object));

        protocol = LOQUI_PROTOCOL(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(protocol->priv);
}
static void 
loqui_protocol_dispose(GObject *object)
{
	LoquiProtocol *protocol;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL(object));

        protocol = LOQUI_PROTOCOL(object);

	G_FREE_UNLESS_NULL(protocol->identifier);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_protocol_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiProtocol *protocol;        

        protocol = LOQUI_PROTOCOL(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_protocol_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiProtocol *protocol;        

        protocol = LOQUI_PROTOCOL(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_protocol_class_init(LoquiProtocolClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_protocol_finalize;
        object_class->dispose = loqui_protocol_dispose;
        object_class->get_property = loqui_protocol_get_property;
        object_class->set_property = loqui_protocol_set_property;
}
static void 
loqui_protocol_init(LoquiProtocol *protocol)
{
	LoquiProtocolPrivate *priv;

	priv = g_new0(LoquiProtocolPrivate, 1);

	protocol->priv = priv;

	protocol->type_account = LOQUI_TYPE_ACCOUNT;
	protocol->type_channel = LOQUI_TYPE_CHANNEL;
	protocol->type_sender = LOQUI_TYPE_USER;
	protocol->type_receiver = LOQUI_TYPE_RECEIVER;
	protocol->type_user = LOQUI_TYPE_USER;
	protocol->type_profile_account = LOQUI_TYPE_PROFILE_ACCOUNT;
}
LoquiProtocol*
loqui_protocol_new(void)
{
        LoquiProtocol *protocol;
	LoquiProtocolPrivate *priv;

	protocol = g_object_new(loqui_protocol_get_type(), NULL);
	
        priv = protocol->priv;

        return protocol;
}
void
loqui_protocol_set_identifier(LoquiProtocol *protocol, const gchar *identifier)
{
	G_FREE_UNLESS_NULL(protocol->identifier);
	if (identifier)
		protocol->identifier = g_strdup(identifier);
}
G_CONST_RETURN gchar *
loqui_protocol_get_identifier(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);

	return protocol->identifier;
}

void
loqui_protocol_set_codeconv_table(LoquiProtocol *protocol, LoquiCodeConvTableItem *codeconv_table)
{
        g_return_if_fail(protocol != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL(protocol));

	protocol->codeconv_table = codeconv_table;
}
LoquiCodeConvTableItem *
loqui_protocol_get_codeconv_table(LoquiProtocol *protocol)
{
        g_return_val_if_fail(protocol != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);
	
	return protocol->codeconv_table;
}

LoquiAccountClass *
loqui_protocol_get_account_class(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);

	return g_type_class_ref(protocol->type_account);
}
LoquiChannelClass *
loqui_protocol_get_channel_class(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);

	return g_type_class_ref(protocol->type_channel);
}
LoquiSenderClass *
loqui_protocol_get_sender_class(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);

	return g_type_class_ref(protocol->type_sender);
}
LoquiReceiverClass *
loqui_protocol_get_receiver_class(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);

	return g_type_class_ref(protocol->type_receiver);
}
LoquiUserClass *
loqui_protocol_get_user_class(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);

	return g_type_class_ref(protocol->type_user);
}
LoquiProfileAccountClass *
loqui_protocol_get_profile_account_class(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);

	return g_type_class_ref(protocol->type_profile_account);
}

LoquiProfileAccount *
loqui_protocol_create_profile_account(LoquiProtocol *protocol)
{
	LoquiProfileAccount *profile;

	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);
	
	/* FIXME: this should be done with argument of g_object_new,
	   but it can't because a property will be serialized. */
	profile = LOQUI_PROFILE_ACCOUNT(g_object_new(protocol->type_profile_account, NULL));
	profile->protocol = protocol;

	return profile;
}
LoquiUser *
loqui_protocol_create_user(LoquiProtocol *protocol)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);
	
	return LOQUI_USER(g_object_new(protocol->type_user, NULL));
}
LoquiAccount *
loqui_protocol_create_account(LoquiProtocol *protocol, LoquiProfileAccount *profile)
{
	g_return_val_if_fail(protocol != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_PROTOCOL(protocol), NULL);
	
	return LOQUI_ACCOUNT(g_object_new(protocol->type_account,
					  "profile", profile,
					  NULL));
}
