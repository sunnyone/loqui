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

#include "msn_login.h"
#include "msn_message.h"
#include "intl.h"
#include "msn_constants.h"
#include "loqui_account_msn.h"
#include "loqui_profile_account.h"

#include <gnet.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _MSNLoginPrivate
{
	GTcpSocket *socket;
	GTcpSocketConnectAsyncID connect_id;
	guint in_watch;
	guint out_watch;

};

static GObjectClass *parent_class = NULL;

/* static guint msn_login_signals[LAST_SIGNAL] = { 0 }; */

static GObject* msn_login_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void msn_login_class_init(MSNLoginClass *klass);
static void msn_login_init(MSNLogin *login);
static void msn_login_finalize(GObject *object);
static void msn_login_dispose(GObject *object);

static void msn_login_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void msn_login_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
msn_login_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(MSNLoginClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) msn_login_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(MSNLogin),
				0,              /* n_preallocs */
				(GInstanceInitFunc) msn_login_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "MSNLogin",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
msn_login_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
msn_login_finalize(GObject *object)
{
	MSNLogin *login;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_MSN_LOGIN(object));

        login = MSN_LOGIN(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(login->priv);
}
static void 
msn_login_dispose(GObject *object)
{
	MSNLogin *login;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_MSN_LOGIN(object));

        login = MSN_LOGIN(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
msn_login_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        MSNLogin *login;        

        login = MSN_LOGIN(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
msn_login_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        MSNLogin *login;        

        login = MSN_LOGIN(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
msn_login_class_init(MSNLoginClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = msn_login_constructor; 
        object_class->finalize = msn_login_finalize;
        object_class->dispose = msn_login_dispose;
        object_class->get_property = msn_login_get_property;
        object_class->set_property = msn_login_set_property;
}
static void 
msn_login_init(MSNLogin *login)
{
	MSNLoginPrivate *priv;

	priv = g_new0(MSNLoginPrivate, 1);

	login->priv = priv;
}
MSNLogin*
msn_login_new(LoquiAccount *account)
{
        MSNLogin *login;
	MSNLoginPrivate *priv;

	login = g_object_new(msn_login_get_type(), NULL);
	
        priv = login->priv;
	login->account = account;

        return login;
}
static void
msn_login_connect_cb(GTcpSocket *socket,
		     GTcpSocketConnectAsyncStatus status,
		     gpointer data)
{
	MSNLogin *login;
	MSNLoginPrivate *priv;
	GIOChannel *ioch;
	MSNMessage *msg;

	login = data;
	priv = login->priv;

	if(status != GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK || socket == NULL) {
		loqui_account_warning(login->account, _("Failed to connect notification server."));
		return;
	}
	
	priv->socket = socket;

	ioch = gnet_tcp_socket_get_io_channel(socket);
/*	priv->in_watch = g_io_add_watch(ioch, G_IO_IN | G_IO_ERR | G_IO_PRI | G_IO_HUP | G_IO_NVAL,
	(GIOFunc) msn_login_watch_in_cb, login); */

	msg = msn_message_create(NULL, NULL,
				 MSN_COMMAND_VER,
				 loqui_account_msn_get_trid_string(LOQUI_ACCOUNT_MSN(login->account)),
				 "MSNP8",
				 "CVR0",
				 NULL);
	
	loqui_account_information(login->account, _("Sending version information..."));
	
}
void
msn_login_connect(MSNLogin *login)
{
	MSNLoginPrivate *priv;
	const gchar *servername;
	gint port;

	g_return_if_fail(login != NULL);
	g_return_if_fail(IS_MSN_LOGIN(login));

        priv = login->priv;

	servername = loqui_profile_account_get_servername(loqui_account_get_profile(login->account));
	port = loqui_profile_account_get_port(loqui_account_get_profile(login->account));

	priv->connect_id = gnet_tcp_socket_connect_async(servername, port,
							 msn_login_connect_cb, login);
}
