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
#include "loqui_profile_account.h"

#include "loqui_protocol.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_NAME,
	PROP_USE,
	
	PROP_NICK,
	
	PROP_SERVERNAME,
	PROP_PORT,
	
	PROP_USERNAME,
	PROP_PASSWORD,

	PROP_CODECONV_MODE,
	PROP_CODECONV_ITEM_NAME,
	PROP_CODESET,

	PROP_NICK_LIST,
	
        LAST_PROP
};

struct _LoquiProfileAccountPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_profile_account_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_profile_account_class_init(LoquiProfileAccountClass *klass);
static void loqui_profile_account_init(LoquiProfileAccount *account);
static void loqui_profile_account_finalize(GObject *object);
static void loqui_profile_account_dispose(GObject *object);

static void loqui_profile_account_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_profile_account_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_profile_account_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProfileAccountClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_profile_account_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProfileAccount),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_profile_account_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiProfileAccount",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_profile_account_finalize(GObject *object)
{
	LoquiProfileAccount *profile;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT(object));

        profile = LOQUI_PROFILE_ACCOUNT(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(profile->priv);
}
static void 
loqui_profile_account_dispose(GObject *object)
{
	LoquiProfileAccount *profile;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT(object));

        profile = LOQUI_PROFILE_ACCOUNT(object);

	G_FREE_UNLESS_NULL(profile->name);
	G_FREE_UNLESS_NULL(profile->nick);
	G_FREE_UNLESS_NULL(profile->servername);
	G_FREE_UNLESS_NULL(profile->username);
	G_FREE_UNLESS_NULL(profile->password);
		
        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_profile_account_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiProfileAccount *profile;        
	GValueArray *value_array;
	GList *tmp_list, *cur;
	GValue tmp_val = {0, };
	
        profile = LOQUI_PROFILE_ACCOUNT(object);

        switch (param_id) {
	case PROP_USE:
		g_value_set_boolean(value, profile->use);
		break;
	case PROP_NAME:
		g_value_set_string(value, loqui_profile_account_get_name(profile));
		break;
	case PROP_NICK:
		g_value_set_string(value, loqui_profile_account_get_nick(profile));
		break;
	case PROP_SERVERNAME:
		g_value_set_string(value, loqui_profile_account_get_servername(profile));
		break;
	case PROP_PORT:
		g_value_set_int(value, loqui_profile_account_get_port(profile));
		break;
	case PROP_USERNAME:
		g_value_set_string(value, loqui_profile_account_get_username(profile));
		break;
	case PROP_PASSWORD:
		g_value_set_string(value, loqui_profile_account_get_password(profile));
		break;
	case PROP_CODECONV_MODE:
		g_value_set_int(value, loqui_profile_account_get_codeconv_mode(profile));
		break;
	case PROP_CODECONV_ITEM_NAME:
		g_value_set_string(value, loqui_profile_account_get_codeconv_item_name(profile));
		break;	
	case PROP_CODESET:
		g_value_set_string(value, loqui_profile_account_get_codeset(profile));
		break;	
	case PROP_NICK_LIST:
		tmp_list = loqui_profile_account_get_nick_list(profile);
		value_array = g_value_array_new(g_list_length(tmp_list));
		for (cur = tmp_list; cur != NULL; cur = cur->next) {
			g_value_init(&tmp_val, G_TYPE_STRING);
			g_value_set_string(&tmp_val, cur->data);
			g_value_array_append(value_array, &tmp_val);
			g_value_unset(&tmp_val);
		}
		g_value_set_boxed(value, value_array);
		g_value_array_free(value_array);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_profile_account_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiProfileAccount *profile;        
	GValueArray *value_array;
	GList *tmp_list;
	int i;
	GValue *tmp_val;        

        profile = LOQUI_PROFILE_ACCOUNT(object);

        switch (param_id) {
	case PROP_USE:
		profile->use = g_value_get_boolean(value);
		break;
       	case PROP_NAME:
		loqui_profile_account_set_name(profile, g_value_get_string(value));
		break;
	case PROP_NICK:
		loqui_profile_account_set_nick(profile, g_value_get_string(value));
		break;
	case PROP_SERVERNAME:
		loqui_profile_account_set_servername(profile, g_value_get_string(value));
		break;
	case PROP_PORT:
		loqui_profile_account_set_port(profile, g_value_get_int(value));
		break;
	case PROP_USERNAME:
		loqui_profile_account_set_username(profile, g_value_get_string(value));
		break;
	case PROP_PASSWORD:
		loqui_profile_account_set_password(profile, g_value_get_string(value));
		break;
	case PROP_CODECONV_MODE:
		loqui_profile_account_set_codeconv_mode(profile, g_value_get_int(value));
		break;
	case PROP_CODECONV_ITEM_NAME:
		loqui_profile_account_set_codeconv_item_name(profile, g_value_get_string(value));
		break;
	case PROP_CODESET:
		loqui_profile_account_set_codeset(profile, g_value_get_string(value));
		break;
	case PROP_NICK_LIST:
		value_array = g_value_get_boxed(value);
		
		tmp_list = NULL;
		for (i = 0; i < value_array->n_values; i++) {
			tmp_val = g_value_array_get_nth(value_array, i);
			tmp_list = g_list_prepend(tmp_list, g_value_dup_string(tmp_val));
			g_value_unset(tmp_val);
		}
		loqui_profile_account_set_nick_list(profile, g_list_reverse(tmp_list));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_profile_account_class_init(LoquiProfileAccountClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_profile_account_finalize;
        object_class->dispose = loqui_profile_account_dispose;
        object_class->get_property = loqui_profile_account_get_property;
        object_class->set_property = loqui_profile_account_set_property;

	g_object_class_install_property(object_class,
					PROP_NAME,
					g_param_spec_string("name",
							    _("Name"),
							    _("Account name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_NICK,
					g_param_spec_string("nick",
							    _("Nick"),
							    _("Nickname at first"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_USE,
					g_param_spec_boolean("use",
							     _("Use account"),
							     _("Whether or not to connect this account by default"),
							     TRUE, G_PARAM_READWRITE));
	
	
	g_object_class_install_property(object_class,
					PROP_SERVERNAME,
					g_param_spec_string("servername",
							    _("Server name"),
							    _("Server name"),
							    "irc.example.com",
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_PORT,
					g_param_spec_int("port",
							_("Server name"),
							_("Server name"),
							0,
							G_MAXINT,
							0,
							G_PARAM_READWRITE));
							    							    
	g_object_class_install_property(object_class,
					PROP_USERNAME,
					g_param_spec_string("username",
							    _("Username"),
							    _("Username"),
							    NULL,
							    G_PARAM_READWRITE));	
	g_object_class_install_property(object_class,
					PROP_PASSWORD,
					g_param_spec_string("password",
							    _("Password"),
							    _("Password"),
							    NULL,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_CODECONV_MODE,
					g_param_spec_int("codeconv_mode",
							 _("CodeConv mode"),
							 _("CodeConv mode"),
							 0,
							 G_MAXINT,
							 0,
							 G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_CODECONV_ITEM_NAME,
					g_param_spec_string("codeconv_item_name",
							    _("CodeConv item name"),
							    _("COdeConv item name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_CODESET,
					g_param_spec_string("codeset",
							    _("Codeset"),
							    _("Codeset"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_NICK_LIST,
					g_param_spec_value_array("nick_list",
							         _("NickList"),
							         _("List of nickname strings"),
							    	 g_param_spec_string("nick", "nick", "nick", NULL, G_PARAM_READWRITE),
							   	 G_PARAM_READWRITE));
							    
}
static void 
loqui_profile_account_init(LoquiProfileAccount *profile)
{
	LoquiProfileAccountPrivate *priv;

	priv = g_new0(LoquiProfileAccountPrivate, 1);

	profile->priv = priv;
	loqui_profile_account_set_name(profile, _("Untitled"));
}
LoquiProfileAccount*
loqui_profile_account_new(LoquiProtocol *protocol)
{
        LoquiProfileAccount *profile;
	LoquiProfileAccountPrivate *priv;

	profile = g_object_new(loqui_profile_account_get_type(), NULL);
	
        priv = profile->priv;
	profile->protocol = protocol;

        return profile;
}

LOQUI_PROFILE_ACCOUNT_ACCESSOR_GENERIC(gboolean, use);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_STRING(name);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_STRING(nick);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_STRING(servername);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_GENERIC(int, port);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_STRING(username);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_STRING(password);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_GENERIC(int, codeconv_mode);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_STRING(codeconv_item_name);
LOQUI_PROFILE_ACCOUNT_ACCESSOR_STRING(codeset);

void
loqui_profile_account_set_nick_list(LoquiProfileAccount *profile, GList *nick_list)
{
        g_return_if_fail(profile != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT(profile));
        
        if (profile->nick_list) {
        	g_list_foreach(profile->nick_list, (GFunc) g_free, NULL);
        	g_list_free(profile->nick_list);
        	profile->nick_list = NULL;
        }
        profile->nick_list = nick_list;
}

GList *
loqui_profile_account_get_nick_list(LoquiProfileAccount *profile)
{
        g_return_val_if_fail(profile != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_PROFILE_ACCOUNT(profile), NULL);
        
        return profile->nick_list;
}
LoquiProtocol *
loqui_profile_account_get_protocol(LoquiProfileAccount *profile)
{
        g_return_val_if_fail(profile != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_PROFILE_ACCOUNT(profile), NULL);
 
	return profile->protocol;
}
void
loqui_profile_account_print(LoquiProfileAccount *profile)
{
	GParamSpec **param_specs;
	guint n_properties;
	gint i;
	const gchar *param_name;
	gchar *value_str;
	GValue value = { 0, };
	
        g_return_if_fail(profile != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_ACCOUNT(profile));

	param_specs = g_object_class_list_properties(G_OBJECT_CLASS(LOQUI_PROFILE_ACCOUNT_GET_CLASS(profile)),
						     &n_properties);
	g_print("Profile { ");

	for (i = 0; i < n_properties; i++) {
		param_name = g_param_spec_get_name(param_specs[i]);
		g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(param_specs[i]));
		g_object_get_property(G_OBJECT(profile), param_name, &value);
		value_str = g_strdup_value_contents(&value);
		g_print("%s = %s; ", param_name, value_str);
		g_free(value_str);
		g_value_unset(&value);
	}

	g_print("}\n");
}
