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

#include "loqui_account_msn.h"
#include "msn_login.h"
#include <libloqui-intl.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiAccountMSNPrivate
{
	MSNLogin *login;
};

static LoquiAccountClass *parent_class = NULL;

/* static guint loqui_account_msn_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_account_msn_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_account_msn_class_init(LoquiAccountMSNClass *klass);
static void loqui_account_msn_init(LoquiAccountMSN *account);
static void loqui_account_msn_finalize(GObject *object);
static void loqui_account_msn_dispose(GObject *object);

static void loqui_account_msn_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_msn_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_account_msn_connect(LoquiAccount *account);

GType
loqui_account_msn_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountMSNClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_msn_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccountMSN),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_msn_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_ACCOUNT,
					      "LoquiAccountMSN",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_account_msn_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_account_msn_finalize(GObject *object)
{
	LoquiAccountMSN *account;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MSN(object));

        account = LOQUI_ACCOUNT_MSN(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(account->priv);
}
static void 
loqui_account_msn_dispose(GObject *object)
{
	LoquiAccountMSN *account;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MSN(object));

        account = LOQUI_ACCOUNT_MSN(object);

	G_FREE_UNLESS_NULL(account->trid_string);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_account_msn_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiAccountMSN *account;        

        account = LOQUI_ACCOUNT_MSN(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_account_msn_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiAccountMSN *account;        

        account = LOQUI_ACCOUNT_MSN(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_account_msn_class_init(LoquiAccountMSNClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	LoquiAccountClass *account_class = LOQUI_ACCOUNT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_account_msn_constructor; 
        object_class->finalize = loqui_account_msn_finalize;
        object_class->dispose = loqui_account_msn_dispose;
        object_class->get_property = loqui_account_msn_get_property;
        object_class->set_property = loqui_account_msn_set_property;

	account_class->connect = loqui_account_msn_connect;
}
static void 
loqui_account_msn_init(LoquiAccountMSN *account)
{
	LoquiAccountMSNPrivate *priv;

	priv = g_new0(LoquiAccountMSNPrivate, 1);

	account->priv = priv;
}
static void
loqui_account_msn_connect(LoquiAccount *account)
{
	loqui_account_information(account, _("Connecting."));
}
LoquiAccountMSN*
loqui_account_msn_new(void)
{
        LoquiAccountMSN *account;
	LoquiAccountMSNPrivate *priv;

	account = g_object_new(loqui_account_msn_get_type(), NULL);
	
        priv = account->priv;

        return account;
}
gint
loqui_account_msn_get_new_trid(LoquiAccountMSN *account)
{
	g_return_val_if_fail(account != NULL, -1);
	g_return_val_if_fail(LOQUI_IS_ACCOUNT_MSN(account), -1);

	return account->trid++;
}
G_CONST_RETURN gchar *
loqui_account_msn_get_trid_string(LoquiAccountMSN *account)
{
	gint trid;
	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_ACCOUNT_MSN(account), NULL);

	G_FREE_UNLESS_NULL(account->trid_string);

	trid = loqui_account_msn_get_new_trid(account);
	account->trid_string = g_strdup_printf("%d", trid);

	return account->trid_string;
}
