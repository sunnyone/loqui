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
#ifndef __LOQUI_USER_H__
#define __LOQUI_USER_H__

#include <glib-object.h>
#include "gobject_utils.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_USER                 (loqui_user_get_type ())
#define LOQUI_USER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_USER, LoquiUser))
#define LOQUI_USER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_USER, LoquiUserClass))
#define LOQUI_IS_USER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_USER))
#define LOQUI_IS_USER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_USER))
#define LOQUI_USER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_USER, LoquiUserClass))

typedef struct _LoquiUser            LoquiUser;
typedef struct _LoquiUserClass       LoquiUserClass;

typedef struct _LoquiUserPrivate     LoquiUserPrivate;

typedef enum {
	LOQUI_BASIC_AWAY_TYPE_UNKNOWN = 0,
	LOQUI_BASIC_AWAY_TYPE_ONLINE,
	LOQUI_BASIC_AWAY_TYPE_OFFLINE,
	LOQUI_BASIC_AWAY_TYPE_AWAY,
	LOQUI_BASIC_AWAY_TYPE_BUSY,
} LoquiBasicAwayType;

typedef enum {
	LOQUI_AWAY_TYPE_UNKNOWN = 0,
	LOQUI_AWAY_TYPE_ONLINE,
	LOQUI_AWAY_TYPE_OFFLINE,
	LOQUI_AWAY_TYPE_AWAY
} LoquiAwayType;
/* AwayType is registerble, 
   for example, IRC has away only, but IM protocol has many away type */

typedef struct _LoquiAwayInfo {
	LoquiAwayType away_type;
	LoquiBasicAwayType basic_away_type;
	gchar *name;
	gchar *nick;
} LoquiAwayInfo;

struct _LoquiUser
{
        GObject parent;
        
	gchar *nick;

	/* mailaddress == username@hostname */
	gchar *username;
	gchar *hostname;

	gchar *realname;

	gchar *servername;

	LoquiAwayType away;
	gchar *away_message;
	
	gint idle_time; /* time_t ? */
	
	gboolean is_ignored;

        LoquiUserPrivate *priv;
};

struct _LoquiUserClass
{
        GObjectClass parent_class;

	GPtrArray *away_type_array;
};

#define LOQUI_USER_ACCESSOR_STRING(attr_name) \
  ATTR_ACCESSOR_POINTER(g_strdup, g_free, const gchar *, G_CONST_RETURN gchar *, LoquiUser, loqui_user, attr_name)
#define LOQUI_USER_ACCESSOR_STRING_PROTOTYPE(attr_name) \
  ATTR_ACCESSOR_POINTER_PROTOTYPE(const gchar *, G_CONST_RETURN gchar *, LoquiUser, loqui_user, attr_name)

#define LOQUI_USER_ACCESSOR_GENERIC(type, attr_name) \
  ATTR_ACCESSOR_GENERIC(type, 0, LoquiUser, loqui_user, attr_name)
#define LOQUI_USER_ACCESSOR_GENERIC_PROTOTYPE(type, attr_name) \
  ATTR_ACCESSOR_GENERIC_PROTOTYPE(type, LoquiUser, loqui_user, attr_name)

GType loqui_user_get_type(void) G_GNUC_CONST;

LoquiUser* loqui_user_new(void);

LoquiAwayType loqui_user_class_install_away_type(LoquiUserClass *user_class,
						 LoquiBasicAwayType basic_away_type,
						 const gchar *name,
						 const gchar *nick);

LoquiAwayInfo* loqui_user_class_away_type_get_info(LoquiUserClass *user_class, LoquiAwayType away_type);

LOQUI_USER_ACCESSOR_GENERIC_PROTOTYPE(gint, idle_time);
LOQUI_USER_ACCESSOR_GENERIC_PROTOTYPE(gboolean, is_ignored);
LOQUI_USER_ACCESSOR_GENERIC_PROTOTYPE(LoquiAwayType, away);
LOQUI_USER_ACCESSOR_STRING_PROTOTYPE(nick);
LOQUI_USER_ACCESSOR_STRING_PROTOTYPE(username);
LOQUI_USER_ACCESSOR_STRING_PROTOTYPE(hostname);
LOQUI_USER_ACCESSOR_STRING_PROTOTYPE(realname);
LOQUI_USER_ACCESSOR_STRING_PROTOTYPE(servername);
LOQUI_USER_ACCESSOR_STRING_PROTOTYPE(away_message);

LoquiBasicAwayType loqui_user_get_basic_away(LoquiUser *user);

G_END_DECLS

#endif /* __LOQUI_USER_H__ */
