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
#ifndef __LOQUI_PROFILE_ACCOUNT_H__
#define __LOQUI_PROFILE_ACCOUNT_H__

#include <glib-object.h>
#include <libloqui/loqui-gobject-utils.h>
#include <libloqui/loqui-property-profile.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_PROFILE_ACCOUNT                 (loqui_profile_account_get_type ())
#define LOQUI_PROFILE_ACCOUNT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROFILE_ACCOUNT, LoquiProfileAccount))
#define LOQUI_PROFILE_ACCOUNT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROFILE_ACCOUNT, LoquiProfileAccountClass))
#define LOQUI_IS_PROFILE_ACCOUNT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROFILE_ACCOUNT))
#define LOQUI_IS_PROFILE_ACCOUNT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROFILE_ACCOUNT))
#define LOQUI_PROFILE_ACCOUNT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROFILE_ACCOUNT, LoquiProfileAccountClass))

typedef struct _LoquiProfileAccount            LoquiProfileAccount;
typedef struct _LoquiProfileAccountClass       LoquiProfileAccountClass;

typedef struct _LoquiProfileAccountPrivate     LoquiProfileAccountPrivate;

#include <libloqui/loqui-protocol.h>

#include "loqui_codeconv.h"

struct _LoquiProfileAccount
{
        LoquiPropertyProfile parent;

        LoquiProtocol *protocol;

	gchar *name;
	gboolean use;
	
	gchar *servername;
	gint port;
	
	gchar *username;
	gchar *password;
	
	gchar *nick;

	LoquiCodeConvMode codeconv_mode;
	gchar *codeconv_item_name;
	gchar *codeset;

	GList *nick_list;
	
        LoquiProfileAccountPrivate *priv;
};

struct _LoquiProfileAccountClass
{
        LoquiPropertyProfileClass parent_class;
};


GType loqui_profile_account_get_type(void) G_GNUC_CONST;

LoquiProfileAccount* loqui_profile_account_new(LoquiProtocol *protocol);

LoquiProtocol* loqui_profile_account_get_protocol(LoquiProfileAccount *profile);

LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, use, gboolean);

LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, name);

LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, nick);

LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, servername);
LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, port, int);

LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, username);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, password);

LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, codeconv_mode, int);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, codeconv_item_name);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiProfileAccount, loqui_profile_account, codeset);

void loqui_profile_account_set_nick_list(LoquiProfileAccount *profile, GList *nick_list);
GList *loqui_profile_account_get_nick_list(LoquiProfileAccount *profile);

void loqui_profile_account_print(LoquiProfileAccount *profile);

G_END_DECLS

#endif /* __LOQUI_PROFILE_ACCOUNT_H__ */
