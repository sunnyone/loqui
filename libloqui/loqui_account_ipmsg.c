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
#include "loqui_receiver_ipmsg.h"

#include "ipmsg_socket.h"
#include "ipmsg_packet.h"
#include "ipmsg.h"
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

	gint current_packet_num;
};

static LoquiAccountClass *parent_class = NULL;

/* static guint loqui_account_ipmsg_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_account_ipmsg_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_account_ipmsg_class_init(LoquiAccountIPMsgClass *klass);
static void loqui_account_ipmsg_init(LoquiAccountIPMsg *account);
static void loqui_account_ipmsg_finalize(GObject *object);
static void loqui_account_ipmsg_dispose(GObject *object);

static void loqui_account_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_account_ipmsg_connect(LoquiAccount *account);
static void loqui_account_ipmsg_disconnect(LoquiAccount *account);

static void loqui_account_ipmsg_socket_arrive_packet_cb(IPMsgSocket *socket, IPMsgPacket *packet, LoquiAccount *account);
static void loqui_account_ipmsg_socket_warn_cb(IPMsgSocket *socket, const gchar *warn, LoquiAccount *account);

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
	LoquiAccountIPMsgPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IPMSG(object));

        account = LOQUI_ACCOUNT_IPMSG(object);
	priv = account->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->sock);

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
        
	object_class->constructor = loqui_account_ipmsg_constructor;
        object_class->finalize = loqui_account_ipmsg_finalize;
        object_class->dispose = loqui_account_ipmsg_dispose;
        object_class->get_property = loqui_account_ipmsg_get_property;
        object_class->set_property = loqui_account_ipmsg_set_property;
	
	account_class->connect = loqui_account_ipmsg_connect;
	account_class->disconnect = loqui_account_ipmsg_disconnect;
}
static GObject*
loqui_account_ipmsg_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
	GObject *object;
	GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	LoquiAccount *account;
	LoquiUser *user;

	object = object_class->constructor(type, n_props, props);
	
	account = LOQUI_ACCOUNT(object);

	loqui_account_set_sender(account, LOQUI_SENDER(loqui_sender_ipmsg_new(account)));
	loqui_account_set_receiver(account, LOQUI_RECEIVER(loqui_receiver_ipmsg_new(account)));

	user = LOQUI_USER(loqui_user_ipmsg_new());
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);
	loqui_account_set_user_self(account, user);

	return object;
}
static void 
loqui_account_ipmsg_init(LoquiAccountIPMsg *account)
{
	LoquiAccountIPMsgPrivate *priv;

	priv = g_new0(LoquiAccountIPMsgPrivate, 1);

	account->priv = priv;

	priv->current_packet_num = (gint) time(NULL);
}
static void
loqui_account_ipmsg_connect(LoquiAccount *account)
{
	LoquiAccountIPMsgPrivate *priv;
	LoquiUser *user_self;
	gchar *str;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IPMSG(account));

        priv = LOQUI_ACCOUNT_IPMSG(account)->priv;

	if (loqui_account_get_is_connected(LOQUI_ACCOUNT(account))) {
		loqui_account_warning(account, _("Already connected."));
		return;
	}

	priv->sock = ipmsg_socket_new();

	if (!ipmsg_socket_bind(priv->sock)) {
		loqui_account_warning(account, _("Failed to create socket. Is used the port?"));
		G_OBJECT_UNREF_UNLESS_NULL(priv->sock);
		return;
	}
	
	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), TRUE);

	g_signal_connect(G_OBJECT(priv->sock), "arrive_packet",
			 G_CALLBACK(loqui_account_ipmsg_socket_arrive_packet_cb), account);
	g_signal_connect(G_OBJECT(priv->sock), "warn",
			 G_CALLBACK(loqui_account_ipmsg_socket_warn_cb), account);

	str = g_strdup_printf(_("Opened the socket."));
	loqui_account_append_text(account, NULL, LOQUI_TEXT_TYPE_INFO, str);
	g_free(str);

	user_self = loqui_account_get_user_self(LOQUI_ACCOUNT(account));
	loqui_user_set_away(user_self, LOQUI_AWAY_TYPE_ONLINE);
	loqui_user_set_nick(user_self, loqui_profile_account_get_nick(loqui_account_get_profile(LOQUI_ACCOUNT(account))));
	loqui_user_set_hostname(user_self,
				gnet_inetaddr_get_host_name());
	/* TODO: set hostname */

	loqui_sender_ipmsg_br_entry(LOQUI_SENDER_IPMSG(LOQUI_ACCOUNT(account)->sender));
}
static void
loqui_account_ipmsg_disconnect(LoquiAccount *account)
{
	LoquiAccountIPMsgPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IPMSG(account));

        priv = LOQUI_ACCOUNT_IPMSG(account)->priv;
	
	if (!loqui_account_get_is_connected(LOQUI_ACCOUNT(account)))
		return;
	
	if (priv->sock)
		ipmsg_socket_unbind(priv->sock);
	G_OBJECT_UNREF_UNLESS_NULL(priv->sock);

	loqui_account_append_text(account, NULL, LOQUI_TEXT_TYPE_INFO, _("Disconnected."));

	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);
	loqui_user_set_away(loqui_account_get_user_self(LOQUI_ACCOUNT(account)), LOQUI_AWAY_TYPE_OFFLINE);
}
static void
loqui_account_ipmsg_socket_arrive_packet_cb(IPMsgSocket *socket, IPMsgPacket *packet, LoquiAccount *account)
{
	loqui_receiver_ipmsg_handle(LOQUI_RECEIVER_IPMSG(loqui_account_get_receiver(account)), packet);
}
static void
loqui_account_ipmsg_socket_warn_cb(IPMsgSocket *socket, const gchar *warn, LoquiAccount *account)
{
	loqui_account_warning(account, "%s", warn);
}
LoquiAccountIPMsg*
loqui_account_ipmsg_new(LoquiProfileAccount *profile)
{
        LoquiAccountIPMsg *account;
	LoquiAccountIPMsgPrivate *priv;

	account = g_object_new(loqui_account_ipmsg_get_type(),  
			       "profile", profile,
			       NULL);
	
        priv = account->priv;

        return account;
}
IPMsgSocket *
loqui_account_ipmsg_get_socket(LoquiAccountIPMsg *account)
{
	return account->priv->sock;
}

IPMsgPacket *
loqui_account_ipmsg_create_packet(LoquiAccountIPMsg *account, gint command_num, const gchar *extra)
{
	LoquiAccountIPMsgPrivate *priv;
	LoquiUser *user;
	LoquiProfileAccount *profile;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IPMSG(account), NULL);

        priv = LOQUI_ACCOUNT_IPMSG(account)->priv;
	
	user = loqui_account_get_user_self(LOQUI_ACCOUNT(account));
	profile = loqui_account_get_profile(LOQUI_ACCOUNT(account));

	return ipmsg_packet_create(IPMSG_VERSION,
				   priv->current_packet_num++,
				   loqui_profile_account_get_username(profile),
				   loqui_user_get_hostname(user),
				   command_num,
				   extra,
				   loqui_user_ipmsg_get_group_name(LOQUI_USER_IPMSG(user)));
}
