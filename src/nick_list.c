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
#include "main.h"
#include "irc_constants.h"
#include "account_manager.h"
#include "loqui_stock.h"

struct _NickListPrivate
{
	LoquiApp *app;

	GdkPixbuf *op_icon;
	GdkPixbuf *speak_ability_icon;

	GdkPixbuf *busy_icon;
	GdkPixbuf *away_icon;
	GdkPixbuf *online_icon;
	GdkPixbuf *offline_icon;
	
	GtkWidget *popup_menu;
};

static GtkTreeViewClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TREE_VIEW

static void nick_list_class_init(NickListClass *klass);
static void nick_list_init(NickList *nick_list);
static void nick_list_finalize(GObject *object);
static void nick_list_destroy(GtkObject *object);

static void nick_list_create_icons(NickList *list);

static void nick_list_cell_data_func_op(GtkTreeViewColumn *tree_column,
					GtkCellRenderer *cell,
					GtkTreeModel *tree_model,
					GtkTreeIter *iter,
					gpointer data);
static void nick_list_cell_data_func_away(GtkTreeViewColumn *tree_column,
					  GtkCellRenderer *cell,
					  GtkTreeModel *tree_model,
					  GtkTreeIter *iter,
					  gpointer data);
static gint nick_list_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, gpointer data);
static void nick_list_row_activated_cb(NickList *list, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data);
static gboolean nick_list_key_press_event(GtkWidget *widget, GdkEventKey *event);

static GSList *nick_list_menu_get_selected_members(NickList *nick_list);

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
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = nick_list_finalize;
        gtk_object_class->destroy = nick_list_destroy;

	widget_class->key_press_event = nick_list_key_press_event;
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
nick_list_destroy(GtkObject *object)
{
        NickList *nick_list;
	NickListPrivate *priv;
        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_NICK_LIST(object));

        nick_list = NICK_LIST(object);
	priv = nick_list->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->op_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->speak_ability_icon);	
	G_OBJECT_UNREF_UNLESS_NULL(priv->away_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->busy_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->online_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->offline_icon);

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

	priv->op_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_OPERATOR, LOQUI_ICON_SIZE_FONT, NULL);
	priv->speak_ability_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_SPEAK_ABILITY, LOQUI_ICON_SIZE_FONT, NULL);
	priv->online_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_ONLINE, LOQUI_ICON_SIZE_FONT, NULL);
	priv->offline_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_OFFLINE, LOQUI_ICON_SIZE_FONT, NULL);
	priv->busy_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_BUSY, LOQUI_ICON_SIZE_FONT, NULL);
	priv->away_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_AWAY, LOQUI_ICON_SIZE_FONT, NULL);
}
static void nick_list_cell_data_func_op(GtkTreeViewColumn *tree_column,
					GtkCellRenderer *cell,
					GtkTreeModel *tree_model,
					GtkTreeIter *iter,
					gpointer data)
{
	NickList *nick_list;
	LoquiMember *member;

	nick_list = NICK_LIST(data);
	gtk_tree_model_get(tree_model, iter, LOQUI_CHANNEL_ENTRY_STORE_COLUMN_MEMBER, &member, -1);

	if (loqui_member_get_is_channel_operator(member))
		g_object_set(G_OBJECT(cell), "pixbuf", nick_list->priv->op_icon, NULL);
	else if (loqui_member_get_speakable(member))
		g_object_set(G_OBJECT(cell), "pixbuf", nick_list->priv->speak_ability_icon, NULL);
	else
		g_object_set(G_OBJECT(cell), "pixbuf", NULL, NULL);
}
static void nick_list_cell_data_func_away(GtkTreeViewColumn *tree_column,
					  GtkCellRenderer *cell,
					  GtkTreeModel *tree_model,
					  GtkTreeIter *iter,
					  gpointer data)
{
	LoquiBasicAwayType basic_away;
	NickList *nick_list;
	NickListPrivate *priv;
	
	nick_list = NICK_LIST(data);
	gtk_tree_model_get(tree_model, iter, LOQUI_CHANNEL_ENTRY_STORE_COLUMN_BASIC_AWAY, &basic_away, -1);

	priv = nick_list->priv;
	
	switch(basic_away) {
	case LOQUI_BASIC_AWAY_TYPE_UNKNOWN:
		g_object_set(G_OBJECT(cell), "pixbuf", NULL, NULL);
		return;
	case LOQUI_BASIC_AWAY_TYPE_AWAY:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->away_icon, NULL);
		return;
	case LOQUI_BASIC_AWAY_TYPE_BUSY:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->busy_icon, NULL);
		return;
	case LOQUI_BASIC_AWAY_TYPE_ONLINE:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->online_icon, NULL);
		return;
	case LOQUI_BASIC_AWAY_TYPE_OFFLINE:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->offline_icon, NULL);
		return;
	default:
		break;
	}
}
static GSList *
nick_list_menu_get_selected_members(NickList *nick_list)
{
	GSList *member_list = NULL;
	GList *row_list, *cur;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *model;
	LoquiMember *member;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(nick_list));
	if(!selection)
		return NULL;

	row_list = gtk_tree_selection_get_selected_rows(selection, NULL);
	if(!row_list)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(nick_list));

	for(cur = row_list; cur != NULL; cur = cur->next) {
		path = cur->data;

		if(!gtk_tree_model_get_iter(model, &iter, path)) {
			continue;
                }
		gtk_tree_model_get(model, &iter, LOQUI_CHANNEL_ENTRY_STORE_COLUMN_MEMBER, &member, -1);
		member_list = g_slist_append(member_list, member);
	}
	g_list_foreach(row_list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(row_list);

	return member_list;
}
void
nick_list_start_private_talk_selected(NickList *nick_list)
{
	NickListPrivate *priv;
	GSList *member_list, *cur;
	Account *account;
	LoquiMember *member;

	priv = nick_list->priv;

	account = loqui_app_get_current_account(priv->app);
	if(!account)
		return;

	member_list = nick_list_menu_get_selected_members(nick_list);
	for(cur = member_list; cur != NULL; cur = cur->next) {
		member = cur->data;
		account_start_private_talk(account, loqui_user_get_nick(member->user));
	}
	g_slist_free(member_list);
}
void
nick_list_whois_selected(NickList *nick_list)
{
	NickListPrivate *priv;
	GSList *member_list, *cur;
	Account *account;
	LoquiMember *member;

	priv = nick_list->priv;

	account = loqui_app_get_current_account(priv->app);
	if(!account)
		return;

	member_list = nick_list_menu_get_selected_members(nick_list);
	for(cur = member_list; cur != NULL; cur = cur->next) {
		member = cur->data;
		account_whois(account, loqui_user_get_nick(member->user));
	}
	g_slist_free(member_list);
}
void
nick_list_change_mode_selected(NickList *nick_list, gboolean is_give, IRCModeFlag flag)
{
	NickListPrivate *priv;
	LoquiChannel *channel;
	GSList *member_list, *cur;
	LoquiMember *member;
	
	priv = nick_list->priv;

	channel = loqui_app_get_current_channel(priv->app);
	if(!channel)
		return;

	member_list = nick_list_menu_get_selected_members(nick_list);
	for(cur = member_list; cur != NULL; cur = cur->next) {
		member = cur->data;
		loqui_channel_push_user_mode_queue(channel, is_give, (IRCModeFlag) flag, loqui_user_get_nick(member->user));
	}
	g_slist_free(member_list);
	loqui_channel_flush_user_mode_queue(channel);
	
}
void
nick_list_ctcp_selected(NickList *nick_list, const gchar *command)
{
	NickListPrivate *priv;
	GSList *member_list, *cur;
	Account *account;
	LoquiMember *member;

	priv = nick_list->priv;

	account = loqui_app_get_current_account(priv->app);
	if(!account)
		return;

	member_list = nick_list_menu_get_selected_members(nick_list);
	for(cur = member_list; cur != NULL; cur = cur->next) {
		member = cur->data;
		account_send_ctcp_request(account, loqui_user_get_nick(member->user), command);
	}
	g_slist_free(member_list);
}
static gint
nick_list_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	NickList *nick_list;
	NickListPrivate *priv;
	GtkMenu *menu;

	nick_list = NICK_LIST(widget);
	priv = nick_list->priv;

	menu = GTK_MENU(priv->popup_menu);

	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		gtkutils_tree_view_popup(GTK_TREE_VIEW(nick_list), event, menu);
		return TRUE;
	}

	return FALSE;
}
static void nick_list_row_activated_cb(NickList *list, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	const gchar *nick;
	Account *account;
	NickListPrivate *priv;

	priv = list->priv;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
	if(!gtk_tree_model_get_iter(model, &iter, path)) {
		return;
	}
	gtk_tree_model_get(model, &iter, LOQUI_CHANNEL_ENTRY_STORE_COLUMN_NICK, &nick, -1);

	account = loqui_app_get_current_account(priv->app);
	if(!account)
		return;

	account_start_private_talk(account, nick);
}
static gboolean
nick_list_key_press_event(GtkWidget *widget,
			  GdkEventKey *event)
{
	loqui_app_grab_focus_if_key_unused(NICK_LIST(widget)->priv->app,
					   "GtkTreeView",
					   event->state,
					   event->keyval);

	if (* GTK_WIDGET_CLASS(parent_class)->key_press_event)
		return (* GTK_WIDGET_CLASS(parent_class)->key_press_event)(widget, event);

	return FALSE;	
}
GtkWidget*
nick_list_new(LoquiApp *app, GtkWidget *menu)
{
        NickList *list;
	NickListPrivate *priv;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *sel;

	list = g_object_new(nick_list_get_type(), NULL);

	priv = list->priv;
	priv->app = app;

	nick_list_create_icons(list);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);
/*	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(list), TRUE); */

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
        gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	
        renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "ypad", 0, NULL);
        column = gtk_tree_view_column_new_with_attributes("Away", renderer, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer,
						nick_list_cell_data_func_away,
						list, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 20);
	gtk_tree_view_column_set_sort_column_id(column, LOQUI_CHANNEL_ENTRY_STORE_COLUMN_BASIC_AWAY);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "ypad", 0, NULL);
        column = gtk_tree_view_column_new_with_attributes("@", renderer, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer,
						nick_list_cell_data_func_op,
						list, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 20);
	gtk_tree_view_column_set_sort_column_id(column, LOQUI_CHANNEL_ENTRY_STORE_COLUMN_POWER);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
        column = gtk_tree_view_column_new_with_attributes(_("Nick"),
							  renderer,
							  "text", LOQUI_CHANNEL_ENTRY_STORE_COLUMN_NICK,
							  NULL);
	gtk_tree_view_column_set_sort_column_id(column, LOQUI_CHANNEL_ENTRY_STORE_COLUMN_NICK);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(list), TRUE);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(list), LOQUI_CHANNEL_ENTRY_STORE_COLUMN_NICK);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(list), TRUE);

	priv->popup_menu = menu;

	g_signal_connect(G_OBJECT(list), "row_activated",
			 G_CALLBACK(nick_list_row_activated_cb), NULL);
	g_signal_connect(G_OBJECT(list), "button_press_event",
			 G_CALLBACK(nick_list_button_press_event_cb), NULL);
	
	return GTK_WIDGET(list);
}
