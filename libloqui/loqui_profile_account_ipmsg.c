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

#include "loqui_profile_account_ipmsg.h"
#include "ipmsg.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiProfileAccountIPMsgPrivate
{
};

static LoquiProfileAccountClass *parent_class = NULL;

/* static guint loqui_profile_account_ipmsg_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_profile_account_ipmsg_class_init(LoquiProfileAccountIPMsgClass *klass);
static void loqui_profile_account_ipmsg_init(LoquiProfileAccountIPMsg *profile);
static void loqui_profile_account_ipmsg_finalize(GObject *object);
static void loqui_profile_account_ipmsg_dispose(GObject *object);

static void loqui_profile_account_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_profile_account_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_profile_account_ipmsg_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProfileAccountIPMsgClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_profile_account_ipmsg_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProfileAccountIPMsg),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_profile_account_ipmsg_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_PROFILE_ACCOUNT,
					      "LoquiProfileAccountIPMsg",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_profile_account_ipmsg_finalize(GObject *object)
{
	LoquiProfileAccountIPMsg *profile;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT_IPMSG(object));

        profile = LOQUI_PROFILE_ACCOUNT_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(profile->priv);
}
static void 
loqui_profile_account_ipmsg_dispose(GObject *object)
{
	LoquiProfileAccountIPMsg *profile;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT_IPMSG(object));

        profile = LOQUI_PROFILE_ACCOUNT_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_profile_account_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiProfileAccountIPMsg *profile;        

        profile = LOQUI_PROFILE_ACCOUNT_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_profile_account_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiProfileAccountIPMsg *profile;        

        profile = LOQUI_PROFILE_ACCOUNT_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_profile_account_ipmsg_class_init(LoquiProfileAccountIPMsgClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_profile_account_ipmsg_finalize;
        object_class->dispose = loqui_profile_account_ipmsg_dispose;
        object_class->get_property = loqui_profile_account_ipmsg_get_property;
        object_class->set_property = loqui_profile_account_ipmsg_set_property;
}
static void 
loqui_profile_account_ipmsg_init(LoquiProfileAccountIPMsg *profile)
{
	LoquiProfileAccountIPMsgPrivate *priv;

	priv = g_new0(LoquiProfileAccountIPMsgPrivate, 1);

	profile->priv = priv;

	g_object_set(profile,
		     "port", IPMSG_DEFAULT_PORT,
		     NULL);
}
LoquiProfileAccountIPMsg*
loqui_profile_account_ipmsg_new(void)
{
        LoquiProfileAccountIPMsg *profile;
	LoquiProfileAccountIPMsgPrivate *priv;

	profile = g_object_new(loqui_profile_account_ipmsg_get_type(), NULL);
	
        priv = profile->priv;

        return profile;
}
