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
#include "prefs_general.h"
#include "account_list_dialog.h"
#include "account_dialog.h"
#include "intl.h"
#include "loqui_toolbar.h"
#include "loqui_channelbar.h"
#include "prefs_dialog.h"
#include "connect_dialog.h"

struct _AccountManagerPrivate
{
	GSList *account_list;
	Account *current_account;
	Channel *current_channel;

	MessageText *last_msgtext;
	ChannelBuffer *common_buffer;
	LoquiApp *app;

	gboolean is_pending_update_account_info;
	gboolean is_pending_update_channel_info;
	gboolean is_scroll;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void account_manager_class_init(AccountManagerClass *klass);
static void account_manager_init(AccountManager *account_manager);
static void account_manager_finalize(GObject *object);

static void account_manager_account_changed_cb(GObject *object, gpointer data);
static void account_manager_channel_changed_cb(GObject *object, gpointer data);
static void account_manager_channel_updated_cb(Channel *channel, gpointer data);

static void account_manager_add_channel_cb(Account *account, Channel *channel, AccountManager *manager);
static void account_manager_remove_channel_cb(Account *account, Channel *channel, AccountManager *manager);
static void account_manager_channel_buffer_append_cb(ChannelBuffer *buffer, MessageText *msgtext, AccountManager *manager);

static gboolean account_manager_update_account_info(AccountManager *manager);
static gboolean account_manager_update_channel_info(AccountManager *manager);

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
	loqui_app_set_common_buffer(priv->app, priv->common_buffer);

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
	g_signal_connect_swapped(G_OBJECT(account), "connected",
				 G_CALLBACK(account_manager_set_current_account), manager);
	g_signal_connect(G_OBJECT(account), "add-channel",
			 G_CALLBACK(account_manager_add_channel_cb), manager);
	g_signal_connect(G_OBJECT(account), "remove-channel",
			 G_CALLBACK(account_manager_remove_channel_cb), manager);
	g_signal_connect(G_OBJECT(account->console_buffer), "append",
			 G_CALLBACK(account_manager_channel_buffer_append_cb), manager);

	channel_tree_add_account(priv->app->channel_tree, account);
	loqui_menu_buffers_add_account(priv->app->menu, account);
	loqui_channelbar_add_account(LOQUI_CHANNELBAR(priv->app->channelbar), account);
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
	channel_tree_remove_account(priv->app->channel_tree, account);
	loqui_channelbar_remove_account(LOQUI_CHANNELBAR(priv->app->channelbar), account);

	/* FIXME: should disconnect signals? */
	g_object_unref(account);
}
void
account_manager_update_account(AccountManager *manager, Account *account)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	priv = manager->priv;
	
	channel_tree_update_account(priv->app->channel_tree, account);
	loqui_menu_buffers_update_account(priv->app->menu, account);
	loqui_channelbar_update_account(LOQUI_CHANNELBAR(priv->app->channelbar), account);
}
void
account_manager_load_accounts(AccountManager *account_manager)
{
        GSList *cur, *slist;
	AccountManagerPrivate *priv;

        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(account_manager));

        priv = account_manager->priv;

	slist = prefs_account_load();
	for(cur = slist; cur != NULL; cur = cur->next) {
		account_manager_add_account(account_manager, cur->data);
	}
	g_slist_free(slist);
}

void
account_manager_save_accounts(AccountManager *account_manager)
{
        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(account_manager));

	prefs_account_save(account_manager->priv->account_list);
}
static void
account_manager_add_channel_cb(Account *account, Channel *channel, AccountManager *manager)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	priv = manager->priv;

	g_signal_connect(G_OBJECT(channel), "updated",
			 G_CALLBACK(account_manager_channel_updated_cb), manager);
	g_signal_connect_swapped(G_OBJECT(channel), "user-number-changed",
				 G_CALLBACK(channel_tree_update_user_number), priv->app->channel_tree);
	g_signal_connect(G_OBJECT(channel->buffer), "append",
			 G_CALLBACK(account_manager_channel_buffer_append_cb), manager);

	channel_tree_add_channel(priv->app->channel_tree, account, channel);
	loqui_menu_buffers_add_channel(priv->app->menu, channel);
	loqui_channelbar_add_channel(LOQUI_CHANNELBAR(priv->app->channelbar), channel);

	account_manager_set_current_channel(manager, channel);
}
static void
account_manager_remove_channel_cb(Account *account, Channel *channel, AccountManager *manager)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	priv = manager->priv;

	account_manager_set_current_account(manager, account);
	g_signal_handlers_disconnect_by_func(channel, account_manager_channel_updated_cb, manager);
	g_signal_handlers_disconnect_by_func(channel, channel_tree_update_user_number, manager->priv->app->channel_tree);
	g_signal_handlers_disconnect_by_func(channel->buffer, account_manager_channel_buffer_append_cb, manager);

	channel_tree_remove_channel(manager->priv->app->channel_tree, channel);
	loqui_menu_buffers_remove_channel(priv->app->menu, channel);
	loqui_channelbar_remove_channel(LOQUI_CHANNELBAR(priv->app->channelbar), channel);
}
static gboolean
account_manager_update_account_info(AccountManager *manager)
{
	AccountManagerPrivate *priv;

        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);
	
	priv = manager->priv;

	loqui_app_update_info(priv->app, 
			      TRUE, account_manager_get_current_account(manager),
			      FALSE, account_manager_get_current_channel(manager));

	priv->is_pending_update_account_info = FALSE;
	return FALSE;
}
static gboolean
account_manager_update_channel_info(AccountManager *manager)
{
	AccountManagerPrivate *priv;

        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);
	
	priv = manager->priv;

	loqui_app_update_info(priv->app, 
			      FALSE, account_manager_get_current_account(manager),
			      TRUE, account_manager_get_current_channel(manager));

	priv->is_pending_update_channel_info = FALSE;
	return FALSE;
}
static void
account_manager_account_changed_cb(GObject *object, gpointer data)
{
	AccountManager *manager;
	AccountManagerPrivate *priv;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(data));
	
	manager = ACCOUNT_MANAGER(data);

	priv = manager->priv;
	
	if(!priv->is_pending_update_account_info) {
		priv->is_pending_update_account_info = TRUE;
		g_idle_add((GSourceFunc) account_manager_update_account_info, manager);
	}
}
static void account_manager_channel_changed_cb(GObject *object, gpointer data)
{
	AccountManager *manager;
	AccountManagerPrivate *priv;

        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(data));
	
	manager = ACCOUNT_MANAGER(data);

	priv = manager->priv;
	
	if(!priv->is_pending_update_channel_info) {
		priv->is_pending_update_channel_info = TRUE;
		g_idle_add((GSourceFunc) account_manager_update_channel_info, manager);
	}
}
static void
account_manager_channel_updated_cb(Channel *channel, gpointer data)
{
	AccountManager *manager;
	AccountManagerPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
        g_return_if_fail(data != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(data));
	
	manager = ACCOUNT_MANAGER(data);

	priv = manager->priv;
	
	if(channel_get_updated(channel) == TRUE &&
	   account_manager_is_current_channel(manager, channel) &&
	   account_manager_get_whether_scrolling(account_manager_get()))
		channel_set_updated(channel, FALSE);

	channel_tree_set_updated(priv->app->channel_tree, NULL, channel);
	loqui_menu_buffers_update_channel(priv->app->menu, channel);
	loqui_channelbar_update_channel(LOQUI_CHANNELBAR(priv->app->channelbar), channel);
}
static void
account_manager_channel_buffer_append_cb(ChannelBuffer *buffer, MessageText *msgtext, AccountManager *manager)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;

	if(!account_manager_get_whether_scrolling(manager)) {
	} else if(priv->current_channel) {
		if(buffer == priv->current_channel->buffer)
			return;
	} else if (priv->current_account) {
		if(buffer == priv->current_account->console_buffer)
			return;
	}

	if(priv->last_msgtext == msgtext)
		return;
	
	channel_buffer_append_message_text(priv->common_buffer, msgtext, TRUE, FALSE);

	if(priv->last_msgtext)
		g_object_unref(priv->last_msgtext);
	g_object_ref(msgtext);
	priv->last_msgtext = msgtext;
}
void account_manager_set_current_channel(AccountManager *manager, Channel *channel)
{
	AccountManagerPrivate *priv;
	gboolean is_account_changed, is_channel_changed;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(IS_CHANNEL(channel));
	
	priv = manager->priv;

	if(priv->current_channel) {
		g_signal_handlers_disconnect_by_func(priv->current_channel, account_manager_channel_changed_cb, manager);
	}
	if(priv->current_account) {
		g_signal_handlers_disconnect_by_func(priv->current_account, account_manager_account_changed_cb, manager);
	}

	is_account_changed = (account_manager_get_current_account(manager) != channel->account) ? TRUE : FALSE;
	is_channel_changed = (account_manager_get_current_channel(manager) != channel) ? TRUE : FALSE;

	priv->current_account = NULL; /* FIXME: this should be not NULL but channel->account */
	priv->current_channel = channel;

	channel_tree_select_channel(manager->priv->app->channel_tree, channel);
	loqui_app_set_channel_buffer(priv->app, channel->buffer);

	nick_list_set_store(priv->app->nick_list, channel->user_list);
	loqui_app_update_info(priv->app, 
			      is_account_changed, channel->account,
			      is_channel_changed, channel);

	channel_set_updated(channel, FALSE);

	g_signal_connect(G_OBJECT(channel), "topic-changed",
			 G_CALLBACK(account_manager_channel_changed_cb), manager);
	g_signal_connect(G_OBJECT(channel), "user-number-changed",
			 G_CALLBACK(account_manager_channel_changed_cb), manager);
	g_signal_connect(G_OBJECT(channel), "mode-changed",
			 G_CALLBACK(account_manager_channel_changed_cb), manager);
	g_signal_connect(G_OBJECT(channel->account), "nick-changed",
			 G_CALLBACK(account_manager_account_changed_cb), manager);
	g_signal_connect(G_OBJECT(channel->account), "away-changed",
			 G_CALLBACK(account_manager_account_changed_cb), manager);

	if(prefs_general.auto_switch_scrolling)
		account_manager_set_whether_scrolling(manager, TRUE);

	loqui_app_set_focus(priv->app);
}

void account_manager_set_current_account(AccountManager *manager, Account *account)
{
	AccountManagerPrivate *priv;
	gboolean is_account_changed, is_channel_changed;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
        g_return_if_fail(IS_ACCOUNT(account));

	priv = manager->priv;

	if(priv->current_channel) {
		g_signal_handlers_disconnect_by_func(priv->current_channel, account_manager_channel_changed_cb, manager);
	}
	if(priv->current_account) {
		g_signal_handlers_disconnect_by_func(priv->current_account, account_manager_account_changed_cb, manager);
	}

	is_account_changed = (account_manager_get_current_account(manager) != account) ? TRUE : FALSE;
	is_channel_changed = (account_manager_get_current_channel(manager) != NULL) ? TRUE : FALSE;

	priv->current_channel = NULL; /* FIXME */
	priv->current_account = account;

	channel_tree_select_account(manager->priv->app->channel_tree, account);
	loqui_app_set_channel_buffer(priv->app, account->console_buffer);

	nick_list_set_store(priv->app->nick_list, NULL);
	loqui_app_update_info(priv->app, 
			      is_account_changed, account,
			      is_channel_changed, NULL);

	g_signal_connect(G_OBJECT(account), "nick-changed",
			 G_CALLBACK(account_manager_account_changed_cb), manager);
	g_signal_connect(G_OBJECT(account), "away-changed",
			 G_CALLBACK(account_manager_account_changed_cb), manager);

	if(prefs_general.auto_switch_scrolling)
		account_manager_set_whether_scrolling(manager, TRUE);

	loqui_app_set_focus(priv->app);
}

Channel *account_manager_get_current_channel(AccountManager *manager)
{
	AccountManagerPrivate *priv;

        g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), NULL);

	priv = manager->priv;

	return priv->current_channel;
}
Account *account_manager_get_current_account(AccountManager *manager)
{
	AccountManagerPrivate *priv;

        g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), NULL);
	priv = manager->priv;

	if(priv->current_account)
		return priv->current_account;
	else if(priv->current_channel)
		return priv->current_channel->account;
	
	return NULL;
}

AccountManager *
account_manager_get(void)
{
	static gboolean made_account_manager = FALSE;

	if(!main_account_manager) {
		if(made_account_manager) {
			g_warning(_("main_account_manager should be created. account_manager_get() may be called recursively."));
			return NULL;
		}
		made_account_manager = TRUE;
		main_account_manager = account_manager_new();
	}

	return main_account_manager;
}
gboolean
account_manager_is_current_account(AccountManager *manager, Account *account)
{
	AccountManagerPrivate *priv;

        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	priv = manager->priv;

	return (account_manager_get_current_account(manager) == account);
}
gboolean
account_manager_is_current_channel(AccountManager *manager, Channel *channel)
{
        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	return (account_manager_get_current_channel(manager) == channel);
}
gboolean
account_manager_is_current_channel_buffer(AccountManager *manager, ChannelBuffer *buffer)
{
        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	return (buffer == loqui_app_get_channel_buffer(manager->priv->app));
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
void account_manager_open_account_list_dialog(AccountManager *manager)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));	

	account_list_dialog_open(GTK_WINDOW(manager->priv->app));
}
void account_manager_open_prefs_dialog(AccountManager *manager)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));	

	prefs_dialog_open(GTK_WINDOW(manager->priv->app));
}
void
account_manager_set_whether_scrolling(AccountManager *manager, gboolean is_scroll)
{
	AccountManagerPrivate *priv;

	priv = manager->priv;

	priv->is_scroll = is_scroll;
	loqui_toolbar_toggle_scrolling_with_signal_handler_blocked(LOQUI_TOOLBAR(priv->app->toolbar),
								   is_scroll);
	debug_puts("Set scroll: %d", is_scroll);
}
gboolean
account_manager_get_whether_scrolling(AccountManager *manager)
{
	return manager->priv->is_scroll;
}
void
account_manager_open_connect_dialog(AccountManager *manager)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	connect_dialog_open(GTK_WINDOW(manager->priv->app));
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

	for(cur = priv->account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);
		if(account_is_connected(account))
			continue;
		if(!account->use)
			continue;

		account_connect(account, NULL);
	}
}
