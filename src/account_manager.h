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
#ifndef __ACCOUNT_MANAGER_H__
#define __ACCOUNT_MANAGER_H__

#include <gnome.h>
#include "loqui_app.h"

G_BEGIN_DECLS

#define TYPE_ACCOUNT_MANAGER                 (account_manager_get_type ())
#define ACCOUNT_MANAGER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_ACCOUNT_MANAGER, AccountManager))
#define ACCOUNT_MANAGER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_ACCOUNT_MANAGER, AccountManagerClass))
#define IS_ACCOUNT_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_ACCOUNT_MANAGER))
#define IS_ACCOUNT_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_ACCOUNT_MANAGER))
#define ACCOUNT_MANAGER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ACCOUNT_MANAGER, AccountManagerClass))

typedef struct _AccountManager            AccountManager;
typedef struct _AccountManagerClass       AccountManagerClass;

typedef struct _AccountManagerPrivate     AccountManagerPrivate;

struct _AccountManager
{
        GObject parent;
        
        AccountManagerPrivate *priv;
};

struct _AccountManagerClass
{
        GObjectClass parent_class;
};


GType account_manager_get_type(void) G_GNUC_CONST;

AccountManager* account_manager_new(LoquiApp *app);

void account_manager_load_accounts(AccountManager *account_manager);

G_END_DECLS

#endif /* __ACCOUNT_MANAGER_H__ */
