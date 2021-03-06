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

#include "irc_message.h"
#include "loqui-utils.h"
#include "irc_constants.h"
#include <libloqui-intl.h>

#include <stdarg.h>
#include <string.h>

typedef struct {
	gchar *command;
	IRCResponse response;
} CommandItem;

static CommandItem command_table[] = {
	{IRCCommandPing, IRC_COMMAND_PING},
	{IRCCommandPrivmsg, IRC_COMMAND_PRIVMSG},
	{IRCCommandJoin, IRC_COMMAND_JOIN},
	{IRCCommandNotice, IRC_COMMAND_NOTICE},
	{IRCCommandTopic, IRC_COMMAND_TOPIC},
	{IRCCommandMode, IRC_COMMAND_MODE},
	{IRCCommandNick, IRC_COMMAND_NICK},
	{IRCCommandPart, IRC_COMMAND_PART},
	{IRCCommandQuit, IRC_COMMAND_QUIT},
	{IRCCommandKick, IRC_COMMAND_KICK},
	{IRCCommandInvite, IRC_COMMAND_INVITE},
	{IRCCommandError, IRC_COMMAND_ERROR},
	{IRCCommandPass, IRC_COMMAND_PASS},
	{IRCCommandNick, IRC_COMMAND_NICK},
	{IRCCommandUser, IRC_COMMAND_USER},
	{IRCCommandOper, IRC_COMMAND_OPER},
	{IRCCommandSQuit, IRC_COMMAND_SQUIT},
	{IRCCommandStats, IRC_COMMAND_STATS},
	{IRCCommandLinks, IRC_COMMAND_LINKS},
	{IRCCommandTime, IRC_COMMAND_TIME},
	{IRCCommandConnect, IRC_COMMAND_CONNECT},
	{IRCCommandTrace, IRC_COMMAND_TRACE},
	{IRCCommandAdmin, IRC_COMMAND_ADMIN},
	{IRCCommandInfo, IRC_COMMAND_INFO},
	{IRCCommandNotice, IRC_COMMAND_NOTICE},
	{IRCCommandWho, IRC_COMMAND_WHO},
	{IRCCommandWhois, IRC_COMMAND_WHOIS},
	{IRCCommandWhowas, IRC_COMMAND_WHOWAS},
	{IRCCommandKill, IRC_COMMAND_KILL},
	{IRCCommandPing, IRC_COMMAND_PING},
	{IRCCommandPong, IRC_COMMAND_PONG},
	{IRCCommandAway, IRC_COMMAND_AWAY},
	{IRCCommandRehash, IRC_COMMAND_REHASH},
	{IRCCommandRestart, IRC_COMMAND_RESTART},
	{IRCCommandSummon, IRC_COMMAND_SUMMON},
	{IRCCommandUsers, IRC_COMMAND_USERS},
	{IRCCommandUserhost, IRC_COMMAND_USERHOST},
	{IRCCommandIson, IRC_COMMAND_ISON},
	{IRCCommandObject, IRC_COMMAND_OBJECT},
	{NULL, 0}
};

struct _IRCMessagePrivate
{
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void irc_message_class_init(IRCMessageClass *klass);
static void irc_message_init(IRCMessage *irc_message);
static void irc_message_finalize(GObject *object);

/* this function returns TRUE if str is nick!user@host
   Then nick, user and host must be freed. */
static gboolean irc_message_parse_prefix(const gchar *str, gchar **nick, gchar **user, gchar **host);

GType
irc_message_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(IRCMessageClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) irc_message_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(IRCMessage),
				0,              /* n_preallocs */
				(GInstanceInitFunc) irc_message_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "IRCMessage",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
irc_message_class_init(IRCMessageClass *klass)
{
	int i;

        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = irc_message_finalize;

	klass->command_hash = g_hash_table_new(g_str_hash, g_str_equal);
	for(i = 0; command_table[i].command != NULL; i++) {
		g_hash_table_insert(klass->command_hash,
				    (gpointer) command_table[i].command,
				    GINT_TO_POINTER(command_table[i].response));
	}
}
static void 
irc_message_init(IRCMessage *irc_message)
{
	IRCMessagePrivate *priv;

	priv = g_new0(IRCMessagePrivate, 1);

	irc_message->priv = priv;
}
static void 
irc_message_finalize(GObject *object)
{
	IRCMessage *msg;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_IRC_MESSAGE(object));

        msg = IRC_MESSAGE(object);

	LOQUI_G_FREE_UNLESS_NULL(msg->prefix);
	LOQUI_G_FREE_UNLESS_NULL(msg->command);
	LOQUI_G_FREE_UNLESS_NULL(msg->nick);
	LOQUI_G_FREE_UNLESS_NULL(msg->user);
	LOQUI_G_FREE_UNLESS_NULL(msg->host);

	if (msg->parameter) {
		g_strfreev(msg->parameter);
		msg->parameter = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(msg->priv);
}

static gboolean
irc_message_parse_prefix(const gchar *str, gchar **nick, gchar **user, gchar **host)
{
	gchar *s, *t, *buf;

	g_return_val_if_fail(str != NULL, FALSE);
	g_return_val_if_fail(nick != NULL, FALSE);
	g_return_val_if_fail(user != NULL, FALSE);
	g_return_val_if_fail(host != NULL, FALSE);

	*nick = NULL;
	*user = NULL;
	*host = NULL;

	buf = t = g_strdup(str);

	s = strchr(t, '!');
	if(!s) {
		g_free(buf);
		return FALSE;
	}
	*s = '\0';
	*nick = g_strdup(t);

	t = ++s;
	s = strchr(t, '@');
	if(!s) {
		g_free(*nick);
		*nick = NULL;
		g_free(buf);
		return FALSE;
	}
	*s = '\0';
	*user = g_strdup(t);

	t = ++s;
	*host = g_strdup(t);

	return TRUE;
}

IRCMessage*
irc_message_new(const gchar *prefix, const gchar *command, gchar **parameter)
{
        IRCMessage *msg;
	IRCMessageClass *klass;

	g_return_val_if_fail(command != NULL, NULL);
	g_return_val_if_fail(parameter != NULL, NULL);

	msg = g_object_new(irc_message_get_type(), NULL);

	if(prefix) {
		msg->prefix = g_strdup(prefix);
		irc_message_parse_prefix(prefix, &msg->nick, &msg->user, &msg->host);
	} else
		msg->prefix = NULL;

	msg->command = g_utf8_strup(command, -1);
	msg->parameter = g_strdupv(parameter);

	msg->response = (int) g_ascii_strtoull(command, NULL, 10);
	if(msg->response == 0) {
		klass = IRC_MESSAGE_GET_CLASS(msg);
		msg->response = GPOINTER_TO_INT(g_hash_table_lookup(klass->command_hash, msg->command));
	}
		
	return msg;
}
gchar *
irc_message_inspect(IRCMessage *msg)
{
	int i;
	GString *string;
	gchar *str;
		
        g_return_val_if_fail(msg != NULL, NULL);
        g_return_val_if_fail(IS_IRC_MESSAGE(msg), NULL);

	string = g_string_new(NULL);
	g_string_printf(string, "command: %s(%d), prefix: %s", msg->command, msg->response, msg->prefix);
	if(msg->prefix != NULL) {
		if(msg->nick) {
			g_string_append_printf(string, "(nick: %s, user: %s, host: %s)", 
					       msg->nick, msg->user, msg->host);
		} else {
			g_string_append_printf(string, "(server: %s)", msg->prefix);
		}
	}
	g_string_append(string, ", args: [");

	for(i = 0; msg->parameter[i] != NULL; i++) {
		g_string_append_printf(string, "'%s'", msg->parameter[i]);
		if(msg->parameter[i+1] != NULL)
			g_string_append_printf(string, ", ");
	}

	g_string_append(string, "]");

	str = string->str;
	g_string_free(string, FALSE);
	return str;
}
void
irc_message_print(IRCMessage *msg)
{
	gchar *str;
	
        g_return_if_fail(msg != NULL);
        g_return_if_fail(IS_IRC_MESSAGE(msg));

	str = irc_message_inspect(msg);
	g_print("%s\n", str);
	g_free(str);
}
IRCMessage*
irc_message_parse_line(const gchar *line)
{
	IRCMessage *msg;
	gchar *array[20];
	gchar *prefix = NULL, *command = NULL, *parameter[16];
	int i = 0, num, start;
	gchar *buf;
	gchar *cur, *tmp;

	g_return_val_if_fail(line != NULL, NULL);
	
	buf = g_strdup(line);
	loqui_utils_remove_return_code(buf);

	if(buf == NULL || *buf == '\0') {
		loqui_debug_puts("Empty line was sent by server.");
		return NULL;
	}

	cur = buf;
	while((tmp = strchr(cur, ' ')) != NULL) {
		while(*tmp == ' ') {
			*tmp = '\0';
			tmp++;
		}

		if(*tmp == '\0')
			break;

		array[i++] = cur;
		cur = tmp;

		/* FIXME: if the message has prefix, this can parse IRC_MESSAGE_PARAMETER_MAX - 1 */
		if(i >= IRC_MESSAGE_PARAMETER_MAX)
			break;

		if(*cur == ':') {
			cur++;
			break;
		}
	}
	array[i++] = cur;
	num = i;

	if(num < 1) {
		loqui_debug_puts("No prefix/command/parameters given: '%s'.", line);
		g_free(buf);
		return NULL;
	}

	if(*array[0] == ':') {
		prefix = array[0]+1;
		command = array[1];
	        start = 2;
	} else {
		prefix = NULL;
		command = array[0];
		start = 1;
	}

	for(i = start; i < num; i++) {
		parameter[i-start] = array[i];
	}
	parameter[i-start] = NULL;

	msg = irc_message_new(prefix, command, parameter);
	
	g_free(buf);

	return msg;
}

IRCMessage *
irc_message_create(const gchar *command, const gchar *param, ...)
{
	IRCMessage *msg;
	va_list args;
	gint num, i;
	gchar **strings;

	va_start(args, param);
	for(num = 0; va_arg(args, gchar *); num++); /* count numbers */
	va_end(args);
	
	strings = g_new(gchar *, num + 2);

	strings[0] = (gchar *) param; /* FIXME: originally this needn't cast */

	va_start(args, param);
	for(i = 1; i < num+1; i++) {
		strings[i] = va_arg(args, gchar *);
	}
	strings[i] = NULL;
	va_end(args);

	msg = irc_message_new(NULL, command, strings);
	g_free(strings);

	return msg;
}

IRCMessage *
irc_message_createv(const gchar *command, gchar *param_array[])
{
	return irc_message_new(NULL, command, param_array);
}

gchar *
irc_message_to_string(IRCMessage *msg)
{
	GString *string;
	gchar *str;
	gint i, num;

        g_return_val_if_fail(msg != NULL, NULL);
        g_return_val_if_fail(IS_IRC_MESSAGE(msg), NULL);

	g_return_val_if_fail(msg->command != NULL, NULL);
	g_return_val_if_fail(msg->parameter != NULL, NULL);

	num = irc_message_count_parameters(msg);

	string = g_string_new(msg->command);

	if(num > 0) {
		for(i = 0; i < num - 1; i++) {
			g_string_append_c(string, ' ');
			g_string_append(string, msg->parameter[i]);
		}
		// only if the last parameter has <SPACE>(' ') or empty (""), add ':' (for madoka)
		if (strlen(msg->parameter[i]) > 0 && strchr(msg->parameter[i], ' ') == NULL) {
			g_string_append_printf(string, " %s", msg->parameter[i]);
		} else {
			g_string_append_printf(string, " :%s", msg->parameter[i]);
		}
	}

	str = string->str;
	g_string_free(string, FALSE);

	return str;
}
gint
irc_message_count_parameters(IRCMessage *msg)
{
	gint num;
	for(num = 0; msg->parameter[num] != NULL; num++);

	return num;
}
gchar *
irc_message_get_target(IRCMessage *msg)
{
	if (irc_message_count_parameters(msg) == 0)
		return 0;
	return msg->parameter[IRC_MESSAGE_PARAM_INDEX_TARGET];
}
gchar *
irc_message_get_last_param(IRCMessage *msg)
{
	guint num;
	
	num = irc_message_count_parameters(msg);
	if (num < 1)
		return NULL;
	return msg->parameter[num-1];
}
gchar *
irc_message_get_param(IRCMessage *msg, guint i)
{
	int num;
	g_return_val_if_fail(0 <= i && i < IRC_MESSAGE_PARAMETER_MAX, NULL);

	num = irc_message_count_parameters(msg);
	if (num < i) {
		g_warning(_("Invalid parameter number"));
		return NULL;
	}
	
	return msg->parameter[i];
}

gchar *
irc_message_format(IRCMessage *msg, const gchar *format)
{
	GString *string;
	const gchar *cur;
	gchar *tmp, *buf, *s;
	guint64 i, end;

        g_return_val_if_fail(msg != NULL, NULL);
        g_return_val_if_fail(IS_IRC_MESSAGE(msg), NULL);
	g_return_val_if_fail(format != NULL, NULL);

	string = g_string_new_len(NULL, strlen(format));
	cur = format;
	while((tmp = strchr(cur, '%')) != NULL) {
		if(tmp > cur) {
			string = g_string_append_len(string, cur, tmp - cur);
			cur = tmp;
		}
		cur++;

		if(*cur == '\0')
			break;

		if((i = g_ascii_strtoull(cur, &tmp, 10)) != 0) {
			buf = irc_message_get_param(msg, i);
			string = g_string_append(string, buf);
			cur = tmp;
			continue;
		}

		switch(*cur) {
		case '*': /* %*n : all params after n */
			cur++;
			i = g_ascii_strtoull(cur, &tmp, 10);
			if (cur == tmp)
				break;
			cur = tmp;

			end = irc_message_count_parameters(msg) - 1;
			for(; i < end; i++) {
				string = g_string_append(string, irc_message_get_param(msg, i));
				string = g_string_append_c(string, ' ');
			}
			string = g_string_append(string, irc_message_get_param(msg, i)); /* not to append space at last */
			break;

		case 'n': /* nick */
			string = g_string_append(string, msg->nick);
			cur++;
			break;
		case 'u': /* user */
			string = g_string_append(string, msg->user);
			cur++;
			break;
		case 'h': /* host */
			string = g_string_append(string, msg->host);
			cur++;
			break;
		case 'L': /* last */
			s = irc_message_get_last_param(msg);
			if (s != NULL)
				string = g_string_append(string, s);
			cur++;
			break;
		case 't': /* target,msgto */
			s = irc_message_get_target(msg);
			if (s != NULL)
				string = g_string_append(string, s);
			cur++;
			break;
		case '%': /* %% */
			string = g_string_append_c(string, '%');
			cur++;
			break;
		default:
			cur++;
			break;
		}
	}
	string = g_string_append(string, cur);

	tmp = string->str;
	g_string_free(string, FALSE);

	return tmp;
}
