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
#include "utils.h"
#include "account.h"
#include "account_manager.h"

struct _IRCHandlePrivate
{
	Connection *connection;
	Account *account;
	Server *server;
	gchar *current_nick;
	gboolean end_motd;

	GThread *thread;
	GThread *send_thread;

	GAsyncQueue *msg_queue;

	gboolean passed_motd;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void irc_handle_class_init(IRCHandleClass *klass);
static void irc_handle_init(IRCHandle *irc_handle);
static void irc_handle_finalize(GObject *object);

static gpointer irc_handle_thread_func(IRCHandle *handle);
static gpointer irc_handle_send_thread_func(IRCHandle *handle);

static void irc_handle_response(IRCHandle *handle, IRCMessage *msg);
static gboolean irc_handle_command(IRCHandle *handle, IRCMessage *msg);
static gboolean irc_handle_reply(IRCHandle *handle, IRCMessage *msg);
static gboolean irc_handle_error(IRCHandle *handle, IRCMessage *msg);

static gboolean irc_handle_is_my_message(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_inspect_message(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_my_command_nick(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_my_command_join(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_my_command_part(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_command_privmsg_notice(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_ping(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_quit(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_account_console_append(IRCHandle *handle, IRCMessage *msg, TextType type, gchar *format);
static void irc_handle_channel_append(IRCHandle *handle, IRCMessage *msg, gboolean make_channel,
				      gint receiver_num, TextType type, gchar *format);

static void irc_handle_reply_names(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_reply_topic(IRCHandle *handle, IRCMessage *msg);

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

	priv->thread = NULL;
	priv->send_thread = NULL;
	priv->msg_queue = NULL;

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
static void
irc_handle_command_privmsg_notice(IRCHandle *handle, IRCMessage *msg)
{
	gchar *receiver_name;
	gchar *remark;
	Channel *channel = NULL;
	TextType type;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	receiver_name = irc_message_get_param(msg, 1);
	remark = irc_message_get_param(msg, 2);

	if(msg->response == IRC_COMMAND_NOTICE) {
		type = TEXT_TYPE_NOTICE;
	} else {
		type = TEXT_TYPE_NORMAL;
	}

	if(receiver_name != NULL && msg->nick != NULL) {
		channel = account_search_channel_by_name(handle->priv->account, receiver_name);
		if(channel == NULL) {
			gdk_threads_enter();
			channel = channel_new(receiver_name);
			account_add_channel(handle->priv->account, channel);
			gdk_threads_leave();
		}
	}

	gdk_threads_enter();
	if(channel != NULL) {
		channel_append_remark(channel, type, msg->nick, remark);
	} else {
		account_console_text_append(handle->priv->account, TRUE, type, remark);
	}
	gdk_threads_leave();

}
static void
irc_handle_command_ping(IRCHandle *handle, IRCMessage *msg)
{
	gchar *server;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	server = irc_message_get_param(msg, 1);

	msg = irc_message_create(IRCCommandPong, server, NULL);
	connection_put_irc_message(handle->priv->connection, msg);
	g_object_unref(msg);

	debug_puts("put PONG to %s", server);
}
static void
irc_handle_command_quit(IRCHandle *handle, IRCMessage *msg)
{
	/* FIXME: this should be broadcast all channels he joined */
	irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** %n has quit IRC(%t)"));
}

static void
irc_handle_my_command_join(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *name;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));
	g_return_if_fail(msg != NULL);

	name = irc_message_get_param(msg, 1);
	if(name == NULL) {
		g_warning(_("Can't get channel name"));
		return;
	}

	gdk_threads_enter();
	channel = channel_new(name);
	account_add_channel(handle->priv->account, channel);
	account_manager_set_current(account_manager_get(), NULL, channel);
	gdk_threads_leave();
}
static void
irc_handle_my_command_part(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *name;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));
	g_return_if_fail(msg != NULL);
	
	name = irc_message_get_param(msg, 1);
	if(name == NULL) {
		g_warning(_("Can't get channel name"));
		return;
	}
	channel = account_search_channel_by_name(handle->priv->account, name);
	if(channel == NULL)
		return;

	irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** You has left %1");

	gdk_threads_enter();
	account_remove_channel(handle->priv->account, channel);
	g_object_unref(channel);
	gdk_threads_leave();
}

static void
irc_handle_my_command_nick(IRCHandle *handle, IRCMessage *msg)
{
        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	if(handle->priv->current_nick) {
		g_free(handle->priv->current_nick);
	}

	handle->priv->current_nick = g_strdup(irc_message_get_param(msg, 1));
	debug_puts("current nick is set to %s", handle->priv->current_nick);
}

static gboolean
irc_handle_is_my_message(IRCHandle *handle, IRCMessage *msg)
{
        g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), FALSE);

	g_return_val_if_fail(msg != NULL, FALSE);
	g_return_val_if_fail(handle->priv->current_nick != NULL, TRUE);
	
	if(msg->nick == NULL)
		return FALSE;

	return (strcmp(msg->nick,handle->priv->current_nick) == 0);
}
static void
irc_handle_inspect_message(IRCHandle *handle, IRCMessage *msg)
{
	gchar *str;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	str = irc_message_inspect(msg);
	
	gdk_threads_enter();
	account_console_text_append(handle->priv->account, TRUE, TEXT_TYPE_NORMAL, str);
	gdk_threads_leave();
	
	g_free(str);
}
/* TODO: reading nicks with this function */
static void
irc_handle_reply_names(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *name, *nick;
	gchar **nick_array;
	gint i;

	name = irc_message_get_param(msg, 3);
	channel = account_search_channel_by_name(handle->priv->account, name);
	if(channel == NULL)
		return;

	channel_clear_user(channel);
	nick_array = g_strsplit(irc_message_get_trailing(msg), " ", 0);
	for(i = 0; nick_array[i] != NULL; i++) {
		channel_append_user(channel, nick_array[i], USER_POWER_UNDETERMINED);
	}
	g_strfreev(nick_array);

	irc_handle_channel_append(handle, msg, FALSE, 3, TEXT_TYPE_NORMAL, "%3: %t");
}
static void
irc_handle_reply_topic(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *topic;
	gchar *name;

	name = irc_message_get_param(msg, 2);
	channel = account_search_channel_by_name(handle->priv->account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_trailing(msg);
	channel_set_topic(channel, topic);

	irc_handle_channel_append(handle, msg, FALSE, 2, TEXT_TYPE_INFO, _("Topic for %2: %t"));
}
static void /* utility function for threading*/
irc_handle_account_console_append(IRCHandle *handle, IRCMessage *msg, TextType type, gchar *format)
{
	gchar *str;

	str = irc_message_format(msg, format);

	gdk_threads_enter();
	account_console_text_append(handle->priv->account, TRUE, type, str);
	gdk_threads_leave();

	g_free(str);
}

static void /* utility function for threading */
irc_handle_channel_append(IRCHandle *handle, IRCMessage *msg, gboolean make_channel,
			  gint receiver_num, TextType type, gchar *format)
{
	Channel *channel;
	gchar *str;
	gchar *receiver_name;

	receiver_name = irc_message_get_param(msg, receiver_num);
	if(receiver_name == NULL) {
		g_warning(_("Can't find the channel from the message"));
		return;
	}

	channel = account_search_channel_by_name(handle->priv->account, receiver_name);
	if(make_channel == TRUE && channel == NULL) { /* FIXME as well as privmsg_notice */
		gdk_threads_enter();
		channel = channel_new(receiver_name);
		account_add_channel(handle->priv->account, channel);
		gdk_threads_leave();
	}

	str = irc_message_format(msg, format);

	gdk_threads_enter();
	if(channel == NULL) {
		account_console_text_append(handle->priv->account, TRUE, type, str);
	} else {
		channel_append_text(channel, TRUE, type, str);

	}
	gdk_threads_leave();

	g_free(str);
}

static gboolean 
irc_handle_reply(IRCHandle *handle, IRCMessage *msg)
{
	g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), FALSE);

	switch(msg->response) {
	case IRC_RPL_LUSERCLIENT:
	case IRC_RPL_LUSERME:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %t");
		return TRUE;
	case IRC_RPL_LUSEROP:
	case IRC_RPL_LUSERUNKNOWN:
	case IRC_RPL_LUSERCHANNELS:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %2 %3");
		return TRUE;
	case IRC_RPL_MOTDSTART:
	case IRC_RPL_MOTD:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %t");
		return TRUE;
	case IRC_RPL_ENDOFMOTD:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %t");
		handle->priv->end_motd = TRUE;
		return TRUE;
	case IRC_RPL_NAMREPLY: /* <nick> = <channel> :... */
		irc_handle_reply_names(handle, msg);
		return TRUE;
	case IRC_RPL_TOPIC:
		irc_handle_reply_topic(handle, msg);
		return TRUE;
	case IRC_RPL_ENDOFNAMES:
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

static gboolean 
irc_handle_error(IRCHandle *handle, IRCMessage *msg)
{
	g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), FALSE);

	return FALSE;
}
static gboolean 
irc_handle_command(IRCHandle *handle, IRCMessage *msg)
{
	g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), FALSE);

	if(irc_handle_is_my_message(handle, msg)) {
		switch(msg->response) {
		case IRC_COMMAND_NICK:
			irc_handle_my_command_nick(handle, msg);
			return TRUE;
		case IRC_COMMAND_JOIN:
			irc_handle_my_command_join(handle, msg);
			return TRUE;
		case IRC_COMMAND_PART:
			irc_handle_my_command_part(handle, msg);
			return TRUE;
		default:
			break;
		}
	} 
	switch (msg->response) {
	case IRC_COMMAND_NOTICE:
	case IRC_COMMAND_PRIVMSG:
		irc_handle_command_privmsg_notice(handle, msg);
		return TRUE;
	case IRC_COMMAND_QUIT:
		irc_handle_command_quit(handle, msg);
		return TRUE;
	case IRC_COMMAND_PING:
		irc_handle_command_ping(handle, msg);
		return TRUE;
	default:
		break;
	}

	return FALSE;
}
static void
irc_handle_response(IRCHandle *handle, IRCMessage *msg)
{
	gboolean proceeded = FALSE;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	if(IRC_MESSAGE_IS_COMMAND(msg))
		proceeded = irc_handle_command(handle, msg);
	if(IRC_MESSAGE_IS_REPLY(msg)) 
		proceeded = irc_handle_reply(handle, msg);
	if(IRC_MESSAGE_IS_ERROR(msg))
		proceeded = irc_handle_error(handle, msg);
	if(msg->response < 10) { /* FIXME: what's this? */
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %*2");
		proceeded = TRUE;
	}

	if(!proceeded)
		irc_handle_inspect_message(handle, msg);
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
	str = g_strdup_printf(_("Connecting to %s:%d"), priv->server->hostname, priv->server->port);
	account_console_text_append(account, TRUE, TEXT_TYPE_INFO, str);
	g_free(str);
	gdk_threads_leave();

	priv->connection = connection_new(priv->server);

	gdk_threads_enter();
	str = g_strdup(_("Connected. Sending Initial command..."));
	account_console_text_append(account, TRUE, TEXT_TYPE_INFO, str);
	g_free(str);
	gdk_threads_leave();

	priv->send_thread = g_thread_create((GThreadFunc) irc_handle_send_thread_func,
					   handle,
					   TRUE,
					   NULL);

	msg = irc_message_create(IRCCommandPass, priv->server->password, NULL);
	irc_handle_push_message(handle, msg);

	msg = irc_message_create(IRCCommandNick, "hogehoge", NULL);
	irc_handle_push_message(handle, msg);
	priv->current_nick = g_strdup("hogehoge");

	msg = irc_message_create(IRCCommandUser, "Loqui", "*", "*", "*", NULL);
	irc_handle_push_message(handle, msg);

	gdk_threads_enter();
	account_console_text_append(account, TRUE, TEXT_TYPE_INFO, _("Done."));
	gdk_threads_leave();

        while((msg = connection_get_irc_message(priv->connection, NULL)) != NULL) {
		irc_handle_response(handle, msg);
		/* irc_handle_inspect_message(handle, msg); */
		g_object_unref(msg);
	}

	gdk_threads_enter();
	account_console_text_append(account, TRUE, TEXT_TYPE_INFO, _("Connection terminated."));
	gdk_threads_leave();

	msg = irc_message_create(IRCMessageEnd, NULL);
	irc_handle_push_message(handle, msg);

	return NULL;
}
static gpointer irc_handle_send_thread_func(IRCHandle *handle)
{
	IRCHandlePrivate *priv;
	gpointer data;
	IRCMessage *msg;

	priv = handle->priv;

	if(!priv->msg_queue)
		return NULL;

	while((data = g_async_queue_pop(priv->msg_queue)) != NULL) {
		msg = IRC_MESSAGE(data);
		if(msg->response == IRC_MESSAGE_END) {
			g_object_unref(msg);
			break;
		}

		connection_put_irc_message(priv->connection, msg);
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

	priv->msg_queue = g_async_queue_new();

	priv->thread = g_thread_create((GThreadFunc) irc_handle_thread_func,
				       handle,
				       TRUE,
				       NULL); 

	return handle;
}
void irc_handle_push_message(IRCHandle *handle, IRCMessage *msg)
{
	IRCHandlePrivate *priv;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	priv = handle->priv;

	if(!priv->thread) {
		g_warning(_("Connection thread is not created."));
		return;
	}
	if(!priv->send_thread) {
		g_warning(_("Thread sending message is not created."));
		return;
	}
	if(!priv->msg_queue) {
		g_warning(_("Message queue is not created."));
		return;
	}

	g_async_queue_push(priv->msg_queue, msg);
}

gchar *
irc_handle_get_current_nick(IRCHandle *handle)
{
        g_return_val_if_fail(handle != NULL, NULL);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), NULL);

	return handle->priv->current_nick;
}
