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
#include "config.h"

#include "account_manager.h"
#include "loqui_gconf.h"
#include "account.h"
#include "utils.h"
#include "loqui_app.h"

struct _AccountManagerPrivate
{
	GSList *account_list;

	LoquiApp *app;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void account_manager_class_init(AccountManagerClass *klass);
static void account_manager_init(AccountManager *account_manager);
static void account_manager_finalize(GObject *object);

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

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(object));

        account_manager = ACCOUNT_MANAGER(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(account_manager->priv);
}

AccountManager*
account_manager_new (void)
{
        AccountManager *account_manager;
	AccountManagerPrivate *priv;

	account_manager = g_object_new(account_manager_get_type(), NULL);

	priv = account_manager->priv;
	priv->app = LOQUI_APP(loqui_app_new());
	gtk_widget_show_all(GTK_WIDGET(priv->app));

	return account_manager;
}
void account_manager_load_accounts(AccountManager *account_manager)
{
	GSList *list;
        GSList *cur;
        gchar *name;
        Account *account;
	AccountManagerPrivate *priv;

        priv = account_manager->priv;

	list = eel_gconf_get_dirs(LOQUI_GCONF_ACCOUNT);
        if(list == NULL) return;

        for(cur = list; cur != NULL; cur = cur->next) {
                name = utils_gconf_get_basename((gchar *) cur->data);
		g_free(cur->data);
		if(!name) continue;
                account = account_new();
                account_restore(account, name);
 		priv->account_list = g_slist_append(priv->account_list, account);
		channel_tree_add_account(CHANNEL_TREE(priv->app->channel_tree), account);

                g_free(name);
        }
        g_slist_free(list);

	loqui_menu_create_connect_submenu(priv->app->menu, priv->account_list);
}
ChannelText *account_manager_add_channel_text(AccountManager *manager)
{
	AccountManagerPrivate *priv;
	ChannelText *text;

	priv = manager->priv;

	text = CHANNEL_TEXT(channel_text_new());
	channel_book_add_channel_text(priv->app->channel_book, text);
	gtk_widget_show_all(GTK_WIDGET(text));

	return text;
}

AccountManager *account_manager_get(void)
{
	if(!main_account_manager)
		main_account_manager = account_manager_new();
	return main_account_manager;
}
gboolean account_manager_whether_scroll(AccountManager *account_manager)
{
	return !loqui_app_is_scroll_locked(account_manager->priv->app);
}
