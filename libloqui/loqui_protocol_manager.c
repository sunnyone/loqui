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

#include "loqui_protocol_manager.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiProtocolManagerPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_protocol_manager_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_protocol_manager_class_init(LoquiProtocolManagerClass *klass);
static void loqui_protocol_manager_init(LoquiProtocolManager *manager);
static void loqui_protocol_manager_finalize(GObject *object);
static void loqui_protocol_manager_dispose(GObject *object);

static void loqui_protocol_manager_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_protocol_manager_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_protocol_manager_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProtocolManagerClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_protocol_manager_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProtocolManager),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_protocol_manager_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiProtocolManager",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_protocol_manager_finalize(GObject *object)
{
	LoquiProtocolManager *manager;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL_MANAGER(object));

        manager = LOQUI_PROTOCOL_MANAGER(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(manager->priv);
}
static void 
loqui_protocol_manager_dispose(GObject *object)
{
	LoquiProtocolManager *manager;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROTOCOL_MANAGER(object));

        manager = LOQUI_PROTOCOL_MANAGER(object);

	if (manager->protocol_table) {
		g_hash_table_destroy(manager->protocol_table);
		manager->protocol_table = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_protocol_manager_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiProtocolManager *manager;        

        manager = LOQUI_PROTOCOL_MANAGER(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_protocol_manager_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiProtocolManager *manager;        

        manager = LOQUI_PROTOCOL_MANAGER(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_protocol_manager_class_init(LoquiProtocolManagerClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_protocol_manager_finalize;
        object_class->dispose = loqui_protocol_manager_dispose;
        object_class->get_property = loqui_protocol_manager_get_property;
        object_class->set_property = loqui_protocol_manager_set_property;
}
static void 
loqui_protocol_manager_init(LoquiProtocolManager *manager)
{
	LoquiProtocolManagerPrivate *priv;

	priv = g_new0(LoquiProtocolManagerPrivate, 1);

	manager->priv = priv;
	
	manager->protocol_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, g_object_unref);
}
LoquiProtocolManager*
loqui_protocol_manager_new(void)
{
        LoquiProtocolManager *manager;
	LoquiProtocolManagerPrivate *priv;

	manager = g_object_new(loqui_protocol_manager_get_type(), NULL);
	
        priv = manager->priv;

        return manager;
}
void
loqui_protocol_manager_register(LoquiProtocolManager *manager, LoquiProtocol *protocol)
{
	const gchar *identifier;

	identifier = loqui_protocol_get_identifier(protocol);
	if (!identifier) {
		g_warning("Protocol identifier is NULL");
		return;
	}

	if (g_hash_table_lookup(manager->protocol_table, identifier)) {
		g_warning("Protocol '%s' is already registered", identifier);
		return;
	}

	g_hash_table_insert(manager->protocol_table, g_strdup(identifier), protocol);
}
LoquiProtocol *
loqui_protocol_manager_get_protocol(LoquiProtocolManager *manager, const gchar *identifier)
{
	return g_hash_table_lookup(manager->protocol_table, identifier);
}
