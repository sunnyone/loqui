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

#include "loqui_receiver_irc.h"
#include "utils.h"

#include "loqui_account_manager.h"
#include "irc_constants.h"
#include "main.h"
#include "intl.h"
#include "prefs_general.h"
#include "ctcp_message.h"
#include "ctcp_handle.h"
#include "loqui_user_irc.h"
#include "loqui_utils_irc.h"
#include "loqui_sender_irc.h"
#include "loqui_account_irc.h"
#include "loqui_channel_irc.h"
#include "loqui_profile_account_irc.h"

#include "codeconv.h"
#include "loqui_string_tokenizer.h"
#include <string.h>
#include <time.h>

struct _LoquiReceiverIRCPrivate
{
	CTCPHandle *ctcp_handle;

	gboolean end_motd;
};

static LoquiReceiverClass *parent_class = NULL;

static void loqui_receiver_irc_class_init(LoquiReceiverIRCClass *klass);
static void loqui_receiver_irc_init(LoquiReceiverIRC *loqui_receiver_irc);
static void loqui_receiver_irc_finalize(GObject *object);

static gboolean loqui_receiver_irc_command(LoquiReceiverIRC *receiver, IRCMessage *msg);
static gboolean loqui_receiver_irc_reply(LoquiReceiverIRC *receiver, IRCMessage *msg);
static gboolean loqui_receiver_irc_error(LoquiReceiverIRC *receiver, IRCMessage *msg);

static void loqui_receiver_irc_inspect_message(LoquiReceiverIRC *receiver, IRCMessage *msg);

static void loqui_receiver_irc_command_privmsg_notice(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_ping(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_quit(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_join(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_part(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_nick(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_mode(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_kick(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_error(LoquiReceiverIRC *receiver, IRCMessage *msg);

static void loqui_receiver_irc_account_console_append(LoquiReceiverIRC *receiver, IRCMessage *msg, TextType type, gchar *format);
static void loqui_receiver_irc_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, gboolean make_channel,
				      gint receiver_num, TextType type, gchar *format);
static void loqui_receiver_irc_joined_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, GList *channel_list, TextType type, gchar *format);

static void loqui_receiver_irc_reply_welcome(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_names(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_topic(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_endofnames(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_channelmodeis(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_creationtime(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_topicwhotime(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_whoisidle(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_error_nick_unusable(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_who(LoquiReceiverIRC *receiver, IRCMessage *msg);

static void loqui_receiver_irc_parse_mode_arguments(LoquiReceiverIRC *receiver, IRCMessage *msg, LoquiChannel *channel, gint mode_start);
static gboolean loqui_receiver_irc_parse_plum_recent(LoquiReceiverIRC *receiver, const gchar *line);

GType
loqui_receiver_irc_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiReceiverIRCClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_receiver_irc_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiReceiverIRC),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_receiver_irc_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_RECEIVER,
					      "LoquiReceiverIRC",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_receiver_irc_class_init(LoquiReceiverIRCClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_receiver_irc_finalize;
}
static void 
loqui_receiver_irc_init(LoquiReceiverIRC *receiver)
{
	LoquiReceiverIRCPrivate *priv;

	priv = g_new0(LoquiReceiverIRCPrivate, 1);

	receiver->priv = priv;

	loqui_receiver_irc_reset(receiver);
}
static void 
loqui_receiver_irc_finalize(GObject *object)
{
	LoquiReceiverIRC *receiver;
	LoquiReceiverIRCPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(object));

        receiver = LOQUI_RECEIVER_IRC(object);
	priv = receiver->priv;
	
        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(receiver->priv);
}

#define PLUM_ALIAS_OF_PERCENT_PREFIX ":*.jp"
static gboolean
loqui_receiver_irc_parse_plum_recent(LoquiReceiverIRC *receiver, const gchar *line)
{
	gchar *buf, *cur, *name;
	gchar *converted_name;
	gchar prefix;
	LoquiChannel *channel;
	LoquiAccount *account;
	LoquiReceiverIRCPrivate *priv;
	
        g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);
	g_return_val_if_fail(line != NULL, FALSE);

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

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

	if (*name == '%' && strchr(name, ':') == NULL) {
		converted_name = g_strconcat("#", name+1, PLUM_ALIAS_OF_PERCENT_PREFIX, NULL);
	} else {
		converted_name = g_strdup(name);
	}

	channel = loqui_account_get_channel_by_identifier(account, converted_name);
	if(channel == NULL) {
		channel = LOQUI_CHANNEL(loqui_channel_irc_new(account, converted_name, FALSE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(converted_name)));
		loqui_account_add_channel(account, channel);
		g_object_unref(channel);
	}
	g_free(converted_name);
	g_free(buf);
	
	buf = g_strdup_printf("[LOG] %s", line);
	loqui_channel_append_text(channel, TEXT_TYPE_NOTICE, buf);
	loqui_channel_entry_set_is_updated_weak(LOQUI_CHANNEL_ENTRY(channel), TRUE);
	g_free(buf);

	return TRUE;

 error:
	g_free(buf);
	return FALSE;
}
static void
loqui_receiver_irc_command_privmsg_notice(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *receiver_name, *sender;
	gchar *channel_name = NULL;
	gchar *remark;
	LoquiAccount *account;
	LoquiChannel *channel = NULL;
	TextType type;
	CTCPMessage *ctcp_msg;
	gboolean is_self, is_priv;
	LoquiUser *user;
	LoquiMember *member;
	gboolean is_from_server;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	receiver_name = irc_message_get_param(msg, 1);
	remark = irc_message_get_param(msg, 2);

	if(remark == NULL) {
		loqui_account_warning(account, _("This PRIVMSG/NOTICE message doesn't contain a remark."));
		return;
	}

	if(msg->nick)
		is_self = loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), msg->nick);
	else
		is_self = FALSE;

	if(priv->end_motd == FALSE &&
	   msg->response == IRC_COMMAND_NOTICE &&
	   prefs_general.parse_plum_recent) {
		if(loqui_receiver_irc_parse_plum_recent(receiver, remark))
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
		channel = loqui_account_get_channel_by_identifier(account, channel_name);
		if(channel == NULL) {
			is_priv = !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(channel_name);
			channel = LOQUI_CHANNEL(loqui_channel_irc_new(account, channel_name, FALSE, is_priv));
			if (is_priv) {
				member = loqui_member_new(loqui_account_get_user_self(account));
				loqui_channel_entry_add_member(LOQUI_CHANNEL_ENTRY(channel), member);
				g_object_unref(member);

				if (!is_self)
					loqui_channel_irc_add_member_by_nick(LOQUI_CHANNEL_IRC(channel), msg->nick, FALSE, FALSE, FALSE);
			}
			loqui_account_add_channel(account, channel);
			g_object_unref(channel);
		}
		sender = msg->nick ? msg->nick : msg->prefix;
		is_from_server = (msg->nick == NULL) ? TRUE : FALSE;
		loqui_channel_append_remark(channel, type, is_self, sender, remark, is_from_server);

		if (msg->nick &&
		    (user = loqui_account_peek_user(account, msg->nick)) != NULL &&
		    (member = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user)) != NULL) {
			loqui_member_set_last_message_time(member, time(NULL));
		}
	} else {
		loqui_account_console_buffer_append(account, type, remark);
	}
}
static void
loqui_receiver_irc_command_ping(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
        g_return_if_fail(receiver != NULL);
	g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	loqui_sender_irc_pong_raw(LOQUI_SENDER_IRC(loqui_account_get_sender(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)))), irc_message_get_param(msg, 1));
}
static void
loqui_receiver_irc_command_quit(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	GList *list = NULL;
	LoquiUser *user;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	
	if (msg->nick == NULL) {
		loqui_account_warning(account, _("The message does not contain nick"));
		return;
	}

	user = loqui_account_peek_user(account, msg->nick);
	if (user) {
		list = loqui_account_search_joined_channel(account, user);
		g_list_foreach(list, (GFunc) loqui_channel_entry_remove_member_by_user, user);
	}

	loqui_receiver_irc_joined_channel_append(receiver, msg, list, TEXT_TYPE_INFO, _("*** %n has quit IRC(%t)"));

	g_list_free(list);
}
static void
loqui_receiver_irc_command_part(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiChannel *channel;
	LoquiUser *user;
	gchar *name;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	
	if(msg->nick == NULL) {
		loqui_account_warning(account, _("The message does not contain nick"));
		return;
	}

	name = irc_message_get_param(msg, 1);
	if(!name) {
		loqui_account_warning(account, _("The message does not contain the channal name"));
		return;
	}

	channel = loqui_account_get_channel_by_identifier(account, name);
	user = loqui_account_peek_user(account, msg->nick);
	if (!channel || !user) {
		loqui_account_warning(account, "Why do you know the user '%s' is joined to %s?", msg->nick, name);
	}

	if (channel && user)
		loqui_channel_entry_remove_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);

	if (loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), msg->nick)) {
		if(channel) {
			loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** You have left %1");
			loqui_account_remove_channel(account, channel);
		}
	} else {
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** %n has just part %1(%t)"));
	}
}
static void
loqui_receiver_irc_command_kick(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiChannel *channel;
	LoquiUser *user;
	gchar *name;
	gchar *sender_name, *receiver_name;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	
	sender_name = msg->nick;
	if(sender_name == NULL) {
		loqui_account_warning(account, _("The message does not contain nick"));
		return;
	}

	name = irc_message_get_param(msg, 1);
	if(!name) {
		loqui_account_warning(account, _("The message does not contain the channal name"));
		return;
	}

	receiver_name = irc_message_get_param(msg, 2);
	if(!receiver_name) {
		loqui_account_warning(account, _("The KICK message doesn't contain the user to be kicked."));
		return;
	}
	
	channel = loqui_account_get_channel_by_identifier(account, name);
	user = loqui_account_peek_user(account, receiver_name);
	if (channel && user)
		loqui_channel_entry_remove_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);

	if(loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), receiver_name)) {
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** You were kicked from %1 by %n (%3)");
		loqui_account_remove_channel(account, channel);
	} else {
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** %2 was kicked from %1 by %n(%3)"));
	}
}
static void
loqui_receiver_irc_command_error(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiAccount *account;

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	
	loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_ERROR, "*** %t");
	loqui_account_remove_all_channel(LOQUI_ACCOUNT(account));
}
static void
loqui_receiver_irc_command_nick(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	gchar *nick_new;
	LoquiUser *user;
	LoquiChannel *channel;
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	if(msg->nick == NULL) {
		loqui_account_warning(account, _("The message does not contain nick"));
		return;
	}

	nick_new = irc_message_get_param(msg, 1);
	if(nick_new == NULL) {
		loqui_account_warning(account, _("The NICK message does not contain new nick"));
		return;
	}

	user = loqui_account_peek_user(account, msg->nick);
	loqui_receiver_irc_joined_channel_append(receiver, msg, NULL, TEXT_TYPE_INFO, _("*** %n is known as %1"));
	if (user)
		loqui_user_set_nick(user, nick_new);

	if(loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), msg->nick))
		loqui_user_set_nick(loqui_account_get_user_self(account), nick_new);

	if (!LOQUI_UTILS_IRC_STRING_IS_CHANNEL(msg->nick)) {
		channel = loqui_account_get_channel_by_identifier(account, msg->nick);
		if (channel) {
			loqui_channel_entry_set_name(LOQUI_CHANNEL_ENTRY(channel), nick_new);
			loqui_channel_set_identifier(channel, nick_new); /* FIXME: priv should not have a identifier */
		}
	}
}

static void
loqui_receiver_irc_parse_mode_arguments(LoquiReceiverIRC *receiver, IRCMessage *msg, LoquiChannel *channel, gint mode_start)
{
	gint cur;
	gint param_num;
	gchar *flags, *target;
	gint is_add = -1; /* -1: uninitialized, 0: false, 1: true */
	LoquiMember *member;
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	cur = mode_start;
	param_num = irc_message_count_parameters(msg);

	flags = irc_message_get_param(msg, cur);
	if(flags == NULL) {
		loqui_account_warning(account, _("Flags are not found in MODE command"));
		return;
	}
	cur++;

#define GET_TARGET_OR_RETURN(msg, i, str_ptr) { \
  *str_ptr = irc_message_get_param(msg, i); \
  if(*str_ptr == NULL) { \
        loqui_account_warning(account, _("Can't find a nick to change mode")); \
        return; \
  } \
  i++; \
}

#define GET_MEMBER_OR_RETURN(msg, i, channel, member_ptr) { \
  gchar *target; \
  LoquiUser *user; \
  GET_TARGET_OR_RETURN(msg, i, &target); \
  user = loqui_account_peek_user(channel->account, target); \
  if (!user) { \
        loqui_account_warning(account, "User not found."); \
        return; \
  } \
  *member_ptr = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user); \
  if (*member_ptr == NULL) { \
       loqui_account_warning(account, "Member not found."); \
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
				loqui_account_warning(account, _("Flags don't have + or -"));
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
			case IRC_CHANNEL_MODE_PRIVATE:
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
				loqui_account_warning(account, _("Unknown mode flag: '%c'"), *flags);
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
			loqui_account_warning(account, _("User mode is not implemented."));
			break;
		}
	}
#undef GET_TARGET_OR_RETURN	
}
static void
loqui_receiver_irc_reply_channelmodeis(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiChannel *channel;
	gint cur;
	gchar *name;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	cur = 2;

	name = irc_message_get_param(msg, cur);
	if(name == NULL) {
		loqui_account_warning(account, _("The target is not found in MODE command"));
		return;
	}
	cur++;

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name)) {
		channel = loqui_account_get_channel_by_identifier(account, name);
		if(!channel)
			return;
	} else {
		loqui_account_warning(account, _("RPL_CHANNELMODEIS didn't return a channel name: %s"), name);
		return;
	}
	loqui_channel_clear_mode(channel);

	loqui_receiver_irc_parse_mode_arguments(receiver, msg, channel, 3);
	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 2, TEXT_TYPE_INFO, _("*** Mode for %2: %*3"));
}
static void
loqui_receiver_irc_command_mode(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *changer = NULL;
	gchar *format, *name;
	LoquiChannel *channel = NULL;
	gint cur;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	if(msg->nick)
		changer = msg->nick;
	else if(msg->prefix)
		changer = msg->prefix;
	else {
		loqui_account_warning(account, _("Who can change mode?"));
		return;
	}

	if(strchr(changer, '%')) {
		loqui_account_warning(account, _("Nick must not contain '%%'"));
		return;
	}

	cur = 1;

	name = irc_message_get_param(msg, cur);
	if(name == NULL) {
		loqui_account_warning(account, _("The target is not found in MODE command"));
		return;
	}
	cur++;

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name)) {
		channel = loqui_account_get_channel_by_identifier(account, name);
		if(!channel) {
			loqui_account_warning(account, _("Why can you know the change of his mode?"));
			return;
		}
	} else {
		channel = NULL;
	}

	loqui_receiver_irc_parse_mode_arguments(receiver, msg, channel, cur);

	format = g_strdup_printf(_("*** New mode for %%1 by %s: %%*2"), changer);

	if(channel)
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, TEXT_TYPE_INFO, format);
	else
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, format);

	g_free(format);
}
static void
loqui_receiver_irc_command_join(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	gchar *name;
	LoquiChannel *channel;
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));
	g_return_if_fail(msg != NULL);

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	if(msg->nick == NULL) {
		loqui_account_warning(account, _("The message does not contain nick."));
		return;
	}
	name = irc_message_get_param(msg, 1);
	if(name == NULL) {
		loqui_account_warning(account, _("Invalid JOIN command"));
		return;
	}
	
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), msg->nick)) {
		if(!channel) {
			channel = LOQUI_CHANNEL(loqui_channel_irc_new(account, name, TRUE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name)));
			loqui_account_add_channel(account, channel);
			g_object_unref(channel);
		} else {
			loqui_channel_set_is_joined(channel, TRUE);
		}
		if(send_status_commands_mode)
			loqui_sender_irc_get_channel_mode(loqui_account_get_sender(account), channel);
	} else {
		if(!channel) {
			loqui_account_warning(account, _("Why do you know that the user join the channel?"));
			return;
		}
		loqui_channel_irc_add_member_by_nick(LOQUI_CHANNEL_IRC(channel), msg->nick, FALSE, FALSE, FALSE);
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** %n (%u@%h) joined channel %t"));
	}
}
static void
loqui_receiver_irc_inspect_message(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	gchar *str;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	str = irc_message_inspect(msg);
	
	loqui_account_console_buffer_append(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)), TEXT_TYPE_NORMAL, str);
	
	g_free(str);
}
static void
loqui_receiver_irc_reply_welcome(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	const gchar *autojoin;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	receiver->passed_welcome = TRUE;

	autojoin = loqui_profile_account_irc_get_autojoin(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account)));
	if (autojoin && strlen(autojoin) > 0) {
		loqui_sender_join_raw(loqui_account_get_sender(account), autojoin, NULL);
		loqui_account_information(account, _("Sent join command for autojoin."));
	}

	loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** %*2");
}
static void
loqui_receiver_irc_reply_names(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiChannel *channel;
	gchar *name;
	LoquiStringTokenizer *st;
	LoquiAccount *account;
	const gchar *nick;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	name = irc_message_get_param(msg, 3);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(channel == NULL)
		return;

	if(channel->end_names == TRUE) {
		loqui_channel_entry_clear_member(LOQUI_CHANNEL_ENTRY(channel));
		loqui_channel_entry_set_do_sort(LOQUI_CHANNEL_ENTRY(channel), FALSE);
		channel->end_names = FALSE;
	}

	st = loqui_string_tokenizer_new(irc_message_get_trailing(msg), " ");
	loqui_string_tokenizer_set_skip_whitespaces_after_delimiter(st, TRUE);
	while ((nick = loqui_string_tokenizer_next_token(st, NULL)) != NULL)
		loqui_channel_irc_add_member_by_nick(LOQUI_CHANNEL_IRC(channel), nick, TRUE, FALSE, FALSE);
	loqui_string_tokenizer_free(st);

	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 3, TEXT_TYPE_NORMAL, "%3: %t");
}
static void
loqui_receiver_irc_reply_endofnames(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiChannel *channel;
	gchar *name;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	name = irc_message_get_param(msg, 2);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(channel == NULL)
		return;

	loqui_channel_entry_set_do_sort(LOQUI_CHANNEL_ENTRY(channel), TRUE);
	channel->end_names = TRUE;
}

static void
loqui_receiver_irc_reply_creationtime(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *time_str;
	time_t t;
	gchar *str;
	gchar *format;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	time_str = irc_message_get_param(msg, 3);
	if(time_str == NULL) {
		loqui_account_warning(account, _("Invalid CREATIONTIME reply."));
		return;
	}
	t = (time_t) g_ascii_strtoull(time_str, NULL, 10);
	if(t == 0) {
		loqui_account_warning(account, _("Invalid time string"));
		return;
	}
	
	str = utils_get_iso8601_date_string(t);
	if(str == NULL) {
		loqui_account_warning(account, _("Invalid time"));
		return;
	}

	format = g_strdup_printf(_("%%2 was created at %s"), str);
	g_free(str);

	loqui_receiver_irc_channel_append(receiver, msg, TRUE, 2, TEXT_TYPE_INFO, format);
	g_free(format);
}

static void
loqui_receiver_irc_reply_topicwhotime(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;
	gchar *time_str;
	time_t t;
	gchar *str;
	gchar *format;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	time_str = irc_message_get_param(msg, 4);
	if(time_str == NULL) {
		loqui_account_warning(account, _("Invalid TOPICWHOTIME reply."));
		return;
	}

	t = (time_t) g_ascii_strtoull(time_str, NULL, 10);
	if(t == 0) {
		loqui_account_warning(account, _("Invalid time string"));
		return;
	}
	
	str = utils_get_iso8601_date_string(t);
	if(str == NULL) {
		loqui_account_warning(account, _("Invalid time"));
		return;
	}

	format = g_strdup_printf(_("Topic for %%2 was set by %%3 at %s"), str);
	g_free(str);

	loqui_receiver_irc_channel_append(receiver, msg, TRUE, 2, TEXT_TYPE_INFO, format);
	g_free(format);
}
static void
loqui_receiver_irc_reply_whoisidle(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *str;
	gchar *sec_str;
	gint sec;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	sec_str = irc_message_get_param(msg, 3);
	if(!sec_str) {
		loqui_account_warning(account, _("Invalid WHOISIDLE reply."));
		return;
	}

	sec = (gint) g_ascii_strtoull(sec_str, NULL, 10);
	if((*sec_str != '0' || *(sec_str+1) != '\0') && sec == 0) {
		loqui_account_warning(account, _("Invalid WHOISIDLE reply."));
		return;
	}

	str = g_strdup_printf("Idle time: %d day(s), %.2d:%.2d:%.2d",
			      sec / (24 * 60 * 60),
			      (sec % (24 * 60 * 60)) / (60 * 60),
			      (sec % (60 * 60)) / 60,
			      (sec % 60));

	loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, str);
	g_free(str);
}

/* 352 RPL_WHOREPLY
    "<channel> <user> <host> <server> <nick> 
    ( "H" / "G" > ["*"] [ ( "@" / "+" ) ] 
    :<hopcount> <real name>" */
static void
loqui_receiver_irc_reply_who(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;
	
	gchar *channel_name, *username, *hostname, *server_name, *nick, *flags, *trailing;
	gchar *away_str = NULL, *hops_str = NULL, *realname = NULL;
	gchar *buf, *buf2, *tmp;
	gchar op_char;
	guint hop_count = 0;
	LoquiChannel *channel = NULL;
	LoquiAwayType away = LOQUI_AWAY_TYPE_UNKNOWN;
	LoquiUser *user = NULL;
	LoquiMember *member = NULL;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	channel_name = irc_message_get_param(msg, 2);
	username = irc_message_get_param(msg, 3);
	hostname = irc_message_get_param(msg, 4);
	server_name = irc_message_get_param(msg, 5);
	nick = irc_message_get_param(msg, 6);
	flags = irc_message_get_param(msg, 7);
	trailing = irc_message_get_param(msg, 8);
	if (!channel_name || !username || !hostname || !server_name || !nick || !flags || !trailing) {
		loqui_account_warning(account, _("Invalid WHO reply."));
		return;
	}
	
	user = loqui_account_peek_user(account, nick);
	/* TODO: Check. username, hostname, server_name should not be changed,
	   if there is difference between new and old, something happend. */
	channel = loqui_account_get_channel_by_identifier(account, channel_name);
	if (channel && user) {
		member = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);
		if (!member)
			member = loqui_channel_irc_add_member_by_nick(LOQUI_CHANNEL_IRC(channel), nick, FALSE, FALSE, FALSE);
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

	if (receiver->prevent_print_who_reply_count == 0) {
		buf2 = g_strdup_printf(_("%c%s(%s) is %s@%s (%s) on %s(%s hops) [%s]"),
					op_char, nick, channel_name, username, hostname,
					realname, server_name, hops_str ? hops_str : "?", away_str);
		loqui_account_console_buffer_append(account, TEXT_TYPE_INFO, buf2);
		g_free(buf2);
	}
	
	g_free(buf);
	
}
static void
loqui_receiver_irc_reply_topic(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiChannel *channel;
	gchar *topic;
	gchar *name;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	name = irc_message_get_param(msg, 2);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_trailing(msg);
	loqui_channel_entry_set_topic(LOQUI_CHANNEL_ENTRY(channel), strlen(topic) ? topic : NULL);

	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 2, TEXT_TYPE_INFO, _("Topic for %2: %t"));
}

static void
loqui_receiver_irc_command_topic(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiChannel *channel;
	LoquiAccount *account;
	gchar *topic;
	gchar *name;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	name = irc_message_get_param(msg, 1);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_trailing(msg);
	loqui_channel_entry_set_topic(LOQUI_CHANNEL_ENTRY(channel), strlen(topic) ? topic : NULL);
	
	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, TEXT_TYPE_INFO, _("*** New topic on %1 by %n: %t"));
}
/* TODO: change nick */
static void
loqui_receiver_irc_error_nick_unusable(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_ERROR, "%t");

	if(!receiver->passed_welcome)
		loqui_account_disconnect(account);
}
static void /* utility function */
loqui_receiver_irc_joined_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, GList *channel_list, TextType type, gchar *format)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *str;
	GList *list = NULL, *cur;
	LoquiUser *user = NULL;
	MessageText *msgtext;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	str = irc_message_format(msg, format);

	if (msg->nick)
		user = loqui_account_peek_user(account, msg->nick);

	if (user && channel_list == NULL)
		list = loqui_account_search_joined_channel(account, user);
	else
		list = channel_list;

	msgtext = message_text_new();
	g_object_set(G_OBJECT(msgtext), 
		     "is_remark", FALSE,
		     "text_type", type,
		     "text", str, NULL);

	if (list != NULL) {
		for(cur = list; cur != NULL; cur = cur->next) {
			channel_buffer_append_message_text(loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(cur->data)),
							   msgtext, FALSE, FALSE);
		}
	} else {
		loqui_account_console_buffer_append(account, type, str);
	}
	g_object_unref(msgtext);

	g_free(str);

	if(channel_list == NULL && list != NULL)
		g_list_free(list);
}
static void /* utility function */
loqui_receiver_irc_account_console_append(LoquiReceiverIRC *receiver, IRCMessage *msg, TextType type, gchar *format)
{
	gchar *str;
	LoquiAccount *account;

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	str = irc_message_format(msg, format);
	loqui_account_console_buffer_append(account, type, str);

	g_free(str);
}

static void /* utility function */
loqui_receiver_irc_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, gboolean make_channel,
			  gint receiver_num, TextType type, gchar *format)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiChannel *channel;
	gchar *str;
	gchar *receiver_name;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	receiver_name = irc_message_get_param(msg, receiver_num);
	if(receiver_name == NULL) {
		loqui_account_warning(account, _("Can't find the channel from the message"));
		return;
	}

	channel = loqui_account_get_channel_by_identifier(account, receiver_name);
	if(make_channel == TRUE && channel == NULL) { /* FIXME as well as privmsg_notice */
		channel = LOQUI_CHANNEL(loqui_channel_irc_new(account, receiver_name, FALSE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(receiver_name))); /* FIXME priv should not have a identifier */
		loqui_account_add_channel(account, channel);
		g_object_unref(channel);
	}

	str = irc_message_format(msg, format);

	if(channel == NULL) {
		loqui_account_console_buffer_append(account, type, str);
	} else {
		loqui_channel_append_text(channel, type, str);

	}

	g_free(str);
}

static gboolean 
loqui_receiver_irc_reply(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;

	g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	switch(msg->response) {
	case IRC_RPL_WELCOME:
		loqui_receiver_irc_reply_welcome(receiver, msg);
		return TRUE;
	case IRC_RPL_YOURHOST:
	case IRC_RPL_CREATED:
	case IRC_RPL_MYINFO:
	case IRC_RPL_BOUCE:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** %*2");
		return TRUE;
	case IRC_RPL_AWAY:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("*** %2 is marked as begin AWAY, but left the message: %3"));
		return TRUE;
	case IRC_RPL_INFO:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** %2");
		return TRUE;
	case IRC_RPL_LUSERCLIENT:
	case IRC_RPL_LUSERME:
	case IRC_RPL_LOCALUSERS:
	case IRC_RPL_GLOBALUSERS:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("*** %t"));
		return TRUE;
	case IRC_RPL_UNAWAY:
		loqui_user_set_away(loqui_account_get_user_self(account), LOQUI_AWAY_TYPE_ONLINE);
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("*** %t"));
		return TRUE;
	case IRC_RPL_NOWAWAY:
		loqui_user_set_away(loqui_account_get_user_self(account), LOQUI_AWAY_TYPE_AWAY);
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("*** %t"));
		return TRUE;
	case IRC_RPL_INVITING:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("*** You are inviting %2 to %3"));
		return TRUE;
	case IRC_RPL_VERSION:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("*** %3 is running IRC version %2 (%4)"));
		return TRUE;
	case IRC_RPL_LUSEROP:
	case IRC_RPL_LUSERUNKNOWN:
	case IRC_RPL_LUSERCHANNELS:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** %2 %3");
		return TRUE;
	case IRC_RPL_LINKS:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "%3 %4");
		return TRUE;
	case IRC_RPL_MOTDSTART:
	case IRC_RPL_MOTD:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** %t");
		return TRUE;
	case IRC_RPL_ENDOFMOTD:
		priv->end_motd = TRUE;
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** %t");
		return TRUE;
	case IRC_RPL_NAMREPLY: /* <nick> = <channel> :... */
		loqui_receiver_irc_reply_names(receiver, msg);
		return TRUE;
	case IRC_RPL_CREATIONTIME:
		loqui_receiver_irc_reply_creationtime(receiver, msg);
		return TRUE;
	case IRC_RPL_TOPICWHOTIME:
		loqui_receiver_irc_reply_topicwhotime(receiver, msg);
		return TRUE;
	case IRC_RPL_TOPIC:
		loqui_receiver_irc_reply_topic(receiver, msg);
		return TRUE;
	case IRC_RPL_WHOREPLY:
		loqui_receiver_irc_reply_who(receiver, msg);
		return TRUE;
	case IRC_RPL_WHOISUSER:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, _("%2 is %3@%4: %t"));
		return TRUE;
	case IRC_RPL_WHOWASUSER:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, _("%2 was %3@%4: %t"));
		return TRUE;
	case IRC_RPL_WHOISCHANNELS:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, _("%2: %t"));
		return TRUE;
	case IRC_RPL_WHOISSERVER:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, _("on via server %3(%t)"));
		return TRUE;
	case IRC_RPL_WHOISIDLE:
		loqui_receiver_irc_reply_whoisidle(receiver, msg);
		return TRUE;
	case IRC_RPL_BANLIST:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, _("Banned on %2 : %3"));
		return TRUE;
	case IRC_RPL_TIME:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, _("Time: %3(%2)"));
		return TRUE;
	case IRC_RPL_ENDOFNAMES:
		loqui_receiver_irc_reply_endofnames(receiver, msg);
		return TRUE;
	case IRC_RPL_CHANNELMODEIS:
		loqui_receiver_irc_reply_channelmodeis(receiver, msg);
		return TRUE;
	case IRC_RPL_ENDOFWHO:
		if (receiver->prevent_print_who_reply_count > 0)
			receiver->prevent_print_who_reply_count--;
		else
			loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("%3"));	
		return TRUE;
	case IRC_RPL_ENDOFWHOIS:
	case IRC_RPL_ENDOFBANLIST:
	case IRC_RPL_ENDOFINFO:
	case IRC_RPL_ENDOFUSERS:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("%3"));
		return TRUE;
	case IRC_RPL_NONE:
		return TRUE;
	case IRC_RPL_ISON:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("Ison: %*2"));
		return TRUE;
	case IRC_RPL_LISTSTART:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("Channel List: Name(Users) Topic"));
		return TRUE;
	case IRC_RPL_LIST:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_NORMAL, _("%2(%3)\t%4"));
		return TRUE;
	case IRC_RPL_LISTEND:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, _("%t"));
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

static gboolean 
loqui_receiver_irc_error(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);

	switch(msg->response) {
	case IRC_ERR_UNAVAILRESOURCE:
	case IRC_ERR_NICKNAMEINUSE:
		loqui_receiver_irc_error_nick_unusable(receiver, msg);
		return TRUE;
	/* TODO: need implements */
	case IRC_ERR_TOOMANYTARGETS:
	case IRC_ERR_NICKCOLLISION:
	case IRC_ERR_USERNOTINCHANNEL:
     /* case IRC_ERR_BANLISTFULL: */
		return FALSE;
	case IRC_ERR_USERONCHANNEL:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_ERROR, _("%2 %t (%3)"));
		return TRUE;
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
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_ERROR, _("%t: %2"));
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
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_ERROR, _("%t"));
		return TRUE;
	default:
		break;
	}
	return FALSE;
}
static gboolean 
loqui_receiver_irc_command(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);

	switch (msg->response) {
	case IRC_COMMAND_NOTICE:
	case IRC_COMMAND_PRIVMSG:
		loqui_receiver_irc_command_privmsg_notice(receiver, msg);
		return TRUE;
	case IRC_COMMAND_MODE:
		loqui_receiver_irc_command_mode(receiver, msg);
		return TRUE;
	case IRC_COMMAND_NICK:
		loqui_receiver_irc_command_nick(receiver, msg);
		return TRUE;
	case IRC_COMMAND_JOIN:
		loqui_receiver_irc_command_join(receiver, msg);
		return TRUE;
	case IRC_COMMAND_PART:
		loqui_receiver_irc_command_part(receiver, msg);
		return TRUE;
	case IRC_COMMAND_KICK:
		loqui_receiver_irc_command_kick(receiver, msg);
		return TRUE;
	case IRC_COMMAND_TOPIC:
		loqui_receiver_irc_command_topic(receiver, msg);
		return TRUE;
	case IRC_COMMAND_QUIT:
		loqui_receiver_irc_command_quit(receiver, msg);
		return TRUE;
	case IRC_COMMAND_PING:
		loqui_receiver_irc_command_ping(receiver, msg);
		return TRUE;
	case IRC_COMMAND_PONG:
		/* do nothing currently */
		return TRUE;
	case IRC_COMMAND_INVITE:
		loqui_receiver_irc_account_console_append(receiver, msg, TEXT_TYPE_INFO, "*** You were invited to %2 by %1");
		return TRUE;
	case IRC_COMMAND_ERROR:
		loqui_receiver_irc_command_error(receiver, msg);
		return TRUE;
	default:
		break;
	}

	return FALSE;
}
void
loqui_receiver_irc_response(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	gboolean proceeded = FALSE;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	if(show_msg_mode)
		irc_message_print(msg);

	if(IRC_MESSAGE_IS_COMMAND(msg)) {
		proceeded = loqui_receiver_irc_command(receiver, msg);
	} else if(IRC_MESSAGE_IS_REPLY(msg)) {
		proceeded = loqui_receiver_irc_reply(receiver, msg);
	} else if(IRC_MESSAGE_IS_ERROR(msg)) {
		proceeded = loqui_receiver_irc_error(receiver, msg);
	}

	if(!proceeded)
		loqui_receiver_irc_inspect_message(receiver, msg);
}

LoquiReceiverIRC*
loqui_receiver_irc_new(LoquiAccount *account)
{
        LoquiReceiverIRC *receiver;
	LoquiReceiverIRCPrivate *priv;

	receiver = g_object_new(loqui_receiver_irc_get_type(), NULL);
	
	priv = receiver->priv;

	LOQUI_RECEIVER(receiver)->account = account;
	priv->ctcp_handle = ctcp_handle_new(receiver, account);

	return receiver;
}
void
loqui_receiver_irc_reset(LoquiReceiverIRC *receiver)
{
	LoquiReceiverIRCPrivate *priv;

	g_return_if_fail(receiver != NULL);
	g_return_if_fail(LOQUI_RECEIVER_IRC(receiver));

	priv = receiver->priv;

	receiver->prevent_print_who_reply_count = 0;

	priv->end_motd = FALSE;
	receiver->passed_welcome = FALSE;
}
