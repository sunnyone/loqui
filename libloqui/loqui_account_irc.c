/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://launchpad.net/loqui/>
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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

#include "loqui_account_irc.h"

#include "loqui_profile_account_irc.h"
#include "loqui_sender_irc.h"
#include "loqui_receiver_irc.h"
#include "loqui_channel_irc.h"

#include "loqui_utils_irc.h"
#include <libloqui-intl.h>
#include "loqui-static-core.h"

#include <string.h>
#include "loqui_codeconv.h"

#include <gio/gio.h>

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
        LAST_PROP
};

struct _LoquiAccountIRCPrivate
{
        GSocketClient *sock_client;
        GSocketConnection *sock_conn;
        
        GDataInputStream *data_in_stream;
        GOutputStream *out_stream;
        
	GQueue *msg_queue;

	LoquiCodeConv *codeconv;
};

static LoquiAccountClass *parent_class = NULL;

/* static guint loqui_account_irc_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_account_irc_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_account_irc_class_init(LoquiAccountIRCClass *klass);
static void loqui_account_irc_init(LoquiAccountIRC *account);
static void loqui_account_irc_finalize(GObject *object);
static void loqui_account_irc_dispose(GObject *object);

static void loqui_account_irc_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_irc_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_account_irc_sock_client_connected_cb(GSocketClient *sock_client, GAsyncResult *res, LoquiAccountIRC *account);
static void loqui_account_irc_stream_write_cb(GOutputStream *stream, GAsyncResult *res, LoquiAccountIRC *account);
static void loqui_account_irc_stream_readline_cb(GDataInputStream *stream, GAsyncResult *res, LoquiAccountIRC *account);

static void loqui_account_irc_connect(LoquiAccount *account);
static void loqui_account_irc_disconnect(LoquiAccount *account);
static void loqui_account_irc_terminate(LoquiAccount *account);
static void loqui_account_irc_closed(LoquiAccount *account, gboolean is_success);

static void loqui_account_irc_flush_queue_one(LoquiAccountIRC *account);

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
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(object));

        account = LOQUI_ACCOUNT_IRC(object);
	priv = account->priv;

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->sock_client);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->sock_conn);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->data_in_stream);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->out_stream);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->codeconv);
	
	if (priv->msg_queue) {
		g_queue_foreach(priv->msg_queue, (GFunc) g_object_unref, NULL);
		g_queue_free(priv->msg_queue);
		priv->msg_queue = NULL;
	}

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
	LoquiAccountClass *account_class = LOQUI_ACCOUNT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_irc_finalize;
        object_class->dispose = loqui_account_irc_dispose;
        object_class->get_property = loqui_account_irc_get_property;
        object_class->set_property = loqui_account_irc_set_property;
	object_class->constructor = loqui_account_irc_constructor;

	account_class->connect = loqui_account_irc_connect;
	account_class->disconnect = loqui_account_irc_disconnect;
	account_class->terminate = loqui_account_irc_terminate;
	account_class->closed = loqui_account_irc_closed;
}
static void 
loqui_account_irc_init(LoquiAccountIRC *account_irc)
{
	LoquiAccountIRCPrivate *priv;
	LoquiAccount *account;

	priv = g_new0(LoquiAccountIRCPrivate, 1);

	account_irc->priv = priv;

	account = LOQUI_ACCOUNT(account_irc);
	
	priv->msg_queue = g_queue_new();
	
	priv->sock_client = NULL;
	priv->sock_conn = NULL;
	priv->data_in_stream = NULL;
	priv->out_stream = NULL;
}
static GObject*
loqui_account_irc_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
	GObject *object;
	GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	LoquiAccount *account;
	LoquiAccountIRCPrivate *priv;
	LoquiUser *user;
	LoquiCodeConv *codeconv;
	LoquiProfileAccount *profile;

	object = object_class->constructor(type, n_props, props);
	
	account = LOQUI_ACCOUNT(object);
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	loqui_account_set_sender(account, LOQUI_SENDER(loqui_sender_irc_new(account)));
	loqui_account_set_receiver(account, LOQUI_RECEIVER(loqui_receiver_irc_new(account)));

	profile = loqui_account_get_profile(account);
	
	user = LOQUI_USER(loqui_user_irc_new());
	loqui_user_set_nick(user, loqui_profile_account_get_nick(profile));
	loqui_user_set_away(user, LOQUI_AWAY_TYPE_OFFLINE);
	
	loqui_account_set_user_self(account, user);
	loqui_account_add_user(account, user);
	g_object_unref(user); /* the account has reference */

	codeconv = loqui_codeconv_new();
	loqui_codeconv_set_table(codeconv, loqui_protocol_get_codeconv_table(profile->protocol));
	priv->codeconv = codeconv;

	return object;
}
LoquiAccountIRC*
loqui_account_irc_new(LoquiProfileAccount *profile)
{
        LoquiAccountIRC *account;
	LoquiAccountIRCPrivate *priv;

	account = g_object_new(loqui_account_irc_get_type(), 
			       "profile", profile,
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

static void
loqui_account_irc_connect(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;
	LoquiProfileAccount *profile;
	const gchar *servername;
	gint port;
	GError *error = NULL;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (loqui_account_get_is_connected(account)) {
		loqui_account_warning(account, _("Already connected."));
		return;
	}
	
	profile = loqui_account_get_profile(account);
	servername = loqui_profile_account_get_servername(profile);
	port = loqui_profile_account_get_port(profile);
	
	loqui_codeconv_set_mode(priv->codeconv, loqui_profile_account_get_codeconv_mode(profile));
	loqui_codeconv_set_table_item_name(priv->codeconv, loqui_profile_account_get_codeconv_item_name(profile));
	loqui_codeconv_set_codeset(priv->codeconv, loqui_profile_account_get_codeset(profile));
	
	loqui_account_set_is_connected(account, TRUE);

	if (!loqui_codeconv_update(priv->codeconv, &error)) {
		loqui_account_warning(account,
				      "Failed to update code converter: %s",
				      error->message);
		return;
	}

        priv->sock_client = g_socket_client_new();
        g_socket_client_connect_to_host_async(priv->sock_client, servername, port, NULL,
                                              (GAsyncReadyCallback) loqui_account_irc_sock_client_connected_cb, account);
        
	loqui_account_information(account, _("Connecting to %s:%d"), servername, port);
	loqui_debug_puts(_("Connecting to %s:%d"), servername, port);
}
static void
loqui_account_irc_sock_client_connected_cb(GSocketClient *sock_client, GAsyncResult *res, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	LoquiSenderIRC *sender;
	const gchar *password, *nick, *username, *realname;
        GError *error = NULL;
        
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;
	
	priv->sock_conn = g_socket_client_connect_to_host_finish(sock_client, res, &error);
        if (priv->sock_conn == NULL) {
		loqui_account_warning(LOQUI_ACCOUNT(account), _("Failed to connect: %s"),
		                      error->message);
		g_error_free(error);
		
		loqui_account_closed(LOQUI_ACCOUNT(account), FALSE);
		return;
	}
        
        priv->data_in_stream = g_data_input_stream_new(g_io_stream_get_input_stream(G_IO_STREAM(priv->sock_conn)));
        
        priv->out_stream = g_io_stream_get_output_stream(G_IO_STREAM(priv->sock_conn));
        g_object_ref(priv->out_stream); /* GIOStraem owns out_stream */
        
        loqui_debug_puts(_("Connected."));
	loqui_account_information(LOQUI_ACCOUNT(account), _("Connected."));
	
	password = loqui_profile_account_get_password(loqui_account_get_profile(LOQUI_ACCOUNT(account)));
	nick = loqui_profile_account_get_nick(loqui_account_get_profile(LOQUI_ACCOUNT(account)));	
	username = loqui_profile_account_get_username(loqui_account_get_profile(LOQUI_ACCOUNT(account)));
	realname = loqui_profile_account_irc_get_realname(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(LOQUI_ACCOUNT(account))));		
	sender = LOQUI_SENDER_IRC(loqui_account_get_sender(LOQUI_ACCOUNT(account)));

	if (password && strlen(password) > 0) {
		loqui_sender_irc_pass(sender, password);
		loqui_debug_puts("Sending PASS...");
	}

	loqui_sender_nick(LOQUI_SENDER(sender), nick);
	loqui_debug_puts("Sending NICK...");
	loqui_user_set_nick(LOQUI_ACCOUNT(account)->user_self, nick);

	loqui_sender_irc_user_raw(sender, username, realname);
	loqui_debug_puts("Sending USER...");

	loqui_account_information(LOQUI_ACCOUNT(account), _("Sent initial commands."));

	loqui_user_set_away(LOQUI_ACCOUNT(account)->user_self, LOQUI_AWAY_TYPE_ONLINE);

        g_data_input_stream_read_line_async(priv->data_in_stream,
                                            G_PRIORITY_DEFAULT,
                                            NULL,
                                            (GAsyncReadyCallback) loqui_account_irc_stream_readline_cb,
                                            account);
}

static void
loqui_account_irc_closed(LoquiAccount *account, gboolean is_success)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	loqui_account_information(LOQUI_ACCOUNT(account), _("Connection closed."));
	
	if (priv->out_stream) {
		g_object_unref(priv->out_stream);
		priv->out_stream = NULL;
	}
	if (priv->data_in_stream) {
		g_object_unref(priv->data_in_stream);
		priv->data_in_stream = NULL;
	}
	if (priv->sock_conn) {
		if (!g_io_stream_is_closed(G_IO_STREAM(priv->sock_conn))) {
			g_io_stream_close(G_IO_STREAM(priv->sock_conn), NULL, NULL); // fail-safe?
		}
		
		g_object_unref(priv->sock_conn);
		priv->sock_conn = NULL;
	}
	if (priv->sock_client) {
		g_object_unref(priv->sock_client);
		priv->sock_client = NULL;
	}
	
	g_queue_foreach(priv->msg_queue, (GFunc) g_object_unref, NULL);

	loqui_account_remove_all_user(LOQUI_ACCOUNT(account));
	loqui_account_set_all_channel_unjoined(LOQUI_ACCOUNT(account));

	loqui_user_set_away(loqui_account_get_user_self(LOQUI_ACCOUNT(account)), LOQUI_AWAY_TYPE_OFFLINE);

	loqui_receiver_irc_reset(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver));
	loqui_sender_irc_reset(LOQUI_SENDER_IRC(LOQUI_ACCOUNT(account)->sender));
	
	loqui_account_set_is_connected(LOQUI_ACCOUNT(account), FALSE);
	
	if (LOQUI_ACCOUNT_CLASS(parent_class)->closed)
		(* LOQUI_ACCOUNT_CLASS(parent_class)->closed) (account, is_success);
}

static void
loqui_account_irc_stream_readline_cb(GDataInputStream *stream, GAsyncResult *res, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	IRCMessage *msg;
	gchar *local;
	GError *error = NULL;
	gchar *buffer;
	gsize size = 0;
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	/* FIXME: This condition should be handled by cancelling.
	   This is caused by terminate(force closing) -> connect */
	if (priv->data_in_stream != stream) {
		return; /* ignore */
	}
	
	buffer = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(stream), res, &size, &error);
	
	if (buffer == NULL) {
		if (error == NULL) {
			/* EOF. just connection close */
			loqui_account_closed(LOQUI_ACCOUNT(account), TRUE);
		} else {
			/* error */
			loqui_debug_puts(_("An error occured on receiving: %s"), error->message);
			loqui_account_warning(LOQUI_ACCOUNT(account), _("An error occured on receiving: %s"), 
				              error->message);
			g_error_free(error);
			
			loqui_account_closed(LOQUI_ACCOUNT(account), FALSE);
		}
		return;
	}

	local = loqui_codeconv_to_local(priv->codeconv, buffer, &error);
	if (local == NULL) {
		loqui_account_warning(LOQUI_ACCOUNT(account), "Failed to convert codeset: (%s).", error->message);
		g_error_free(error);
		goto nextline;
	}
	g_free(buffer);
	
	msg = irc_message_parse_line(local);
	if (msg == NULL) {
		loqui_account_warning(LOQUI_ACCOUNT(account), "Failed to parse a line: %s", local);
		g_free(local);
		goto nextline;
	}
	g_free(local);

	if (loqui_core_get_show_msg_mode(loqui_get_core())) {
		gchar *tmp;

		tmp = irc_message_inspect(msg);
		loqui_account_print_communication(LOQUI_ACCOUNT(account), TRUE, tmp);
		g_free(tmp);
	}

	loqui_receiver_irc_response(LOQUI_RECEIVER_IRC(LOQUI_ACCOUNT(account)->receiver), msg);
	g_object_unref(msg);

nextline:
	if (g_io_stream_is_closed(G_IO_STREAM(priv->sock_conn))) {
		loqui_account_closed(LOQUI_ACCOUNT(account), TRUE);
		
		return;
	}
	
        g_data_input_stream_read_line_async(priv->data_in_stream,
                                            G_PRIORITY_DEFAULT,
                                            NULL, // TODO: cancellable
                                            (GAsyncReadyCallback) loqui_account_irc_stream_readline_cb,
                                            account);

	return;
}
static void
loqui_account_irc_flush_queue_one(LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	gchar *buf, *serv_str, *line;
	GError *error = NULL;
	IRCMessage *msg;
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
        
	priv = LOQUI_ACCOUNT_IRC(account)->priv;
        
        /* if has pending, skip flushing now. write_cb will flush again. */
        if (g_output_stream_has_pending(priv->out_stream)) {
        	return;
        }

	if ((msg = g_queue_pop_head(priv->msg_queue)) == NULL) {
		return;
	}

	buf = irc_message_to_string(msg);
	if ((serv_str = loqui_codeconv_to_server(priv->codeconv, buf, &error)) == NULL) {
		loqui_account_warning(LOQUI_ACCOUNT(account), "Failed to convert codeset(%s): %s.", error->message, buf);
		g_error_free(error);
		g_free(buf);
		g_object_unref(msg);
		return;
	}
	g_free(buf);
	
	line = g_strdup_printf("%s\r\n", serv_str);
	g_free(serv_str);

	g_output_stream_write_async(priv->out_stream, line, strlen(line),
                                    G_PRIORITY_DEFAULT,
                                    NULL, // TODO: cancellable
                                    (GAsyncReadyCallback) loqui_account_irc_stream_write_cb,
                                    account);

	if (loqui_core_get_show_msg_mode(loqui_get_core())) {
		gchar *tmp;

		tmp = irc_message_inspect(msg);
		loqui_account_print_communication(LOQUI_ACCOUNT(account), FALSE, tmp);
		g_free(tmp);
	}

	loqui_sender_irc_message_sent(LOQUI_SENDER_IRC(loqui_account_get_sender(LOQUI_ACCOUNT(account))), msg);
	g_object_unref(msg);
}

static void
loqui_account_irc_stream_write_cb(GOutputStream *stream, GAsyncResult *res, LoquiAccountIRC *account)
{
	LoquiAccountIRCPrivate *priv;
	GError *error = NULL;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;
	
	/* FIXME: This condition should be handled by cancelling.
	   This is caused by terminate(force closing) -> connect */
	if (priv->out_stream != stream) {
		return; /* ignore */
	}
	
	g_output_stream_write_finish(stream, res, &error);
	
	if (error != NULL) {
		loqui_debug_puts(_("An error occured on sending: %s"), error->message);
		loqui_account_warning(LOQUI_ACCOUNT(account), _("An error occured on sending: %s"), 
		                      error->message);
		g_error_free(error);
		
		loqui_account_closed(LOQUI_ACCOUNT(account), FALSE);
		
		return;
	}
	
	loqui_account_irc_flush_queue_one(account);
}

static void
loqui_account_irc_disconnect(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (LOQUI_ACCOUNT_CLASS(parent_class)->disconnect)
		(* LOQUI_ACCOUNT_CLASS(parent_class)->disconnect) (account);

	if (priv->sock_conn && LOQUI_RECEIVER_IRC(loqui_account_get_receiver(account))->passed_welcome) {
		loqui_sender_quit(loqui_account_get_sender(account),
				  loqui_profile_account_irc_get_quit_message(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account))));
	} else {
		loqui_account_irc_terminate(account);
	}
}
static void
loqui_account_irc_terminate(LoquiAccount *account)
{
	LoquiAccountIRCPrivate *priv;

        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));
	
	priv = LOQUI_ACCOUNT_IRC(account)->priv;

	if (LOQUI_ACCOUNT_CLASS(parent_class)->terminate)
		(* LOQUI_ACCOUNT_CLASS(parent_class)->terminate) (account);

	if (priv->sock_conn) {
		loqui_account_information(LOQUI_ACCOUNT(account), _("Terminating the connection."));

		g_io_stream_close(G_IO_STREAM(priv->sock_conn), NULL, NULL); /* TODO: check error, need async? */

		loqui_account_closed(LOQUI_ACCOUNT(account), TRUE);
	}
}
gboolean
loqui_account_irc_is_current_nick(LoquiAccountIRC *account, const gchar *str)
{
	LoquiAccountIRCPrivate *priv;

        g_return_val_if_fail(account != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), FALSE);

	priv = account->priv;

	if(str == NULL)
		return FALSE;
	
	return (strcmp(loqui_user_get_nick(loqui_account_get_user_self(LOQUI_ACCOUNT(account))), str) == 0 ? TRUE : FALSE);
}
void
loqui_account_irc_push_message(LoquiAccountIRC *account, IRCMessage *msg)
{
	LoquiAccountIRCPrivate *priv;
	
        g_return_if_fail(account != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_IRC(account));

	priv = account->priv;
	
	if (!priv->sock_conn) { /* TODO: fix condition */
		loqui_account_warning(LOQUI_ACCOUNT(account), _("The account is not connected."));
		return;
	}
	
	g_object_ref(msg);
	g_queue_push_tail(priv->msg_queue, msg);
	
	loqui_account_irc_flush_queue_one(account);
}

LoquiChannel *
loqui_account_irc_fetch_channel(LoquiAccountIRC *account, gboolean is_self, const gchar *msg_nick, const gchar *msg_target)
{
	LoquiChannel *channel;
	const gchar *name;
	gboolean is_priv;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), NULL);
	g_return_val_if_fail(msg_target != NULL, NULL);

	is_priv = !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(msg_target);

	if (is_priv)
		name = is_self ? msg_target : msg_nick;
	else
		name = msg_target;

	if (name == NULL)
		return NULL;

	channel = loqui_account_get_channel_by_identifier(LOQUI_ACCOUNT(account), name);
	if (channel == NULL) {
		if (is_priv) {
			channel = loqui_account_irc_add_private_talk_with_nick(account, name);
		} else {
			channel = LOQUI_CHANNEL(loqui_channel_irc_new(LOQUI_ACCOUNT(account), name, FALSE, is_priv));
			loqui_account_add_channel(LOQUI_ACCOUNT(account), channel);
			g_object_unref(channel);
		}
	}
	return channel;
}
LoquiChannel *
loqui_account_irc_add_private_talk_with_nick(LoquiAccountIRC *account, const gchar *nick)
{
	LoquiAccountIRCPrivate *priv;
	LoquiChannel *channel;
	LoquiMember *member;

        g_return_val_if_fail(account != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_IRC(account), NULL);

	priv = account->priv;
	
	channel = LOQUI_CHANNEL(loqui_channel_irc_new(LOQUI_ACCOUNT(account), nick, FALSE, TRUE));

	member = loqui_member_new(loqui_account_get_user_self(LOQUI_ACCOUNT(account)));
	loqui_channel_entry_add_member(LOQUI_CHANNEL_ENTRY(channel), member);
	g_object_unref(member);
	
	if (!loqui_account_irc_is_current_nick(account, nick))
		loqui_channel_irc_add_member_by_nick(LOQUI_CHANNEL_IRC(channel), nick, FALSE, FALSE, FALSE);

	loqui_account_add_channel(LOQUI_ACCOUNT(account), channel);
	g_object_unref(channel);

	return channel;
}
