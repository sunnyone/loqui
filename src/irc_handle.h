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
#ifndef __IRC_HANDLE_H__
#define __IRC_HANDLE_H__

#include <gnome.h>
#include "account.h"
#include "irc_message.h"

G_BEGIN_DECLS

#define TYPE_IRC_HANDLE                 (irc_handle_get_type ())
#define IRC_HANDLE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_IRC_HANDLE, IRCHandle))
#define IRC_HANDLE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_IRC_HANDLE, IRCHandleClass))
#define IS_IRC_HANDLE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_IRC_HANDLE))
#define IS_IRC_HANDLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_IRC_HANDLE))
#define IRC_HANDLE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IRC_HANDLE, IRCHandleClass))

typedef struct _IRCHandle            IRCHandle;
typedef struct _IRCHandleClass       IRCHandleClass;

typedef struct _IRCHandlePrivate     IRCHandlePrivate;

struct _IRCHandle
{
        GObject parent;
        
	gboolean fallback;
	guint server_num;

	GError *error;
	
        IRCHandlePrivate *priv;
};

struct _IRCHandleClass
{
        GObjectClass parent_class;
};

GType irc_handle_get_type(void) G_GNUC_CONST;

IRCHandle* irc_handle_new(Account *account, guint server_num, gboolean fallback);
void irc_handle_push_message(IRCHandle *handle, IRCMessage *msg);

gchar *irc_handle_get_current_nick(IRCHandle *handle);

G_END_DECLS

#endif /* __IRC_HANDLE_H__ */
