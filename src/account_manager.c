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

struct _AccountManagerPrivate
{
	GSList *account_list;
	Account *current_account;
	Channel *current_channel;

	ChannelBuffer *common_buffer;
	LoquiApp *app;

	gboolean is_scroll;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void account_manager_class_init(AccountManagerClass *klass);
static void account_manager_init(AccountManager *account_manager);
static void account_manager_finalize(GObject *object);
/* static Account* account_manager_search_account(AccountManager *manager, Channel *channel); */

static void account_manager_add_account(AccountManager *manager, Account *account);
static void account_manager_update_account(AccountManager *manager, Account *account);
static void account_manager_remove_account(AccountManager *manager, Account *account);

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

static void
account_manager_add_account(AccountManager *manager, Account *account)
{
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	priv = manager->priv;

	priv->account_list = g_slist_append(priv->account_list, account);
	channel_tree_add_account(priv->app->channel_tree, account);

	loqui_menu_update_connect_submenu(priv->app->menu, priv->account_list);
}
static void
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
	g_object_unref(account);

	loqui_menu_update_connect_submenu(priv->app->menu, priv->account_list);
}
static void
account_manager_update_account(AccountManager *manager, Account *account)
{
	AccountManagerPrivate *priv;
	GSList *cur;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	priv = manager->priv;
	
	for(cur = priv->account_list; cur != NULL; cur = cur->next) {
		channel_tree_update_account(priv->app->channel_tree, account);
	}

	loqui_menu_update_connect_submenu(priv->app->menu, priv->account_list);
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
		account_manager_update_current_info(manager);
		channel_set_fresh(channel, FALSE);
		account_manager_update_away_status(manager, account_get_away_status(channel->account));
	} else if(account) {
		loqui_app_set_channel_buffer(priv->app, GTK_TEXT_BUFFER(account->console_buffer));
		nick_list_set_store(priv->app->nick_list, NULL);
		account_manager_update_current_info(manager);
		account_manager_update_away_status(manager, account_get_away_status(account));
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

#if 0
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
#endif

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
		account = priv->current_channel->account;
	else
		account = priv->current_account;

	if(account == NULL)
		return;

	account_speak(account, priv->current_channel, str);
}
void
account_manager_current_account_set_away(AccountManager *manager, gboolean is_away)
{
	AccountManagerPrivate *priv;
	Account *account;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;
	g_return_if_fail(priv->current_account != NULL || priv->current_channel != NULL);
	
	if(priv->current_channel)
		account = priv->current_channel->account;
	else
		account = priv->current_account;

	if(account == NULL || !account_is_connected(account))
		return;

	if(is_away)
		account_set_away(account, prefs_general->away_message);
	else
		account_set_away(account, NULL);

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
	AccountManagerPrivate *priv;

        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	priv = manager->priv;

	if(priv->current_account == account)
		return (priv->current_account == account);
	else if(priv->current_channel) {
		return (priv->current_channel->account == account);
	}
	return FALSE;
}
gboolean
account_manager_is_current_channel(AccountManager *manager, Channel *channel)
{
        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	return (manager->priv->current_channel == channel);
}
gboolean
account_manager_is_current_channel_buffer(AccountManager *manager, ChannelBuffer *buffer)
{
	ChannelBuffer *buffer2;
        g_return_val_if_fail(manager != NULL, FALSE);
        g_return_val_if_fail(IS_ACCOUNT_MANAGER(manager), FALSE);

	buffer2 = CHANNEL_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(manager->priv->app->channel_textview)));
	return (buffer == buffer2);
}

void
account_manager_scroll_channel_textview(AccountManager *manager)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));	

	if(account_manager_get_whether_scrolling(manager)) {
		loqui_app_scroll_channel_textview(manager->priv->app);
	}
}
void
account_manager_common_buffer_append(AccountManager *manager, TextType type, gchar *str)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	channel_buffer_append_line(manager->priv->common_buffer, type, str);
	if(account_manager_get_whether_scrolling(manager)) {
		loqui_app_scroll_common_textview(manager->priv->app);
	}
}

void account_manager_common_buffer_append_remark(AccountManager *manager, TextType type,
						 gboolean is_self, gboolean is_priv,
						 const gchar *channel_name, const gchar *nick, const gchar *remark)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	channel_buffer_append_remark(manager->priv->common_buffer, type, FALSE,
				     is_self, is_priv, channel_name, nick, remark);
	if(account_manager_get_whether_scrolling(manager)) {
		loqui_app_scroll_common_textview(manager->priv->app);
	}	
}
void account_manager_update_current_info(AccountManager *manager)
{
	gchar *account_name = NULL;
	gchar *channel_name = NULL, *channel_mode = NULL;
	gchar *topic = NULL;
	gint user_number = -1, op_number = -1;
	Account *account = NULL;
	Channel *channel = NULL;
	AccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;

	if(priv->current_channel) {
		account = priv->current_channel->account;
		channel = priv->current_channel;
	} else
		account = priv->current_account;

	if(!account) {
		loqui_app_set_current_info(manager->priv->app, NULL, NULL, NULL, NULL, -1, -1);
		return;
	}

	account_name = account_get_name(account);
	if(channel) {
		channel_name = channel->name;
		topic = channel_get_topic(channel);
		channel_get_user_number(channel, (guint *) &user_number, (guint *) & op_number);
		/* TODO: mode */
	}

	loqui_app_set_current_info(manager->priv->app, account_name, 
				   channel_name, channel_mode,
				   topic, user_number, op_number);
}
void account_manager_update_channel_user_number(AccountManager *manager, Channel *channel)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	account_manager_update_current_info(manager);
	channel_tree_update_user_number(manager->priv->app->channel_tree, channel);
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

	account_list_dialog_open();
}
void
account_manager_add_account_with_dialog(AccountManager *manager)
{
	AccountDialog *dialog;
	Account *account;
	gint response;

	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));

	account = account_new();
	dialog = ACCOUNT_DIALOG(account_dialog_new(account));
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));

	switch(response) {
	case GTK_RESPONSE_OK:
		account_manager_add_account(manager, account);
		break;
	default:
		g_object_unref(account);
		break;
	}
	account_manager_save_accounts(manager);
}

void
account_manager_configure_account_with_dialog(AccountManager *manager, Account *account)
{
	GtkWidget *dialog;
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	
	dialog = account_dialog_new(account);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	account_manager_update_account(manager, account);

	account_manager_save_accounts(manager);
}

void
account_manager_remove_account_with_dialog(AccountManager *manager, Account *account)
{
	GtkWidget *dialog;
	gint response;

	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(manager->priv->app),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_YES_NO,
					_("This account's configuration and connection will be removed.\n"
					  "Do you really want to remove this account?"));

	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	switch(response) {

	case GTK_RESPONSE_YES:
		account_manager_remove_account(manager, account);
		account_manager_save_accounts(manager);
		debug_puts("Removed account.");
		break;
	default:
		break;
	}
	account_manager_save_accounts(manager);

}
void
account_manager_set_whether_scrolling(AccountManager *manager, gboolean is_scroll)
{
	AccountManagerPrivate *priv;

	priv = manager->priv;

	priv->is_scroll = is_scroll;
	loqui_toolbar_toggle_scrolling_without_signal_emission(LOQUI_TOOLBAR(priv->app->toolbar),
								   is_scroll);
	debug_puts("Set scroll: %d", is_scroll);
}
gboolean
account_manager_get_whether_scrolling(AccountManager *manager)
{
	return manager->priv->is_scroll;
}
void
account_manager_update_away_status(AccountManager *manager, gboolean is_away)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	
	loqui_toolbar_toggle_away_without_signal_emission(LOQUI_TOOLBAR(manager->priv->app->toolbar),
							  is_away);
}
