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
/* Abstract Factory (Factory Method, Singleton(?)) */

#ifndef __LOQUI_PROTOCOL_H__
#define __LOQUI_PROTOCOL_H__

#include <glib-object.h>
#include "gobject_utils.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_PROTOCOL                 (loqui_protocol_get_type ())
#define LOQUI_PROTOCOL(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROTOCOL, LoquiProtocol))
#define LOQUI_PROTOCOL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROTOCOL, LoquiProtocolClass))
#define LOQUI_IS_PROTOCOL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROTOCOL))
#define LOQUI_IS_PROTOCOL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROTOCOL))
#define LOQUI_PROTOCOL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROTOCOL, LoquiProtocolClass))

typedef struct _LoquiProtocol            LoquiProtocol;
typedef struct _LoquiProtocolClass       LoquiProtocolClass;

typedef struct _LoquiProtocolPrivate     LoquiProtocolPrivate;

#include "loqui_account.h"
#include "loqui_channel.h"
#include "loqui_sender.h"
#include "loqui_receiver.h"
#include "loqui_user.h"
#include "loqui_profile_account.h"
#include "loqui_codeconv.h"

struct _LoquiProtocol
{
        GObject parent;
        
	gchar *identifier;

	LoquiCodeConvTableItem *codeconv_table;

	GType type_account;
	GType type_channel;
	GType type_sender;
	GType type_receiver;
	GType type_user;
	GType type_profile_account;

        LoquiProtocolPrivate *priv;
};

struct _LoquiProtocolClass
{
        GObjectClass parent_class;
};

GType loqui_protocol_get_type(void) G_GNUC_CONST;

LoquiProtocol* loqui_protocol_new(void);

void loqui_protocol_set_identifier(LoquiProtocol *protocol, const gchar *identifier);
G_CONST_RETURN gchar *loqui_protocol_get_identifier(LoquiProtocol *protocol);

void loqui_protocol_set_codeconv_table(LoquiProtocol *protocol, LoquiCodeConvTableItem *codeconv_table);
LoquiCodeConvTableItem *loqui_protocol_get_codeconv_table(LoquiProtocol *protocol);

/* you must unref classes */
LoquiAccountClass *loqui_protocol_get_account_class(LoquiProtocol *protocol);
LoquiChannelClass *loqui_protocol_get_channel_class(LoquiProtocol *protocol);
LoquiSenderClass *loqui_protocol_get_sender_class(LoquiProtocol *protocol);
LoquiReceiverClass *loqui_protocol_get_receiver_class(LoquiProtocol *protocol);
LoquiUserClass *loqui_protocol_get_user_class(LoquiProtocol *protocol);
LoquiProfileAccountClass *loqui_protocol_get_profile_account_class(LoquiProtocol *protocol);

LoquiProfileAccount *loqui_protocol_create_profile_account(LoquiProtocol *protocol);
LoquiUser* loqui_protocol_create_user(LoquiProtocol *protocol);
LoquiAccount* loqui_protocol_create_account(LoquiProtocol *protocol, LoquiProfileAccount *profile);

G_END_DECLS

#endif /* __LOQUI_PROTOCOL_H__ */
