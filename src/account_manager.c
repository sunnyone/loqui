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
#include "config.h"

#include "account_manager.h"
#include "account.h"
#include "utils.h"
#include "intl.h"
#include "main.h"
#include "loqui_profile_handle.h"
#include "loqui_profile_account_irc.h"
#include "loqui_app_actions.h"

struct _AccountManagerPrivate
{
	GSList *account_list;
};

enum {
	SIGNAL_ADD_ACCOUNT,
	SIGNAL_REMOVE_ACCOUNT,
	LAST_SIGNAL
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT
#define ACCOUNT_CONFIG_FILENAME "account.xml"

static guint account_manager_signals[LAST_SIGNAL] = { 0 };

static void account_manager_class_init(AccountManagerClass *klass);
static void account_manager_init(AccountManager *account_manager);
static void account_manager_finalize(GObject *object);

GType
account_manager_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(AccountManagerClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) account_manager_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(AccountManager),
				0,              /* n_preallocs */
				(GInstanceInitFunc) account_manager_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "AccountManager",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
account_manager_class_init (AccountManagerClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = account_manager_finalize;

	account_manager_signals[SIGNAL_ADD_ACCOUNT] = g_signal_new("add-account",
								   G_OBJECT_CLASS_TYPE(object_class),
								   G_SIGNAL_RUN_LAST,
								   G_STRUCT_OFFSET(AccountManagerClass, add_account),
								   NULL, NULL,
								   g_cclosure_marshal_VOID__OBJECT,
								   G_TYPE_NONE, 1,
								   TYPE_ACCOUNT);
	account_manager_signals[SIGNAL_REMOVE_ACCOUNT] = g_signal_new("remove-account",
								      G_OBJECT_CLASS_TYPE(object_class),
								      G_SIGNAL_RUN_LAST,
								      G_STRUCT_OFFSET(AccountManagerClass, remove_account),
								      NULL, NULL,
								      g_cclosure_marshal_VOID__OBJECT,
								      G_TYPE_NONE, 1,
								      TYPE_ACCOUNT);
}
static void 
account_manager_init (AccountManager *account_manager)
{
	AccountManagerPrivate *priv;

	priv = g_new0(AccountManagerPrivate, 1);

	account_manager->priv = priv;
}
static void 
account_manager_finalize (GObject *object)
{
	AccountManager *account_manager;
	AccountManagerPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(object));

        account_manager = ACCOUNT_MANAGER(object);
	priv = account_manager->priv;

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(priv);
}

AccountManager*
account_manager_new (void)
{
        AccountManager *account_manager;
	AccountManagerPrivate *priv;

	account_manager = g_object_new(account_manager_get_type(), NULL);

	priv = account_manager->priv;
	return account_manager;
}

void
account_manager_add_account(AccountManager *manager, Account *account)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	priv = manager->priv;

	priv->account_list = g_slist_append(priv->account_list, account);
	g_signal_emit(G_OBJECT(manager), account_manager_signals[SIGNAL_ADD_ACCOUNT], 0, account);
}
void
account_manager_remove_account(AccountManager *manager, Account *account)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	priv = manager->priv;

	priv->account_list = g_slist_remove(priv->account_list, account);
	g_signal_emit(G_OBJECT(manager), account_manager_signals[SIGNAL_REMOVE_ACCOUNT], 0, account);
}
void
account_manager_load_accounts(AccountManager *account_manager)
{
        GList *cur, *list = NULL;
	AccountManagerPrivate *priv;
	gchar *path;
	LoquiProfileHandle *handle;
	Account *account;

        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(account_manager));

        priv = account_manager->priv;

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, ACCOUNT_CONFIG_FILENAME, NULL);
	handle = loqui_profile_handle_new();
	loqui_profile_handle_register_type(handle, "IRC", LOQUI_TYPE_PROFILE_ACCOUNT_IRC);
	loqui_profile_handle_read_from_file(handle, &list, path);

	for(cur = list; cur != NULL; cur = cur->next) {
		account = account_new();
		account_set_profile(account, cur->data);
		account_manager_add_account(account_manager, account);
	}
	g_list_free(list);
}

void
account_manager_save_accounts(AccountManager *account_manager)
{
        GSList *cur;
	GList *list = NULL;
	gchar *path;
	LoquiProfileHandle *handle;

        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(account_manager));

	for(cur = account_manager->priv->account_list; cur != NULL; cur = cur->next) {
		list = g_list_append(list, account_get_profile(cur->data));
	}

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, ACCOUNT_CONFIG_FILENAME, NULL);
	handle = loqui_profile_handle_new();
	loqui_profile_handle_register_type(handle, "IRC", LOQUI_TYPE_PROFILE_ACCOUNT_IRC);
	loqui_profile_handle_write_to_file(handle, list, path);
	g_list_free(list);
}
void account_manager_disconnect_all(AccountManager *manager)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	g_slist_foreach(manager->priv->account_list, (GFunc) account_disconnect, NULL);
}
GSList *account_manager_get_account_list(AccountManager *manager)
{
	g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), NULL);

	return manager->priv->account_list;
}
void
account_manager_connect_all_default(AccountManager *manager)
{
	GSList *cur;
	Account *account;
	AccountManagerPrivate *priv;

	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;

	for (cur = priv->account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);
		if (account_is_connected(account))
			continue;
		if (!loqui_profile_account_get_use(account_get_profile(account)))
			continue;
		
		account_connect(account);
	}
}
