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

#include "loqui_protocol_irc.h"

#include "loqui_account_irc.h"
#include "loqui_user_irc.h"
#include "loqui_channel_irc.h"
#include "loqui_sender_irc.h"
#include "loqui_receiver_irc.h"
#include "loqui_profile_account_irc.h"

enum {
        LAST_SIGNAL
};

static LoquiProtocolClass *parent_class = NULL;

/* static guint loqui_protocol_irc_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_protocol_irc_class_init(LoquiProtocolIRCClass *klass);
static void loqui_protocol_irc_init(LoquiProtocolIRC *protocol);
static void loqui_protocol_irc_finalize(GObject *object);
static void loqui_protocol_irc_dispose(GObject *object);

static LoquiProtocolIRC* loqui_protocol_irc_new(void);

GType
loqui_protocol_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProtocolIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_protocol_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProtocolIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_protocol_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_PROTOCOL,
					      "LoquiProtocolIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_protocol_irc_finalize(GObject *object)
{
	LoquiProtocolIRC *protocol;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL_IRC(object));

        protocol = LOQUI_PROTOCOL_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(protocol->priv);
}
static void 
loqui_protocol_irc_dispose(GObject *object)
{
	LoquiProtocolIRC *protocol;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL_IRC(object));

        protocol = LOQUI_PROTOCOL_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_protocol_irc_class_init(LoquiProtocolIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_protocol_irc_finalize;
        object_class->dispose = loqui_protocol_irc_dispose;
}
static void 
loqui_protocol_irc_init(LoquiProtocolIRC *protocol)
{
	LOQUI_PROTOCOL(protocol)->type_account = LOQUI_TYPE_ACCOUNT_IRC;
	LOQUI_PROTOCOL(protocol)->type_user = LOQUI_TYPE_USER_IRC;
	LOQUI_PROTOCOL(protocol)->type_channel = LOQUI_TYPE_CHANNEL_IRC;
	LOQUI_PROTOCOL(protocol)->type_sender = LOQUI_TYPE_SENDER_IRC;
	LOQUI_PROTOCOL(protocol)->type_receiver = LOQUI_TYPE_RECEIVER_IRC;
	LOQUI_PROTOCOL(protocol)->type_profile_account = LOQUI_TYPE_PROFILE_ACCOUNT_IRC;

	loqui_protocol_set_identifier(LOQUI_PROTOCOL(protocol), "IRC");
}
static LoquiProtocolIRC*
loqui_protocol_irc_new(void)
{
        LoquiProtocolIRC *protocol;

	protocol = g_object_new(loqui_protocol_irc_get_type(), NULL);
	
        return protocol;
}
LoquiProtocol*
loqui_protocol_irc_get(void)
{
	static LoquiProtocol *protocol = NULL;

	if (protocol == NULL)
		protocol = LOQUI_PROTOCOL(loqui_protocol_irc_new());

	return protocol;
}
