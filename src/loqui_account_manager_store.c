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
#include "config.h"

#include "loqui_account_manager_store.h"
#include "loqui_gtk.h"
#include "loqui_stock.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiAccountManagerStorePrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_account_manager_store_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_account_manager_store_class_init(LoquiAccountManagerStoreClass *klass);
static void loqui_account_manager_store_init(LoquiAccountManagerStore *store);
static void loqui_account_manager_store_finalize(GObject *object);
static void loqui_account_manager_store_dispose(GObject *object);

static void loqui_account_manager_store_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_account_manager_store_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_account_manager_store_tree_model_init(GtkTreeModelIface *iface);

static GtkTreeModelFlags loqui_account_manager_store_get_flags  (GtkTreeModel *tree_model);
static gint loqui_account_manager_store_get_n_columns(GtkTreeModel *tree_model);
static GType loqui_account_manager_store_get_column_type(GtkTreeModel *tree_model,
						       gint index);
static gboolean loqui_account_manager_store_get_iter(GtkTreeModel *tree_model,
						   GtkTreeIter *iter,
						   GtkTreePath *path);
static GtkTreePath *loqui_account_manager_store_get_path(GtkTreeModel *tree_model,
						       GtkTreeIter *iter);
static void loqui_account_manager_store_get_value(GtkTreeModel *tree_model,
						GtkTreeIter *iter,
						gint column,
						GValue *value);
static gboolean loqui_account_manager_store_iter_next(GtkTreeModel *tree_model,
						    GtkTreeIter *iter);
static gboolean loqui_account_manager_store_iter_children(GtkTreeModel *tree_model,
							GtkTreeIter *iter,
							GtkTreeIter *parent);
static gboolean loqui_account_manager_store_iter_has_child(GtkTreeModel *tree_model,
							 GtkTreeIter *iter);
static gint loqui_account_manager_store_iter_n_children(GtkTreeModel *tree_model,
						      GtkTreeIter *iter);
static gboolean loqui_account_manager_store_iter_nth_child(GtkTreeModel *tree_model,
							 GtkTreeIter *iter,
							 GtkTreeIter *parent,
							 gint n);
static gboolean loqui_account_manager_store_iter_parent(GtkTreeModel *tree_model,
						      GtkTreeIter *iter,
						      GtkTreeIter *child);

static void loqui_account_manager_store_add_channel_after_cb(Account *account,
							     LoquiChannel *channel,
							     LoquiAccountManagerStore *store);
static void loqui_account_manager_store_add_account_after_cb(AccountManager *manager,
							     Account *account,
							     LoquiAccountManagerStore *store);
static void loqui_account_manager_store_remove_channel_cb(Account *account,
							  LoquiChannel *channel,
							  LoquiAccountManagerStore *store);
static void loqui_account_manager_store_remove_account_cb(AccountManager *manager,
							  Account *account,
							  LoquiAccountManagerStore *store);
static void loqui_account_manager_store_channel_notify_cb(LoquiChannel *channel, GParamSpec *pspec, LoquiAccountManagerStore *store);
static void loqui_account_manager_store_account_notify_cb(Account *account, GParamSpec *pspec, LoquiAccountManagerStore *store);
static void loqui_account_manager_store_user_self_changed_cb(Account *account, LoquiAccountManagerStore *store);

static void loqui_account_manager_store_account_row_changed(LoquiAccountManagerStore *store, Account *account);
GType
loqui_account_manager_store_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAccountManagerStoreClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_account_manager_store_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAccountManagerStore),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_account_manager_store_init
			};
		static const GInterfaceInfo tree_model_info =
			{
				(GInterfaceInitFunc) loqui_account_manager_store_tree_model_init,
				NULL,
				NULL
			};

		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiAccountManagerStore",
					      &our_info,
					      0);
		g_type_add_interface_static(type, GTK_TYPE_TREE_MODEL, &tree_model_info);
	}
	
	return type;
}
static void 
loqui_account_manager_store_finalize(GObject *object)
{
	LoquiAccountManagerStore *store;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(object));

        store = LOQUI_ACCOUNT_MANAGER_STORE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(store->priv);
}
static void 
loqui_account_manager_store_dispose(GObject *object)
{
	LoquiAccountManagerStore *store;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(object));

        store = LOQUI_ACCOUNT_MANAGER_STORE(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_account_manager_store_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiAccountManagerStore *store;        

        store = LOQUI_ACCOUNT_MANAGER_STORE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_account_manager_store_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiAccountManagerStore *store;        

        store = LOQUI_ACCOUNT_MANAGER_STORE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_account_manager_store_class_init(LoquiAccountManagerStoreClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_account_manager_store_finalize;
        object_class->dispose = loqui_account_manager_store_dispose;
        object_class->get_property = loqui_account_manager_store_get_property;
        object_class->set_property = loqui_account_manager_store_set_property;
}
static void 
loqui_account_manager_store_init(LoquiAccountManagerStore *store)
{
	LoquiAccountManagerStorePrivate *priv;

	priv = g_new0(LoquiAccountManagerStorePrivate, 1);

	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_CHANNEL_ENTRY] = LOQUI_TYPE_CHANNEL_ENTRY;
	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_POSITION] = G_TYPE_INT;
	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_NAME] = G_TYPE_STRING;
	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_USERS] = G_TYPE_INT;
	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_OP_USERS] = G_TYPE_INT;
	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_COLOR] = G_TYPE_STRING;
	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY] = G_TYPE_INT;
	store->column_types[LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY_STOCK_ID] = G_TYPE_STRING;

	store->stamp = g_random_int();

	store->priv = priv;
}
static void
loqui_account_manager_store_tree_model_init(GtkTreeModelIface *iface)
{
	iface->get_flags       = loqui_account_manager_store_get_flags;
	iface->get_n_columns   = loqui_account_manager_store_get_n_columns;
	iface->get_column_type = loqui_account_manager_store_get_column_type;
	iface->get_iter        = loqui_account_manager_store_get_iter;
	iface->get_path        = loqui_account_manager_store_get_path;
	iface->get_value       = loqui_account_manager_store_get_value;
	iface->iter_next       = loqui_account_manager_store_iter_next;
	iface->iter_children   = loqui_account_manager_store_iter_children;
	iface->iter_has_child  = loqui_account_manager_store_iter_has_child;
	iface->iter_n_children = loqui_account_manager_store_iter_n_children;
	iface->iter_nth_child  = loqui_account_manager_store_iter_nth_child;
	iface->iter_parent     = loqui_account_manager_store_iter_parent;
}

static GtkTreeModelFlags
loqui_account_manager_store_get_flags(GtkTreeModel *tree_model)
{
        g_return_val_if_fail(tree_model != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), 0);

	return GTK_TREE_MODEL_ITERS_PERSIST;
}
static gint
loqui_account_manager_store_get_n_columns(GtkTreeModel *tree_model)
{
        g_return_val_if_fail(tree_model != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), 0);

	return LOQUI_ACCOUNT_MANAGER_STORE_N_COLUMNS;
}
static GType
loqui_account_manager_store_get_column_type(GtkTreeModel *tree_model, gint index)
{
	LoquiAccountManagerStore *store;

        g_return_val_if_fail(tree_model != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), 0);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);

	g_return_val_if_fail(index >= 0 && index < LOQUI_ACCOUNT_MANAGER_STORE_N_COLUMNS,
			     G_TYPE_INVALID);

	return store->column_types[index];
}

static gboolean
loqui_account_manager_store_get_iter(GtkTreeModel *tree_model,
				     GtkTreeIter *iter,
				     GtkTreePath *path)
{
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;
	gint *indices, n, depth;
	GList *account_list, *channel_list;
	Account *account;
	LoquiChannel *channel;
	GList *cur;

        g_return_val_if_fail(tree_model != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), FALSE);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;

	indices = gtk_tree_path_get_indices(path);
	depth   = gtk_tree_path_get_depth(path);

	g_assert(depth == 1 || depth == 2);
	n = indices[0];

	account_list = account_manager_get_account_list(store->manager);
	if (g_list_length(account_list) <= n) {
		return FALSE;
	}
	cur = g_list_nth(account_list, n);
	g_assert(cur->data != NULL);
	account = ACCOUNT(cur->data);

	if (depth == 1) {
		iter->stamp = store->stamp;
		iter->user_data = account;
		iter->user_data2 = cur;
		return TRUE;
	}
	
	n = indices[1];
	channel_list = account_get_channel_list(account);
	if (g_list_length(channel_list) <= n)
		return FALSE;

	cur = g_list_nth(channel_list, n);
	g_assert(cur->data != NULL);
	channel = LOQUI_CHANNEL(cur->data);

	iter->stamp = store->stamp;
	iter->user_data = channel;
	iter->user_data2 = cur;

	return TRUE;
}
static GtkTreePath *
loqui_account_manager_store_get_path(GtkTreeModel *tree_model,
				   GtkTreeIter *iter)
{
	GtkTreePath *path;
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;
	LoquiChannelEntry *chent;
	Account *account = NULL;
	LoquiChannel *channel = NULL;

	gint pos_ac, pos_ch;

	g_return_val_if_fail(tree_model != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), NULL);
	g_return_val_if_fail(iter != NULL, NULL);
	g_return_val_if_fail(iter->user_data != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(iter->user_data), NULL);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;
	
	chent = iter->user_data;

	path = gtk_tree_path_new();

	if (IS_ACCOUNT(chent)) {
		account = ACCOUNT(chent);
		channel = NULL;
	} else if (LOQUI_IS_CHANNEL(chent)) {
		channel = LOQUI_CHANNEL(chent);
		account = loqui_channel_get_account(channel);
	}

	pos_ac = g_list_index(account_manager_get_account_list(store->manager), account);
	gtk_tree_path_append_index(path, pos_ac);

	if (channel) {
		pos_ch = g_list_index(account_get_channel_list(account), channel);
		gtk_tree_path_append_index(path, pos_ch);
	}

	return path;
}
static void
loqui_account_manager_store_get_value(GtkTreeModel *tree_model,
				      GtkTreeIter *iter,
				      gint column,
				      GValue *value)
{
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;
	LoquiChannelEntry *chent;
	LoquiUser *user_self;

	g_return_if_fail(tree_model != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model));
	g_return_if_fail(iter != NULL);
	g_return_if_fail(iter->user_data != NULL);
	g_return_if_fail(column < LOQUI_ACCOUNT_MANAGER_STORE_N_COLUMNS);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;

	g_value_init(value, store->column_types[column]);

	chent = LOQUI_CHANNEL_ENTRY(iter->user_data);

	switch (column) {
	case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_CHANNEL_ENTRY:
		g_value_set_object(value, chent);
		return;
	case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_POSITION:
		g_value_set_int(value, loqui_channel_entry_get_position(chent));
		return;
	case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_NAME:
		g_value_set_string(value, loqui_channel_entry_get_name(chent));
		return;
	case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_COLOR:
		if (loqui_channel_entry_get_is_updated(chent)) {
			g_value_set_string(value, FRESH_COLOR);
		} else {
			g_value_set_string(value, NONFRESH_COLOR);
		}
		return;
	case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_USERS:
		g_value_set_int(value, loqui_channel_entry_get_member_number(chent));
		return;
	case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_OP_USERS:
		g_value_set_int(value, loqui_channel_entry_get_op_number(chent));
		return;
	default:
		break;
	}

	if (IS_ACCOUNT(chent)) {
		user_self = account_get_user_self(ACCOUNT(chent));

		switch (column) {
		case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY:
			g_value_set_int(value, loqui_user_get_basic_away(user_self));
			break;
		case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY_STOCK_ID:
			g_value_set_string(value, 
					   loqui_stock_get_id_from_basic_away_type(loqui_user_get_basic_away(user_self)));
			break;
		default:
			break;
		}
	} else {
		switch (column) {
		case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY:
			g_value_set_int(value, LOQUI_BASIC_AWAY_TYPE_UNKNOWN);
			break;
		case LOQUI_ACCOUNT_MANAGER_STORE_COLUMN_BASIC_AWAY_STOCK_ID:
			g_value_set_string(value, NULL);
			break;
		default:
			break;
		}
	}
}

static gboolean
loqui_account_manager_store_iter_next(GtkTreeModel *tree_model,
				      GtkTreeIter *iter)
{
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;
	GList *cur;

	g_return_val_if_fail(tree_model != NULL, FALSE);
	g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), FALSE);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;

	g_return_val_if_fail(iter != NULL, FALSE);
	g_return_val_if_fail(store->stamp == iter->stamp, FALSE);
	g_return_val_if_fail(iter->user_data != NULL, FALSE);
	g_return_val_if_fail(iter->user_data2 != NULL, FALSE);

	cur = iter->user_data2;
	cur = cur->next;

	if (cur == NULL)
		return FALSE;

	iter->user_data = cur->data;
	iter->user_data2 = cur;

	return TRUE;
}
static gboolean
loqui_account_manager_store_iter_children(GtkTreeModel *tree_model,
					  GtkTreeIter *iter,
					  GtkTreeIter *parent)
{
	return loqui_account_manager_store_iter_nth_child(tree_model, iter, parent, 0);
}
static gboolean
loqui_account_manager_store_iter_has_child(GtkTreeModel *tree_model,
					   GtkTreeIter *iter)
{
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;

        g_return_val_if_fail(tree_model != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), FALSE);
	g_return_val_if_fail(iter != NULL, FALSE);
	g_return_val_if_fail(iter->user_data != NULL, FALSE);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;

	if(IS_ACCOUNT(iter->user_data) &&
	   g_list_length(account_get_channel_list(ACCOUNT(iter->user_data))) > 0) {
		return TRUE;
	}

	return FALSE;
}
static gint
loqui_account_manager_store_iter_n_children(GtkTreeModel *tree_model,
					    GtkTreeIter *iter)
{
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;

        g_return_val_if_fail(tree_model != NULL, -1);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), -1);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;
	
	if (!store->manager)
		return -1;

	if (iter->user_data == NULL)
		return g_list_length(account_manager_get_account_list(store->manager));
	else if (IS_ACCOUNT(iter->user_data))
		return g_list_length(account_get_channel_list(ACCOUNT(iter->user_data)));

	return 0;
}
static gboolean
loqui_account_manager_store_iter_nth_child(GtkTreeModel *tree_model,
					   GtkTreeIter *iter,
					   GtkTreeIter *parent,
					   gint n)
{
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;
	GList *account_list, *channel_list;
	GList *cur;
	
        g_return_val_if_fail(tree_model != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), FALSE);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;

	if (parent == NULL) {
		account_list = account_manager_get_account_list(store->manager);
		cur = g_list_nth(account_list, n);
		if (cur == NULL)
			return FALSE;
		iter->stamp = store->stamp;
		iter->user_data = cur->data;
		iter->user_data2 = cur;

		return TRUE;
	} else if (IS_ACCOUNT(parent->user_data)) {
		channel_list = account_get_channel_list(parent->user_data);
		cur = g_list_nth(channel_list, n);
		if (cur == NULL)
			return FALSE;
		iter->stamp = store->stamp;
		iter->user_data = cur->data;
		iter->user_data2 = cur;
		return TRUE;
	}

	return FALSE;
}
static gboolean
loqui_account_manager_store_iter_parent(GtkTreeModel *tree_model,
					GtkTreeIter *iter,
					GtkTreeIter *child)
{
	LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;

        g_return_val_if_fail(tree_model != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(tree_model), FALSE);

	store = LOQUI_ACCOUNT_MANAGER_STORE(tree_model);
	priv = store->priv;
	
	if (child == NULL)
		return FALSE;
	else if (IS_ACCOUNT(child->user_data)) {
		return FALSE;
	} else if (LOQUI_IS_CHANNEL(child->user_data)) {
		iter->stamp = store->stamp;
		iter->user_data = loqui_channel_get_account(LOQUI_CHANNEL(child->user_data));
		iter->user_data2 = g_list_find(account_manager_get_account_list(store->manager), iter->user_data);
		return TRUE;
	}
	return FALSE;
}
static void
loqui_account_manager_store_add_channel_after_cb(Account *account,
						 LoquiChannel *channel,
						 LoquiAccountManagerStore *store)
{
	GtkTreeIter iter;
	GtkTreePath *path;

	iter.stamp = store->stamp;
	iter.user_data = channel;
	iter.user_data2 = g_list_find(account_get_channel_list(account), channel);

	path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);

	g_signal_connect(G_OBJECT(channel), "notify", 
			 G_CALLBACK(loqui_account_manager_store_channel_notify_cb), store);

	if (iter.user_data2 == account_get_channel_list(account)) { /* first appears */
		iter.stamp = store->stamp;
		iter.user_data = account;
		iter.user_data2 = g_list_find(account_manager_get_account_list(store->manager), account);
		
		path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
		gtk_tree_model_row_has_child_toggled(GTK_TREE_MODEL(store), path, &iter);
		gtk_tree_path_free(path);
	}
}
static void
loqui_account_manager_store_add_account_after_cb(AccountManager *manager,
						 Account *account,
						 LoquiAccountManagerStore *store)
{
	GtkTreeIter iter;
	GtkTreePath *path;

	iter.stamp = store->stamp;
	iter.user_data = account;
	iter.user_data2 = g_list_find(account_manager_get_account_list(manager), account);

	path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);

	g_signal_connect(G_OBJECT(account), "notify",
			 G_CALLBACK(loqui_account_manager_store_account_notify_cb), store);
	g_signal_connect_after(G_OBJECT(account), "add-channel",
			       G_CALLBACK(loqui_account_manager_store_add_channel_after_cb), store);
	g_signal_connect(G_OBJECT(account), "remove-channel",
			 G_CALLBACK(loqui_account_manager_store_remove_channel_cb), store);
	g_signal_connect(G_OBJECT(account), "user-self-changed",
			 G_CALLBACK(loqui_account_manager_store_user_self_changed_cb), store);
}

static void
loqui_account_manager_store_remove_channel_cb(Account *account,
					      LoquiChannel *channel,
					      LoquiAccountManagerStore *store)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	iter.stamp = store->stamp;
	iter.user_data = channel;
	iter.user_data2 = g_list_find(account_get_channel_list(account), channel);
	
	g_signal_handlers_disconnect_by_func(G_OBJECT(channel), loqui_account_manager_store_channel_notify_cb, store);
	
	path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_deleted(GTK_TREE_MODEL(store), path);
	gtk_tree_path_free(path);

	iter.stamp = store->stamp;
	iter.user_data = account;
	iter.user_data2 = g_list_find(account_manager_get_account_list(store->manager), account);

	path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_has_child_toggled(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);
}
static void
loqui_account_manager_store_remove_account_cb(AccountManager *manager,
					      Account *account,
					      LoquiAccountManagerStore *store)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *channel_list, *cur;

	iter.stamp = store->stamp;
	iter.user_data = account;
	iter.user_data2 = g_list_find(account_manager_get_account_list(manager), account);

	channel_list = account_get_channel_list(account);
	for (cur = channel_list; cur != NULL; cur = cur->next)
		loqui_account_manager_store_remove_channel_cb(account, cur->data, store);
	
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_account_manager_store_account_notify_cb, store);
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_account_manager_store_add_channel_after_cb, store);
	
	path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_deleted(GTK_TREE_MODEL(store), path);
	gtk_tree_path_free(path);
}
static void
loqui_account_manager_store_channel_notify_cb(LoquiChannel *channel, GParamSpec *pspec, LoquiAccountManagerStore *store)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	iter.stamp = store->stamp;
	iter.user_data = channel;
	iter.user_data2 = g_list_find(account_get_channel_list(loqui_channel_get_account(channel)), channel);

	path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);
}
static void
loqui_account_manager_store_account_row_changed(LoquiAccountManagerStore *store, Account *account)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	iter.stamp = store->stamp;
	iter.user_data = account;
	iter.user_data2 = g_list_find(account_manager_get_account_list(store->manager), account);

	path = loqui_account_manager_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);
}
static void
loqui_account_manager_store_user_self_changed_cb(Account *account, LoquiAccountManagerStore *store)
{
	loqui_account_manager_store_account_row_changed(store, account);	
}
static void
loqui_account_manager_store_account_notify_cb(Account *account, GParamSpec *pspec, LoquiAccountManagerStore *store)
{
	loqui_account_manager_store_account_row_changed(store, account);
}
LoquiAccountManagerStore*
loqui_account_manager_store_new(AccountManager *manager)
{
        LoquiAccountManagerStore *store;
	LoquiAccountManagerStorePrivate *priv;
	GList *cur_ac, *cur_ch;
	GList *account_list, *channel_list;

	g_return_val_if_fail(manager != NULL, NULL);

	store = g_object_new(loqui_account_manager_store_get_type(), NULL);
	
        priv = store->priv;
	store->manager = manager;

	account_list = account_manager_get_account_list(manager);

	for (cur_ac = account_list; cur_ac != NULL; cur_ac = cur_ac->next) {
		loqui_account_manager_store_add_account_after_cb(manager, cur_ac->data, store);
		channel_list = account_get_channel_list(cur_ac->data);

		for (cur_ch = channel_list; cur_ch != NULL; cur_ch = cur_ch->next) {
			loqui_account_manager_store_add_channel_after_cb(cur_ac->data, cur_ch->data, store);
		}
	}

	g_signal_connect_after(G_OBJECT(manager), "add-account",
			       G_CALLBACK(loqui_account_manager_store_add_account_after_cb), store);
	g_signal_connect(G_OBJECT(manager), "remove-account",
			 G_CALLBACK(loqui_account_manager_store_remove_account_cb), store);
        return store;
}
void
loqui_account_manager_store_get_iter_by_channel_entry(LoquiAccountManagerStore* store,
						      GtkTreeIter *iter,
						      LoquiChannelEntry *chent)
{
	iter->stamp = store->stamp;
	iter->user_data = chent;

        g_return_if_fail(store != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER_STORE(store));
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	if (IS_ACCOUNT(chent)) {
		iter->user_data2 = g_list_find(account_manager_get_account_list(store->manager), chent);
	} else if (LOQUI_IS_CHANNEL(chent)) {
		iter->user_data2 = g_list_find(account_get_channel_list(loqui_channel_get_account(LOQUI_CHANNEL(chent))),
					       chent);
	}
}
