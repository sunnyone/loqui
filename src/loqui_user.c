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

#include "loqui_user.h"
#include "intl.h"

#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_NICK,
	PROP_USERNAME,
	PROP_HOSTNAME,
	PROP_REALNAME,
	PROP_SERVERNAME,
	PROP_AWAY,
	PROP_AWAY_MESSAGE,
	PROP_IDLE_TIME,
	PROP_IS_IGNORED,
        LAST_PROP
};

struct _LoquiUserPrivate
{
};

static GObjectClass *parent_class = NULL;

#define AWAY_TYPE_ARRAY_DEFAULT_SIZE 10

/* static guint loqui_user_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_user_class_init(LoquiUserClass *klass);
static void loqui_user_init(LoquiUser *user);
static void loqui_user_finalize(GObject *object);
static void loqui_user_dispose(GObject *object);

static void loqui_user_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_user_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_user_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiUserClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_user_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiUser),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_user_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiUser",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_user_finalize(GObject *object)
{
	LoquiUser *user;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_USER(object));

        user = LOQUI_USER(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(user->priv);
}
static void 
loqui_user_dispose(GObject *object)
{
	LoquiUser *user;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_USER(object));

        user = LOQUI_USER(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_user_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiUser *user;        

        user = LOQUI_USER(object);

        switch (param_id) {
	case PROP_NICK:
		g_value_set_string(value, user->nick);
		break;
	case PROP_USERNAME:
		g_value_set_string(value, user->username);
		break;
	case PROP_HOSTNAME:
		g_value_set_string(value, user->hostname);
		break;
	case PROP_REALNAME:
		g_value_set_string(value, user->realname);
		break;
	case PROP_SERVERNAME:
		g_value_set_string(value, user->servername);
		break;
	case PROP_AWAY_MESSAGE:
		g_value_set_string(value, user->away_message);
		break;
	case PROP_AWAY:
		g_value_set_int(value, user->away);
		break;
	case PROP_IDLE_TIME:
		g_value_set_int(value, user->idle_time);
		break;
	case PROP_IS_IGNORED:
		g_value_set_boolean(value, user->is_ignored);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_user_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiUser *user;        

        user = LOQUI_USER(object);

        switch (param_id) {
	case PROP_NICK:
		loqui_user_set_nick(user, g_value_get_string(value));
		break;
	case PROP_USERNAME:
		loqui_user_set_username(user, g_value_get_string(value));
		break;
	case PROP_HOSTNAME:
		loqui_user_set_hostname(user, g_value_get_string(value));
		break;
	case PROP_REALNAME:
		loqui_user_set_realname(user, g_value_get_string(value));
		break;
	case PROP_SERVERNAME:
		loqui_user_set_servername(user, g_value_get_string(value));
		break;
	case PROP_AWAY_MESSAGE:
		loqui_user_set_away_message(user, g_value_get_string(value));
		break;
	case PROP_AWAY:
		loqui_user_set_away(user, g_value_get_int(value));
		break;
	case PROP_IDLE_TIME:
		loqui_user_set_idle_time(user, g_value_get_int(value));
		break;
	case PROP_IS_IGNORED:
		loqui_user_set_is_ignored(user, g_value_get_boolean(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_user_class_init(LoquiUserClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	LoquiAwayType atype;
	gpointer null_ptr = NULL;

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_user_finalize;
        object_class->dispose = loqui_user_dispose;
        object_class->get_property = loqui_user_get_property;
        object_class->set_property = loqui_user_set_property;

	g_object_class_install_property(object_class,
					PROP_NICK,
					g_param_spec_string("nick",
							    _("Nick"),
							    _("Nick name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_USERNAME,
					g_param_spec_string("username",
							    _("Username"),
							    _("Username"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_HOSTNAME,
					g_param_spec_string("hostname",
							    _("Hostname"),
							    _("Hostname"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_REALNAME,
					g_param_spec_string("realname",
							    _("Realname"),
							    _("Realname"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_SERVERNAME,
					g_param_spec_string("servername",
							    _("Server name"),
							    _("Server name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_AWAY_MESSAGE,
					g_param_spec_string("away_message",
							    _("Away message"),
							    _("Away message"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_AWAY,
					g_param_spec_int("away",
							 _("Away type"),
							 _("Away type (LoquiAwayType)"),
							 1, G_MAXINT,
							 LOQUI_AWAY_TYPE_ONLINE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IDLE_TIME,
					g_param_spec_int("idle_time",
							 _("Idle time"),
							 _("Idle time (from epoch seconds)"),
							 G_MININT, G_MAXINT,
							 0, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IS_IGNORED,
					g_param_spec_boolean("is_ignored",
							     _("Ignored"),
							     _("Ignored or not"),
							     FALSE, G_PARAM_READWRITE));

	klass->away_type_array = g_ptr_array_sized_new(AWAY_TYPE_ARRAY_DEFAULT_SIZE);
	g_ptr_array_add(klass->away_type_array, null_ptr);

	atype = loqui_user_class_install_away_type(klass, LOQUI_BASIC_AWAY_TYPE_ONLINE, "online", _("Online"));
	g_assert(atype == LOQUI_AWAY_TYPE_ONLINE);

	atype = loqui_user_class_install_away_type(klass, LOQUI_BASIC_AWAY_TYPE_OFFLINE, "offline", _("Offline"));
	g_assert(atype == LOQUI_AWAY_TYPE_OFFLINE);

	atype = loqui_user_class_install_away_type(klass, LOQUI_BASIC_AWAY_TYPE_AWAY, "away", _("Away"));
	g_assert(atype == LOQUI_AWAY_TYPE_AWAY);
}

static void 
loqui_user_init(LoquiUser *user)
{
	LoquiUserPrivate *priv;

	priv = g_new0(LoquiUserPrivate, 1);

	user->priv = priv;
}
LoquiUser*
loqui_user_new(void)
{
        LoquiUser *user;
	LoquiUserPrivate *priv;

	user = g_object_new(loqui_user_get_type(), NULL);
	
        priv = user->priv;

        return user;
}

/**
   loqui_user_class_install_away_type:
   @name: away name like "protocol-busy". must be unique.
   @nick: away type nick, like "Online".
*/
LoquiAwayType
loqui_user_class_install_away_type(LoquiUserClass *user_class,
				   LoquiBasicAwayType basic_away_type,
				   const gchar *name,
				   const gchar *nick)
{
	LoquiAwayInfo *info;
	guint i;

        g_return_val_if_fail(user_class != NULL, LOQUI_AWAY_TYPE_UNKNOWN);
        g_return_val_if_fail(LOQUI_IS_USER_CLASS(user_class), LOQUI_AWAY_TYPE_UNKNOWN);
	g_return_val_if_fail(name != NULL, LOQUI_AWAY_TYPE_UNKNOWN);
	g_return_val_if_fail(nick != NULL, LOQUI_AWAY_TYPE_UNKNOWN);

	/* if zero, uninitialized */
	g_assert(user_class->away_type_array->len > 0);
	
	for (i = 0; i < user_class->away_type_array->len; i++) {
		info = g_ptr_array_index(user_class->away_type_array, i);
		if(!info)
			continue;

		if (strcmp(info->name, name) == 0)
			return LOQUI_AWAY_TYPE_UNKNOWN;
	}

	info = g_new0(LoquiAwayInfo, 1);
	
	info->basic_away_type = basic_away_type;
	info->name = g_strdup(name);
	info->nick = g_strdup(nick);

	g_ptr_array_add(user_class->away_type_array, info);
	
	info->away_type = user_class->away_type_array->len - 1;

	return info->away_type;
}

LoquiAwayInfo *
loqui_user_class_away_type_get_info(LoquiUserClass *user_class, LoquiAwayType away_type)
{
	LoquiAwayInfo *info;
	
        g_return_val_if_fail(user_class != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_USER_CLASS(user_class), NULL);
	
	if (away_type < 0 || away_type >= user_class->away_type_array->len)
		return LOQUI_AWAY_TYPE_UNKNOWN;

	info = g_ptr_array_index(user_class->away_type_array, away_type);
	
	return info;
}

LOQUI_USER_ACCESSOR_GENERIC(gint, idle_time);
LOQUI_USER_ACCESSOR_GENERIC(gboolean, is_ignored);
LOQUI_USER_ACCESSOR_GENERIC(LoquiAwayType, away);
LOQUI_USER_ACCESSOR_STRING(nick);
LOQUI_USER_ACCESSOR_STRING(username);
LOQUI_USER_ACCESSOR_STRING(hostname);
LOQUI_USER_ACCESSOR_STRING(realname);
LOQUI_USER_ACCESSOR_STRING(servername);
LOQUI_USER_ACCESSOR_STRING(away_message);

LoquiBasicAwayType
loqui_user_get_basic_away(LoquiUser *user)
{
	LoquiAwayInfo *away_info;

        g_return_val_if_fail(user != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_USER(user), 0);
	
	if (user->away == LOQUI_AWAY_TYPE_UNKNOWN)
		return LOQUI_BASIC_AWAY_TYPE_UNKNOWN;

	away_info = loqui_user_class_away_type_get_info(LOQUI_USER_GET_CLASS(user), user->away);
	if (away_info)
		return LOQUI_BASIC_AWAY_TYPE_UNKNOWN;

	return away_info->basic_away_type;
}
