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

#include "loqui_profile_factory.h"
/*
enum {
	LAST_SIGNAL
};

static guint loqui_profile_factory_signals[LAST_SIGNAL] = { 0 };
*/

static void loqui_profile_factory_base_init(gpointer object_class);

static void
loqui_profile_factory_base_init(gpointer object_class)
{
        static gboolean initialized = FALSE;

        if (!initialized) {
/*
		loqui_profile_factory_signals[SIGNAL_APPEND_MESSAGE_TEXT] =
			g_signal_new("create_profile",
				     LOQUI_TYPE_PROFILE_FACTORY,
				     G_SIGNAL_RUN_LAST,
				     G_STRUCT_OFFSET(LoquiProfileFactoryIface, create_profile),
				     NULL, NULL,
				     g_cclosure_marshal_OBJECT__VOID,
				     G_TYPE_OBJECT, 0);
*/
	
                initialized = TRUE;
        }
}


GType
loqui_profile_factory_get_type(void)
{
        static GType type = 0;

        if (type == 0) {
                static const GTypeInfo info = {
                        sizeof (LoquiProfileFactoryIface),
                        loqui_profile_factory_base_init,   /* base_init */
                        NULL,   /* base_finalize */
                        NULL,   /* class_init */
                        NULL,   /* class_finalize */
                        NULL,   /* class_data */
                        0,
                        0,      /* n_preallocs */
                        NULL    /* instance_init */
                };
                type = g_type_register_static(G_TYPE_INTERFACE, "LoquiProfileFactory", &info, 0);
        }

        return type;
}

G_CONST_RETURN gchar *
loqui_profile_factory_get_type_name_static(LoquiProfileFactory *self)
{
	if (LOQUI_PROFILE_FACTORY_GET_IFACE(self)->get_type_name_static) {
		return LOQUI_PROFILE_FACTORY_GET_IFACE(self)->get_type_name_static(self);
	}

	return NULL;
}

LoquiProfile *
loqui_profile_factory_create_profile(LoquiProfileFactory *self)
{
	if (LOQUI_PROFILE_FACTORY_GET_IFACE(self)->create_profile) {
		return LOQUI_PROFILE_FACTORY_GET_IFACE(self)->create_profile(self);
	}

	return NULL;
}
