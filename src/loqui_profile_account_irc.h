/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_PROFILE_ACCOUNT_IRC_H__
#define __LOQUI_PROFILE_ACCOUNT_IRC_H__

#include <glib-object.h>
#include "loqui_profile_account.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_PROFILE_ACCOUNT_IRC                 (loqui_profile_account_irc_get_type ())
#define LOQUI_PROFILE_ACCOUNT_IRC(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROFILE_ACCOUNT_IRC, LoquiProfileAccountIRC))
#define LOQUI_PROFILE_ACCOUNT_IRC_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROFILE_ACCOUNT_IRC, LoquiProfileAccountIRCClass))
#define LOQUI_IS_PROFILE_ACCOUNT_IRC(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROFILE_ACCOUNT_IRC))
#define LOQUI_IS_PROFILE_ACCOUNT_IRC_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROFILE_ACCOUNT_IRC))
#define LOQUI_PROFILE_ACCOUNT_IRC_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROFILE_ACCOUNT_IRC, LoquiProfileAccountIRCClass))

typedef struct _LoquiProfileAccountIRC            LoquiProfileAccountIRC;
typedef struct _LoquiProfileAccountIRCClass       LoquiProfileAccountIRCClass;

typedef struct _LoquiProfileAccountIRCPrivate     LoquiProfileAccountIRCPrivate;

struct _LoquiProfileAccountIRC
{
        LoquiProfileAccount parent;
        
	gchar *realname;
	gchar *userinfo;
	gchar *autojoin;
	
	gint codeset_type;
	gchar *codeset;
	
	GList *nick_list; /* list of gchar * */
	
        LoquiProfileAccountIRCPrivate *priv;
};

struct _LoquiProfileAccountIRCClass
{
        LoquiProfileAccountClass parent_class;
};


GType loqui_profile_account_irc_get_type(void) G_GNUC_CONST;

#define LOQUI_PROFILE_ACCOUNT_IRC_ACCESSOR_STRING(attr_name) \
  ATTR_ACCESSOR_POINTER(g_strdup, g_free, const gchar *, G_CONST_RETURN gchar *, LoquiProfileAccountIRC, loqui_profile_account_irc, attr_name)
#define LOQUI_PROFILE_ACCOUNT_IRC_ACCESSOR_STRING_PROTOTYPE(attr_name) \
  ATTR_ACCESSOR_POINTER_PROTOTYPE(const gchar *, G_CONST_RETURN gchar *, LoquiProfileAccountIRC, loqui_profile_account_irc, attr_name)

LOQUI_PROFILE_ACCOUNT_IRC_ACCESSOR_STRING_PROTOTYPE(realname);
LOQUI_PROFILE_ACCOUNT_IRC_ACCESSOR_STRING_PROTOTYPE(userinfo);
LOQUI_PROFILE_ACCOUNT_IRC_ACCESSOR_STRING_PROTOTYPE(autojoin);

ATTR_ACCESSOR_GENERIC_PROTOTYPE(int, LoquiProfileAccountIRC, loqui_profile_account_irc, codeset_type);
LOQUI_PROFILE_ACCOUNT_IRC_ACCESSOR_STRING_PROTOTYPE(codeset);

LoquiProfileAccountIRC* loqui_profile_account_irc_new(void);

G_END_DECLS

#endif /* __LOQUI_PROFILE_ACCOUNT_IRC_H__ */
