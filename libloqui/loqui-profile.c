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

#include "loqui-profile.h"
#include "gobject_utils.h"

enum {
	LAST_SIGNAL
};
/*
static guint loqui_profile_signals[LAST_SIGNAL] = { 0 };
*/

LOQUI_DEFINE_INTERFACE(LoquiProfile, loqui_profile);

static void
loqui_profile_base_init(gpointer object_class)
{
        static gboolean initialized = FALSE;

        if (!initialized) {
                initialized = TRUE;
        }
}

LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG1(LoquiProfile, loqui_profile, set_profile_type_name, const gchar *)
LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG0_WITH_RETURN(LoquiProfile, loqui_profile, get_profile_type_name, gchar *)

LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG2(LoquiProfile, loqui_profile, set_profile_value, const gchar *, const GValue *)
LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG2(LoquiProfile, loqui_profile, get_profile_value, const gchar *, GValue *)

LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG2_WITH_RETURN(LoquiProfile, loqui_profile, get_profile_value_spec, GParamSpec *, const gchar *, const gchar *)

LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG0_WITH_RETURN(LoquiProfile, loqui_profile, get_profile_key_list, GList *)
