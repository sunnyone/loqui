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
#ifndef __LOQUI_PROTOCOL_IRC_H__
#define __LOQUI_PROTOCOL_IRC_H__

#include <glib-object.h>
#include "loqui_protocol.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_PROTOCOL_IRC                 (loqui_protocol_irc_get_type ())
#define LOQUI_PROTOCOL_IRC(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROTOCOL_IRC, LoquiProtocolIRC))
#define LOQUI_PROTOCOL_IRC_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROTOCOL_IRC, LoquiProtocolIRCClass))
#define LOQUI_IS_PROTOCOL_IRC(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROTOCOL_IRC))
#define LOQUI_IS_PROTOCOL_IRC_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROTOCOL_IRC))
#define LOQUI_PROTOCOL_IRC_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROTOCOL_IRC, LoquiProtocolIRCClass))

typedef struct _LoquiProtocolIRC            LoquiProtocolIRC;
typedef struct _LoquiProtocolIRCClass       LoquiProtocolIRCClass;

typedef struct _LoquiProtocolIRCPrivate     LoquiProtocolIRCPrivate;

struct _LoquiProtocolIRC
{
        LoquiProtocol parent;
        
        LoquiProtocolIRCPrivate *priv;
};

struct _LoquiProtocolIRCClass
{
        LoquiProtocolClass parent_class;
};


GType loqui_protocol_irc_get_type(void) G_GNUC_CONST;

LoquiProtocol* loqui_protocol_irc_get(void);

G_END_DECLS

#endif /* __LOQUI_PROTOCOL_IRC_H__ */
