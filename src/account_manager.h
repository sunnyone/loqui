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
#include "channel.h"

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

AccountManager* account_manager_new(void);
AccountManager *account_manager_get(void);

void account_manager_load_accounts(AccountManager *account_manager);
void account_manager_save_accounts(AccountManager *account_manager);

void account_manager_add_account(AccountManager *manager, Account *account);
void account_manager_update_account(AccountManager *manager, Account *account);
void account_manager_remove_account(AccountManager *manager, Account *account);

void account_manager_add_channel(AccountManager *manager, Account *account, Channel *channel);
void account_manager_remove_channel(AccountManager *manager, Account *account, Channel *channel);

void account_manager_set_current_channel(AccountManager *manager, Channel *channel);
Channel *account_manager_get_current_channel(AccountManager *manager);

void account_manager_set_current_account(AccountManager *manager, Account *account);
Account *account_manager_get_current_account(AccountManager *manager);

void account_manager_set_updated(AccountManager *manager, Account *account, Channel *channel);
void account_manager_update_channel_user_number(AccountManager *manager, Channel *channel);

gboolean account_manager_is_current_account(AccountManager *manager, Account *account);
gboolean account_manager_is_current_channel(AccountManager *manager, Channel *channel);
gboolean account_manager_is_current_channel_buffer(AccountManager *manager, ChannelBuffer *buffer);

void account_manager_common_buffer_append(AccountManager *manager, TextType type, gchar *str);
void account_manager_common_buffer_append_remark(AccountManager *manager, TextType type,
						 gboolean is_self, gboolean is_priv,
						 const gchar *channel_name, const gchar *nick, const gchar *remark);
void account_manager_update_current_info(AccountManager *manager);

void account_manager_remove_channels_of_account(AccountManager *manager, Account *account);

void account_manager_disconnect_all(AccountManager *manager);

void account_manager_open_account_list_dialog(AccountManager *manager);
void account_manager_open_prefs_dialog(AccountManager *manager);

GSList *account_manager_get_account_list(AccountManager *manager);

void account_manager_set_whether_scrolling(AccountManager *manager, gboolean is_scroll);
gboolean account_manager_get_whether_scrolling(AccountManager *manager);

void account_manager_update_away_status(AccountManager *manager, gboolean is_away);

void account_manager_open_connect_dialog(AccountManager *manager);

G_END_DECLS

#endif /* __ACCOUNT_MANAGER_H__ */
