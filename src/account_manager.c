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
#include "loqui_app.h"
#include "prefs_account.h"

struct _AccountManagerPrivate
{
	GSList *account_list;
	Account *current_account;
	Channel *current_channel;

	ChannelBuffer *common_buffer;
	LoquiApp *app;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void account_manager_class_init(AccountManagerClass *klass);
static void account_manager_init(AccountManager *account_manager);
static void account_manager_finalize(GObject *object);
static Account* account_manager_search_account(AccountManager *manager, Channel *channel);
static gboolean account_manager_whether_scroll(AccountManager *account_manager);

static AccountManager *main_account_manager = NULL;

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

	if(priv->common_buffer) {
		g_object_unref(priv->common_buffer);
		priv->common_buffer = NULL;
	}

	g_free(priv);
}

AccountManager*
account_manager_new (void)
{
        AccountManager *account_manager;
	AccountManagerPrivate *priv;

	account_manager = g_object_new(account_manager_get_type(), NULL);

	priv = account_manager->priv;
	priv->app = LOQUI_APP(loqui_app_new());
	priv->common_buffer = channel_buffer_new();
	loqui_app_set_common_buffer(priv->app, GTK_TEXT_BUFFER(priv->common_buffer));
	gtk_widget_show_all(GTK_WIDGET(priv->app));

	return account_manager;
}
void
account_manager_load_accounts(AccountManager *account_manager)
{
        GSList *cur;
	AccountManagerPrivate *priv;

        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(account_manager));

        priv = account_manager->priv;

	priv->account_list = prefs_account_load();
	for(cur = priv->account_list; cur != NULL; cur = cur->next) {
		channel_tree_add_account(CHANNEL_TREE(priv->app->channel_tree), ACCOUNT(cur->data));
	}

	loqui_menu_create_connect_submenu(priv->app->menu, priv->account_list);
}
void
account_manager_save_accounts(AccountManager *account_manager)
{
        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(account_manager));

	prefs_account_save(account_manager->priv->account_list);
}
void
account_manager_add_channel(AccountManager *manager, Account *account, Channel *channel)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	channel_tree_add_channel(manager->priv->app->channel_tree, account, channel);
}
void
account_manager_remove_channel(AccountManager *manager, Account *account, Channel *channel)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	channel_tree_remove_channel(manager->priv->app->channel_tree, channel);
}
void
account_manager_set_fresh(AccountManager *manager, Account *account, Channel *channel)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;

	channel_tree_set_fresh(priv->app->channel_tree, account, channel);
}
/* called from select_cb */
void
account_manager_set_current(AccountManager *manager, Account *account, Channel *channel)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;

	priv->current_channel = channel;
	priv->current_account = account;

	if(channel) {
		loqui_app_set_channel_buffer(priv->app, GTK_TEXT_BUFFER(channel->buffer));
		nick_list_set_store(priv->app->nick_list, channel->user_list);
		account_manager_set_topic(manager, channel_get_topic(channel));
		channel_set_fresh(channel, FALSE);
	} else if(account) {
		loqui_app_set_channel_buffer(priv->app, GTK_TEXT_BUFFER(account->console_buffer));
		nick_list_set_store(priv->app->nick_list, NULL);
		account_manager_set_topic(manager, "");
	}
	loqui_app_set_focus(priv->app);
}
/* mainly called */
void
account_manager_select_channel(AccountManager *manager, Channel *channel)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
        g_return_if_fail(channel != NULL);
        g_return_if_fail(IS_CHANNEL(channel));
		
	channel_tree_select_channel(manager->priv->app->channel_tree, channel);
}
void
account_manager_select_account(AccountManager *manager, Account *account)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
        g_return_if_fail(account != NULL);
        g_return_if_fail(IS_ACCOUNT(account));
	
	channel_tree_select_account(manager->priv->app->channel_tree, account);
}
static Account*
account_manager_search_account(AccountManager *manager, Channel *channel)
{
	AccountManagerPrivate *priv;
	Account *account;
	GSList *cur;

        g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), NULL);

	priv = manager->priv;

	for(cur = priv->account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);
		if(account_has_channel(account, channel))
			return account;
	}
	return NULL;
}
static gboolean
account_manager_whether_scroll(AccountManager *account_manager)
{
	return loqui_app_is_scroll(account_manager->priv->app);
}
void account_manager_speak(AccountManager *manager, const gchar *str)
{
	AccountManagerPrivate *priv;
	Account *account;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(str != NULL);

	priv = manager->priv;
	g_return_if_fail(priv->current_account != NULL || priv->current_channel != NULL);
	
	if(priv->current_channel)
		account = account_manager_search_account(manager, priv->current_channel);
	else
		account = priv->current_account;

	if(account == NULL)
		return;

	account_speak(account, priv->current_channel, str);
}
AccountManager *
account_manager_get(void)
{
	if(!main_account_manager)
		main_account_manager = account_manager_new();
	return main_account_manager;
}

gboolean
account_manager_is_current_account(AccountManager *manager, Account *account)
{
        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	return (manager->priv->current_account == account);
}
gboolean
account_manager_is_current_channel(AccountManager *manager, Channel *channel)
{
        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	return (manager->priv->current_channel == channel);
}
void
account_manager_scroll_channel_textview(AccountManager *manager)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));	

	if(account_manager_whether_scroll(manager)) {
		loqui_app_scroll_channel_textview(manager->priv->app);
	}
}
void
account_manager_common_buffer_append(AccountManager *manager, TextType type, gchar *str)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	channel_buffer_append(manager->priv->common_buffer, type, str);
	if(account_manager_whether_scroll(manager)) {
		loqui_app_scroll_common_textview(manager->priv->app);
	}
}
void account_manager_set_topic(AccountManager *manager, const gchar *topic)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	loqui_app_set_topic(manager->priv->app, topic);
}
void account_manager_disconnect_all(AccountManager *manager)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	g_slist_foreach(manager->priv->account_list, (GFunc) account_disconnect, NULL);
}
void account_manager_remove_channels_of_account(AccountManager *manager, Account *account)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	channel_tree_remove_channels_of_account(manager->priv->app->channel_tree, 
						account);
}
