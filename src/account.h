/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2
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
#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include <gnome.h>
#include "channel_text.h"

G_BEGIN_DECLS

#define TYPE_ACCOUNT                 (account_get_type ())
#define ACCOUNT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_ACCOUNT, Account))
#define ACCOUNT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_ACCOUNT, AccountClass))
#define IS_ACCOUNT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_ACCOUNT))
#define IS_ACCOUNT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_ACCOUNT))
#define ACCOUNT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ACCOUNT, AccountClass))

typedef struct _Account            Account;
typedef struct _AccountClass       AccountClass;

typedef struct _AccountPrivate     AccountPrivate;
typedef struct _Server Server;

struct _Account
{
        GObject parent;

	gchar *name;

	GSList *server_list; /* list of Server */
	GSList *use_server_list; /* list of integer */

	gchar *nick;
	gchar *username;
	gchar *realname;
	gchar *userinfo;
	gchar *autojoin;

	ChannelText *console_text;

        AccountPrivate *priv;
};

struct _AccountClass
{
        GObjectClass parent_class;
};
struct _Server
{
	gchar *hostname;
	guint port;
	gchar *password;
	gboolean use;
};

GType account_get_type(void) G_GNUC_CONST;

Account* account_new(void);
void account_set(Account *account,
		 const gchar *name,
		 const gchar *nick,
		 const gchar *username,
		 const gchar *realname,
		 const gchar *userinfo,
		 const gchar *autojoin);
void account_add_server(Account *account, const gchar *hostname,
			gint port, const gchar *password,
			gboolean use);
void account_restore(Account *account, const gchar *name);
void account_connect(Account *account, gint server_num, gboolean fallback);

G_END_DECLS

#endif /* __ACCOUNT_H__ */
