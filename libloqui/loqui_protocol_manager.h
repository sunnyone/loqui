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
#ifndef __LOQUI_PROTOCOL_MANAGER_H__
#define __LOQUI_PROTOCOL_MANAGER_H__

#include <glib-object.h>

#include <libloqui/loqui-protocol.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_PROTOCOL_MANAGER                 (loqui_protocol_manager_get_type ())
#define LOQUI_PROTOCOL_MANAGER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROTOCOL_MANAGER, LoquiProtocolManager))
#define LOQUI_PROTOCOL_MANAGER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROTOCOL_MANAGER, LoquiProtocolManagerClass))
#define LOQUI_IS_PROTOCOL_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROTOCOL_MANAGER))
#define LOQUI_IS_PROTOCOL_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROTOCOL_MANAGER))
#define LOQUI_PROTOCOL_MANAGER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROTOCOL_MANAGER, LoquiProtocolManagerClass))

typedef struct _LoquiProtocolManager            LoquiProtocolManager;
typedef struct _LoquiProtocolManagerClass       LoquiProtocolManagerClass;

typedef struct _LoquiProtocolManagerPrivate     LoquiProtocolManagerPrivate;

struct _LoquiProtocolManager
{
        GObject parent;

	/* key: identifier (gchar *), value: protocol (LoquiProtocol *) */
        GHashTable *protocol_table;

        LoquiProtocolManagerPrivate *priv;
};

struct _LoquiProtocolManagerClass
{
        GObjectClass parent_class;
};


GType loqui_protocol_manager_get_type(void) G_GNUC_CONST;

LoquiProtocolManager* loqui_protocol_manager_new(void);
void loqui_protocol_manager_register(LoquiProtocolManager *manager, LoquiProtocol *protocol);
LoquiProtocol *loqui_protocol_manager_get_protocol(LoquiProtocolManager *manager, const gchar *identifier);

GList *loqui_protocol_manager_get_protocol_list(LoquiProtocolManager *manager); /* must be freed */

G_END_DECLS

#endif /* __LOQUI_PROTOCOL_MANAGER_H__ */
