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

#include "loqui_account_manager.h"
#include "loqui_account.h"
#include "utils.h"
#include "intl.h"
#include "loqui_profile_handle.h"

#include "loqui_account_manager_iter.h"
#include "loqui-static-core.h"

struct _LoquiAccountManagerPrivate
{
	GList *account_list;
};

enum {
	SIGNAL_ADD_ACCOUNT,
	SIGNAL_REMOVE_ACCOUNT,
	LAST_SIGNAL
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT
#define ACCOUNT_CONFIG_FILENAME "account.xml"

static guint loqui_account_manager_signals[LAST_SIGNAL] = { 0 };

static void loqui_account_manager_class_init(LoquiAccountManagerClass *klass);
static void loqui_account_manager_init(LoquiAccountManager *account_manager);
static void loqui_account_manager_finalize(GObject *object);
static void loqui_account_manager_dispose(GObject *object);

static void loqui_account_manager_add_account_real(LoquiAccountManager *manager, LoquiAccount *account);
static void loqui_account_manager_remove_account_real(LoquiAccountManager *manager, LoquiAccount *account);

static void loqui_account_manager_add_channel_after_cb(LoquiAccount *account, LoquiChannel *channel, LoquiAccountManager *manager);
static void loqui_account_manager_remove_channel_cb(LoquiAccount *account, LoquiChannel *channel, LoquiAccountManager *manager);
static void loqui_account_manager_remove_channel_after_cb(LoquiAccount *account, LoquiChannel *channel, LoquiAccountManager *manager);

GType
loqui_account_manager_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountManagerClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_manager_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccountManager),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_manager_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiAccountManager",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_account_manager_class_init (LoquiAccountManagerClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_manager_finalize;
	object_class->dispose = loqui_account_manager_dispose;
	klass->add_account = loqui_account_manager_add_account_real;
	klass->remove_account = loqui_account_manager_remove_account_real;

	loqui_account_manager_signals[SIGNAL_ADD_ACCOUNT] = g_signal_new("add-account",
								   G_OBJECT_CLASS_TYPE(object_class),
								   G_SIGNAL_RUN_LAST,
								   G_STRUCT_OFFSET(LoquiAccountManagerClass, add_account),
								   NULL, NULL,
								   g_cclosure_marshal_VOID__OBJECT,
								   G_TYPE_NONE, 1,
								   LOQUI_TYPE_ACCOUNT);
	loqui_account_manager_signals[SIGNAL_REMOVE_ACCOUNT] = g_signal_new("remove-account",
								      G_OBJECT_CLASS_TYPE(object_class),
								      G_SIGNAL_RUN_LAST,
								      G_STRUCT_OFFSET(LoquiAccountManagerClass, remove_account),
								      NULL, NULL,
								      g_cclosure_marshal_VOID__OBJECT,
								      G_TYPE_NONE, 1,
								      LOQUI_TYPE_ACCOUNT);
}
static void 
loqui_account_manager_init (LoquiAccountManager *account_manager)
{
	LoquiAccountManagerPrivate *priv;

	priv = g_new0(LoquiAccountManagerPrivate, 1);

	account_manager->priv = priv;

	account_manager->max_channel_entry_id = -1;
}
static void 
loqui_account_manager_dispose(GObject *object)
{
	LoquiAccountManager *account_manager;
	LoquiAccountManagerPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(object));

        account_manager = LOQUI_ACCOUNT_MANAGER(object);
	priv = account_manager->priv;

	loqui_account_manager_remove_all_account(account_manager);

	G_OBJECT_UNREF_UNLESS_NULL(account_manager->protocol_manager);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose) (object);
}

static void 
loqui_account_manager_finalize (GObject *object)
{
	LoquiAccountManager *account_manager;
	LoquiAccountManagerPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(object));

        account_manager = LOQUI_ACCOUNT_MANAGER(object);
	priv = account_manager->priv;

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(priv);
}

LoquiAccountManager*
loqui_account_manager_new (LoquiProtocolManager *protocol_manager)
{
        LoquiAccountManager *account_manager;
	LoquiAccountManagerPrivate *priv;

	account_manager = g_object_new(loqui_account_manager_get_type(), NULL);

	priv = account_manager->priv;

	g_object_ref(protocol_manager);
	account_manager->protocol_manager = protocol_manager;

	return account_manager;
}

static void
loqui_account_manager_add_account_real(LoquiAccountManager *manager, LoquiAccount *account)
{
	LoquiAccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));	

	priv = manager->priv;

	g_object_ref(account);
	priv->account_list = g_list_append(priv->account_list, account);

	g_signal_connect_after(G_OBJECT(account), "add-channel",
			       G_CALLBACK(loqui_account_manager_add_channel_after_cb), manager);
	g_signal_connect(G_OBJECT(account), "remove-channel",
			 G_CALLBACK(loqui_account_manager_remove_channel_cb), manager);
	g_signal_connect_after(G_OBJECT(account), "remove-channel",
			       G_CALLBACK(loqui_account_manager_remove_channel_after_cb), manager);

	loqui_channel_entry_set_id(LOQUI_CHANNEL_ENTRY(account),
				   loqui_account_manager_new_channel_entry_id(manager));
	loqui_account_manager_update_positions(manager);
}
static void
loqui_account_manager_remove_account_real(LoquiAccountManager *manager, LoquiAccount *account)
{
	LoquiAccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	priv = manager->priv;

	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_account_manager_add_channel_after_cb, manager);
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_account_manager_remove_channel_cb, manager);
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_account_manager_remove_channel_after_cb, manager);

	priv->account_list = g_list_remove(priv->account_list, account);
	g_object_unref(account);

	loqui_account_manager_update_positions(manager);
}
static void
loqui_account_manager_add_channel_after_cb(LoquiAccount *account, LoquiChannel *channel, LoquiAccountManager *manager)
{
	loqui_channel_entry_set_id(LOQUI_CHANNEL_ENTRY(channel),
				   loqui_account_manager_new_channel_entry_id(manager));
	loqui_account_manager_update_positions(manager);
}
static void
loqui_account_manager_remove_channel_cb(LoquiAccount *account, LoquiChannel *channel, LoquiAccountManager *manager)
{
	
}
static void
loqui_account_manager_remove_channel_after_cb(LoquiAccount *account, LoquiChannel *channel, LoquiAccountManager *manager)
{
	loqui_account_manager_update_positions(manager);
}

void
loqui_account_manager_add_account(LoquiAccountManager *manager, LoquiAccount *account)
{
	LoquiAccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	priv = manager->priv;

	g_signal_emit(G_OBJECT(manager), loqui_account_manager_signals[SIGNAL_ADD_ACCOUNT], 0, account);
}
void
loqui_account_manager_remove_account(LoquiAccountManager *manager, LoquiAccount *account)
{
	LoquiAccountManagerPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	priv = manager->priv;

	g_signal_emit(G_OBJECT(manager), loqui_account_manager_signals[SIGNAL_REMOVE_ACCOUNT], 0, account);
}
void
loqui_account_manager_remove_all_account(LoquiAccountManager *manager)
{
	LoquiAccountManagerPrivate *priv;
	GList *list, *cur;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;	

	list = g_list_copy(priv->account_list);
	for (cur = list; cur != NULL; cur = cur->next) {
		loqui_account_manager_remove_account(manager, cur->data);
	}
	g_list_free(list);
}

void
loqui_account_manager_load_accounts(LoquiAccountManager *account_manager)
{
        GList *cur, *list = NULL;
	LoquiAccountManagerPrivate *priv;
	gchar *path;
	LoquiProfileHandle *handle;
	LoquiAccount *account;
	LoquiProfileAccount *profile;

        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(account_manager));

        priv = account_manager->priv;

	path = g_build_filename(loqui_core_get_user_dir(loqui_get_core()), ACCOUNT_CONFIG_FILENAME, NULL);
	handle = loqui_profile_handle_new(account_manager->protocol_manager);
	loqui_profile_handle_read_from_file(handle, &list, path);

	for(cur = list; cur != NULL; cur = cur->next) {
		profile = LOQUI_PROFILE_ACCOUNT(cur->data);
		account = loqui_protocol_create_account(profile->protocol, profile);

		loqui_account_manager_add_account(account_manager, account);
		g_object_unref(account);
	}
	g_list_free(list);
	g_object_unref(handle);
}

void
loqui_account_manager_save_accounts(LoquiAccountManager *account_manager)
{
        GList *cur;
	GList *list = NULL;
	gchar *path;
	LoquiProfileHandle *handle;

        g_return_if_fail(account_manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(account_manager));

	for(cur = account_manager->priv->account_list; cur != NULL; cur = cur->next) {
		list = g_list_append(list, loqui_account_get_profile(cur->data));
	}

	path = g_build_filename(loqui_core_get_user_dir(loqui_get_core()), ACCOUNT_CONFIG_FILENAME, NULL);
	handle = loqui_profile_handle_new(account_manager->protocol_manager);
	loqui_profile_handle_write_to_file(handle, list, path);
	g_object_unref(handle);
	g_list_free(list);
}
void loqui_account_manager_disconnect_all(LoquiAccountManager *manager)
{
	g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));

	g_list_foreach(manager->priv->account_list, (GFunc) loqui_account_disconnect, NULL);
}
GList *loqui_account_manager_get_account_list(LoquiAccountManager *manager)
{
	g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager), NULL);

	return manager->priv->account_list;
}
void
loqui_account_manager_connect_all_default(LoquiAccountManager *manager)
{
	GList *cur;
	LoquiAccount *account;
	LoquiAccountManagerPrivate *priv;

	g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;

	for (cur = priv->account_list; cur != NULL; cur = cur->next) {
		account = LOQUI_ACCOUNT(cur->data);
		if (loqui_account_get_is_connected(account))
			continue;
		if (!loqui_profile_account_get_use(loqui_account_get_profile(account)))
			continue;
		
		loqui_account_connect(account);
	}
}

/**
   @chent: ChannelEntry or NULL
   @returns: next channel entry or NULL(not changed)
*/
LoquiChannelEntry *
loqui_account_manager_get_next_channel_entry(LoquiAccountManager *manager, LoquiChannelEntry *chent, gboolean require_unread)
{
	LoquiAccountManagerIter iter, iter_chent;
	LoquiAccountManagerPrivate *priv;
	gboolean is_exist;
	LoquiChannelEntry *tmp_chent;

	g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager), NULL);

	priv = manager->priv;

	loqui_account_manager_iter_init(manager, &iter);
	if (chent)
		is_exist = loqui_account_manager_iter_set_channel_entry(&iter, chent);
	else
		is_exist = FALSE;
	iter_chent = iter;
	if (is_exist) {
		loqui_account_manager_iter_channel_entry_next(&iter);
		while ((tmp_chent = loqui_account_manager_iter_channel_entry_next(&iter))) {
			if (!require_unread || loqui_channel_entry_get_whether_unread(tmp_chent))
				return tmp_chent;
		}
	}
	loqui_account_manager_iter_set_first_channel_entry(&iter);			
	while ((tmp_chent = loqui_account_manager_iter_channel_entry_next(&iter))) {
		if (!require_unread || loqui_channel_entry_get_whether_unread(tmp_chent))
			return tmp_chent;
	}
	return NULL;
}
LoquiChannelEntry *
loqui_account_manager_get_previous_channel_entry(LoquiAccountManager *manager, LoquiChannelEntry *chent, gboolean require_unread)
{
	LoquiAccountManagerIter iter;
	LoquiAccountManagerPrivate *priv;
	gboolean is_exist;
	LoquiChannelEntry *tmp_chent;

	g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager), NULL);

	priv = manager->priv;

	loqui_account_manager_iter_init(manager, &iter);
	if (chent)
		is_exist = loqui_account_manager_iter_set_channel_entry(&iter, chent);
	else
		is_exist = FALSE;

	if (is_exist) {
		loqui_account_manager_iter_channel_entry_previous(&iter);
		while ((tmp_chent = loqui_account_manager_iter_channel_entry_previous(&iter))) {
			if (!require_unread || loqui_channel_entry_get_whether_unread(tmp_chent))
				return tmp_chent;
		}
	}
	loqui_account_manager_iter_set_last_channel_entry(&iter);			
	while ((tmp_chent = loqui_account_manager_iter_channel_entry_previous(&iter))) {
		if (!require_unread || loqui_channel_entry_get_whether_unread(tmp_chent))
			return tmp_chent;
	}
	return NULL;
}

gint
loqui_account_manager_new_channel_entry_id(LoquiAccountManager *manager)
{
	return ++manager->max_channel_entry_id;
}
void
loqui_account_manager_update_positions(LoquiAccountManager *manager)
{
	LoquiAccountManagerPrivate *priv;
	LoquiAccountManagerIter iter;
	LoquiChannelEntry *chent;
	gint i;

	g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));

	priv = manager->priv;
	
	i = 0;
	loqui_account_manager_iter_init(manager, &iter);
	loqui_account_manager_iter_set_first_channel_entry(&iter);
	while ((chent = loqui_account_manager_iter_channel_entry_next(&iter))) {
		loqui_channel_entry_set_position(chent, i);
		i++;
	}
}
