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
#include "loqui_user_irc.h"
#include "loqui_utils_irc.h"

#include "codeconv.h"
#include <string.h>
#include <time.h>

struct _IRCHandlePrivate
{
	CTCPHandle *ctcp_handle;

	Account *account;
	gboolean fallback;

	gboolean end_motd;
	gboolean passed_welcome;
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
static void irc_handle_reply_who(IRCHandle *handle, IRCMessage *msg);

static void irc_handle_parse_mode_arguments(IRCHandle *handle, IRCMessage *msg, LoquiChannel *channel, gint mode_start);
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
	LoquiChannel *channel;
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
	
	channel = account_get_channel_by_name(priv->account, name);
	if(channel == NULL) {
		channel = loqui_channel_new(priv->account, name, FALSE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name));
		account_add_channel(priv->account, channel);
		g_object_unref(channel);
	}
	g_free(buf);

	buf = g_strdup_printf("[LOG] %s", line);
	loqui_channel_append_text(channel, TEXT_TYPE_NOTICE, buf);
	loqui_channel_entry_set_is_updated(LOQUI_CHANNEL_ENTRY(channel), TRUE);
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
	gchar *receiver_name, *sender;
	gchar *channel_name = NULL;
	gchar *remark;
	LoquiChannel *channel = NULL;
	TextType type;
	CTCPMessage *ctcp_msg;
	gboolean is_self;
	LoquiUser *user;
	LoquiMember *member;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(IS_IRC_HANDLE(handle));

	priv = handle->priv;

	receiver_name = irc_message_get_param(msg, 1);
	remark = irc_message_get_param(msg, 2);

	if(remark == NULL) {
		g_warning(_("This PRIVMSG/NOTICE message doesn't contain a remark."));
		return;
	}

	if(msg->nick)
		is_self = account_is_current_nick(handle->priv->account, msg->nick);
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

	if (msg->nick) {
		if(ctcp_message_parse_line(remark, &ctcp_msg)) {
			g_object_set_data(G_OBJECT(ctcp_msg), "sender", msg->nick);
			g_object_set_data(G_OBJECT(ctcp_msg), "receiver", receiver_name);
			ctcp_handle_message(priv->ctcp_handle, ctcp_msg,
					    (msg->response == IRC_COMMAND_PRIVMSG) ? TRUE : FALSE);
			g_object_unref(ctcp_msg);
		        return;
                }
	}

	if (receiver_name != NULL) {
		if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(receiver_name))
			channel_name = receiver_name;
		else
			channel_name = is_self ? receiver_name : msg->nick;
	}
	
	if (channel_name) {
		channel = account_get_channel_by_name(priv->account, channel_name);
		if(channel == NULL) {
			channel = loqui_channel_new(priv->account, channel_name, FALSE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(channel_name));
			account_add_channel(priv->account, channel);
			g_object_unref(channel);
		}
		sender = msg->nick ? msg->nick : msg->prefix;
		loqui_channel_append_remark(channel, type, is_self, sender, remark);

		if (msg->nick &&
		    (user = account_peek_user(priv->account, msg->nick)) != NULL &&
		    (member = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user)) != NULL) {
			loqui_member_set_last_message_time(member, time(NULL));
		}
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
	LoquiChannel *channel;
	LoquiUser *user;

	if(msg->nick == NULL) {
		g_warning(_("The message does not contain nick"));
		return;
	}

	slist = account_search_joined_channel(handle->priv->account, msg->nick);
	for(cur = slist; cur != NULL; cur = cur->next) {
		channel = LOQUI_CHANNEL(cur->data);
		if(!channel) {
			g_warning("NULL channel");
			continue;
		}
		user = account_peek_user(handle->priv->account, msg->nick);
		if (user)
			loqui_channel_entry_remove_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);
	}

	irc_handle_joined_channel_append(handle, msg, slist, TEXT_TYPE_INFO, _("*** %n has quit IRC(%t)"));

	g_slist_free(slist);
}
static void
irc_handle_command_part(IRCHandle *handle, IRCMessage *msg)
{
	LoquiChannel *channel;
	LoquiUser *user;
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

	channel = account_get_channel_by_name(handle->priv->account, name);
	user = account_peek_user(handle->priv->account, msg->nick);
	if (channel && user)
		loqui_channel_entry_remove_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);

	if(account_is_current_nick(handle->priv->account, msg->nick)) {
		if(channel) {
			irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** You have left %1");
			account_remove_channel(handle->priv->account, channel);
		}
	} else {
		irc_handle_channel_append(handle, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** %n has just part %1(%t)"));
	}
}
static void
irc_handle_command_kick(IRCHandle *handle, IRCMessage *msg)
{
	LoquiChannel *channel;
	LoquiUser *user;
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
	
	channel = account_get_channel_by_name(handle->priv->account, name);
	user = account_peek_user(handle->priv->account, receiver);
	if (channel && user)
		loqui_channel_entry_remove_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);

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
	gchar *nick_new;
	LoquiUser *user;

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

	user = account_peek_user(handle->priv->account, msg->nick);
	irc_handle_joined_channel_append(handle, msg, NULL, TEXT_TYPE_INFO, _("*** %n is known as %1"));
	if (user)
		loqui_user_set_nick(user, nick_new);

	if(account_is_current_nick(handle->priv->account, msg->nick))
		loqui_user_set_nick(account_get_user_self(handle->priv->account), nick_new);
}

static void
irc_handle_parse_mode_arguments(IRCHandle *handle, IRCMessage *msg, LoquiChannel *channel, gint mode_start)
{
	gint cur;
	gint param_num;
	gchar *flags, *target;
	gint is_add = -1; /* -1: uninitialized, 0: false, 1: true */
	LoquiMember *member;

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

#define GET_MEMBER_OR_RETURN(msg, i, channel, member_ptr) { \
  gchar *target; \
  LoquiUser *user; \
  GET_TARGET_OR_RETURN(msg, i, &target); \
  user = account_peek_user(channel->account, target); \
  if (!user) { \
        g_warning("User not found."); \
        return; \
  } \
  *member_ptr = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user); \
  if (*member_ptr == NULL) { \
       g_warning("Member not found."); \
       return; \
  } \
}
	if(channel) {
		for (; *flags; flags++) {
			if (*flags == '+') {
				is_add = 1;
				continue;
			} else if (*flags == '-') {
				is_add = 0;
				continue;
			}
			
			if (is_add < 0) {
				g_warning(_("Flags don't have + or -"));
				break;
			}
			
			switch(*flags) {
			case IRC_CHANNEL_MODE_OPERATOR:
				GET_MEMBER_OR_RETURN(msg, cur, channel, &member);
				loqui_member_set_is_channel_operator(member, is_add);
				break;
			case IRC_CHANNEL_MODE_VOICE:
				GET_MEMBER_OR_RETURN(msg, cur, channel, &member);
				loqui_member_set_speakable(member, is_add);
				break;
			case IRC_CHANNEL_MODE_CREATOR:
				break;
			case IRC_CHANNEL_MODE_ANONYMOUS:
			case IRC_CHANNEL_MODE_INVITE_ONLY:
			case IRC_CHANNEL_MODE_MODERATED:
			case IRC_CHANNEL_MODE_NO_MESSAGES_FROM_CLIENT:
			case IRC_CHANNEL_MODE_QUIET:
			case IRC_CHANNEL_MODE_SECRET:
			case IRC_CHANNEL_MODE_SERVER_REOP:
			case IRC_CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY:
				loqui_channel_change_mode(channel, (gboolean) is_add, *flags, NULL);
				break;
			case IRC_CHANNEL_MODE_CHANNEL_KEY:
			case IRC_CHANNEL_MODE_USER_LIMIT:
				GET_TARGET_OR_RETURN(msg, cur, &target);
				loqui_channel_change_mode(channel, (gboolean) is_add, *flags, target);
				break;
			case IRC_CHANNEL_MODE_BAN_MASK:
			case IRC_CHANNEL_MODE_EXCEPTION_TO_OVERIDE_BAN_MASK:
			case IRC_CHANNEL_MODE_INVITATION_MASK:
				break;
			default:
				g_warning(_("Unknown mode flag"));
				break;
			}
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
#undef GET_TARGET_OR_RETURN	
}
static void
irc_handle_reply_channelmodeis(IRCHandle *handle, IRCMessage *msg)
{
	LoquiChannel *channel;
	gint cur;
	gchar *name;

	cur = 2;

	name = irc_message_get_param(msg, cur);
	if(name == NULL) {
		g_warning(_("The target is not found in MODE command"));
		return;
	}
	cur++;

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name)) {
		channel = account_get_channel_by_name(handle->priv->account, name);
		if(!channel)
			return;
	} else {
		g_warning(_("RPL_CHANNELMODEIS didn't return a channel name: %s"), name);
		return;
	}
	loqui_channel_clear_mode(channel);

	irc_handle_parse_mode_arguments(handle, msg, channel, 3);
	irc_handle_channel_append(handle, msg, FALSE, 2, TEXT_TYPE_INFO, _("*** Mode for %2: %*3"));
}
static void
irc_handle_command_mode(IRCHandle *handle, IRCMessage *msg)
{
	gchar *changer = NULL;
	gchar *format, *name;
	LoquiChannel *channel = NULL;
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

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name)) {
		channel = account_get_channel_by_name(handle->priv->account, name);
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
}
static void
irc_handle_command_join(IRCHandle *handle, IRCMessage *msg)
{
	gchar *name;
	LoquiChannel *channel;
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
	
	channel = account_get_channel_by_name(handle->priv->account, name);
	if(account_is_current_nick(handle->priv->account, msg->nick)) {
		if(!channel) {
			channel = loqui_channel_new(priv->account, name, TRUE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name));
			account_add_channel(priv->account, channel);
			g_object_unref(channel);
		} else {
			loqui_channel_set_is_joined(channel, TRUE);
		}
		if(send_status_commands_mode)
			account_get_channel_mode(handle->priv->account, loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)));
	} else {
		if(!channel) {
			g_warning(_("Why do you know that the user join the channel?"));
			return;
		}
		loqui_channel_add_member_by_nick(channel, msg->nick, FALSE, FALSE);
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
	LoquiChannel *channel;
	gchar *name;
	gchar **nick_array;
	gint i;

	name = irc_message_get_param(msg, 3);
	channel = account_get_channel_by_name(handle->priv->account, name);
	if(channel == NULL)
		return;

	if(channel->end_names == TRUE) {
		loqui_channel_entry_clear_member(LOQUI_CHANNEL_ENTRY(channel));
		loqui_channel_entry_set_do_sort(LOQUI_CHANNEL_ENTRY(channel), FALSE);
		channel->end_names = FALSE;
	}

	nick_array = g_strsplit(irc_message_get_trailing(msg), " ", 0);
	for(i = 0; nick_array[i] != NULL; i++) {
		loqui_channel_add_member_by_nick(channel, nick_array[i], FALSE, FALSE);
	}
	g_strfreev(nick_array);

	irc_handle_channel_append(handle, msg, FALSE, 3, TEXT_TYPE_NORMAL, "%3: %t");
}
static void
irc_handle_reply_endofnames(IRCHandle *handle, IRCMessage *msg)
{
	LoquiChannel *channel;
	gchar *name;

	name = irc_message_get_param(msg, 2);
	channel = account_get_channel_by_name(handle->priv->account, name);
	if(channel == NULL)
		return;

	loqui_channel_entry_set_do_sort(LOQUI_CHANNEL_ENTRY(channel), TRUE);
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

/* 352 RPL_WHOREPLY
    "<channel> <user> <host> <server> <nick> 
    ( "H" / "G" > ["*"] [ ( "@" / "+" ) ] 
    :<hopcount> <real name>" */
static void
irc_handle_reply_who(IRCHandle *handle, IRCMessage *msg)
{
	IRCHandlePrivate *priv;
	
	gchar *channel_name, *username, *hostname, *server_name, *nick, *flags, *trailing;
	gchar *away_str = NULL, *hops_str = NULL, *realname = NULL;
	gchar *buf, *buf2, *tmp;
	gchar op_char;
	guint hop_count = 0;
	LoquiChannel *channel = NULL;
	LoquiAwayType away = LOQUI_AWAY_TYPE_UNKNOWN;
	LoquiUser *user = NULL;
	LoquiMember *member = NULL;

	priv = handle->priv;

	channel_name = irc_message_get_param(msg, 2);
	username = irc_message_get_param(msg, 3);
	hostname = irc_message_get_param(msg, 4);
	server_name = irc_message_get_param(msg, 5);
	nick = irc_message_get_param(msg, 6);
	flags = irc_message_get_param(msg, 7);
	trailing = irc_message_get_param(msg, 8);
	if (!channel_name || !username || !hostname || !server_name || !nick || !flags || !trailing) {
		account_console_buffer_append(priv->account, TEXT_TYPE_ERROR, _("Invalid WHO reply"));
		return;
	}
	
	user = account_peek_user(priv->account, nick);
	/* TODO: Check. username, hostname, server_name should not be changed,
	   if there is difference between new and old, something happend. */
	channel = account_get_channel_by_name(priv->account, channel_name);
	if (channel && user) {
		member = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);
		if (!member)
			member = loqui_channel_add_member_by_nick(channel, nick, FALSE, FALSE);
	}
	if (user) {
		loqui_user_set_username(user, username);
		loqui_user_set_hostname(user, hostname);
		loqui_user_set_servername(user, server_name);
	}
	if (flags[0] == 'H') {
		away_str = "Home";
		away = LOQUI_AWAY_TYPE_ONLINE;
	} else if (flags[0] == 'G') {
		away_str = "Gone";
		away = LOQUI_AWAY_TYPE_AWAY;
	}
	if (user)
		loqui_user_set_away(user, away);

	op_char = ' ';
	if (flags[0] != '\0') {
		switch(flags[1]) {
		case '@':
			if (member)
				loqui_member_set_is_channel_operator(member, TRUE);
			op_char = flags[1];
			break;
		case '+':
			if (member)
				loqui_member_set_speakable(member, TRUE);
			op_char = flags[1];
			break;
		default:
			break;
		}
	}

	buf = g_strdup(trailing);
	tmp = strchr(buf, ' ');
	if (tmp) {
		*tmp = '\0';
		hops_str = buf;
		hop_count = (guint) g_ascii_strtoull(hops_str, NULL, 10);
		realname = tmp + 1;
	} else {
		realname = buf;
	}
	if (user) {
		loqui_user_set_realname(user, realname);
		loqui_user_irc_set_hop_count(LOQUI_USER_IRC(user), hop_count);
	}

	if (handle->prevent_print_who_reply_count == 0) {
		buf2 = g_strdup_printf(_("%c%s(%s) is %s@%s (%s) on %s(%s hops) [%s]"),
					op_char, nick, channel_name, username, hostname,
					realname, server_name, hops_str ? hops_str : "?", away_str);
		account_console_buffer_append(handle->priv->account, TEXT_TYPE_INFO, buf2);
		g_free(buf2);
	}
	
	g_free(buf);
	
}
static void
irc_handle_reply_topic(IRCHandle *handle, IRCMessage *msg)
{
	LoquiChannel *channel;
	gchar *topic;
	gchar *name;

	name = irc_message_get_param(msg, 2);
	channel = account_get_channel_by_name(handle->priv->account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_trailing(msg);
	loqui_channel_entry_set_topic(LOQUI_CHANNEL_ENTRY(channel), topic);

	irc_handle_channel_append(handle, msg, FALSE, 2, TEXT_TYPE_INFO, _("Topic for %2: %t"));
}

static void
irc_handle_command_topic(IRCHandle *handle, IRCMessage *msg)
{
	LoquiChannel *channel;
	gchar *topic;
	gchar *name;

	name = irc_message_get_param(msg, 1);
	channel = account_get_channel_by_name(handle->priv->account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_trailing(msg);
	loqui_channel_entry_set_topic(LOQUI_CHANNEL_ENTRY(channel), topic);
	
	irc_handle_channel_append(handle, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** New topic on %1 by %n: %t"));
}
/* TODO: change nick */
static void
irc_handle_error_nick_unusable(IRCHandle *handle, IRCMessage *msg)
{
	IRCHandlePrivate *priv;

	priv = handle->priv;

	irc_handle_account_console_append(handle, msg, TEXT_TYPE_ERROR, "%t");

	if(!priv->passed_welcome)
		account_disconnect(handle->priv->account);
}
static void /* utility function */
irc_handle_joined_channel_append(IRCHandle *handle, IRCMessage *msg, GSList *channel_slist, TextType type, gchar *format)
{
	gchar *str;
	GSList *slist = NULL, *cur;
	LoquiChannel *channel;
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
			channel = LOQUI_CHANNEL(cur->data);
			channel_buffer_append_message_text(loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(channel)),
							   msgtext, FALSE, FALSE);
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
	LoquiChannel *channel;
	gchar *str;
	gchar *receiver_name;

	priv = handle->priv;

	receiver_name = irc_message_get_param(msg, receiver_num);
	if(receiver_name == NULL) {
		g_warning(_("Can't find the channel from the message"));
		return;
	}

	channel = account_get_channel_by_name(priv->account, receiver_name);
	if(make_channel == TRUE && channel == NULL) { /* FIXME as well as privmsg_notice */
		channel = loqui_channel_new(priv->account, receiver_name, FALSE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(receiver_name));
		account_add_channel(priv->account, channel);
		g_object_unref(channel);
	}

	str = irc_message_format(msg, format);

	if(channel == NULL) {
		account_console_buffer_append(priv->account, type, str);
	} else {
		loqui_channel_append_text(channel, type, str);

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
	case IRC_RPL_WELCOME:
		priv->passed_welcome = TRUE;
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %*2");
		return TRUE;
	case IRC_RPL_YOURHOST:
	case IRC_RPL_CREATED:
	case IRC_RPL_MYINFO:
	case IRC_RPL_BOUCE:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, "*** %*2");
		return TRUE;
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
		loqui_user_set_away(account_get_user_self(priv->account), LOQUI_AWAY_TYPE_ONLINE);
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("*** %t"));
		return TRUE;
	case IRC_RPL_NOWAWAY:
		loqui_user_set_away(account_get_user_self(priv->account), LOQUI_AWAY_TYPE_AWAY);
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
	case IRC_RPL_WHOREPLY:
		irc_handle_reply_who(handle, msg);
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
	case IRC_RPL_ENDOFWHO:
		if (handle->prevent_print_who_reply_count > 0)
			handle->prevent_print_who_reply_count--;
		else
			irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("%3"));	
		return TRUE;
	case IRC_RPL_ENDOFWHOIS:
	case IRC_RPL_ENDOFBANLIST:
	case IRC_RPL_ENDOFINFO:
	case IRC_RPL_ENDOFUSERS:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_INFO, _("%3"));
		return TRUE;
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
	/* TODO: need implements */
	case IRC_ERR_TOOMANYTARGETS:
	case IRC_ERR_NICKCOLLISION:
	case IRC_ERR_USERNOTINCHANNEL:
     /* case IRC_ERR_BANLISTFULL: */
		return FALSE;
	case IRC_ERR_NOSUCHNICK:
	case IRC_ERR_NOSUCHSERVER:
	case IRC_ERR_NOSUCHCHANNEL:
	case IRC_ERR_CANNOTSENDTOCHAN:
	case IRC_ERR_TOOMANYCHANNELS:
	case IRC_ERR_WASNOSUCHNICK:
	case IRC_ERR_NOSUCHSERVICE:
	case IRC_ERR_NOTOPLEVEL:
	case IRC_ERR_WILDTOPLEVEL:
	case IRC_ERR_BADMASK:
	case IRC_ERR_UNKNOWNCOMMAND:
	case IRC_ERR_NOADMININFO:
	case IRC_ERR_ERRONEUSNICKNAME:
	case IRC_ERR_NOTONCHANNEL:
	case IRC_ERR_NOLOGIN:
	case IRC_ERR_NEEDMOREPARAMS:
	case IRC_ERR_KEYSET:
	case IRC_ERR_CHANNELISFULL:
	case IRC_ERR_UNKNOWNMODE:
	case IRC_ERR_INVITEONLYCHAN:
	case IRC_ERR_BANNEDFROMCHAN:
	case IRC_ERR_BADCHANNELKEY:
	case IRC_ERR_BADCHANMASK:
	case IRC_ERR_NOCHANMODES:
	case IRC_ERR_CHANOPRIVSNEEDED:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_ERROR, _("%t: %2"));
		return TRUE;
	case IRC_ERR_NOORIGIN:
	case IRC_ERR_NORECIPIENT:
	case IRC_ERR_NOTEXTTOSEND:
	case IRC_ERR_NOMOTD:
	case IRC_ERR_FILEERROR:
	case IRC_ERR_NONICKNAMEGIVEN:
	case IRC_ERR_SUMMONDISABLED:
	case IRC_ERR_USERSDISABLED:
	case IRC_ERR_NOTREGISTERED:
	case IRC_ERR_ALREADYREGISTRED:
	case IRC_ERR_NOPERMFORHOST:
	case IRC_ERR_PASSWDMISMATCH: /* need to implement? */
	case IRC_ERR_YOUREBANNEDCREEP:
	case IRC_ERR_YOUWILLBEBANNED:
	case IRC_ERR_NOPRIVILEGES:
	case IRC_ERR_CANTKILLSERVER:
	case IRC_ERR_RESTRICTED:
	case IRC_ERR_UNIQOPPRIVSNEEDED:
	case IRC_ERR_NOOPERHOST:
	case IRC_ERR_UMODEUNKNOWNFLAG:
	case IRC_ERR_USERSDONTMATCH:
		irc_handle_account_console_append(handle, msg, TEXT_TYPE_ERROR, _("%t"));
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
	case IRC_COMMAND_PONG:
		/* do nothing currently */
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
