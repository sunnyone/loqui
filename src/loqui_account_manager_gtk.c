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

#include "loqui_account_manager_gtk.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiAccountManagerGtkPrivate
{
};

static LoquiAccountManagerClass *parent_class = NULL;

/* static guint loqui_account_manager_gtk_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_account_manager_gtk_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_account_manager_gtk_class_init(LoquiAccountManagerGtkClass *klass);
static void loqui_account_manager_gtk_init(LoquiAccountManagerGtk *manager);
static void loqui_account_manager_gtk_finalize(GObject *object);
static void loqui_account_manager_gtk_dispose(GObject *object);

static void loqui_account_manager_gtk_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_manager_gtk_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_account_manager_gtk_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountManagerGtkClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_manager_gtk_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccountManagerGtk),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_manager_gtk_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_ACCOUNT_MANAGER,
					      "LoquiAccountManagerGtk",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_account_manager_gtk_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_account_manager_gtk_finalize(GObject *object)
{
	LoquiAccountManagerGtk *manager;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER_GTK(object));

        manager = LOQUI_ACCOUNT_MANAGER_GTK(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(manager->priv);
}
static void 
loqui_account_manager_gtk_dispose(GObject *object)
{
	LoquiAccountManagerGtk *manager;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER_GTK(object));

        manager = LOQUI_ACCOUNT_MANAGER_GTK(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_account_manager_gtk_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiAccountManagerGtk *manager;        

        manager = LOQUI_ACCOUNT_MANAGER_GTK(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_account_manager_gtk_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiAccountManagerGtk *manager;        

        manager = LOQUI_ACCOUNT_MANAGER_GTK(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_account_manager_gtk_class_init(LoquiAccountManagerGtkClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_account_manager_gtk_constructor; 
        object_class->finalize = loqui_account_manager_gtk_finalize;
        object_class->dispose = loqui_account_manager_gtk_dispose;
        object_class->get_property = loqui_account_manager_gtk_get_property;
        object_class->set_property = loqui_account_manager_gtk_set_property;
}
static void 
loqui_account_manager_gtk_init(LoquiAccountManagerGtk *manager)
{
	LoquiAccountManagerGtkPrivate *priv;

	priv = g_new0(LoquiAccountManagerGtkPrivate, 1);

	manager->priv = priv;
}
LoquiAccountManagerGtk*
loqui_account_manager_gtk_new(LoquiProtocolManager *pmanag)
{
        LoquiAccountManagerGtk *manager;
	LoquiAccountManagerGtkPrivate *priv;

	manager = g_object_new(loqui_account_manager_gtk_get_type(), NULL);
	
        priv = manager->priv;

	g_object_ref(pmanag);
	LOQUI_ACCOUNT_MANAGER(manager)->protocol_manager = pmanag;

        return manager;
}
