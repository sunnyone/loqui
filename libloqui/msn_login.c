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

#include "msn_login.h"
#include "msn_message.h"
#include "intl.h"
#include "msn_constants.h"
#include "loqui_account_msn.h"
#include "loqui_profile_account.h"

#include <gnet.h>
#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _MSNLoginPrivate
{
	GConn *conn;
	gboolean is_conn_connected;
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
msn_login_conn_error_cb(GConn *conn, MSNLogin *login)
{
	MSNLoginPrivate *priv;
	LoquiAccount *account;

	priv = login->priv;
	account = login->account;

	if (!priv->is_conn_connected) {
		loqui_account_warning(LOQUI_ACCOUNT(account), _("Failed to connect."));
	} else {
		loqui_account_warning(LOQUI_ACCOUNT(account), _("An error occured on connection."));
	}

	priv->is_conn_connected = FALSE;
	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);

	if (priv->conn) {
		gnet_conn_unref(priv->conn);
		priv->conn = NULL;
	}

	loqui_account_closed(LOQUI_ACCOUNT(account));
}
static void
msn_login_conn_connected_cb(GConn *conn, MSNLogin *login)
{
	MSNLoginPrivate *priv;
	MSNMessage *msg;

	priv = login->priv;

	priv->is_conn_connected = TRUE;
	loqui_account_information(LOQUI_ACCOUNT(login->account), _("Connected."));

	msg = msn_message_create(NULL, NULL,
				 MSN_COMMAND_VER,
				 loqui_account_msn_get_trid_string(LOQUI_ACCOUNT_MSN(login->account)),
				 "MSNP8",
				 "CVR0",
				 NULL);
	
	loqui_account_information(login->account, _("Sending version information..."));
	msn_login_send_message(login, msg);
	g_object_unref(msg);
}
static void
msn_login_conn_closed_cb(GConn *conn, MSNLogin *login)
{
	MSNLoginPrivate *priv;
	GList *cur;
	LoquiAccount *account;

	priv = login->priv;
	account = login->account;

	if (priv->conn) {
		gnet_conn_unref(priv->conn);
		priv->conn = NULL;
	}
	
	priv->is_conn_connected = FALSE;
	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);

	loqui_account_information(LOQUI_ACCOUNT(account), _("Connection closed."));
	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_OFFLINE);

/*	for (cur = LOQUI_ACCOUNT(account)->channel_list; cur != NULL; cur = cur->next)
	loqui_channel_set_is_joined(LOQUI_CHANNEL(cur->data), FALSE); */
	
	loqui_account_closed(LOQUI_ACCOUNT(account));
}
static void
msn_login_conn_readline_cb(GConn *conn, const gchar *buffer, MSNLogin *login)
{
	MSNLoginPrivate *priv;

	priv = login->priv;

	g_print("buffer: %s\n", buffer);

	gnet_conn_readline(priv->conn);
}
static void
msn_login_conn_cb(GConn *conn, GConnEvent *event, MSNLogin *login)
{
	switch (event->type) {
	case GNET_CONN_ERROR:
		msn_login_conn_error_cb(conn, login);
		break;
	case GNET_CONN_CONNECT:
		msn_login_conn_connected_cb(conn, login);
		break;
	case GNET_CONN_CLOSE:
		msn_login_conn_closed_cb(conn, login);
		break;
	case GNET_CONN_READ:
		msn_login_conn_readline_cb(conn, event->buffer, login);
	default:
		break;
	} 
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

	priv->conn = gnet_conn_new(servername, port, (GConnFunc) msn_login_conn_cb, login);

	loqui_account_information(login->account, _("Connecting to login server (%s:%d)"), servername, port);
	
	gnet_conn_connect(priv->conn);

	priv->is_conn_connected = FALSE;
}
void
msn_login_send_message(MSNLogin *login, MSNMessage *msg)
{
	MSNLoginPrivate *priv;
	gchar *buf, *tmp;

	priv = login->priv;
	
	/* TODO: buf = msn_message_to_string(msg); */
	buf = g_strdup("hoge");
	tmp = g_strconcat(buf, "\r\n", NULL);
	g_free(buf);

	gnet_conn_write(priv->conn, tmp, strlen(tmp));

	g_free(tmp);
}
