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

#include "irc_message.h"
#include "command_table.h"
#include "utils.h"
#include <stdarg.h>

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
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = irc_message_finalize;
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

	if(msg->prefix) { 
		g_free(msg->prefix); 
		msg->prefix = NULL; 
	}
	if(msg->command) { 
		g_free(msg->command); 
		msg->command = NULL; 
	}
	if(msg->parameter) { 
		g_strfreev(msg->parameter); 
		msg->parameter = NULL;
	}
	if(msg->nick) { 
		g_free(msg->nick);
		msg->nick = NULL;
	}
	if(msg->user) { 
		g_free(msg->user);
		msg->user = NULL;
	}
	if(msg->host) { 
		g_free(msg->host); 
		msg->host = NULL;
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

	g_return_val_if_fail(command != NULL, NULL);
	g_return_val_if_fail(parameter != NULL, NULL);

	msg = g_object_new(irc_message_get_type(), NULL);

	if(prefix) {
		msg->prefix = g_strdup(prefix);
		irc_message_parse_prefix(prefix, &msg->nick, &msg->user, &msg->host);
	} else
		msg->prefix = NULL;

	msg->command = g_strdup(command);
	msg->parameter = g_strdupv(parameter);

	msg->response = (int) g_ascii_strtod(command, NULL);
	if(msg->response == 0) {
		msg->response = command_table_make_command_numeric(command);
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
	g_string_printf(string, "prefix: %s, command: %s, numeric: %d\n", msg->prefix, msg->command, msg->response);
	if(msg->prefix != NULL) {
		if(msg->nick) {
			g_string_append_printf(string, "nick: %s, user: %s, host: %s\n", 
					       msg->nick, msg->user, msg->host);
		} else {
			g_string_append_printf(string, "server: %s\n", msg->prefix);
		}
	}
	
	for(i = 0; msg->parameter[i] != NULL; i++) {
		g_string_append_printf(string, "para[%d]: '%s'\n", i, msg->parameter[i]);
	}
	
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
	g_print("%s", str);
	g_free(str);

}
IRCMessage*
irc_message_parse_line(const gchar *line)
{
	IRCMessage *msg;
	gchar **array;
	gchar *prefix, *command, *parameter[16], *last_param;
	int i, j;

	g_return_val_if_fail(line != NULL, NULL);

	/* prefix(1-) + command(1) + parameter(15-) = 17- */
	array = g_strsplit(line, " ", 17);

	prefix = NULL;

	i = 0;
	if(*array[i] == ':') {
		prefix = array[i]+1;
		i++;
	}
	command = array[i];
	i++;

	for(j = i; j < 14+i; j++) {
		if(array[j] == NULL || *array[j] == ':')
			break;
		parameter[j-i] = array[j];
	}
	last_param = g_strjoinv(" ", array+j);
	parameter[j-i] = last_param;
	if(*parameter[j-i] == ':')
		parameter[j-i]++;
	utils_remove_return_code(parameter[j-i]);
	parameter[j-i+1] = NULL;

	msg = irc_message_new(prefix, command, parameter);
	
	g_free(last_param);
	g_strfreev(array);
	
	return msg;
}

IRCMessage *
irc_message_create(gchar *command, gchar *param, ...)
{
	IRCMessage *msg;
	va_list args;
	gint num, i;
	gchar **strings;

	va_start(args, param);
	for(num = 0; va_arg(args, gchar *); num++); /* count numbers */
	va_end(args);
	
	strings = g_new(gchar *, num + 2);

	strings[0] = param;

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
irc_message_createv(gchar *command, gchar *param_array[])
{
	return irc_message_new(NULL, command, param_array);
}

/* FIXME: the last parameter always has : as a first character currently. 
   it may be cause problems. */
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

	for(num = 0; msg->parameter[num] != NULL; num++); /* count the number of parameters */
	if(num < 1)
		return NULL;

	string = g_string_new(msg->command);

	for(i = 0; i < num - 1; i++) {
		g_string_append_c(string, ' ');
		g_string_append(string, msg->parameter[i]);
	}
	g_string_append_printf(string, " :%s", msg->parameter[i]);

	str = string->str;
	g_string_free(string, FALSE);

	return str;
}
gchar *
irc_message_get_param(IRCMessage *msg, guint i)
{
	g_return_val_if_fail(1 <= i && i <= 15, NULL);
	int num;

	i--;
	for(num = 0; msg->parameter[num] != NULL; num++); /* count the number of parameters */
	g_return_val_if_fail(num >= i, NULL);
	
	return msg->parameter[i];
}
gchar *
irc_message_format(IRCMessage *msg, const gchar *format)
{
}
