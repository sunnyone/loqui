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
#ifndef __LOQUI_ACCOUNT_MANAGER_STORE_H__
#define __LOQUI_ACCOUNT_MANAGER_STORE_H__

#include <gtk/gtk.h>
#include "account_manager.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_ACCOUNT_MANAGER_STORE                 (loqui_account_manager_store_get_type ())
#define LOQUI_ACCOUNT_MANAGER_STORE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_ACCOUNT_MANAGER_STORE, LoquiAccountManagerStore))
#define LOQUI_ACCOUNT_MANAGER_STORE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_ACCOUNT_MANAGER_STORE, LoquiAccountManagerStoreClass))
#define LOQUI_IS_ACCOUNT_MANAGER_STORE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_ACCOUNT_MANAGER_STORE))
#define LOQUI_IS_ACCOUNT_MANAGER_STORE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_ACCOUNT_MANAGER_STORE))
#define LOQUI_ACCOUNT_MANAGER_STORE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_ACCOUNT_MANAGER_STORE, LoquiAccountManagerStoreClass))

typedef struct _LoquiAccountManagerStore            LoquiAccountManagerStore;
typedef struct _LoquiAccountManagerStoreClass       LoquiAccountManagerStoreClass;

typedef struct _LoquiAccountManagerStorePrivate     LoquiAccountManagerStorePrivate;

enum {
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_CHANNEL_ENTRY,
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_POSITION,
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_NAME,
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_USERS,
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_OP_USERS,
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_COLOR,
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY,
	LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY_STOCK_ID,
	LOQUI_ACCOUNT_MANAGER_STORE_N_COLUMNS,
};

struct _LoquiAccountManagerStore
{
        GObject parent;
        
	AccountManager *manager;

	GType column_types[LOQUI_ACCOUNT_MANAGER_STORE_N_COLUMNS];

	gint stamp;

        LoquiAccountManagerStorePrivate *priv;
};

struct _LoquiAccountManagerStoreClass
{
        GObjectClass parent_class;
};


GType loqui_account_manager_store_get_type(void) G_GNUC_CONST;

LoquiAccountManagerStore* loqui_account_manager_store_new(AccountManager *manager);

void loqui_account_manager_store_get_iter_by_channel_entry(LoquiAccountManagerStore* store,
							   GtkTreeIter *iter,
							   LoquiChannelEntry *chent);

G_END_DECLS

#endif /* __LOQUI_ACCOUNT_MANAGER_STORE_H__ */
