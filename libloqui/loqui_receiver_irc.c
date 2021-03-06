/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2002-2004 Yoichi Imai <sunnyone41@gmail.com>
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

#include "loqui_receiver_irc.h"
#include "loqui-utils.h"

#include "loqui-account-manager.h"
#include "irc_constants.h"
#include <libloqui-intl.h>
#include "ctcp_message.h"
#include "ctcp_handle.h"
#include "loqui_user_irc.h"
#include "loqui_utils_irc.h"
#include "loqui_sender_irc.h"
#include "loqui_account_irc.h"
#include "loqui_channel_irc.h"
#include "loqui_profile_account_irc.h"

#include "loqui_codeconv.h"
#include "loqui_string_tokenizer.h"

#include <string.h>
#include <time.h>

#include "loqui-static-core.h"
#include "loqui-general-pref-default.h"
#include "loqui-general-pref-groups.h"

#include "loqui-mode-manager.h"

struct _LoquiReceiverIRCPrivate
{
	CTCPHandle *ctcp_handle;

	gboolean end_motd;
	gboolean in_recent_log;

	GRegex *regex_recent;
	gboolean failed_to_compile_regex_once;

	/* just parser */
	LoquiModeManager *channel_mode_manager;
};

LoquiModeTableItem channel_mode_table[] = {
	{ IRC_CHANNEL_MODE_CREATOR, 'O', TRUE, TRUE, "CHANNEL_MODE_CREATOR", NULL, NULL },
	{ IRC_CHANNEL_MODE_OPERATOR, 'o', TRUE, TRUE, "CHANNEL_MODE_OPERATOR", NULL, NULL },
	{ IRC_CHANNEL_MODE_VOICE, 'v', TRUE, TRUE, "CHANNEL_MODE_VOICE", NULL, NULL },
	    
	{ IRC_CHANNEL_MODE_ANONYMOUS, 'a', FALSE,  FALSE, "CHANNEL_MODE_ANONYMOUS", NULL, NULL },
	{ IRC_CHANNEL_MODE_INVITE_ONLY, 'i', FALSE,  FALSE, "CHANNEL_MODE_INVITE_ONLY", NULL, NULL },
	{ IRC_CHANNEL_MODE_MODERATED, 'm', FALSE,  FALSE, "CHANNEL_MODE_MODERATED", NULL, NULL },
	{ IRC_CHANNEL_MODE_NO_MESSAGES_FROM_CLIENT, 'n', FALSE,  FALSE, "CHANNEL_MODE_NO_MESSAGES_FROM_CLIENT", NULL, NULL },
	{ IRC_CHANNEL_MODE_QUIET, 'q', FALSE,  FALSE, "CHANNEL_MODE_QUIET", NULL, NULL },
	{ IRC_CHANNEL_MODE_SECRET, 's', FALSE,  FALSE, "CHANNEL_MODE_SECRET", NULL, NULL },
	{ IRC_CHANNEL_MODE_SERVER_REOP, 'r', FALSE,  FALSE, "CHANNEL_MODE_SERVER_REOP", NULL, NULL },
	{ IRC_CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY, 't', FALSE,  FALSE, "CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY", NULL, NULL },
	
	{ IRC_CHANNEL_MODE_CHANNEL_KEY, 'k', TRUE, FALSE, "CHANNEL_MODE_CHANNEL_KEY", NULL, NULL },
	{ IRC_CHANNEL_MODE_USER_LIMIT, 'l', TRUE, FALSE, "CHANNEL_MODE_USER_LIMIT", NULL, NULL },

	{ IRC_CHANNEL_MODE_BAN_MASK, 'b', TRUE, TRUE, "CHANNEL_MODE_BAN_MASK", NULL, NULL },
	{ IRC_CHANNEL_MODE_EXCEPTION_TO_OVERIDE_BAN_MASK, 'e', TRUE, TRUE, "CHANNEL_MODE_EXCEPTION_TO_OVERIDE_BAN_MASK", NULL, NULL },
	{ IRC_CHANNEL_MODE_INVITATION_MASK, 'I', TRUE, TRUE, "CHANNEL_MODE_INVITATION_MASK", NULL, NULL },
	
	{ IRC_CHANNEL_MODE_PRIVATE, 'p', FALSE, FALSE, "CHANNEL_MODE_PRIVATE", NULL, NULL },
	{ -1, 0, FALSE, FALSE, NULL, NULL, NULL },
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
static void loqui_receiver_irc_command_pong(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_quit(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_join(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_part(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_nick(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_mode(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_kick(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_command_error(LoquiReceiverIRC *receiver, IRCMessage *msg);

static void loqui_receiver_irc_account_console_append(LoquiReceiverIRC *receiver, IRCMessage *msg, LoquiTextType type, gchar *format);
static void loqui_receiver_irc_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, gboolean make_channel,
				      gint receiver_num, LoquiTextType type, gchar *format);
static void loqui_receiver_irc_joined_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, GList *channel_list, LoquiTextType type, gchar *format);

static void loqui_receiver_irc_reply_welcome(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_names(LoquiReceiverIRC *receiver, IRCMessage *msg);
static void loqui_receiver_irc_reply_notopic(LoquiReceiverIRC *receiver, IRCMessage *msg);
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

	priv->channel_mode_manager = loqui_mode_manager_new(channel_mode_table);

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

static gboolean
loqui_receiver_irc_validate_nick_not_null(LoquiReceiverIRC *receiver, const gchar *place, IRCMessage *msg)
{
	gchar *inspect;

        g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);

	if (msg->nick == NULL) {
		inspect = irc_message_inspect(msg);
		loqui_account_warning(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)),
				      _("(%1$s): The message doesn't contains a nick: %2$s"), place, inspect);
		g_free(inspect);

		return FALSE;
	}

	return TRUE;
}
static gboolean
loqui_receiver_irc_validate_target_not_null(LoquiReceiverIRC *receiver, const gchar *place, IRCMessage *msg)
{
	gchar *inspect;

        g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);

	if (irc_message_get_target(msg) == NULL) {
		inspect = irc_message_inspect(msg);
		loqui_account_warning(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)),
				      _("(%1$s): The message doesn't contains a target (the first parameter): %2$s"), place, inspect);
		g_free(inspect);

		return FALSE;
	}

	return TRUE;
}
static gboolean
loqui_receiver_irc_validate_is_channel_name(LoquiReceiverIRC *receiver, const gchar *place, IRCMessage *msg, gint index)
{
	gchar *inspect;
	const gchar *str = NULL;

        g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);

	if ((str = irc_message_get_param(msg, index)) == NULL ||
	    !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(str)) {
		inspect = irc_message_inspect(msg);
		loqui_account_warning(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)),
				      _("(%1$s): msg.param[%2$d] is not a channel name(%3$s): %4$s"),
				      place, index, str ? str : "NULL", inspect);
		g_free(inspect);

		return FALSE;
	}

	return TRUE;
}

static void
loqui_receiver_irc_append_recent_log(LoquiReceiverIRC *receiver, LoquiChannel *channel, const gchar *name, const gchar *line)
{
	gchar *buf;

	buf = g_strdup_printf("[%s] %s", name, line);
	loqui_channel_append_text(channel, LOQUI_TEXT_TYPE_NOTICE, buf);
	g_free(buf);
}

#define PLUM_ALIAS_OF_PERCENT_PREFIX ":*.jp"
static gboolean
loqui_receiver_irc_parse_plum_recent(LoquiReceiverIRC *receiver, const gchar *line)
{
	gchar *regexp_str;
	gchar *tmp;
	gchar *time, *pre_char_str, *from, *post_char_str, *text;
	gchar *converted_from, *from_after;
	LoquiChannel *channel;
	LoquiAccount *account;
	LoquiReceiverIRCPrivate *priv;
	GError *error = NULL;
	GMatchInfo *match_info;

        g_return_val_if_fail(receiver != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_RECEIVER_IRC(receiver), FALSE);
	g_return_val_if_fail(line != NULL, FALSE);

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	if (priv->failed_to_compile_regex_once) {
		return FALSE;
	}

	if (priv->regex_recent == NULL) {
		regexp_str = loqui_pref_get_with_default_string(loqui_core_get_general_pref(loqui_get_core()),
								LOQUI_GENERAL_PREF_GROUP_PROXY, "RecentLogRegexp",
								LOQUI_GENERAL_PREF_DEFAULT_PROXY_RECENT_LOG_REGEXP, NULL);
		if (regexp_str == NULL) {
			loqui_account_warning(account, _("Failed to get the regexp string for recent logs."));
			priv->failed_to_compile_regex_once = TRUE;
			return FALSE;
		}

		priv->regex_recent = g_regex_new(regexp_str, 0, 0, &error);
		g_free(regexp_str);

		if (!priv->regex_recent) {
			loqui_account_warning(account, _("Failed to compile the regexp string for recent logs: %s"), error->message);
			g_error_free(error);
			priv->failed_to_compile_regex_once = TRUE;
			return FALSE;
		}
	}
	
	g_regex_match(priv->regex_recent, line, 0, &match_info);
	if (!g_match_info_matches(match_info)) {
		g_match_info_free(match_info);
		return FALSE;
	}
	time = pre_char_str = from = post_char_str = text = NULL;
	
#define FETCH_NAMED(name, var) { \
	var = g_match_info_fetch_named(match_info, name); \
	if (var == NULL) { \
		loqui_account_warning(account, _("Failed to get the string for '%s' with named capture."), name); \
		g_free(time); \
		g_free(pre_char_str); \
		g_free(from); \
		g_free(post_char_str); \
		g_free(text); \
		return FALSE; \
	} \
}
	
	FETCH_NAMED("time", time);
	FETCH_NAMED("pre_char", pre_char_str);
	FETCH_NAMED("from", from);
	FETCH_NAMED("post_char", post_char_str);
	FETCH_NAMED("text", text);

	g_match_info_free(match_info);
	
	from_after = NULL;
	if ((tmp = strrchr(from, ':')) != NULL) {
		from_after = tmp + 1;
		*tmp = '\0';
	}

	if (from_after != NULL && *from_after != '\0' && *from == '%') {
		converted_from = g_strconcat("#", from+1, PLUM_ALIAS_OF_PERCENT_PREFIX, NULL);
	} else {
		converted_from = g_strdup(from);
	}

	channel = loqui_account_get_channel_by_identifier(account, converted_from);
	if(channel == NULL) {
		channel = LOQUI_CHANNEL(loqui_channel_irc_new(account, converted_from, FALSE, !LOQUI_UTILS_IRC_STRING_IS_CHANNEL(converted_from)));
		loqui_account_add_channel(account, channel);
		g_object_unref(channel);
	}
	g_free(converted_from);
	
	loqui_receiver_irc_append_recent_log(receiver, channel, "LOG", line);
	loqui_channel_entry_set_is_updated_weak(LOQUI_CHANNEL_ENTRY(channel), TRUE);

	g_free(time);
	g_free(pre_char_str);
	g_free(from);
	g_free(post_char_str);
	g_free(text);

	return TRUE;
}
static void
loqui_receiver_irc_command_privmsg_notice(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *receiver_name, *sender;
	gchar *remark;
	LoquiAccount *account;
	LoquiChannel *channel = NULL;
	LoquiTextType type;
	CTCPMessage *ctcp_msg;
	gboolean is_self;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	receiver_name = irc_message_get_target(msg);
	remark = irc_message_get_param(msg, 1);

	if (receiver_name == NULL) {
		loqui_account_warning(account, _("This PRIVMSG/NOTICE message doesn't contain a target."));
		return;
	}

	if (remark == NULL) {
		loqui_account_warning(account, _("This PRIVMSG/NOTICE message doesn't contain a remark."));
		return;
	}

	if(priv->end_motd == FALSE &&
	   msg->response == IRC_COMMAND_NOTICE &&
	   loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
					       LOQUI_GENERAL_PREF_GROUP_PROXY, "ParsePlumRecent",
					       LOQUI_GENERAL_PREF_DEFAULT_PROXY_PARSE_PLUM_RECENT, NULL)) {
		if(loqui_receiver_irc_parse_plum_recent(receiver, remark))
			return;
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

	if(msg->response == IRC_COMMAND_NOTICE) {
		type = LOQUI_TEXT_TYPE_NOTICE;
	} else {
		type = LOQUI_TEXT_TYPE_NORMAL;
	}

	if(msg->nick)
		is_self = loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), msg->nick);
	else
		is_self = FALSE;

	channel = loqui_account_irc_fetch_channel(LOQUI_ACCOUNT_IRC(account), is_self, msg->nick, receiver_name);
	if (channel == NULL) {
		loqui_account_append_text(account, NULL, type, remark);
		return;
	}

	sender = msg->nick ? msg->nick : msg->prefix;

	if (priv->in_recent_log && msg->nick == NULL) {
		loqui_receiver_irc_append_recent_log(receiver, channel, msg->prefix, remark);
	} else {
		loqui_channel_append_remark(channel, type, is_self, sender, remark);
	}
}
static void
loqui_receiver_irc_command_ping(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
        g_return_if_fail(receiver != NULL);
	g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	loqui_sender_irc_pong_raw(LOQUI_SENDER_IRC(loqui_account_get_sender(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)))), irc_message_get_target(msg));
}
static void
loqui_receiver_irc_command_pong(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;

        g_return_if_fail(receiver != NULL);
	g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;

	priv->in_recent_log = FALSE; /* for TreatAsRecentLogUntilFirstPongReceived */
}
static void
loqui_receiver_irc_command_quit(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiUser *user;
	LoquiAccount *account;
	GList *list;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	
	if (msg->nick == NULL) {
		loqui_account_warning(account, _("The message does not contain nick"));
		return;
	}

	user = loqui_account_peek_user(account, msg->nick);
	if (!user) {
		loqui_account_warning(account, "Why do you know the user '%s' quit?", msg->nick);
		return;
	}

	loqui_account_remove_user_from_all(account, user, FALSE, &list);
	loqui_receiver_irc_joined_channel_append(receiver, msg, list, LOQUI_TEXT_TYPE_INFO, _("*** %n has quit IRC(%L)"));

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

	name = irc_message_get_target(msg);
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
			loqui_receiver_irc_channel_append(receiver, msg, FALSE, 0, LOQUI_TEXT_TYPE_INFO, _("*** You have left %t (%1)"));
			loqui_channel_set_is_joined(channel, FALSE);
			loqui_channel_entry_clear_member(LOQUI_CHANNEL_ENTRY(channel));
		}
	} else {
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 0, LOQUI_TEXT_TYPE_INFO, _("*** %n has just part %t(%L)"));
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

	name = irc_message_get_target(msg);
	if(!name) {
		loqui_account_warning(account, _("The message does not contain the channal name"));
		return;
	}

	receiver_name = irc_message_get_param(msg, 1);
	if(!receiver_name) {
		loqui_account_warning(account, _("The KICK message doesn't contain the user to be kicked."));
		return;
	}
	
	channel = loqui_account_get_channel_by_identifier(account, name);
	user = loqui_account_peek_user(account, receiver_name);
	if (channel && user)
		loqui_channel_entry_remove_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);

	if(loqui_account_irc_is_current_nick(LOQUI_ACCOUNT_IRC(account), receiver_name)) {
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 0, LOQUI_TEXT_TYPE_INFO, _("*** You were kicked from %t by %n (%2)"));
		loqui_channel_set_is_joined(channel, FALSE);
	} else {
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 0, LOQUI_TEXT_TYPE_INFO, _("*** %1 was kicked from %t by %n(%2)"));
	}
}
static void
loqui_receiver_irc_command_error(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiAccount *account;

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	
	loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_ERROR, "*** %L");
	loqui_account_set_all_channel_unjoined(LOQUI_ACCOUNT(account));
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

	if (msg->nick == NULL) {
		loqui_account_warning(account, _("The message does not contain nick"));
		return;
	}

	nick_new = irc_message_get_target(msg);
	if (nick_new == NULL) {
		loqui_account_warning(account, _("The NICK message does not contain new nick"));
		return;
	}

	user = loqui_account_peek_user(account, msg->nick);
	loqui_receiver_irc_joined_channel_append(receiver, msg, NULL, LOQUI_TEXT_TYPE_INFO, _("*** %n is known as %t"));
	if (user)
		loqui_user_set_nick(user, nick_new);

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
	gint i;
	gint param_num;
	GList *param_list = NULL;
	GList *mode_item_list = NULL, *mode_item_list_used = NULL;
	LoquiMember *member;
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;
	LoquiModeItem *mode_item;
	GError *error = NULL;
	GList *l;
	LoquiUser *user;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	param_num = irc_message_count_parameters(msg);
	for (i = mode_start; i < param_num; i++)
		param_list = g_list_append(param_list, irc_message_get_param(msg, i));

	if (channel) {
		mode_item_list = loqui_mode_manager_parse(priv->channel_mode_manager, param_list, &error);
		if (error) {
			loqui_account_warning(account, "Channel mode parse error: %s", error->message);
			return;
		}

		for (l = mode_item_list; l != NULL; l = l->next) {
			mode_item = LOQUI_MODE_ITEM(l->data);

			gint mode_id = mode_item->table_item->mode_id;
			if (mode_id == IRC_CHANNEL_MODE_OPERATOR ||
			    mode_id == IRC_CHANNEL_MODE_VOICE) {
				g_assert(mode_item->data_str != NULL); /* should be parse error if NULL */
				user = loqui_account_peek_user(channel->account, mode_item->data_str);
				if (!user) {
					loqui_account_warning(account, "User not found.");
					return;
				}
				member = loqui_channel_entry_get_member_by_user(LOQUI_CHANNEL_ENTRY(channel), user);
				if (member == NULL) {
					loqui_account_warning(account, "Member not found.");
					return;
				}

				if (mode_id == IRC_CHANNEL_MODE_OPERATOR) {
					loqui_member_set_is_channel_operator(member, mode_item->is_set);
				} else {
					loqui_member_set_speakable(member, mode_item->is_set);
				}
			} else if (mode_id == IRC_CHANNEL_MODE_CREATOR ||
				   mode_id == IRC_CHANNEL_MODE_BAN_MASK ||
				   mode_id == IRC_CHANNEL_MODE_EXCEPTION_TO_OVERIDE_BAN_MASK ||
				   mode_id == IRC_CHANNEL_MODE_INVITATION_MASK) {
			} else {
				mode_item_list_used = g_list_append(mode_item_list_used, l->data);
			}
		}
		
		if (channel->channel_mode_manager)
			loqui_mode_manager_apply(channel->channel_mode_manager, mode_item_list_used);
		g_list_foreach(mode_item_list, (GFunc) g_object_unref, NULL);
		g_list_free(mode_item_list);
		g_list_free(mode_item_list_used);
	} else {
		/* FIXME: handle user modes */
		/* switch(*flags) {
		case IRC_USER_MODE_FLAGGED_AS_AWAY:
		case IRC_USER_MODE_INVISIBLE:
		case IRC_USER_MODE_RECEIVES_WALLOPS:
		case IRC_USER_MODE_RESTRICTED_CONNECTION:
		case IRC_USER_MODE_OPERATOR:
		case IRC_USER_MODE_LOCAL_OPERATOR:
		case IRC_USER_MODE_RECEIVES_SERVER_NOTICES:
		default: */
			loqui_account_warning(account, _("User mode is not implemented."));
			/* break;
			   } */
	}
#undef GET_TARGET_OR_RETURN	
}
static void
loqui_receiver_irc_reply_channelmodeis(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiChannel *channel;
	gchar *name;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	name = irc_message_get_param(msg, 1);
	if(name == NULL) {
		loqui_account_warning(account, _("The target is not found in MODE command"));
		return;
	}

	if(LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name)) {
		channel = loqui_account_get_channel_by_identifier(account, name);
		if(!channel)
			return;
	} else {
		loqui_account_warning(account, _("RPL_CHANNELMODEIS didn't return a channel name: %s"), name);
		return;
	}
	loqui_mode_manager_clear(channel->channel_mode_manager);

	loqui_receiver_irc_parse_mode_arguments(receiver, msg, channel, 2);
	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, LOQUI_TEXT_TYPE_INFO, _("*** Mode for %1: %*2"));
}
static void
loqui_receiver_irc_command_mode(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *changer = NULL;
	gchar *format, *name;
	LoquiChannel *channel = NULL;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	if(msg->nick)
		changer = msg->nick;
	else if(msg->prefix)
		changer = msg->prefix;
	else
		changer = "the server"; /* NULL prefix assumes the server */

	if(strchr(changer, '%')) {
		loqui_account_warning(account, _("Nick must not contain '%%'"));
		return;
	}

	name = irc_message_get_target(msg);
	if (name == NULL) {
		loqui_account_warning(account, _("The target is not found in MODE command"));
		return;
	}

	if (LOQUI_UTILS_IRC_STRING_IS_CHANNEL(name)) {
		channel = loqui_account_get_channel_by_identifier(account, name);
		if (!channel) {
			loqui_account_warning(account, _("Why can you know the change of his mode?"));
			return;
		}
	} else {
		channel = NULL;
	}

	loqui_receiver_irc_parse_mode_arguments(receiver, msg, channel, 1);

	format = g_strdup_printf(_("*** New mode for %%t by %s: %%*1"), changer);

	if(channel)
		loqui_receiver_irc_channel_append(receiver, msg, FALSE, 0, LOQUI_TEXT_TYPE_INFO, format);
	else
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, format);

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

	if (!loqui_receiver_irc_validate_nick_not_null(receiver, "command-join", msg))
		return;
	if (!loqui_receiver_irc_validate_is_channel_name(receiver, "command-join", msg, IRC_MESSAGE_PARAM_INDEX_TARGET))
		return;
	
	name = irc_message_get_target(msg);

	channel = loqui_account_get_channel_by_identifier(account, name);

	/* Received a JOIN command => You have just/already joined. */
	if (channel == NULL) {
		channel = LOQUI_CHANNEL(loqui_channel_irc_new(account, name, TRUE, FALSE));
		loqui_account_add_channel(account, channel);
		g_object_unref(channel);

		if(loqui_core_get_send_status_commands_mode(loqui_get_core()))
			loqui_sender_irc_get_channel_mode(loqui_account_get_sender(account), channel);
	} else {
		loqui_channel_set_is_joined(channel, TRUE);
	}
	
	loqui_channel_irc_add_member_by_nick(LOQUI_CHANNEL_IRC(channel), msg->nick, FALSE, FALSE, FALSE);
	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 0, LOQUI_TEXT_TYPE_INFO, _("*** %n (%u@%h) joined channel %L"));
}
static void
loqui_receiver_irc_inspect_message(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	gchar *str;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	str = irc_message_inspect(msg);
	
	loqui_account_append_text(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)), NULL, LOQUI_TEXT_TYPE_NORMAL, str);
	
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

	/* set the string of target to the 'user_self' */
	loqui_user_set_nick(loqui_account_get_user_self(account), irc_message_get_target(msg));
	
	receiver->passed_welcome = TRUE;

	autojoin = loqui_profile_account_irc_get_autojoin(LOQUI_PROFILE_ACCOUNT_IRC(loqui_account_get_profile(account)));
	if (autojoin && strlen(autojoin) > 0) {
		LoquiStringTokenizer *st;
		int token_num;
		gchar *names, *keys;

		st = loqui_string_tokenizer_new(autojoin, " ");
		token_num = loqui_string_tokenizer_count_tokens(st);
		if (token_num == 1) {
			loqui_sender_join_raw(loqui_account_get_sender(account), autojoin, NULL);
			loqui_account_information(account, _("Sent a command to join %s."), autojoin);
		} else if (token_num == 2) {
			names = g_strdup(loqui_string_tokenizer_next_token(st, NULL));
			keys = g_strdup(loqui_string_tokenizer_next_token(st, NULL));
	
			loqui_sender_join_raw(loqui_account_get_sender(account), names, keys);
			loqui_account_information(account, _("Sent a command to join %s (with the channel key(s))."), names);
			g_free(names);
			g_free(keys);
		} else {
			loqui_account_warning(account, _("The string for autojoin is invalid. It must be comma-separated, like '#a,#b,#c' or '#a,#b,#c keyA,keyB'."));
		}
		loqui_string_tokenizer_free(st);
	}

	/* treat as "recent log" of irc-proxy with the messages sent by the server initially */
	if (loqui_pref_get_with_default_boolean(loqui_core_get_general_pref(loqui_get_core()),
						LOQUI_GENERAL_PREF_GROUP_PROXY, "TreatAsRecentLogUntilFirstPongReceived",
						LOQUI_GENERAL_PREF_DEFAULT_PROXY_TREAT_AS_RECENT_LOG_UNTIL_FIRST_PONG_RECEIVED, NULL)) {
		priv->in_recent_log = TRUE;
		loqui_sender_irc_ping_raw(LOQUI_SENDER_IRC(loqui_account_get_sender(loqui_receiver_get_account(LOQUI_RECEIVER(receiver)))), msg->prefix);
	}

	loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "*** %*1");
	
	/* reset reconnect count */
	account->reconnect_try_count = 0;
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
	name = irc_message_get_param(msg, 2);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(channel == NULL)
		return;

	if(channel->end_names == TRUE) {
		loqui_channel_entry_clear_member(LOQUI_CHANNEL_ENTRY(channel));
		loqui_channel_entry_set_do_sort(LOQUI_CHANNEL_ENTRY(channel), FALSE);
		channel->end_names = FALSE;
	}

	st = loqui_string_tokenizer_new(irc_message_get_last_param(msg), " ");
	loqui_string_tokenizer_set_skip_whitespaces_after_delimiter(st, TRUE);
	while ((nick = loqui_string_tokenizer_next_token(st, NULL)) != NULL)
		loqui_channel_irc_add_member_by_nick(LOQUI_CHANNEL_IRC(channel), nick, TRUE, FALSE, FALSE);
	loqui_string_tokenizer_free(st);

	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 2, LOQUI_TEXT_TYPE_NORMAL, "%2: %L");
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

	name = irc_message_get_param(msg, 1);
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

	time_str = irc_message_get_param(msg, 2);
	if(time_str == NULL) {
		loqui_account_warning(account, _("Invalid CREATIONTIME reply."));
		return;
	}
	t = (time_t) g_ascii_strtoull(time_str, NULL, 10);
	if(t == 0) {
		loqui_account_warning(account, _("Invalid time string"));
		return;
	}
	
	str = loqui_utils_get_iso8601_date_string(t);
	if(str == NULL) {
		loqui_account_warning(account, _("Invalid time"));
		return;
	}

	format = g_strdup_printf(_("%%1 was created at %s"), str);
	g_free(str);

	loqui_receiver_irc_channel_append(receiver, msg, TRUE, 1, LOQUI_TEXT_TYPE_INFO, format);
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

	time_str = irc_message_get_param(msg, 3);
	if(time_str == NULL) {
		loqui_account_warning(account, _("Invalid TOPICWHOTIME reply."));
		return;
	}

	t = (time_t) g_ascii_strtoull(time_str, NULL, 10);
	if(t == 0) {
		loqui_account_warning(account, _("Invalid time string"));
		return;
	}
	
	str = loqui_utils_get_iso8601_date_string(t);
	if(str == NULL) {
		loqui_account_warning(account, _("Invalid time"));
		return;
	}

	format = g_strdup_printf(_("Topic for %%2 was set by %%3 at %s"), str);
	g_free(str);

	loqui_receiver_irc_channel_append(receiver, msg, TRUE, 1, LOQUI_TEXT_TYPE_INFO, format);
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

	sec_str = irc_message_get_param(msg, 2);
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

	loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, str);
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
	
	gchar *channel_name, *username, *hostname, *server_name, *nick, *flags, *last_param;
	gchar *away_str = NULL, *realname = NULL;
	const gchar *hops_str;
	gchar *buf;
	gchar op_char;
	guint hop_count = 0;
	LoquiChannel *channel = NULL;
	LoquiAwayType away = LOQUI_AWAY_TYPE_UNKNOWN;
	LoquiUser *user = NULL;
	LoquiMember *member = NULL;
	LoquiStringTokenizer *st;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	channel_name = irc_message_get_param(msg, 1);
	username = irc_message_get_param(msg, 2);
	hostname = irc_message_get_param(msg, 3);
	server_name = irc_message_get_param(msg, 4);
	nick = irc_message_get_param(msg, 5);
	flags = irc_message_get_param(msg, 6);
	last_param = irc_message_get_last_param(msg);
	if (!channel_name || !username || !hostname || !server_name || !nick || !flags || !last_param) {
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

	st = loqui_string_tokenizer_new(last_param, " ");
	
	if (loqui_string_tokenizer_count_tokens(st) > 1) {
		hops_str = loqui_string_tokenizer_next_token(st, NULL);
		hop_count = (guint) g_ascii_strtoull(hops_str, NULL, 10);
	} else {
		hop_count = -1;
	}
	realname = g_strdup(loqui_string_tokenizer_next_token(st, NULL));
	loqui_string_tokenizer_free(st);

	if (user) {
		loqui_user_set_realname(user, realname);
		loqui_user_irc_set_hop_count(LOQUI_USER_IRC(user), hop_count);
	}

	if (receiver->prevent_print_who_reply_count == 0) {
		buf = g_strdup_printf(_("%c%s(%s) is %s@%s (%s) on %s(%d hops) [%s]"),
					op_char, nick, channel_name, username, hostname,
					realname, server_name, hop_count, away_str);
		loqui_account_append_text(account, NULL, LOQUI_TEXT_TYPE_INFO, buf);
		g_free(buf);
	}

	g_free(realname);
}
static void
loqui_receiver_irc_reply_notopic(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiChannel *channel;
	gchar *topic;
	gchar *name;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));
	
	name = irc_message_get_param(msg, 1);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if (channel == NULL)
		return;

	loqui_channel_entry_set_topic(LOQUI_CHANNEL_ENTRY(channel), NULL);
	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, LOQUI_TEXT_TYPE_INFO, _("*** No topic is set for %1"));
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

	name = irc_message_get_param(msg, 1);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_last_param(msg);
	loqui_channel_entry_set_topic(LOQUI_CHANNEL_ENTRY(channel), strlen(topic) ? topic : NULL);

	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 1, LOQUI_TEXT_TYPE_INFO, _("Topic for %1: %L"));
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

	name = irc_message_get_target(msg);
	channel = loqui_account_get_channel_by_identifier(account, name);
	if(channel == NULL)
		return;

	topic = irc_message_get_last_param(msg);
	loqui_channel_entry_set_topic(LOQUI_CHANNEL_ENTRY(channel), strlen(topic) ? topic : NULL);
	
	loqui_receiver_irc_channel_append(receiver, msg, FALSE, 0, LOQUI_TEXT_TYPE_INFO, _("*** New topic on %t by %n: %L"));
}
/* TODO: change nick */
static void
loqui_receiver_irc_error_nick_unusable(LoquiReceiverIRC *receiver, IRCMessage *msg)
{
	LoquiReceiverIRCPrivate *priv;
	LoquiAccount *account;

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_ERROR, "%1: %L");

	if(!receiver->passed_welcome)
		loqui_account_disconnect(account);
}
static void /* utility function */
loqui_receiver_irc_joined_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, GList *channel_list, LoquiTextType type, gchar *format)
{
	LoquiReceiverIRCPrivate *priv;
	gchar *str;
	LoquiUser *user = NULL;
	LoquiAccount *account;

        g_return_if_fail(receiver != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER_IRC(receiver));

	priv = receiver->priv;
	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	str = irc_message_format(msg, format);

	if (msg->nick)
		user = loqui_account_peek_user(account, msg->nick);

	if (!channel_list && user)
		loqui_account_append_text_to_joined_channels(account, user, TRUE, type, str);
	else
		loqui_account_append_text(account, channel_list, type, str);

	g_free(str);
}
static void /* utility function */
loqui_receiver_irc_account_console_append(LoquiReceiverIRC *receiver, IRCMessage *msg, LoquiTextType type, gchar *format)
{
	gchar *str;
	LoquiAccount *account;

	account = loqui_receiver_get_account(LOQUI_RECEIVER(receiver));

	str = irc_message_format(msg, format);
	loqui_account_append_text(account, NULL, type, str);

	g_free(str);
}

static void /* utility function */
loqui_receiver_irc_channel_append(LoquiReceiverIRC *receiver, IRCMessage *msg, gboolean make_channel,
			  gint receiver_num, LoquiTextType type, gchar *format)
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
		loqui_account_append_text(account, NULL, type, str);
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
	case IRC_RPL_HELLO:
	case IRC_RPL_YOURID:
	case IRC_RPL_YOURHOST:
	case IRC_RPL_CREATED:
	case IRC_RPL_MYINFO:
	case IRC_RPL_BOUCE:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "*** %*1");
		return TRUE;
	case IRC_RPL_AWAY:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("*** %1 is marked as begin AWAY, but left the message: %2"));
		return TRUE;
	case IRC_RPL_INFO:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "*** %1");
		return TRUE;
	case IRC_RPL_LUSERCLIENT:
	case IRC_RPL_LUSERME:
	case IRC_RPL_LOCALUSERS:
	case IRC_RPL_GLOBALUSERS:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("*** %L"));
		return TRUE;
	case IRC_RPL_UNAWAY:
		loqui_user_set_away(loqui_account_get_user_self(account), LOQUI_AWAY_TYPE_ONLINE);
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("*** %L"));
		return TRUE;
	case IRC_RPL_NOWAWAY:
		loqui_user_set_away(loqui_account_get_user_self(account), LOQUI_AWAY_TYPE_AWAY);
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("*** %L"));
		return TRUE;
	case IRC_RPL_INVITING:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("*** You are inviting %1 to %2"));
		return TRUE;
	case IRC_RPL_VERSION:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("*** %2 is running IRC version %1 (%3)"));
		return TRUE;
	case IRC_RPL_LUSEROP:
	case IRC_RPL_LUSERUNKNOWN:
	case IRC_RPL_LUSERCHANNELS:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "*** %1 %2");
		return TRUE;
	case IRC_RPL_LINKS:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "%2 %3");
		return TRUE;
	case IRC_RPL_MOTDSTART:
	case IRC_RPL_MOTD:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "*** %L");
		return TRUE;
	case IRC_RPL_ENDOFMOTD:
		priv->end_motd = TRUE;
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "*** %L");
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
	case IRC_RPL_NOTOPIC:
		loqui_receiver_irc_reply_notopic(receiver, msg);
		return TRUE;
	case IRC_RPL_TOPIC:
		loqui_receiver_irc_reply_topic(receiver, msg);
		return TRUE;
	case IRC_RPL_WHOREPLY:
		loqui_receiver_irc_reply_who(receiver, msg);
		return TRUE;
	case IRC_RPL_WHOISUSER:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, _("%1 is %2@%3: %L"));
		return TRUE;
	case IRC_RPL_WHOWASUSER:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, _("%1 was %2@%3: %L"));
		return TRUE;
	case IRC_RPL_WHOISCHANNELS:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, _("%1: %L"));
		return TRUE;
	case IRC_RPL_WHOISSERVER:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, _("on via server %2(%L)"));
		return TRUE;
	case IRC_RPL_WHOISIDLE:
		loqui_receiver_irc_reply_whoisidle(receiver, msg);
		return TRUE;
	case IRC_RPL_BANLIST:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, _("Banned on %1 : %2"));
		return TRUE;
	case IRC_RPL_TIME:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, _("Time: %2(%1)"));
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
			loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("%2"));	
		return TRUE;
	case IRC_RPL_ENDOFWHOIS:
	case IRC_RPL_ENDOFBANLIST:
	case IRC_RPL_ENDOFINFO:
	case IRC_RPL_ENDOFUSERS:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("%2"));
		return TRUE;
	case IRC_RPL_NONE:
		return TRUE;
	case IRC_RPL_ISON:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("Ison: %*1"));
		return TRUE;
	case IRC_RPL_LISTSTART:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("Channel List: Name(Users) Topic"));
		return TRUE;
	case IRC_RPL_LIST:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_NORMAL, _("%1(%2)\t%3"));
		return TRUE;
	case IRC_RPL_LISTEND:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, _("%L"));
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
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_ERROR, _("%1 %L (%2)"));
		return TRUE;
	case IRC_ERR_UNKNOWNMODE: /* [$msgto, ?, is unknown mode char to me for ...] */
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_ERROR, _("%1 %2"));
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
	case IRC_ERR_INVITEONLYCHAN:
	case IRC_ERR_BANNEDFROMCHAN:
	case IRC_ERR_BADCHANNELKEY:
	case IRC_ERR_BADCHANMASK:
	case IRC_ERR_NOCHANMODES:
	case IRC_ERR_CHANOPRIVSNEEDED:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_ERROR, _("%L: %1"));
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
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_ERROR, _("%L"));
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
		loqui_receiver_irc_command_pong(receiver, msg);
		return TRUE;
	case IRC_COMMAND_INVITE:
		loqui_receiver_irc_account_console_append(receiver, msg, LOQUI_TEXT_TYPE_INFO, "*** You were invited to %1 by %n");
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

	priv->in_recent_log = FALSE;

	if (priv->regex_recent) {
		g_regex_unref(priv->regex_recent);
		priv->regex_recent = NULL;
	}
	priv->failed_to_compile_regex_once = FALSE;
}
