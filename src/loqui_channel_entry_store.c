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

#include "loqui_channel_entry_store.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiChannelEntryStorePrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_channel_entry_store_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_channel_entry_store_class_init(LoquiChannelEntryStoreClass *klass);
static void loqui_channel_entry_store_init(LoquiChannelEntryStore *store);
static void loqui_channel_entry_store_finalize(GObject *object);
static void loqui_channel_entry_store_dispose(GObject *object);

static void loqui_channel_entry_store_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_entry_store_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_channel_entry_store_tree_model_init(GtkTreeModelIface *iface);

static GtkTreeModelFlags loqui_channel_entry_store_get_flags  (GtkTreeModel *tree_model);
static gint loqui_channel_entry_store_get_n_columns(GtkTreeModel *tree_model);
static GType loqui_channel_entry_store_get_column_type(GtkTreeModel *tree_model,
						       gint index);
static gboolean loqui_channel_entry_store_get_iter(GtkTreeModel *tree_model,
						   GtkTreeIter *iter,
						   GtkTreePath *path);
static GtkTreePath *loqui_channel_entry_store_get_path(GtkTreeModel *tree_model,
						       GtkTreeIter *iter);
static void loqui_channel_entry_store_get_value(GtkTreeModel *tree_model,
						GtkTreeIter *iter,
						gint column,
						GValue *value);
static gboolean loqui_channel_entry_store_iter_next(GtkTreeModel *tree_model,
						    GtkTreeIter *iter);
static gboolean loqui_channel_entry_store_iter_children(GtkTreeModel *tree_model,
							GtkTreeIter *iter,
							GtkTreeIter *parent);
static gboolean loqui_channel_entry_store_iter_has_child(GtkTreeModel *tree_model,
							 GtkTreeIter *iter);
static gint loqui_channel_entry_store_iter_n_children(GtkTreeModel *tree_model,
						      GtkTreeIter *iter);
static gboolean loqui_channel_entry_store_iter_nth_child(GtkTreeModel *tree_model,
							 GtkTreeIter *iter,
							 GtkTreeIter *parent,
							 gint n);
static gboolean loqui_channel_entry_store_iter_parent(GtkTreeModel *tree_model,
						      GtkTreeIter *iter,
						      GtkTreeIter *child);


static void loqui_channel_entry_store_add_after_cb(LoquiChannelEntry *entry,
						   LoquiMember *member,
						   LoquiChannelEntryStore *store);
static void loqui_channel_entry_store_remove_cb(LoquiChannelEntry *chent,
						LoquiMember *member,
						LoquiChannelEntryStore *store);
static void loqui_channel_entry_store_member_notify_cb(LoquiMember *member, GParamSpec *pspec, LoquiChannelEntryStore *store);
static void loqui_channel_entry_store_user_notify_cb(LoquiUser *user, GParamSpec *pspec, LoquiChannelEntryStore *store);

GType
loqui_channel_entry_store_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelEntryStoreClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_entry_store_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelEntryStore),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_entry_store_init
			};
		static const GInterfaceInfo tree_model_info =
			{
				(GInterfaceInitFunc) loqui_channel_entry_store_tree_model_init,
				NULL,
				NULL
			};

		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiChannelEntryStore",
					      &our_info,
					      0);
		g_type_add_interface_static(type, GTK_TYPE_TREE_MODEL, &tree_model_info);
	}
	
	return type;
}
static void 
loqui_channel_entry_store_finalize(GObject *object)
{
	LoquiChannelEntryStore *store;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(object));

        store = LOQUI_CHANNEL_ENTRY_STORE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(store->priv);
}
static void 
loqui_channel_entry_store_dispose(GObject *object)
{
	LoquiChannelEntryStore *store;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(object));

        store = LOQUI_CHANNEL_ENTRY_STORE(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_channel_entry_store_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiChannelEntryStore *store;        

        store = LOQUI_CHANNEL_ENTRY_STORE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_entry_store_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiChannelEntryStore *store;        

        store = LOQUI_CHANNEL_ENTRY_STORE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_channel_entry_store_class_init(LoquiChannelEntryStoreClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_entry_store_finalize;
        object_class->dispose = loqui_channel_entry_store_dispose;
        object_class->get_property = loqui_channel_entry_store_get_property;
        object_class->set_property = loqui_channel_entry_store_set_property;
}
static void 
loqui_channel_entry_store_init(LoquiChannelEntryStore *store)
{
	LoquiChannelEntryStorePrivate *priv;

	priv = g_new0(LoquiChannelEntryStorePrivate, 1);

	store->column_types[LOQUI_CHANNEL_ENTRY_STORE_COLUMN_MEMBER] = LOQUI_TYPE_MEMBER;
	store->column_types[LOQUI_CHANNEL_ENTRY_STORE_COLUMN_BASIC_AWAY] = G_TYPE_INT;
	store->column_types[LOQUI_CHANNEL_ENTRY_STORE_COLUMN_POWER] = G_TYPE_INT;
	store->column_types[LOQUI_CHANNEL_ENTRY_STORE_COLUMN_NICK] = G_TYPE_STRING;

	store->stamp = g_random_int();

	store->priv = priv;
}
static void
loqui_channel_entry_store_tree_model_init(GtkTreeModelIface *iface)
{
	iface->get_flags       = loqui_channel_entry_store_get_flags;
	iface->get_n_columns   = loqui_channel_entry_store_get_n_columns;
	iface->get_column_type = loqui_channel_entry_store_get_column_type;
	iface->get_iter        = loqui_channel_entry_store_get_iter;
	iface->get_path        = loqui_channel_entry_store_get_path;
	iface->get_value       = loqui_channel_entry_store_get_value;
	iface->iter_next       = loqui_channel_entry_store_iter_next;
	iface->iter_children   = loqui_channel_entry_store_iter_children;
	iface->iter_has_child  = loqui_channel_entry_store_iter_has_child;
	iface->iter_n_children = loqui_channel_entry_store_iter_n_children;
	iface->iter_nth_child  = loqui_channel_entry_store_iter_nth_child;
	iface->iter_parent     = loqui_channel_entry_store_iter_parent;
}

static GtkTreeModelFlags
loqui_channel_entry_store_get_flags(GtkTreeModel *tree_model)
{
        g_return_val_if_fail(tree_model != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), 0);

	return GTK_TREE_MODEL_LIST_ONLY;
}
static gint
loqui_channel_entry_store_get_n_columns(GtkTreeModel *tree_model)
{
        g_return_val_if_fail(tree_model != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), 0);

	return LOQUI_CHANNEL_ENTRY_STORE_N_COLUMNS;
}
static GType
loqui_channel_entry_store_get_column_type(GtkTreeModel *tree_model, gint index)
{
	LoquiChannelEntryStore *store;

        g_return_val_if_fail(tree_model != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), 0);

	store = LOQUI_CHANNEL_ENTRY_STORE(tree_model);

	g_return_val_if_fail(index >= 0 && index < LOQUI_CHANNEL_ENTRY_STORE_N_COLUMNS,
			     G_TYPE_INVALID);

	return store->column_types[index];
}

static gboolean
loqui_channel_entry_store_get_iter(GtkTreeModel *tree_model,
				   GtkTreeIter *iter,
				   GtkTreePath *path)
{
	LoquiChannelEntryStore *store;
	LoquiChannelEntryStorePrivate *priv;
	gint *indices, n, depth;
	LoquiMember *member;
	
        g_return_val_if_fail(tree_model != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), FALSE);

	store = LOQUI_CHANNEL_ENTRY_STORE(tree_model);
	priv = store->priv;

	indices = gtk_tree_path_get_indices(path);
	depth   = gtk_tree_path_get_depth(path);

	g_assert(depth == 1);
	n = indices[0];

	if(loqui_channel_entry_get_member_number(store->chent) <= n) {
		return FALSE;
	}

	member = loqui_channel_entry_get_nth_member(store->chent, n);
	g_assert(member != NULL);

	iter->stamp = store->stamp;
	iter->user_data = member;
	iter->user_data2 = GINT_TO_POINTER(n) + 1;

	return TRUE;
}
static GtkTreePath *
loqui_channel_entry_store_get_path(GtkTreeModel *tree_model,
				   GtkTreeIter *iter)
{
	GtkTreePath *path;
	LoquiChannelEntryStore *store;
	LoquiChannelEntryStorePrivate *priv;
	LoquiMember *member;
	gint pos;

	g_return_val_if_fail(tree_model != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), NULL);
	g_return_val_if_fail(iter != NULL, NULL);
	g_return_val_if_fail(iter->user_data != NULL, NULL);
	
	store = LOQUI_CHANNEL_ENTRY_STORE(tree_model);
	priv = store->priv;
	
	member = LOQUI_MEMBER(iter->user_data);
	pos = GPOINTER_TO_INT(iter->user_data2) - 1;
	g_assert(pos >= 0);

	path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, pos);
	
	return path;
}
static void
loqui_channel_entry_store_get_value(GtkTreeModel *tree_model,
				    GtkTreeIter *iter,
				    gint column,
				    GValue *value)
{
	LoquiChannelEntryStore *store;
	LoquiChannelEntryStorePrivate *priv;
	LoquiMember *member;
	gint pos;
	gint power;

	g_return_if_fail(tree_model != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model));
	g_return_if_fail(iter != NULL);
	g_return_if_fail(iter->user_data != NULL);
	g_return_if_fail(column < LOQUI_CHANNEL_ENTRY_STORE_N_COLUMNS);

	store = LOQUI_CHANNEL_ENTRY_STORE(tree_model);
	priv = store->priv;

	g_value_init(value, store->column_types[column]);

	member = LOQUI_MEMBER(iter->user_data);
	pos = GPOINTER_TO_INT(iter->user_data2) - 1;
	g_return_if_fail(member != NULL);

	switch (column) {
	case LOQUI_CHANNEL_ENTRY_STORE_COLUMN_MEMBER:
		g_value_set_object(value, member);
		break;
	case LOQUI_CHANNEL_ENTRY_STORE_COLUMN_NICK:
		g_value_set_string(value, loqui_user_get_nick(member->user));
		break;
	case LOQUI_CHANNEL_ENTRY_STORE_COLUMN_BASIC_AWAY:
		g_value_set_int(value, loqui_user_get_basic_away(member->user));
		break;
	case LOQUI_CHANNEL_ENTRY_STORE_COLUMN_POWER:
		power = 0; /* FIXME */
		if (loqui_member_get_is_channel_operator(member))
			power = 2;
		else if (loqui_member_get_speakable(member))
			power = 1;
		g_value_set_int(value, power);
		break;
	}
}
static gboolean
loqui_channel_entry_store_iter_next(GtkTreeModel *tree_model,
				    GtkTreeIter *iter)
{
	LoquiChannelEntryStore *store;
	LoquiChannelEntryStorePrivate *priv;
	LoquiMember *member;
	gint pos;

	g_return_val_if_fail(tree_model != NULL, FALSE);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), FALSE);

	store = LOQUI_CHANNEL_ENTRY_STORE(tree_model);
	priv = store->priv;

	g_return_val_if_fail(iter != NULL, FALSE);
	g_return_val_if_fail(store->stamp == iter->stamp, FALSE);
	g_return_val_if_fail(iter->user_data != NULL, FALSE);

	pos = GPOINTER_TO_INT(iter->user_data2) - 1;
	pos++;

	member = loqui_channel_entry_get_nth_member(store->chent, pos);
	if (member == NULL)
		return FALSE;

	iter->user_data = member;
	iter->user_data2 = GINT_TO_POINTER(pos + 1);

	return TRUE;
}
static gboolean
loqui_channel_entry_store_iter_children(GtkTreeModel *tree_model,
					GtkTreeIter *iter,
					GtkTreeIter *parent)
{
	if (parent != NULL)
		return FALSE;

	return loqui_channel_entry_store_iter_nth_child(tree_model, iter, parent, 0);
}
static gboolean
loqui_channel_entry_store_iter_has_child(GtkTreeModel *tree_model,
					 GtkTreeIter *iter)
{
	return FALSE;
}
static gint
loqui_channel_entry_store_iter_n_children(GtkTreeModel *tree_model,
					  GtkTreeIter *iter)
{
	LoquiChannelEntryStore *store;
	LoquiChannelEntryStorePrivate *priv;

        g_return_val_if_fail(tree_model != NULL, -1);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), -1);

	store = LOQUI_CHANNEL_ENTRY_STORE(tree_model);
	priv = store->priv;
	
	if (!store->chent)
		return -1;

	return loqui_channel_entry_get_member_number(store->chent);
}
static gboolean
loqui_channel_entry_store_iter_nth_child(GtkTreeModel *tree_model,
					 GtkTreeIter *iter,
					 GtkTreeIter *parent,
					 gint n)
{
	LoquiChannelEntryStore *store;
	LoquiChannelEntryStorePrivate *priv;
	LoquiMember *member;

        g_return_val_if_fail(tree_model != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_STORE(tree_model), FALSE);

	store = LOQUI_CHANNEL_ENTRY_STORE(tree_model);
	priv = store->priv;

	if (parent)
		return FALSE;
	if (!store->chent)
		return FALSE;

	member = loqui_channel_entry_get_nth_member(store->chent, n);
	g_assert(member != NULL);
	
	iter->stamp = store->stamp;
	iter->user_data = member;
	iter->user_data2 = GINT_TO_POINTER(n + 1);

	return TRUE;
}
static gboolean
loqui_channel_entry_store_iter_parent(GtkTreeModel *tree_model,
						      GtkTreeIter *iter,
						      GtkTreeIter *child)
{
	return FALSE;
}
static void
loqui_channel_entry_store_add_after_cb(LoquiChannelEntry *entry,
				       LoquiMember *member,
				       LoquiChannelEntryStore *store)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gint pos;

	iter.user_data = member;
	pos = loqui_channel_entry_get_member_pos(entry, member);
	g_return_if_fail(pos >= 0);
	iter.user_data2 = GINT_TO_POINTER(pos + 1);

	path = loqui_channel_entry_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);

	g_signal_connect(G_OBJECT(member), "notify::is-channel-operator", 
			 G_CALLBACK(loqui_channel_entry_store_member_notify_cb), store);
	g_signal_connect(G_OBJECT(member), "notify::speakable", 
			 G_CALLBACK(loqui_channel_entry_store_member_notify_cb), store);
	g_signal_connect(G_OBJECT(member->user), "notify::nick",
			 G_CALLBACK(loqui_channel_entry_store_user_notify_cb), store);
	g_signal_connect(G_OBJECT(member->user), "notify::away",
			 G_CALLBACK(loqui_channel_entry_store_user_notify_cb), store);
}
static void
loqui_channel_entry_store_remove_cb(LoquiChannelEntry *chent,
				    LoquiMember *member,
				    LoquiChannelEntryStore *store)
{
	gint pos;
	GtkTreePath *path;
	GtkTreeIter iter;

	pos = loqui_channel_entry_get_member_pos(chent, member);
	g_return_if_fail(pos >= 0);

	iter.user_data = member;
	iter.user_data2 = GINT_TO_POINTER(pos + 1);

	g_signal_handlers_disconnect_by_func(G_OBJECT(member), loqui_channel_entry_store_member_notify_cb, store);
	g_signal_handlers_disconnect_by_func(G_OBJECT(member->user), loqui_channel_entry_store_user_notify_cb, store);
	
	path = loqui_channel_entry_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_deleted(GTK_TREE_MODEL(store), path);
	gtk_tree_path_free(path);
}
static void
loqui_channel_entry_store_user_notify_cb(LoquiUser *user, GParamSpec *pspec, LoquiChannelEntryStore *store)
{
	gint pos;
	GtkTreePath *path;
	GtkTreeIter iter;
	LoquiMember *member;

	g_return_if_fail(user != NULL);
	g_return_if_fail(LOQUI_IS_USER(user));

	member = loqui_channel_entry_get_member_by_user(store->chent, user);

	pos = loqui_channel_entry_get_member_pos(store->chent, member);
	g_return_if_fail(pos >= 0);

	iter.user_data = member;
	iter.user_data2 = GINT_TO_POINTER(pos + 1);

	path = loqui_channel_entry_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);
}
static void
loqui_channel_entry_store_member_notify_cb(LoquiMember *member, GParamSpec *pspec, LoquiChannelEntryStore *store)
{
	gint pos;
	GtkTreePath *path;
	GtkTreeIter iter;

	g_return_if_fail(member != NULL);
	g_return_if_fail(LOQUI_IS_MEMBER(member));

	pos = loqui_channel_entry_get_member_pos(store->chent, member);
	g_return_if_fail(pos >= 0);

	iter.user_data = member;
	iter.user_data2 = GINT_TO_POINTER(pos + 1);

	path = loqui_channel_entry_store_get_path(GTK_TREE_MODEL(store), &iter);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(store), path, &iter);
	gtk_tree_path_free(path);	
}
LoquiChannelEntryStore*
loqui_channel_entry_store_new(LoquiChannelEntry *chent)
{
        LoquiChannelEntryStore *store;
	LoquiChannelEntryStorePrivate *priv;
	int i, num;
	LoquiMember *member;

	g_return_val_if_fail(chent != NULL, NULL);

	store = g_object_new(loqui_channel_entry_store_get_type(), NULL);
	
        priv = store->priv;
	store->chent = chent;
	num = loqui_channel_entry_get_member_number(chent);

	for(i = 0; i < num; i++) {
		member = loqui_channel_entry_get_nth_member(chent, i);
		loqui_channel_entry_store_add_after_cb(chent, member, store);
	}

	g_signal_connect_after(G_OBJECT(chent), "add",
			       G_CALLBACK(loqui_channel_entry_store_add_after_cb), store);
	g_signal_connect(G_OBJECT(chent), "remove",
			 G_CALLBACK(loqui_channel_entry_store_remove_cb), store);
        return store;
}
