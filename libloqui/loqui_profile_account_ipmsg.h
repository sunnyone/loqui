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
#ifndef __LOQUI_PROFILE_ACCOUNT_IPMSG_H__
#define __LOQUI_PROFILE_ACCOUNT_IPMSG_H__

#include <glib-object.h>
#include "loqui_profile_account.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_PROFILE_ACCOUNT_IPMSG                 (loqui_profile_account_ipmsg_get_type ())
#define LOQUI_PROFILE_ACCOUNT_IPMSG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROFILE_ACCOUNT_IPMSG, LoquiProfileAccountIPMsg))
#define LOQUI_PROFILE_ACCOUNT_IPMSG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROFILE_ACCOUNT_IPMSG, LoquiProfileAccountIPMsgClass))
#define LOQUI_IS_PROFILE_ACCOUNT_IPMSG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROFILE_ACCOUNT_IPMSG))
#define LOQUI_IS_PROFILE_ACCOUNT_IPMSG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROFILE_ACCOUNT_IPMSG))
#define LOQUI_PROFILE_ACCOUNT_IPMSG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROFILE_ACCOUNT_IPMSG, LoquiProfileAccountIPMsgClass))

typedef struct _LoquiProfileAccountIPMsg            LoquiProfileAccountIPMsg;
typedef struct _LoquiProfileAccountIPMsgClass       LoquiProfileAccountIPMsgClass;

typedef struct _LoquiProfileAccountIPMsgPrivate     LoquiProfileAccountIPMsgPrivate;

struct _LoquiProfileAccountIPMsg
{
        LoquiProfileAccount parent;
        
        LoquiProfileAccountIPMsgPrivate *priv;
};

struct _LoquiProfileAccountIPMsgClass
{
        LoquiProfileAccountClass parent_class;
};


GType loqui_profile_account_ipmsg_get_type(void) G_GNUC_CONST;

LoquiProfileAccountIPMsg* loqui_profile_account_ipmsg_new(void);
G_END_DECLS

#endif /* __LOQUI_PROFILE_ACCOUNT_IPMSG_H__ */
