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

#include "nick_list.h"
#include "gtkutils.h"
#include "intl.h"

struct _NickListPrivate
{
	GdkPixbuf *op_icon;
	GdkPixbuf *nonop_icon;
	GdkPixbuf *speak_ability_icon;

	GdkPixbuf *home_icon;
	GdkPixbuf *away_icon;
};
enum {
	COLUMN_HOMEAWAY,
	COLUMN_OP,
	COLUMN_NICK,
	COLUMN_POINTER,
	COLUMN_NUMBER
};

static GtkTreeViewClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TREE_VIEW

static void nick_list_class_init(NickListClass *klass);
static void nick_list_init(NickList *nick_list);
static void nick_list_finalize(GObject *object);
static void nick_list_destroy(GtkObject *object);

static void nick_list_create_icons(NickList *list);

static GdkPixbuf* nick_list_get_icon_op(NickList *list, UserPower power);
static GdkPixbuf* nick_list_get_icon_away(NickList *list, UserPower power);

GType
nick_list_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(NickListClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) nick_list_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(NickList),
				0,              /* n_preallocs */
				(GInstanceInitFunc) nick_list_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "NickList",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
nick_list_class_init (NickListClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = nick_list_finalize;
        gtk_object_class->destroy = nick_list_destroy;
}
static void 
nick_list_init (NickList *nick_list)
{
	NickListPrivate *priv;

	priv = g_new0(NickListPrivate, 1);

	nick_list->priv = priv;
}
static void 
nick_list_finalize (GObject *object)
{
	NickList *nick_list;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_NICK_LIST(object));

        nick_list = NICK_LIST(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(nick_list->priv);
}
static void 
nick_list_destroy (GtkObject *object)
{
        NickList *nick_list;
	NickListPrivate *priv;
        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_NICK_LIST(object));

        nick_list = NICK_LIST(object);
	priv = nick_list->priv;

	if(priv->op_icon) {
		gdk_pixbuf_unref(priv->op_icon);
		priv->op_icon = NULL;
	}
	if(priv->nonop_icon) {
		gdk_pixbuf_unref(priv->nonop_icon);
		priv->nonop_icon = NULL;
	}
	if(priv->speak_ability_icon) {
		gdk_pixbuf_unref(priv->speak_ability_icon);
		priv->speak_ability_icon = NULL;
	}
	if(priv->home_icon) {
		gdk_pixbuf_unref(priv->home_icon);
		priv->home_icon = NULL;
	}
	if(priv->away_icon) {
		gdk_pixbuf_unref(priv->away_icon);
		priv->away_icon = NULL;
	}
        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
nick_list_create_icons(NickList *list)
{
	NickListPrivate *priv;

        g_return_if_fail(list != NULL);
        g_return_if_fail(IS_NICK_LIST(list));

	priv = list->priv;

	priv->op_icon = gtk_widget_render_icon(GTK_WIDGET(list), GTK_STOCK_YES,
					       GTK_ICON_SIZE_MENU, NULL);
	priv->nonop_icon = gtk_widget_render_icon(GTK_WIDGET(list), GTK_STOCK_NO,
						  GTK_ICON_SIZE_MENU, NULL);
	priv->speak_ability_icon = gtk_widget_render_icon(GTK_WIDGET(list), GTK_STOCK_OK,
							  GTK_ICON_SIZE_MENU, NULL);
	priv->home_icon = gtk_widget_render_icon(GTK_WIDGET(list), GTK_STOCK_HOME,
						 GTK_ICON_SIZE_MENU, NULL);
	priv->away_icon = gtk_widget_render_icon(GTK_WIDGET(list), GTK_STOCK_QUIT,
						 GTK_ICON_SIZE_MENU, NULL);
}
GtkWidget*
nick_list_new (void)
{
        NickList *list;
	NickListPrivate *priv;
	GtkListStore *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *sel;

	list = g_object_new(nick_list_get_type(), NULL);

	priv = list->priv;

	nick_list_create_icons(list);
	model = gtk_list_store_new(COLUMN_NUMBER, 
				   GDK_TYPE_PIXBUF,
				   GDK_TYPE_PIXBUF,
                                   G_TYPE_STRING, 
                                   G_TYPE_POINTER);
        gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(model));

	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(list), TRUE);

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
        gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	
        renderer = gtk_cell_renderer_pixbuf_new();
        column = gtk_tree_view_column_new_with_attributes("Away",
							  renderer,
							  "pixbuf", COLUMN_HOMEAWAY,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 20);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_pixbuf_new();
        column = gtk_tree_view_column_new_with_attributes("@",
							  renderer,
							  "pixbuf", COLUMN_OP,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 20);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Nick"),
							  renderer,
							  "text", COLUMN_NICK,
							  NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ptr",
							  renderer,
							  "text", COLUMN_POINTER,
							  NULL);
        gtk_tree_view_column_set_visible (column, FALSE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

#if 0
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				   COLUMN_HOMEAWAY, priv->away_icon,
				   COLUMN_OP, priv->speak_ability_icon,
				   COLUMN_NICK, "Hogeo",
				   -1);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				   COLUMN_HOMEAWAY, priv->home_icon,
				   COLUMN_OP, priv->nonop_icon,
				   COLUMN_NICK, "Fugao",
				   -1);
	}
#endif

	return GTK_WIDGET(list);
}
static GdkPixbuf *
nick_list_get_icon_op(NickList *list, UserPower power)
{
	NickListPrivate *priv;

	priv = list->priv;

	switch(power) {
	case USER_POWER_OP:
		return priv->op_icon;
	case USER_POWER_V:
		return priv->speak_ability_icon;
	case USER_POWER_NOTHING:
		return priv->nonop_icon;
	default:
		break;
	}
	return NULL;
}
static GdkPixbuf *
nick_list_get_icon_away(NickList *list, UserExistence exist)
{
	NickListPrivate *priv;

	priv = list->priv;

	switch(exist) {
	case USER_EXISTENCE_HOME:
		return priv->home_icon;
	case USER_EXISTENCE_AWAY:
		return priv->away_icon;
	default:
		break;
	}
	return NULL;
}
void nick_list_append(NickList *list, User *user)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

        g_return_if_fail(list != NULL);
        g_return_if_fail(IS_NICK_LIST(list));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   COLUMN_HOMEAWAY, nick_list_get_icon_away(list, user->exist),
			   COLUMN_OP, nick_list_get_icon_op(list, user->power),
			   COLUMN_NICK, user->nick,
			   COLUMN_POINTER, user,
			   -1);
}
void nick_list_remove(NickList *list, User *user)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

        g_return_if_fail(list != NULL);
        g_return_if_fail(IS_NICK_LIST(list));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));

	if(!gtk_tree_model_find_by_column_data(model, &iter, NULL,
					       COLUMN_POINTER, user))
		return;

	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}
void nick_list_update(NickList *list, User *user)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

        g_return_if_fail(list != NULL);
        g_return_if_fail(IS_NICK_LIST(list));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));

	if(!gtk_tree_model_find_by_column_data(model, &iter, NULL,
					       COLUMN_POINTER, user))
		return;

	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   COLUMN_HOMEAWAY, nick_list_get_icon_away(list, user->exist),
			   COLUMN_OP, nick_list_get_icon_op(list, user->power),
			   COLUMN_NICK, user->nick,
			   COLUMN_POINTER, user,
			   -1);
}
void nick_list_clear(NickList *list)
{
	GtkTreeModel *model;

        g_return_if_fail(list != NULL);
        g_return_if_fail(IS_NICK_LIST(list));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
	
	gtk_list_store_clear(GTK_LIST_STORE(model));
}
