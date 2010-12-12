/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://launchpad.net/loqui/>
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

#include "ctcp_message.h"
#include "irc_constants.h"
#include "loqui-utils.h"
#include <libloqui-intl.h>
#include <string.h>

struct _CTCPMessagePrivate
{
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void ctcp_message_class_init(CTCPMessageClass *klass);
static void ctcp_message_init(CTCPMessage *ctcp_message);
static void ctcp_message_finalize(GObject *object);

GType
ctcp_message_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(CTCPMessageClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) ctcp_message_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(CTCPMessage),
				0,              /* n_preallocs */
				(GInstanceInitFunc) ctcp_message_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "CTCPMessage",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
ctcp_message_class_init(CTCPMessageClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = ctcp_message_finalize;
}
static void 
ctcp_message_init(CTCPMessage *ctcp_message)
{
	CTCPMessagePrivate *priv;

	priv = g_new0(CTCPMessagePrivate, 1);

	ctcp_message->priv = priv;
}
static void 
ctcp_message_finalize(GObject *object)
{
	CTCPMessage *ctcp_msg;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CTCP_MESSAGE(object));

        ctcp_msg = CTCP_MESSAGE(object);

	LOQUI_G_FREE_UNLESS_NULL(ctcp_msg->command);
	LOQUI_G_FREE_UNLESS_NULL(ctcp_msg->argument);
	if(ctcp_msg->parameters) {
		g_strfreev(ctcp_msg->parameters);
		ctcp_msg->parameters = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(ctcp_msg->priv);
}

CTCPMessage*
ctcp_message_new(const gchar *command, const gchar *argument)
{
        CTCPMessage *ctcp_msg;

	ctcp_msg = g_object_new(ctcp_message_get_type(), NULL);
	
	ctcp_msg->command = g_strdup(command);
	if(argument) {
		ctcp_msg->argument = g_strdup(argument);
		ctcp_msg->parameters = g_strsplit(argument, " ", 0);
	}

	return ctcp_msg;
}
gboolean
ctcp_message_parse_line(const gchar *line, CTCPMessage **ctcp_msg)
{
	gchar *cur, *start, *end, *command, *argument = NULL;
	gchar *buf;

	if(line == NULL)
		return FALSE;

	buf = g_strdup(line);

	start = strchr(buf, IRCCommandChar);
	if(start == NULL) {
		g_free(buf);
		return FALSE;
	}
	
	cur = start+1;
	if(*cur == '\0') {
		g_free(buf);
		return FALSE;
	}

	end = strchr(cur, IRCCommandChar);
	if(end == NULL) {
		g_free(buf);
		return FALSE;
	}
	*end = '\0';

	command = cur;
	cur = strchr(cur, ' ');
	if(cur != NULL) {
		*cur = '\0';
		cur++;
		if(*cur != '\0')
			argument = cur;
	}

	if(ctcp_msg)
		*ctcp_msg = ctcp_message_new(command, argument);

	g_free(buf);
	return TRUE;
}
gchar *
ctcp_message_to_str(CTCPMessage *ctcp_msg)
{
	gchar *str;

	str = g_strdup_printf("%c%s %s%c", 
			      IRCCommandChar,
			      ctcp_msg->command, 
			      ctcp_msg->argument ? ctcp_msg->argument : "",
			      IRCCommandChar);
	
	return str;
}
gint
ctcp_message_count_parameters(CTCPMessage *ctcp_msg)
{
        g_return_val_if_fail(ctcp_msg != NULL, 0);
        g_return_val_if_fail(IS_CTCP_MESSAGE(ctcp_msg), 0);

	return loqui_utils_count_strarray((const gchar **) ctcp_msg->parameters);
}

G_CONST_RETURN gchar *
ctcp_message_get_param(CTCPMessage *ctcp_msg, gint i)
{
        g_return_val_if_fail(ctcp_msg != NULL, NULL);
        g_return_val_if_fail(IS_CTCP_MESSAGE(ctcp_msg), NULL);
	
	if (ctcp_message_count_parameters(ctcp_msg) <= i) {
		return NULL;
	}

	return ctcp_msg->parameters[i];
}
