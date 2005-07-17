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

#include <libloqui-intl.h>
#include "loqui_codeconv.h"

#include "loqui_profile_account_irc.h"
#include "loqui_protocol_irc.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_REALNAME,
	PROP_USERINFO,
	PROP_AUTOJOIN,
	PROP_QUIT_MESSAGE,
        LAST_PROP
};

struct _LoquiProfileAccountIRCPrivate
{
};

#define IRC_DEFAULT_PORT 6667

static LoquiProfileAccountClass *parent_class = NULL;

/* static guint loqui_profile_account_irc_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_profile_account_irc_class_init(LoquiProfileAccountIRCClass *klass);
static void loqui_profile_account_irc_init(LoquiProfileAccountIRC *profile);
static void loqui_profile_account_irc_finalize(GObject *object);
static void loqui_profile_account_irc_dispose(GObject *object);

static void loqui_profile_account_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_profile_account_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_profile_account_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProfileAccountIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_profile_account_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProfileAccountIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_profile_account_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_PROFILE_ACCOUNT,
					      "LoquiProfileAccountIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_profile_account_irc_finalize(GObject *object)
{
	LoquiProfileAccountIRC *profile;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT_IRC(object));

        profile = LOQUI_PROFILE_ACCOUNT_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(profile->priv);
}
static void 
loqui_profile_account_irc_dispose(GObject *object)
{
	LoquiProfileAccountIRC *profile;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT_IRC(object));

        profile = LOQUI_PROFILE_ACCOUNT_IRC(object);

	LOQUI_G_FREE_UNLESS_NULL(profile->realname);
	LOQUI_G_FREE_UNLESS_NULL(profile->userinfo);
	LOQUI_G_FREE_UNLESS_NULL(profile->autojoin);
	
        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_profile_account_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiProfileAccountIRC *profile;        

        profile = LOQUI_PROFILE_ACCOUNT_IRC(object);

        switch (param_id) {
	case PROP_REALNAME:
		g_value_set_string(value, loqui_profile_account_irc_get_realname(profile));
		break;
	case PROP_USERINFO:
		g_value_set_string(value, loqui_profile_account_irc_get_userinfo(profile));
		break;
	case PROP_AUTOJOIN:
		g_value_set_string(value, loqui_profile_account_irc_get_autojoin(profile));
		break;
	case PROP_QUIT_MESSAGE:
		g_value_set_string(value, loqui_profile_account_irc_get_quit_message(profile));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_profile_account_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiProfileAccountIRC *profile;        

        profile = LOQUI_PROFILE_ACCOUNT_IRC(object);

        switch (param_id) {
	case PROP_REALNAME:
		loqui_profile_account_irc_set_realname(profile, g_value_get_string(value));
		break;
	case PROP_USERINFO:
		loqui_profile_account_irc_set_userinfo(profile, g_value_get_string(value));
		break;
	case PROP_AUTOJOIN:
		loqui_profile_account_irc_set_autojoin(profile, g_value_get_string(value));
		break;
	case PROP_QUIT_MESSAGE:
		loqui_profile_account_irc_set_quit_message(profile, g_value_get_string(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_profile_account_irc_class_init(LoquiProfileAccountIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_profile_account_irc_finalize;
        object_class->dispose = loqui_profile_account_irc_dispose;
        object_class->get_property = loqui_profile_account_irc_get_property;
        object_class->set_property = loqui_profile_account_irc_set_property;
        
	g_object_class_install_property(object_class,
					PROP_REALNAME,
					g_param_spec_string("realname",
							    _("Realname"),
							    _("Realname"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_USERINFO,
					g_param_spec_string("userinfo",
							    _("User information"),
							    _("User information used with CTCP USERINFO"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_AUTOJOIN,
					g_param_spec_string("autojoin",
							    _("Autojoin channels"),
							    _("Channels which are joined automatically"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_QUIT_MESSAGE,
					g_param_spec_string("quit_message",
							    _("Quit Message"),
							    _("Default quit message"),
							    NULL, G_PARAM_READWRITE));
}
static void 
loqui_profile_account_irc_init(LoquiProfileAccountIRC *profile)
{
	LoquiProfileAccountIRCPrivate *priv;

	priv = g_new0(LoquiProfileAccountIRCPrivate, 1);

	profile->priv = priv;

	/* set default */
	g_object_set(profile,
		     "port", 6667,
		     "username", "loqui",
		     "quit_message", "Loqui",
		     NULL);
}
LoquiProfileAccountIRC*
loqui_profile_account_irc_new(void)
{
        LoquiProfileAccountIRC *profile;
	LoquiProfileAccountIRCPrivate *priv;

	profile = g_object_new(loqui_profile_account_irc_get_type(), NULL);
	
        priv = profile->priv;

	LOQUI_PROFILE_ACCOUNT(profile)->protocol = loqui_protocol_irc_get();

        return profile;
}

LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiProfileAccountIRC, loqui_profile_account_irc, realname);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiProfileAccountIRC, loqui_profile_account_irc, userinfo);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiProfileAccountIRC, loqui_profile_account_irc, autojoin);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiProfileAccountIRC, loqui_profile_account_irc, quit_message);
