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

#include "loqui_user_irc.h"
#include "intl.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_HOP_COUNT,
	PROP_SERVER_INFO,
	PROP_JOINED_CHANNELS_STRING,
	PROP_IS_IRC_OPERATOR,
        LAST_PROP
};

struct _LoquiUserIRCPrivate
{
};

static LoquiUserClass *parent_class = NULL;

/* static guint loqui_user_irc_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_user_irc_class_init(LoquiUserIRCClass *klass);
static void loqui_user_irc_init(LoquiUserIRC *user);
static void loqui_user_irc_finalize(GObject *object);
static void loqui_user_irc_dispose(GObject *object);

static void loqui_user_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_user_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_user_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiUserIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_user_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiUserIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_user_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_USER,
					      "LoquiUserIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_user_irc_finalize(GObject *object)
{
	LoquiUserIRC *user;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_USER_IRC(object));

        user = LOQUI_USER_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(user->priv);
}
static void 
loqui_user_irc_dispose(GObject *object)
{
	LoquiUserIRC *user;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_USER_IRC(object));

        user = LOQUI_USER_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_user_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiUserIRC *user;        

        user = LOQUI_USER_IRC(object);

        switch (param_id) {
	case PROP_SERVER_INFO:
		g_value_set_string(value, user->server_info);
		break;
	case PROP_JOINED_CHANNELS_STRING:
		g_value_set_string(value, user->joined_channels_string);
		break;
	case PROP_IS_IRC_OPERATOR:
		g_value_set_boolean(value, user->is_irc_operator);
		break;
	case PROP_HOP_COUNT:
		g_value_set_uint(value, user->hop_count);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_user_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiUserIRC *user;        

        user = LOQUI_USER_IRC(object);

        switch (param_id) {
	case PROP_SERVER_INFO:
		loqui_user_irc_set_server_info(user, g_value_get_string(value));
		break;
	case PROP_JOINED_CHANNELS_STRING:
		loqui_user_irc_set_joined_channels_string(user, g_value_get_string(value));
		break;
	case PROP_IS_IRC_OPERATOR:
		loqui_user_irc_set_is_irc_operator(user, g_value_get_boolean(value));
		break;
	case PROP_HOP_COUNT:
		loqui_user_irc_set_hop_count(user, g_value_get_uint(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_user_irc_class_init(LoquiUserIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_user_irc_finalize;
        object_class->dispose = loqui_user_irc_dispose;
        object_class->get_property = loqui_user_irc_get_property;
        object_class->set_property = loqui_user_irc_set_property;

	g_object_class_install_property(object_class,
					PROP_SERVER_INFO,
					g_param_spec_string("server_info",
							    _("ServerInfo"),
							    _("Server information"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_JOINED_CHANNELS_STRING,
					g_param_spec_string("joined_channels_string",
							    _("Joined channels string"),
							    _("String of channels the user joined (from whois)"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_HOP_COUNT,
					g_param_spec_uint("hop_count",
							  _("Hop Count"),
							  _("Hop count"),
							  0, G_MAXUINT,
							  0, G_PARAM_READWRITE));
}
static void 
loqui_user_irc_init(LoquiUserIRC *user)
{
	LoquiUserIRCPrivate *priv;

	priv = g_new0(LoquiUserIRCPrivate, 1);

	user->priv = priv;
}
LoquiUserIRC*
loqui_user_irc_new(void)
{
        LoquiUserIRC *user;
	LoquiUserIRCPrivate *priv;

	user = g_object_new(loqui_user_irc_get_type(), NULL);
	
        priv = user->priv;

        return user;
}

LOQUI_USER_IRC_ACCESSOR_GENERIC(guint, hop_count);
LOQUI_USER_IRC_ACCESSOR_GENERIC(gboolean, is_irc_operator);
LOQUI_USER_IRC_ACCESSOR_STRING(server_info);
LOQUI_USER_IRC_ACCESSOR_STRING(joined_channels_string);