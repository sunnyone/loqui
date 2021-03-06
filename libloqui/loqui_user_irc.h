/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
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
#ifndef __LOQUI_USER_IRC_H__
#define __LOQUI_USER_IRC_H__

#include <glib-object.h>
#include "loqui_user.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_USER_IRC                 (loqui_user_irc_get_type ())
#define LOQUI_USER_IRC(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_USER_IRC, LoquiUserIRC))
#define LOQUI_USER_IRC_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_USER_IRC, LoquiUserIRCClass))
#define LOQUI_IS_USER_IRC(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_USER_IRC))
#define LOQUI_IS_USER_IRC_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_USER_IRC))
#define LOQUI_USER_IRC_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_USER_IRC, LoquiUserIRCClass))

typedef struct _LoquiUserIRC            LoquiUserIRC;
typedef struct _LoquiUserIRCClass       LoquiUserIRCClass;

typedef struct _LoquiUserIRCPrivate     LoquiUserIRCPrivate;

struct _LoquiUserIRC
{
        LoquiUser parent;
        
	gchar *server_info;
	gchar *joined_channels_string;
	gboolean is_irc_operator;
	guint hop_count;
};

struct _LoquiUserIRCClass
{
        LoquiUserClass parent_class;
};


GType loqui_user_irc_get_type(void) G_GNUC_CONST;

LoquiUserIRC* loqui_user_irc_new(void);

LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiUserIRC, loqui_user_irc, hop_count, guint);
LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiUserIRC, loqui_user_irc, is_irc_operator, gboolean);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiUserIRC, loqui_user_irc, server_info);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiUserIRC, loqui_user_irc, joined_channels_string);

G_END_DECLS

#endif /* __LOQUI_USER_IRC_H__ */
