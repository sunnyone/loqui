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
#ifndef __LOQUI_MEMBER_H__
#define __LOQUI_MEMBER_H__

#include <glib-object.h>
#include <time.h>

#include "loqui_user.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_MEMBER                 (loqui_member_get_type ())
#define LOQUI_MEMBER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_MEMBER, LoquiMember))
#define LOQUI_MEMBER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_MEMBER, LoquiMemberClass))
#define LOQUI_IS_MEMBER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_MEMBER))
#define LOQUI_IS_MEMBER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_MEMBER))
#define LOQUI_MEMBER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_MEMBER, LoquiMemberClass))

typedef struct _LoquiMember            LoquiMember;
typedef struct _LoquiMemberClass       LoquiMemberClass;

struct _LoquiMember
{
        GObject parent;
        
	LoquiUser *user;

	gboolean is_channel_operator;
	gboolean speakable;

	glong last_message_time;
};

struct _LoquiMemberClass
{
        GObjectClass parent_class;
};


GType loqui_member_get_type(void) G_GNUC_CONST;

LoquiMember* loqui_member_new(LoquiUser *user);

void loqui_member_set_user(LoquiMember *member, LoquiUser *user);
LoquiUser* loqui_member_get_user(LoquiMember *member);

void loqui_member_set_is_channel_operator(LoquiMember *member, gboolean is_channel_operator);
gboolean loqui_member_get_is_channel_operator(LoquiMember *member);

void loqui_member_set_speakable(LoquiMember *member, gboolean speakable);
gboolean loqui_member_get_speakable(LoquiMember *member);

void loqui_member_set_last_message_time(LoquiMember *member, time_t last_message_time);
time_t loqui_member_get_last_message_time(LoquiMember *member);

G_END_DECLS

#endif /* __LOQUI_MEMBER_H__ */
