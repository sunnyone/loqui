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

#include "loqui_user_ipmsg.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiUserIPMsgPrivate
{
};

static LoquiUserClass *parent_class = NULL;

/* static guint loqui_user_ipmsg_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_user_ipmsg_class_init(LoquiUserIPMsgClass *klass);
static void loqui_user_ipmsg_init(LoquiUserIPMsg *user);
static void loqui_user_ipmsg_finalize(GObject *object);
static void loqui_user_ipmsg_dispose(GObject *object);

static void loqui_user_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_user_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static gchar *loqui_user_ipmsg_get_identifier(LoquiUser *user);

GType
loqui_user_ipmsg_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiUserIPMsgClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_user_ipmsg_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiUserIPMsg),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_user_ipmsg_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_USER,
					      "LoquiUserIPMsg",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_user_ipmsg_finalize(GObject *object)
{
	LoquiUserIPMsg *user;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_USER_IPMSG(object));

        user = LOQUI_USER_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(user->priv);
}
static void 
loqui_user_ipmsg_dispose(GObject *object)
{
	LoquiUserIPMsg *user;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_USER_IPMSG(object));

        user = LOQUI_USER_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_user_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiUserIPMsg *user;        

        user = LOQUI_USER_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_user_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiUserIPMsg *user;        

        user = LOQUI_USER_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_user_ipmsg_class_init(LoquiUserIPMsgClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_user_ipmsg_finalize;
        object_class->dispose = loqui_user_ipmsg_dispose;
        object_class->get_property = loqui_user_ipmsg_get_property;
        object_class->set_property = loqui_user_ipmsg_set_property;
	LOQUI_USER_CLASS(klass)->get_identifier = loqui_user_ipmsg_get_identifier;
}
static void 
loqui_user_ipmsg_init(LoquiUserIPMsg *user)
{
	LoquiUserIPMsgPrivate *priv;

	priv = g_new0(LoquiUserIPMsgPrivate, 1);

	user->priv = priv;
}

static gchar *
loqui_user_ipmsg_get_identifier(LoquiUser *user)
{
        g_return_val_if_fail(user != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_USER_IPMSG(user), NULL);

	return g_strdup("");
}
LoquiUserIPMsg*
loqui_user_ipmsg_new(void)
{
        LoquiUserIPMsg *user;
	LoquiUserIPMsgPrivate *priv;

	user = g_object_new(loqui_user_ipmsg_get_type(), NULL);
	
        priv = user->priv;

        return user;
}
