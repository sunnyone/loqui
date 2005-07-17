/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_USER_IPMSG_H__
#define __LOQUI_USER_IPMSG_H__

#include <glib-object.h>
#include "loqui_user.h"
#include "gobject_utils.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_USER_IPMSG                 (loqui_user_ipmsg_get_type ())
#define LOQUI_USER_IPMSG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_USER_IPMSG, LoquiUserIPMsg))
#define LOQUI_USER_IPMSG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_USER_IPMSG, LoquiUserIPMsgClass))
#define LOQUI_IS_USER_IPMSG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_USER_IPMSG))
#define LOQUI_IS_USER_IPMSG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_USER_IPMSG))
#define LOQUI_USER_IPMSG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_USER_IPMSG, LoquiUserIPMsgClass))

typedef struct _LoquiUserIPMsg            LoquiUserIPMsg;
typedef struct _LoquiUserIPMsgClass       LoquiUserIPMsgClass;

typedef struct _LoquiUserIPMsgPrivate     LoquiUserIPMsgPrivate;

struct _LoquiUserIPMsg
{
        LoquiUser parent;
        
	gchar *ip_addr;
	gint port;

	gchar *group_name;

        LoquiUserIPMsgPrivate *priv;
};

struct _LoquiUserIPMsgClass
{
        LoquiUserClass parent_class;
};


GType loqui_user_ipmsg_get_type(void) G_GNUC_CONST;

LoquiUserIPMsg* loqui_user_ipmsg_new(void);

#define LOQUI_USER_IPMSG_ACCESSOR_STRING(attr_name) \
  LOQUI_DEFINE_ACCESSOR_POINTER(g_strdup, g_free, const gchar *, G_CONST_RETURN gchar *, LoquiUserIPMsg, loqui_user_ipmsg, attr_name)
#define LOQUI_USER_IPMSG_ACCESSOR_STRING_PROTOTYPE(attr_name) \
  ATTR_ACCESSOR_POINTER_PROTOTYPE(const gchar *, G_CONST_RETURN gchar *, LoquiUserIPMsg, loqui_user_ipmsg, attr_name)

#define LOQUI_USER_IPMSG_ACCESSOR_GENERIC(type, attr_name) \
  LOQUI_DEFINE_ACCESSOR_GENERIC(type, 0, LoquiUserIPMsg, loqui_user_ipmsg, attr_name)
#define LOQUI_USER_IPMSG_ACCESSOR_GENERIC_PROTOTYPE(type, attr_name) \
  LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(type, LoquiUserIPMsg, loqui_user_ipmsg, attr_name)

LOQUI_USER_IPMSG_ACCESSOR_GENERIC_PROTOTYPE(gint, port);
LOQUI_USER_IPMSG_ACCESSOR_STRING_PROTOTYPE(ip_addr);
LOQUI_USER_IPMSG_ACCESSOR_STRING_PROTOTYPE(group_name);

G_END_DECLS

#endif /* __LOQUI_USER_IPMSG_H__ */
