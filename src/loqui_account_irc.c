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

#include "loqui_account_irc.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiAccountIRCPrivate
{
};

static LoquiAccountClass *parent_class = NULL;

/* static guint loqui_account_irc_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_account_irc_class_init(LoquiAccountIRCClass *klass);
static void loqui_account_irc_init(LoquiAccountIRC *account);
static void loqui_account_irc_finalize(GObject *object);
static void loqui_account_irc_dispose(GObject *object);

static void loqui_account_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_account_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccountIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_ACCOUNT,
					      "LoquiAccountIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_account_irc_finalize(GObject *object)
{
	LoquiAccountIRC *account;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(object));

        account = LOQUI_ACCOUNT_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(account->priv);
}
static void 
loqui_account_irc_dispose(GObject *object)
{
	LoquiAccountIRC *account;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(object));

        account = LOQUI_ACCOUNT_IRC(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_account_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiAccountIRC *account;        

        account = LOQUI_ACCOUNT_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_account_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiAccountIRC *account;        

        account = LOQUI_ACCOUNT_IRC(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_account_irc_class_init(LoquiAccountIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_irc_finalize;
        object_class->dispose = loqui_account_irc_dispose;
        object_class->get_property = loqui_account_irc_get_property;
        object_class->set_property = loqui_account_irc_set_property;
}
static void 
loqui_account_irc_init(LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;

	priv = g_new0(LoquiAccountIRCPrivate, 1);

	account->priv = priv;
}
LoquiAccountIRC*
loqui_account_irc_new(LoquiProfileAccount *profile)
{
        LoquiAccountIRC *account;
	LoquiAccountIRCPrivate *priv;
	LoquiUser *user;

	user = LOQUI_USER(loqui_user_irc_new());
	loqui_user_set_nick(user, loqui_profile_account_get_nick(profile));
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);

	account = g_object_new(loqui_account_irc_get_type(), 
			       "buffer", channel_buffer_new(),
			       "profile", profile,
			       "user_self", user,
			       NULL);

        priv = account->priv;

        return account;
}
LoquiUserIRC *
loqui_account_irc_fetch_user(LoquiAccountIRC *account, const gchar *nick)
{
	LoquiUser *user;
		
        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), NULL);

	if((user = loqui_account_peek_user(LOQUI_ACCOUNT(account), nick)) == NULL) {
		user = LOQUI_USER(loqui_user_irc_new());
		loqui_user_set_nick(user, nick);
		loqui_account_add_user(LOQUI_ACCOUNT(account), user);
	} else {
		g_object_ref(user);
	}

	return LOQUI_USER_IRC(user);
}
