/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
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

#include "account.h"
#include "loqui_channel.h"

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

        gint max_channel_entry_id;

        AccountManagerPrivate *priv;
};

struct _AccountManagerClass
{
        GObjectClass parent_class;

	void (* add_account) (AccountManager *manager, Account *account);
	void (* remove_account) (AccountManager *manager, Account *account);
};


GType account_manager_get_type(void) G_GNUC_CONST;

AccountManager* account_manager_new(void);
AccountManager *account_manager_get(void);

void account_manager_load_accounts(AccountManager *account_manager);
void account_manager_save_accounts(AccountManager *account_manager);

void account_manager_add_account(AccountManager *manager, Account *account);
void account_manager_remove_account(AccountManager *manager, Account *account);
void account_manager_remove_all_account(AccountManager *manager);

void account_manager_connect_all_default(AccountManager *manager);
void account_manager_disconnect_all(AccountManager *manager);

GList *account_manager_get_account_list(AccountManager *manager);

void account_manager_update_positions(AccountManager *manager);

gint account_manager_new_channel_entry_id(AccountManager *manager);

LoquiChannelEntry * account_manager_get_next_channel_entry(AccountManager *manager,
							   LoquiChannelEntry *chent,
							   gboolean require_updated);

LoquiChannelEntry * account_manager_get_previous_channel_entry(AccountManager *manager,
							       LoquiChannelEntry *chent,
							       gboolean require_updated);
G_END_DECLS

#endif /* __ACCOUNT_MANAGER_H__ */
