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
#ifndef __LOQUI_PROTOCOL_MSN_H__
#define __LOQUI_PROTOCOL_MSN_H__

#include <glib-object.h>
#include "loqui_protocol.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_PROTOCOL_MSN                 (loqui_protocol_msn_get_type ())
#define LOQUI_PROTOCOL_MSN(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROTOCOL_MSN, LoquiProtocolMSN))
#define LOQUI_PROTOCOL_MSN_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROTOCOL_MSN, LoquiProtocolMSNClass))
#define LOQUI_IS_PROTOCOL_MSN(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROTOCOL_MSN))
#define LOQUI_IS_PROTOCOL_MSN_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROTOCOL_MSN))
#define LOQUI_PROTOCOL_MSN_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROTOCOL_MSN, LoquiProtocolMSNClass))

typedef struct _LoquiProtocolMSN            LoquiProtocolMSN;
typedef struct _LoquiProtocolMSNClass       LoquiProtocolMSNClass;

typedef struct _LoquiProtocolMSNPrivate     LoquiProtocolMSNPrivate;

struct _LoquiProtocolMSN
{
        LoquiProtocol parent;
        
        LoquiProtocolMSNPrivate *priv;
};

struct _LoquiProtocolMSNClass
{
        LoquiProtocolClass parent_class;
};


GType loqui_protocol_msn_get_type(void) G_GNUC_CONST;

LoquiProtocolMSN* loqui_protocol_msn_new(void);
LoquiProtocol* loqui_protocol_msn_get(void);

G_END_DECLS

#endif /* __LOQUI_PROTOCOL_MSN_H__ */
