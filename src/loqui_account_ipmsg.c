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

#include "loqui_account_ipmsg.h"
#include "loqui_user_ipmsg.h"
#include "loqui_sender_ipmsg.h"

#include "ipmsg_socket.h"
#include "intl.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiAccountIPMsgPrivate
{
	IPMsgSocket *sock;
};

static LoquiAccountClass *parent_class = NULL;

/* static guint loqui_account_ipmsg_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_account_ipmsg_class_init(LoquiAccountIPMsgClass *klass);
static void loqui_account_ipmsg_init(LoquiAccountIPMsg *account);
static void loqui_account_ipmsg_finalize(GObject *object);
static void loqui_account_ipmsg_dispose(GObject *object);

static void loqui_account_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_account_ipmsg_connect(LoquiAccount *account);

GType
loqui_account_ipmsg_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountIPMsgClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_ipmsg_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccountIPMsg),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_ipmsg_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_ACCOUNT,
					      "LoquiAccountIPMsg",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_account_ipmsg_finalize(GObject *object)
{
	LoquiAccountIPMsg *account;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IPMSG(object));

        account = LOQUI_ACCOUNT_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(account->priv);
}
static void 
loqui_account_ipmsg_dispose(GObject *object)
{
	LoquiAccountIPMsg *account;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IPMSG(object));

        account = LOQUI_ACCOUNT_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_account_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiAccountIPMsg *account;        

        account = LOQUI_ACCOUNT_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_account_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiAccountIPMsg *account;        

        account = LOQUI_ACCOUNT_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_account_ipmsg_class_init(LoquiAccountIPMsgClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	LoquiAccountClass *account_class = LOQUI_ACCOUNT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_ipmsg_finalize;
        object_class->dispose = loqui_account_ipmsg_dispose;
        object_class->get_property = loqui_account_ipmsg_get_property;
        object_class->set_property = loqui_account_ipmsg_set_property;
	
	account_class->connect = loqui_account_ipmsg_connect;
}
static void 
loqui_account_ipmsg_init(LoquiAccountIPMsg *account)
{
	LoquiAccountIPMsgPrivate *priv;

	priv = g_new0(LoquiAccountIPMsgPrivate, 1);

	account->priv = priv;

	loqui_account_set_sender(LOQUI_ACCOUNT(account), LOQUI_SENDER(loqui_sender_ipmsg_new(LOQUI_ACCOUNT(account))));
}
static void
loqui_account_ipmsg_connect(LoquiAccount *account)
{
	LoquiAccountIPMsgPrivate *priv;
	gchar *str;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IPMSG(account));

        priv = LOQUI_ACCOUNT_IPMSG(account)->priv;
	priv->sock = ipmsg_socket_new();

	if (!ipmsg_socket_bind(priv->sock)) {
		loqui_account_warning(account, _("Failed to create socket. Is used the port?"));
		return;
	}
	
	str = g_strdup_printf(_("Opened the socket."));
	loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, str);
	g_free(str);
}
LoquiAccountIPMsg*
loqui_account_ipmsg_new(LoquiProfileAccount *profile)
{
        LoquiAccountIPMsg *account;
	LoquiAccountIPMsgPrivate *priv;
	LoquiUser *user;

	user = LOQUI_USER(loqui_user_ipmsg_new());
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);

	account = g_object_new(loqui_account_ipmsg_get_type(),  
 			       "buffer", channel_buffer_new(),
			       "profile", profile,
			       "user_self", user,
			       NULL);
	
        priv = account->priv;

        return account;
}
