/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2 <http://loqui.good-day.net/>
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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

#include "irc_handle.h"
#include "connection.h"
#include "irc_message.h"

struct _IRCHandlePrivate
{
	Connection *connection;
	Account *account;
	Server *server;

	GThread *thread;

	gboolean passed_motd;

};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void irc_handle_class_init(IRCHandleClass *klass);
static void irc_handle_init(IRCHandle *irc_handle);
static void irc_handle_finalize(GObject *object);

static gpointer irc_handle_thread_func(IRCHandle *handle);

GType
irc_handle_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(IRCHandleClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) irc_handle_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(IRCHandle),
				0,              /* n_preallocs */
				(GInstanceInitFunc) irc_handle_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "IRCHandle",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
irc_handle_class_init(IRCHandleClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = irc_handle_finalize;
}
static void 
irc_handle_init(IRCHandle *irc_handle)
{
	IRCHandlePrivate *priv;

	priv = g_new0(IRCHandlePrivate, 1);

	irc_handle->priv = priv;
}
static void 
irc_handle_finalize(GObject *object)
{
	IRCHandle *irc_handle;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IRC_HANDLE(object));

        irc_handle = IRC_HANDLE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(irc_handle->priv);
}
static gpointer irc_handle_thread_func(IRCHandle *handle)
{
	IRCHandlePrivate *priv;
	IRCMessage *msg;
	Account *account;
	gchar *str;

	priv = handle->priv;
	account = priv->account;

	gdk_threads_enter();
	str = g_strdup_printf(_("%s: Connecting to %s:%d\n"), 
			      priv->account->name, priv->server->hostname, priv->server->port);
	account_console_text_append(account, str);
	g_free(str);
	gdk_threads_leave();

	priv->connection = connection_new(priv->server);

	gdk_threads_enter();
	str = g_strdup_printf(_("%s: Connected. Sending Initial command..."), priv->account->name);
	account_console_text_append(account, str);
	g_free(str);
	gdk_threads_leave();

	msg = irc_message_create(IRCCommandPass, priv->server->password, NULL);
	connection_put_irc_message(priv->connection, msg);
	g_object_unref(msg);

	msg = irc_message_create(IRCCommandNick, "hogehoge", NULL);
	connection_put_irc_message(priv->connection, msg);
	g_object_unref(msg);

	msg = irc_message_create(IRCCommandUser, "test", "*", "*", "*", NULL);
	connection_put_irc_message(priv->connection, msg);
	g_object_unref(msg);

	gdk_threads_enter();
	account_console_text_append(account, _("Done.\n"));
	gdk_threads_leave();

        while((msg = connection_get_irc_message(priv->connection, NULL)) != NULL) {
		str = irc_message_inspect(msg);

		gdk_threads_enter();
		account_console_text_append(account, str);
		gdk_threads_leave();

/*		irc_message_print(msg); */
		g_object_unref(msg);
	}

	return NULL;
}
IRCHandle*
irc_handle_new(Account *account, guint server_num, gboolean fallback)
{
        IRCHandle *handle;
	IRCHandlePrivate *priv;

	handle = g_object_new(irc_handle_get_type(), NULL);
	
	priv = handle->priv;

	g_return_val_if_fail(server_num < g_slist_length(account->server_list), NULL);
	priv->server = g_slist_nth(account->server_list, server_num)->data;
	g_return_val_if_fail(priv->server != NULL, NULL);

	priv->account = account;

	handle->server_num = server_num;
	handle->fallback = fallback;

//	irc_handle_thread_func(handle);
	priv->thread = g_thread_create((GThreadFunc) irc_handle_thread_func,
				       handle,
				       TRUE,
				       NULL); 

	return handle;
}
