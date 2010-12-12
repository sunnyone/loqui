/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM library with GNet/GObject <http://launchpad.net/loqui/>
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

#include <glib-object.h>

#ifndef __LOQUI_MESSAGE_H__
#define __LOQUI_MESSAGE_H__

G_BEGIN_DECLS

#define LOQUI_TYPE_MESSAGE                 (loqui_message_get_type ())
#define LOQUI_MESSAGE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_MESSAGE, LoquiMessage))
#define LOQUI_MESSAGE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_MESSAGE, LoquiMessageClass))
#define LOQUI_IS_MESSAGE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_MESSAGE))
#define LOQUI_IS_MESSAGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_MESSAGE))
#define LOQUI_MESSAGE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_MESSAGE, LoquiMessageClass))

typedef struct _LoquiMessage            LoquiMessage;
typedef struct _LoquiMessageClass       LoquiMessageClass;

typedef struct _LoquiMessagePrivate     LoquiMessagePrivate;

typedef enum {
	LOQUI_COMMAND_FLAG_NONE = 0,
	LOQUI_COMMAND_FLAG_INTERNAL = 1 << 0, /* the command should be invisible */
	LOQUI_COMMAND_FLAG_RECEIVE = 1 << 1,
	LOQUI_COMMAND_FLAG_SEND = 1 << 2
} LoquiMessageCommandFlags;

typedef struct _LoquiMessageCommandInfo {
	gchar *name; /* name must be unique */
	GData *attr_def_dlist; /* ex. attr_dl["channel"] = LOQUI_TYPE_CHANNEL */
	LoquiMessageCommandFlags flags;
	gboolean usable;
} LoquiMessageCommandInfo;

struct _LoquiMessage
{
        GObject parent;
        
	GQuark command_quark;
	GData *attr_dlist; /* value: GValue */

        LoquiMessagePrivate *priv;
};

struct _LoquiMessageClass
{
        GObjectClass parent_class;

	GData *command_info_table;
};

GType loqui_message_get_type(void) G_GNUC_CONST;

LoquiMessage* loqui_message_new(const gchar *command);

void loqui_message_set_attribute(LoquiMessage *message, const gchar *first_attribute_name, ...);
void loqui_message_get_attribute(LoquiMessage *message, const gchar *first_attribute_name, ...);

void loqui_message_class_install_command(LoquiMessageClass *message_class,
					 const gchar *name,
					 LoquiMessageCommandFlags flags,
					 const gchar *first_attribute_name,
					 ...); /* key, type, key, type...; null terminated */
LoquiMessageCommandInfo *loqui_message_class_get_command_info(LoquiMessageClass *message_class,
							      const gchar *name);

/* default commands */
#define LOQUI_COMMAND_UNHANDLED "unhandled"

/* these general messages can be used when no action accompanied. */
#define LOQUI_COMMAND_ERROR "error"
#define LOQUI_COMMAND_GLOBAL_INFO "global-info"
#define LOQUI_COMMAND_CHANNEL_INFO "channel-info"
#define LOQUI_COMMAND_USER_INFO "user-info"


#define LOQUI_COMMAND_MESSAGE "message"


#define LOQUI_COMMAND_NICK "nick"
#define LOQUI_COMMAND_AWAY "away"

#define LOQUI_COMMAND_WHO "who"
#define LOQUI_COMMAND_WHOIS "whois"

#define LOQUI_COMMAND_QUIT "quit"


#define LOQUI_COMMAND_TOPIC "topic"

#define LOQUI_COMMAND_JOIN "join"
#define LOQUI_COMMAND_PART "part"

#define LOQUI_COMMAND_INVITE "invite"
#define LOQUI_COMMAND_KICK "kick"

/* TODO: add "online", "offline" */

G_END_DECLS

#endif /* __LOQUI_MESSAGE_H__ */
