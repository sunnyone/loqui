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

#include "loqui_profile.h"

enum {
	LAST_SIGNAL
};

static guint loqui_profile_signals[LAST_SIGNAL] = { 0 };

static void loqui_profile_base_init(gpointer object_class);

static void
loqui_profile_base_init(gpointer object_class)
{
        static gboolean initialized = FALSE;

        if (!initialized) {
                initialized = TRUE;
        }
}


GType
loqui_profile_get_type(void)
{
        static GType type = 0;

        if (type == 0) {
                static const GTypeInfo info = {
                        sizeof (LoquiProfileIface),
                        loqui_profile_base_init,   /* base_init */
                        NULL,   /* base_finalize */
                        NULL,   /* class_init */
                        NULL,   /* class_finalize */
                        NULL,   /* class_data */
                        0,
                        0,      /* n_preallocs */
                        NULL    /* instance_init */
                };
                type = g_type_register_static(G_TYPE_INTERFACE, "LoquiProfile", &info, 0);
        }

        return type;
}
