/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
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
#include "utils.h"
#include "account.h"
#include "account_manager.h"
#include "irc_constants.h"
#include "main.h"
#include "intl.h"
#include "prefs_general.h"
#include "ctcp_message.h"
#include "ctcp_handle.h"

#include "codeconv.h"
#include <string.h>

struct _IRCHandlePrivate
{
	CTCPHandle *ctcp_handle;

	Account *account;
	gboolean fallback;

	gboolean end_motd;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void irc_handle_class_init(IRCHandleClass *klass);
static void irc_handle_init(IRCHandle *irc_handle);
static void irc_handle_finalize(GObject *object);

static gboolean irc_handle_command(IRCHandle *handle, IRCMessage *msg);
static gboolean irc_handle_reply(IRCHandle *handle, IRCMessage *msg);
static gboolean irc_handle_error(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_inspect_message(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_command_privmsg_notice(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_ping(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_quit(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_join(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_part(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_nick(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_mode(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_command_kick(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_account_console_append(IRCHandle *handle, IRCMessage *msg, TextType type, gchar *format);
static void irc_handle_channel_append(IRCHandle *handle, IRCMessage *msg, gboolean make_channel,
				      gint receiver_num, TextType type, gchar *format);
static void irc_handle_joined_channel_append(IRCHandle *handle, IRCMessage *msg, GSList *channel_slist, TextType type, gchar *format);

static void irc_handle_reply_names(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_reply_topic(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_reply_endofnames(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_reply_channelmodeis(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_reply_creationtime(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_reply_topicwhotime(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_reply_whoisidle(IRCHandle *handle, IRCMessage *msg);
static void irc_handle_error_nick_unusable(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_parse_mode_arguments(IRCHandle *handle, IRCMessage *msg, Channel *channel, gint mode_start);
static gboolean irc_handle_parse_plum_recent(IRCHandle *handle, const gchar *line);

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
	IRCHandle *handle;
	IRCHandlePrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IRC_HANDLE(object));

        handle = IRC_HANDLE(object);
	priv = handle->priv;
	
        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(handle->priv);
}
static gboolean
irc_handle_parse_plum_recent(IRCHandle *handle, const gchar *line)
{
	gchar *buf, *cur, *name;
	gchar prefix;
	Channel *channel;
	IRCHandlePrivate *priv;

        g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), FALSE);
	g_return_val_if_fail(line != NULL, FALSE);

	priv = handle->priv;

	cur = buf = g_strdup(line);

	if(!g_ascii_isdigit(*cur) ||
	   !g_ascii_isdigit(*(++cur)) ||
	   *(++cur) != ':' ||
	   !g_ascii_isdigit(*(++cur)) ||
	   !g_ascii_isdigit(*(++cur)) ||
	   *(++cur) != ' ')
		goto error;
	prefix = *(++cur);
	if(prefix != '<' && prefix != '>' && prefix != '=')
		goto error;
	name = ++cur;

	switch(prefix) {
	case '<':
		if((cur = strstr(buf, "> ")) == NULL)
			goto error;
		*cur = '\0';
		
		if((cur = strrchr(buf, ':')) == NULL)
			goto error;
		*cur = '\0';

		break;
	case '>':
		if((cur = strstr(buf, "< ")) == NULL)
			goto error;
		*cur = '\0';

		if((cur = strrchr(buf, ':')) != NULL)
			*cur = '\0';

		break;
	case '=':
		if((cur = strstr(buf, "= ")) == NULL)
			goto error;
		*cur = '\0';

		break;
	default:
		g_assert_not_reached();
	}
	
	channel = account_get_channel(priv->account, name);
	if(channel == NULL) {
		channel = channel_new(priv->account, name);
		account_add_channel(priv->account, channel);
	}
	g_free(buf);

	buf = g_strdup_printf("[LOG] %s", line);
	channel_append_text(channel, TEXT_TYPE_NOTICE, buf);
	channel_set_updated(channel, TRUE);
	g_free(buf);

	return TRUE;

 error:
	g_free(buf);
	return FALSE;
}
static void
irc_handle_command_privmsg_notice(IRCHandle *handle, IRCMessage *msg)
{
	IRCHandlePrivate *priv;
	gchar *receiver_name, *sender_nick;
	gchar *channel_name;
	gchar *remark;
	Channel *channel = NULL;
	TextType type;
	CTCPMessage *ctcp_msg;
	gboolean is_self;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	priv = handle->priv;

	receiver_name = irc_message_get_param(msg, 1);
	sender_nick = msg->nick; /* null if message was sent by a server */
	remark = irc_message_get_param(msg, 2);

	if(remark == NULL) {
		g_warning(_("This PRIVMSG/NOTICE message doesn't contain a remark."));
		return;
	}

	if(sender_nick)
		is_self = account_is_current_nick(handle->priv->account, sender_nick);
	else
		is_self = FALSE;

	if(priv->end_motd == FALSE &&
	   msg->response == IRC_COMMAND_NOTICE &&
	   prefs_general.parse_plum_recent) {
		if(irc_handle_parse_plum_recent(handle, remark))
			return;
	}

	if(msg->response == IRC_COMMAND_NOTICE) {
		type = TEXT_TYPE_NOTICE;
	} else {
		type = TEXT_TYPE_NORMAL;
	}

	if(sender_nick) {
		if(ctcp_message_parse_line(remark, &ctcp_msg)) {
			g_object_set_data(G_OBJECT(ctcp_msg), "sender", msg->nick);
			g_object_set_data(G_OBJECT(ctcp_msg), "receiver", receiver_name);
			ctcp_handle_message(priv->ctcp_handle, ctcp_msg,
					    (msg->response == IRC_COMMAND_PRIVMSG) ? TRUE : FALSE);
			g_object_unref(ctcp_msg);
		        return;
                }
	}

	if(receiver_name != NULL && sender_nick != NULL) {
		if(STRING_IS_CHANNEL(receiver_name)) {
			channel_name = receiver_name;
		} else {
			if(is_self)
				channel_name = receiver_name;
			else
				channel_name = sender_nick;
		}

		channel = account_get_channel(priv->account, channel_name);
		if(channel == NULL) {
			channel = channel_new(priv->account, channel_name);
			account_add_channel(priv->account, channel);
		}
	}
	
	if(channel != NULL) {
		channel_append_remark(channel, type, is_self, sender_nick, remark);
	} else {
		account_console_buffer_append(priv->account, type, remark);
	}
}
static void
irc_handle_command_ping(IRCHandle *handle, IRCMessage *msg)
{
        g_return_if_fail(handle != NULL);
	g_return_if_fail(IS_IRC_HANDLE(handle));

	account_pong(handle->priv->account, irc_message_get_param(msg, 1));
}
static void
irc_handle_command_quit(IRCHandle *handle, IRCMessage *msg)
{
	GSList *slist, *cur;
	Channel *channel;

	if(msg->nick == NULL) {
		g_warning(_("The message does not contain nick"));
		return;
	}

	slist = account_search_joined_channel(handle->priv->account, msg->nick);
	for(cur = slist; cur != NULL; cur = cur->next) {
		channel = (Channel *) cur->data;
		if(!channel) {
			g_warning("NULL channel");
			continue;
		}
		channel_remove_user(channel, msg->nick);
	}

	irc_handle_joined_channel_append(handle, msg, slist, TEXT_TYPE_INFO, _("*** %n has quit IRC(%t)"));

	g_slist_free(slist);
}
static void
irc_handle_command_part(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *name;

	if(msg->nick == NULL) {
		g_warning(_("The message does not contain nick"));
		return;
	}

	name = irc_message_get_param(msg, 1);
	if(!name) {
		g_warning(_("The message does not contain the channal name"));
		return;
	}

	channel = account_get_channel(handle->priv->account, name);
	if(channel)
		channel_remove_user(channel, msg->nick);
	
	if(account_is_current_nick(handle->priv->account, msg->nick)) {
		if(channel) {
			irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** You has left %1");
			account_remove_channel(handle->priv->account, channel);
		}
	} else {
		irc_handle_channel_append(handle, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** %n has just part %1(%t)"));
	}
}
static void
irc_handle_command_kick(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *name;
	gchar *sender, *receiver;

	sender = msg->nick;
	if(sender == NULL) {
		g_warning(_("The message does not contain nick"));
		return;
	}

	name = irc_message_get_param(msg, 1);
	if(!name) {
		g_warning(_("The message does not contain the channal name"));
		return;
	}

	receiver = irc_message_get_param(msg, 2);
	if(!receiver) {
		g_warning(_("The KICK message doesn't contain the user to be kicked."));
		return;
	}
	
	channel = account_get_channel(handle->priv->account, name);
	if(channel)
		channel_remove_user(channel, receiver);

	if(account_is_current_nick(handle->priv->account, receiver)) {
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** You were kicked from %1 by %n (%3)");
		account_remove_channel(handle->priv->account, channel);
	} else {
		irc_handle_channel_append(handle, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** %2 was kicked from %1 by %n(%3)"));
	}
}
static void
irc_handle_command_nick(IRCHandle *handle, IRCMessage *msg)
{
	GSList *slist, *cur;
	gchar *nick_new;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	if(msg->nick == NULL) {
		g_warning(_("The message does not contain nick"));
		return;
	}

	nick_new = irc_message_get_param(msg, 1);
	if(nick_new == NULL) {
		g_warning(_("The NICK message does not contain new nick"));
		return;
	}

	slist = account_search_joined_channel(handle->priv->account, msg->nick);
	for(cur = slist; cur != NULL; cur = cur->next) {
		channel_change_user_nick((Channel *) cur->data, msg->nick, nick_new);
	}

	irc_handle_joined_channel_append(handle, msg, slist, TEXT_TYPE_INFO, _("*** %n is known as %1"));

	if(account_is_current_nick(handle->priv->account, msg->nick))
		account_set_current_nick(handle->priv->account, nick_new);

	if(slist)
		g_slist_free(slist);
}

static void
irc_handle_parse_mode_arguments(IRCHandle *handle, IRCMessage *msg, Channel *channel, gint mode_start)
{
	gint cur;
	gint param_num;
	gchar *flags, *target;
	gint is_add = -1; /* -1: uninitialized, 0: false, 1: true */

	cur = mode_start;
	param_num = irc_message_count_parameters(msg);


	flags = irc_message_get_param(msg, cur);
	if(flags == NULL) {
		g_warning(_("Flags are not found in MODE command"));
		return;
	}
	cur++;

#define GET_TARGET_OR_RETURN(msg, i, str_ptr) { \
  *str_ptr = irc_message_get_param(msg, i); \
  if(*str_ptr == NULL) { \
	g_warning(_("Can't find a nick to change mode")); \
	return; \
  } \
  i++; \
}
#define BREAK_IF_ADD_FLAG_IS_UNINITIALIZED() if(is_add < 0) { g_warning(_("Flags don't have + or -")); break; }

	if(channel) {
		while (*flags) {
			switch(*flags) {
			case '+':
				is_add = 1;
				break;
			case '-':
				is_add = 0;
				break;
			case IRC_CHANNEL_MODE_OPERATOR:
				BREAK_IF_ADD_FLAG_IS_UNINITIALIZED();
				GET_TARGET_OR_RETURN(msg, cur, &target);
				channel_change_user_power(channel, target,
							  is_add ? USER_POWER_OP : USER_POWER_NOTHING); /* FIXME */
				break;
			case IRC_CHANNEL_MODE_VOICE:
				BREAK_IF_ADD_FLAG_IS_UNINITIALIZED();
				GET_TARGET_OR_RETURN(msg, cur, &target);
				channel_change_user_power(channel, target,
							  is_add ? USER_POWER_VOICE : USER_POWER_NOTHING); /* FIXME */
				break;
			case IRC_CHANNEL_MODE_CREATOR:
				BREAK_IF_ADD_FLAG_IS_UNINITIALIZED();
				break;
			case IRC_CHANNEL_MODE_ANONYMOUS:
			case IRC_CHANNEL_MODE_INVITE_ONLY:
			case IRC_CHANNEL_MODE_MODERATED:
			case IRC_CHANNEL_MODE_NO_MESSAGES_FROM_CLIENT:
			case IRC_CHANNEL_MODE_QUIET:
			case IRC_CHANNEL_MODE_SECRET:
			case IRC_CHANNEL_MODE_SERVER_REOP:
			case IRC_CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY:
				BREAK_IF_ADD_FLAG_IS_UNINITIALIZED();
				channel_change_mode(channel, (gboolean) is_add, *flags, NULL);
				break;
			case IRC_CHANNEL_MODE_CHANNEL_KEY:
			case IRC_CHANNEL_MODE_USER_LIMIT:
				BREAK_IF_ADD_FLAG_IS_UNINITIALIZED();
				GET_TARGET_OR_RETURN(msg, cur, &target);
				channel_change_mode(channel, (gboolean) is_add, *flags, NULL);
				break;
			case IRC_CHANNEL_MODE_BAN_MASK:
			case IRC_CHANNEL_MODE_EXCEPTION_TO_OVERIDE_BAN_MASK:
			case IRC_CHANNEL_MODE_INVITATION_MASK:
				BREAK_IF_ADD_FLAG_IS_UNINITIALIZED();
				break;
			default:
				g_warning(_("Unknown mode flag"));
				break;
			}
			flags++;
		}
	} else {
		/* FIXME: handle user modes */
		switch(*flags) {
		case IRC_USER_MODE_FLAGGED_AS_AWAY:
		case IRC_USER_MODE_INVISIBLE:
		case IRC_USER_MODE_RECEIVES_WALLOPS:
		case IRC_USER_MODE_RESTRICTED_CONNECTION:
		case IRC_USER_MODE_OPERATOR:
		case IRC_USER_MODE_LOCAL_OPERATOR:
		case IRC_USER_MODE_RECEIVES_SERVER_NOTICES:
		default:
			g_warning(_("User mode is not implemented."));
			break;
		}
	}
}
static void
irc_handle_reply_channelmodeis(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gint cur;
	gchar *name;

	cur = 2;

	name = irc_message_get_param(msg, cur);
	if(name == NULL) {
		g_warning(_("The target is not found in MODE command"));
		return;
	}
	cur++;

	if(STRING_IS_CHANNEL(name)) {
		channel = account_get_channel(handle->priv->account, name);
		if(!channel)
			return;
	} else {
		g_warning(_("RPL_CHANNELMODEIS didn't return a channel name: %s"), name);
		return;
	}
	channel_clear_mode(channel);

	irc_handle_parse_mode_arguments(handle, msg, channel, 3);
	irc_handle_channel_append(handle, msg, FALSE, 2, TEXT_TYPE_INFO, _("*** Mode for %2: %*3"));
}
static void
irc_handle_command_mode(IRCHandle *handle, IRCMessage *msg)
{
	gchar *changer = NULL;
	gchar *format, *name;
	Channel *channel = NULL;
	gint cur;

	if(msg->nick)
		changer = msg->nick;
	else if(msg->prefix)
		changer = msg->prefix;
	else {
		g_warning(_("Who can change mode?"));
		return;
	}

	if(strchr(changer, '%')) {
		g_warning(_("Nick must not contain '%%'"));
		return;
	}

	cur = 1;

	name = irc_message_get_param(msg, cur);
	if(name == NULL) {
		g_warning(_("The target is not found in MODE command"));
		return;
	}
	cur++;

	if(STRING_IS_CHANNEL(name)) {
		channel = account_get_channel(handle->priv->account, name);
		if(!channel) {
			g_warning(_("Why can you know the change of his mode?"));
			return;
		}
	} else {
		channel = NULL;
	}

	irc_handle_parse_mode_arguments(handle, msg, channel, cur);

	format = g_strdup_printf(_("*** New mode for %%1 by %s: %%*2"), changer);

	if(channel)
		irc_handle_channel_append(handle, msg, FALSE, 1, TEXT_TYPE_INFO, format);
	else
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, format);

	g_free(format);
#undef GET_TARGET_OR_RETURN
}
static void
irc_handle_command_join(IRCHandle *handle, IRCMessage *msg)
{
	gchar *name;
	Channel *channel;
	IRCHandlePrivate *priv;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));
	g_return_if_fail(msg != NULL);

	priv = handle->priv;

	if(msg->nick == NULL) {
		g_warning(_("The message does not contain nick."));
		return;
	}
	name = irc_message_get_param(msg, 1);
	if(name == NULL) {
		g_warning(_("Invalid JOIN command"));
		return;
	}
	
	channel = account_get_channel(handle->priv->account, name);
	if(account_is_current_nick(handle->priv->account, msg->nick)) {
		if(!channel) {
			channel = channel_new(priv->account, name);
			account_add_channel(priv->account, channel);
		}
		account_get_channel_mode(handle->priv->account, channel_get_name(channel));
	} else {
		if(!channel) {
			g_warning(_("Why do you know that the user join the channel?"));
			return;
		}
		channel_append_user(channel, msg->nick, USER_POWER_NOTHING, USER_EXISTENCE_UNKNOWN);
		irc_handle_channel_append(handle, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** %n (%u@%h) joined channel %t"));
	}
}
static void
irc_handle_inspect_message(IRCHandle *handle, IRCMessage *msg)
{
	gchar *str;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	str = irc_message_inspect(msg);
	
	account_console_buffer_append(handle->priv->account, TEXT_TYPE_NORMAL, str);
	
	g_free(str);
}
static void
irc_handle_reply_names(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *name;
	gchar **nick_array;
	gint i;

	name = irc_message_get_param(msg, 3);
	channel = account_get_channel(handle->priv->account, name);
	if(channel == NULL)
		return;

	if(channel->end_names == TRUE) {
		channel_clear_user(channel);
		channel->end_names = FALSE;
	}

	nick_array = g_strsplit(irc_message_get_trailing(msg), " ", 0);
	for(i = 0; nick_array[i] != NULL; i++) {
		channel_append_user(channel, nick_array[i],
				    USER_POWER_UNDETERMINED, USER_EXISTENCE_UNKNOWN);
	}
	g_strfreev(nick_array);

	irc_handle_channel_append(handle, msg, FALSE, 3, TEXT_TYPE_NORMAL, "%3: %t");
}
static void
irc_handle_reply_endofnames(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *name;

	name = irc_message_get_param(msg, 2);
	channel = account_get_channel(handle->priv->account, name);
	if(channel == NULL)
		return;

	channel->end_names = TRUE;
}

static void
irc_handle_reply_creationtime(IRCHandle *handle, IRCMessage *msg)
{
	gchar *time_str;
	time_t t;
	gchar *str;
	gchar *format;

	time_str = irc_message_get_param(msg, 3);
	if(time_str == NULL) {
		g_warning(_("Invalid CREATIONTIME reply."));
		return;
	}
	t = (time_t) g_ascii_strtoull(time_str, NULL, 10);
	if(t == 0) {
		g_warning(_("Invalid time string"));
		return;
	}
	
	str = utils_get_iso8601_date_string(t);
	if(str == NULL) {
		g_warning(_("Invalid time"));
		return;
	}

	format = g_strdup_printf(_("%%2 was created at %s"), str);
	g_free(str);

	irc_handle_channel_append(handle, msg, TRUE, 2, TEXT_TYPE_INFO, format);
	g_free(format);
}

static void
irc_handle_reply_topicwhotime(IRCHandle *handle, IRCMessage *msg)
{
	gchar *time_str;
	time_t t;
	gchar *str;
	gchar *format;

	time_str = irc_message_get_param(msg, 4);
	if(time_str == NULL) {
		g_warning(_("Invalid TOPICWHOTIME reply."));
		return;
	}

	t = (time_t) g_ascii_strtoull(time_str, NULL, 10);
	if(t == 0) {
		g_warning(_("Invalid time string"));
		return;
	}
	
	str = utils_get_iso8601_date_string(t);
	if(str == NULL) {
		g_warning(_("Invalid time"));
		return;
	}

	format = g_strdup_printf(_("Topic for %%2 was set by %%3 at %s"), str);
	g_free(str);

	irc_handle_channel_append(handle, msg, TRUE, 2, TEXT_TYPE_INFO, format);
	g_free(format);
}
static void
irc_handle_reply_whoisidle(IRCHandle *handle, IRCMessage *msg)
{
	gchar *str;
	gchar *sec_str;
	gint sec;

	sec_str = irc_message_get_param(msg, 3);
	if(!sec_str) {
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_ERROR, _("Invalid WHOISIDLE reply."));
		return;
	}

	sec = (gint) g_ascii_strtoull(sec_str, NULL, 10);
	if((*sec_str != '0' || *(sec_str+1) != '\0') && sec == 0) {
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_ERROR, _("Invalid WHOISIDLE reply."));
		return;
	}

	str = g_strdup_printf("Idle time: %d day(s), %.2d:%.2d:%.2d",
			      sec / (24 * 60 * 60),
			      (sec % (24 * 60 * 60)) / (60 * 60),
			      (sec % (60 * 60)) / 60,
			      (sec % 60));

	irc_handle_account_console_append(handle, msg, TEXT_TYPE_NORMAL, str);
	g_free(str);
}

static void
irc_handle_reply_topic(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *topic;
	gchar *name;

	name = irc_message_get_param(msg, 2);
	channel = account_get_channel(handle->priv->account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_trailing(msg);
	channel_set_topic(channel, topic);

	irc_handle_channel_append(handle, msg, FALSE, 2, TEXT_TYPE_INFO, _("Topic for %2: %t"));
}

static void
irc_handle_command_topic(IRCHandle *handle, IRCMessage *msg)
{
	Channel *channel;
	gchar *topic;
	gchar *name;

	name = irc_message_get_param(msg, 1);
	channel = account_get_channel(handle->priv->account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_trailing(msg);
	channel_set_topic(channel, topic);
	
	irc_handle_channel_append(handle, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** New topic on %1 by %n: %t"));
}
/* TODO: change nick */
static void
irc_handle_error_nick_unusable(IRCHandle *handle, IRCMessage *msg)
{
	IRCHandlePrivate *priv;

	priv = handle->priv;

	irc_handle_account_console_append(handle, msg, TEXT_TYPE_ERROR, "%t");

	if(!priv->end_motd)
		account_disconnect(handle->priv->account);
}
static void /* utility function */
irc_handle_joined_channel_append(IRCHandle *handle, IRCMessage *msg, GSList *channel_slist, TextType type, gchar *format)
{
	gchar *str;
	GSList *slist = NULL, *cur;
	Channel *channel;
	MessageText *msgtext;

	str = irc_message_format(msg, format);

	if(channel_slist == NULL) {
		if(msg->nick)
			slist = account_search_joined_channel(handle->priv->account, msg->nick);
	} else {
		slist = channel_slist;
	}

	msgtext = message_text_new();
	g_object_set(G_OBJECT(msgtext), 
		     "is_remark", FALSE,
		     "text_type", type,
		     "text", str, NULL);

	if(slist != NULL) {
		for(cur = slist; cur != NULL; cur = cur->next) {
			channel = CHANNEL(cur->data);
			channel_buffer_append_message_text(channel->buffer, msgtext, FALSE, FALSE);
		}
	} else {
		account_console_buffer_append(handle->priv->account, type, str);
	}
	g_object_unref(msgtext);

	g_free(str);
	if(channel_slist == NULL && slist != NULL)
		g_slist_free(slist);
}
static void /* utility function */
irc_handle_account_console_append(IRCHandle *handle, IRCMessage *msg, TextType type, gchar *format)
{
	gchar *str;

	str = irc_message_format(msg, format);

	account_console_buffer_append(handle->priv->account, type, str);

	g_free(str);
}

static void /* utility function */
irc_handle_channel_append(IRCHandle *handle, IRCMessage *msg, gboolean make_channel,
			  gint receiver_num, TextType type, gchar *format)
{
	IRCHandlePrivate *priv;
	Channel *channel;
	gchar *str;
	gchar *receiver_name;

	priv = handle->priv;

	receiver_name = irc_message_get_param(msg, receiver_num);
	if(receiver_name == NULL) {
		g_warning(_("Can't find the channel from the message"));
		return;
	}

	channel = account_get_channel(priv->account, receiver_name);
	if(make_channel == TRUE && channel == NULL) { /* FIXME as well as privmsg_notice */
		channel = channel_new(priv->account, receiver_name);
		account_add_channel(priv->account, channel);
	}

	str = irc_message_format(msg, format);

	if(channel == NULL) {
		account_console_buffer_append(priv->account, type, str);
	} else {
		channel_append_text(channel, type, str);

	}

	g_free(str);
}

static gboolean 
irc_handle_reply(IRCHandle *handle, IRCMessage *msg)
{
	IRCHandlePrivate *priv;

	g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), FALSE);

	priv = handle->priv;

	switch(msg->response) {
	case IRC_RPL_AWAY:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** %2 is marked as begin AWAY, but left the message: %3"));
		return TRUE;
	case IRC_RPL_INFO:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %2");
		return TRUE;
	case IRC_RPL_LUSERCLIENT:
	case IRC_RPL_LUSERME:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** %t"));
		return TRUE;
	case IRC_RPL_UNAWAY:
		account_set_away_status(priv->account, FALSE);
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** %t"));
		return TRUE;
	case IRC_RPL_NOWAWAY:
		account_set_away_status(priv->account, TRUE);
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** %t"));
		return TRUE;
	case IRC_RPL_INVITING:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** You are inviting %2 to %3"));
		return TRUE;
	case IRC_RPL_VERSION:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** %3 is running IRC version %2 (%4)"));
		return TRUE;
	case IRC_RPL_LUSEROP:
	case IRC_RPL_LUSERUNKNOWN:
	case IRC_RPL_LUSERCHANNELS:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %2 %3");
		return TRUE;
	case IRC_RPL_LINKS:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "%3 %4");
		return TRUE;
	case IRC_RPL_MOTDSTART:
	case IRC_RPL_MOTD:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %t");
		return TRUE;
	case IRC_RPL_ENDOFMOTD:
		priv->end_motd = TRUE;
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %t");
		return TRUE;
	case IRC_RPL_NAMREPLY: /* <nick> = <channel> :... */
		irc_handle_reply_names(handle, msg);
		return TRUE;
	case IRC_RPL_CREATIONTIME:
		irc_handle_reply_creationtime(handle, msg);
		return TRUE;
	case IRC_RPL_TOPICWHOTIME:
		irc_handle_reply_topicwhotime(handle, msg);
		return TRUE;
	case IRC_RPL_TOPIC:
		irc_handle_reply_topic(handle, msg);
		return TRUE;
	case IRC_RPL_WHOISUSER:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_NORMAL, _("%2 is %3@%4: %t"));
		return TRUE;
	case IRC_RPL_WHOWASUSER:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_NORMAL, _("%2 was %3@%4: %t"));
		return TRUE;
	case IRC_RPL_WHOISCHANNELS:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_NORMAL, _("%2: %t"));
		return TRUE;
	case IRC_RPL_WHOISSERVER:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_NORMAL, _("on via server %3(%t)"));
		return TRUE;
	case IRC_RPL_WHOISIDLE:
		irc_handle_reply_whoisidle(handle, msg);
		return TRUE;
	case IRC_RPL_BANLIST:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_NORMAL, _("Banned on %2 : %3"));
		return TRUE;
	case IRC_RPL_TIME:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_NORMAL, _("Time: %3(%2)"));
		return TRUE;
	case IRC_RPL_ENDOFNAMES:
		irc_handle_reply_endofnames(handle, msg);
		return TRUE;
	case IRC_RPL_CHANNELMODEIS:
		irc_handle_reply_channelmodeis(handle, msg);
		return TRUE;
	case IRC_RPL_ENDOFWHOIS:
	case IRC_RPL_ENDOFWHO:
	case IRC_RPL_ENDOFBANLIST:
	case IRC_RPL_ENDOFINFO:
	case IRC_RPL_ENDOFUSERS:
	case IRC_RPL_NONE:
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

	switch(msg->response) {
	case IRC_ERR_UNAVAILRESOURCE:
	case IRC_ERR_NICKNAMEINUSE:
		irc_handle_error_nick_unusable(handle, msg);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}
static gboolean 
irc_handle_command(IRCHandle *handle, IRCMessage *msg)
{
	g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(IS_IRC_HANDLE(handle), FALSE);

	switch (msg->response) {
	case IRC_COMMAND_NOTICE:
	case IRC_COMMAND_PRIVMSG:
		irc_handle_command_privmsg_notice(handle, msg);
		return TRUE;
	case IRC_COMMAND_MODE:
		irc_handle_command_mode(handle, msg);
		return TRUE;
	case IRC_COMMAND_NICK:
		irc_handle_command_nick(handle, msg);
		return TRUE;
	case IRC_COMMAND_JOIN:
		irc_handle_command_join(handle, msg);
		return TRUE;
	case IRC_COMMAND_PART:
		irc_handle_command_part(handle, msg);
		return TRUE;
	case IRC_COMMAND_KICK:
		irc_handle_command_kick(handle, msg);
		return TRUE;
	case IRC_COMMAND_TOPIC:
		irc_handle_command_topic(handle, msg);
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
void
irc_handle_response(IRCHandle *handle, IRCMessage *msg)
{
	gboolean proceeded = FALSE;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	if(show_msg_mode)
		irc_message_print(msg);

	if(IRC_MESSAGE_IS_COMMAND(msg)) {
		proceeded = irc_handle_command(handle, msg);
	} else if(IRC_MESSAGE_IS_REPLY(msg)) {
		proceeded = irc_handle_reply(handle, msg);
	} else if(IRC_MESSAGE_IS_ERROR(msg)) {
		proceeded = irc_handle_error(handle, msg);
	} else if(msg->response < 10) { /* FIXME: what's this? */
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %*2");
		proceeded = TRUE;
	}

	if(!proceeded)
		irc_handle_inspect_message(handle, msg);
}

IRCHandle*
irc_handle_new(Account *account)
{
        IRCHandle *handle;
	IRCHandlePrivate *priv;

	handle = g_object_new(irc_handle_get_type(), NULL);
	
	priv = handle->priv;

	priv->account = account;
	priv->ctcp_handle = ctcp_handle_new(handle, account);

	return handle;
}
