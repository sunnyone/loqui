/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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

#include "loqui_protocol_ipmsg.h"

#include "loqui-account-ipmsg.h"
#include "loqui_user_ipmsg.h"
/* #include "loqui_channel_ipmsg.h" */
#include "loqui-sender-ipmsg.h"
#include "loqui-receiver-ipmsg.h"
#include "loqui_profile_account_ipmsg.h"
#include "loqui_codeconv.h"

#include <libloqui-intl.h>

static LoquiProtocolClass *parent_class = NULL;

static void loqui_protocol_ipmsg_class_init(LoquiProtocolIPMsgClass *klass);
static void loqui_protocol_ipmsg_init(LoquiProtocolIPMsg *protocol);

static LoquiProtocolIPMsg* loqui_protocol_ipmsg_new(void);

static LoquiCodeConvTableItem codeconv_table[] = {
	{"ja",
	 N_("Japanese"),
	 N_("CP932"),
	 "ja_JP",
	 NULL,
	 "CP932", "Shift_JIS",
	},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};

GType
loqui_protocol_ipmsg_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProtocolIPMsgClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_protocol_ipmsg_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProtocolIPMsg),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_protocol_ipmsg_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_PROTOCOL,
					      "LoquiProtocolIPMsg",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_protocol_ipmsg_class_init(LoquiProtocolIPMsgClass *klass)
{
        parent_class = g_type_class_peek_parent(klass);
}
static void 
loqui_protocol_ipmsg_init(LoquiProtocolIPMsg *protocol)
{
	LOQUI_PROTOCOL(protocol)->type_account = LOQUI_TYPE_ACCOUNT_IPMSG;
	LOQUI_PROTOCOL(protocol)->type_user = LOQUI_TYPE_USER_IPMSG;
/*	LOQUI_PROTOCOL(protocol)->type_channel = LOQUI_TYPE_CHANNEL_IPMSG; */
	LOQUI_PROTOCOL(protocol)->type_channel = LOQUI_TYPE_CHANNEL;
	LOQUI_PROTOCOL(protocol)->type_sender = LOQUI_TYPE_SENDER_IPMSG;
	LOQUI_PROTOCOL(protocol)->type_receiver = LOQUI_TYPE_RECEIVER_IPMSG;
	LOQUI_PROTOCOL(protocol)->type_profile_account = LOQUI_TYPE_PROFILE_ACCOUNT_IPMSG;

	loqui_protocol_set_identifier(LOQUI_PROTOCOL(protocol), "IPMsg");
	loqui_protocol_set_codeconv_table(LOQUI_PROTOCOL(protocol), codeconv_table);
}
static LoquiProtocolIPMsg*
loqui_protocol_ipmsg_new(void)
{
        LoquiProtocolIPMsg *protocol;

	protocol = g_object_new(loqui_protocol_ipmsg_get_type(), NULL);
	
        return protocol;
}
LoquiProtocol*
loqui_protocol_ipmsg_get(void)
{
	static LoquiProtocol *protocol = NULL;

	if (protocol == NULL)
		protocol = LOQUI_PROTOCOL(loqui_protocol_ipmsg_new());

	return protocol;
}
