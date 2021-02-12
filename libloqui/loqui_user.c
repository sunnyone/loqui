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

#include "loqui_user.h"
#include <libloqui-intl.h>

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
	PROP_IDENTIFIER,
        LAST_PROP
};

static GObjectClass *parent_class = NULL;

#define AWAY_TYPE_ARRAY_DEFAULT_SIZE 10

/* static guint user_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_user_class_init(LoquiUserClass *klass);
static void loqui_user_init(LoquiUser *user);
static void loqui_user_finalize(GObject *object);
static void loqui_user_dispose(GObject *object);

static void loqui_user_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_user_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static GList *loqui_user_class_get_away_type_list_real(LoquiUserClass *user_class);

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
}
static void 
loqui_user_dispose(GObject *object)
{
	LoquiUser *user;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_USER(object));

        user = LOQUI_USER(object);

	LOQUI_G_FREE_UNLESS_NULL(user->nick);
	LOQUI_G_FREE_UNLESS_NULL(user->nick_key);
	LOQUI_G_FREE_UNLESS_NULL(user->username);
	LOQUI_G_FREE_UNLESS_NULL(user->hostname);
	LOQUI_G_FREE_UNLESS_NULL(user->realname);
	LOQUI_G_FREE_UNLESS_NULL(user->servername);
	LOQUI_G_FREE_UNLESS_NULL(user->away_message);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_user_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiUser *user;        
	gchar *tmp;

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
	case PROP_IDENTIFIER:
		tmp = loqui_user_get_identifier(user);
		g_value_set_string(value, tmp);
		g_free(tmp);
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
	klass->get_away_type_list = loqui_user_class_get_away_type_list_real;

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
	g_object_class_install_property(object_class,
					PROP_IDENTIFIER,
					g_param_spec_string("identifier",
							    _("Identifier"),
							    _("The string to determine the user"),
							    NULL, G_PARAM_READABLE));

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
}
LoquiUser*
loqui_user_new(void)
{
        LoquiUser *user;

	user = g_object_new(loqui_user_get_type(), NULL);
	
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
	g_return_val_if_fail(away_type >= 0, NULL);
	g_return_val_if_fail(away_type < user_class->away_type_array->len, NULL);

	info = g_ptr_array_index(user_class->away_type_array, away_type);
	
	return info;
}

/**
   @returns: GList contains LoquiAwayType and 0(can be used as separator). must be freed.
*/
GList *
loqui_user_class_get_away_type_list(LoquiUserClass *user_class)
{
	return user_class->get_away_type_list(user_class);
}

/* default implemetation */
static GList *
loqui_user_class_get_away_type_list_real(LoquiUserClass *user_class)
{
	LoquiAwayInfo *info;
	GList *list = NULL;
	int i;

	list = g_list_append(list, GINT_TO_POINTER(LOQUI_AWAY_TYPE_ONLINE));
	list = g_list_append(list, GINT_TO_POINTER(0));

	for (i = LOQUI_AWAY_TYPE_AWAY; i < user_class->away_type_array->len; i++) {
		info = g_ptr_array_index(user_class->away_type_array, i);
		list = g_list_append(list, GINT_TO_POINTER(info->away_type));
	}

	list = g_list_append(list, GINT_TO_POINTER(0));
	list = g_list_append(list, GINT_TO_POINTER(LOQUI_AWAY_TYPE_OFFLINE));

	return list;
}

LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiUser, loqui_user, idle_time, gint);
LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiUser, loqui_user, is_ignored, gboolean);
LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiUser, loqui_user, away, LoquiAwayType);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiUser, loqui_user, username);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiUser, loqui_user, hostname);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiUser, loqui_user, realname);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiUser, loqui_user, servername);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiUser, loqui_user, away_message);

LoquiBasicAwayType
loqui_user_get_basic_away(LoquiUser *user)
{
	LoquiAwayInfo *away_info;

        g_return_val_if_fail(user != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_USER(user), 0);
	
	if (user->away == LOQUI_AWAY_TYPE_UNKNOWN)
		return LOQUI_BASIC_AWAY_TYPE_UNKNOWN;

	away_info = loqui_user_class_away_type_get_info(LOQUI_USER_GET_CLASS(user), user->away);
	if (!away_info)
		return LOQUI_BASIC_AWAY_TYPE_UNKNOWN;

	return away_info->basic_away_type;
}

void
loqui_user_set_nick(LoquiUser *user, const gchar* nick)
{
	gchar *tmp;

	g_return_if_fail(user != NULL);
        g_return_if_fail(LOQUI_IS_USER(user));

	LOQUI_G_FREE_UNLESS_NULL(user->nick);
	LOQUI_G_FREE_UNLESS_NULL(user->nick_key);

	user->nick = g_strdup(nick);
	tmp = g_ascii_strdown(nick, -1);
	user->nick_key = g_utf8_collate_key(tmp, -1);
	g_free(tmp);

	g_object_notify(G_OBJECT(user), "nick");
}
G_CONST_RETURN gchar *
loqui_user_get_nick(LoquiUser *user)
{
        g_return_val_if_fail(user != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_USER(user), 0);

	return user->nick;
}

gchar *
loqui_user_get_identifier(LoquiUser *user)
{
	LoquiUserClass *klass;

        g_return_val_if_fail(user != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_USER(user), NULL);

	klass = LOQUI_USER_GET_CLASS(user);
        if (klass->get_identifier)
                return (* klass->get_identifier)(user);

	g_warning("Not implemented User#get_identifier");
	return NULL;
}
