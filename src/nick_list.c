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

struct _NickListPrivate
{
	GdkPixbuf *op_icon;
	GdkPixbuf *nonop_icon;
	GdkPixbuf *speak_ability_icon;

	GdkPixbuf *busy_icon;
	GdkPixbuf *away_icon;
	GdkPixbuf *online_icon;
	GdkPixbuf *offline_icon;
	
	GtkItemFactory *item_factory;
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
static GSList *nick_list_menu_get_selected_nicks(NickList *nick_list);

static void nick_list_menu_start_private_talk_cb(gpointer data, guint action, GtkWidget *widget);
static void nick_list_menu_whois_cb(gpointer data, guint action, GtkWidget *widget);
static void nick_list_menu_mode_give_cb(gpointer data, guint action, GtkWidget *widget);
static void nick_list_menu_mode_deprive_cb(gpointer data, guint action, GtkWidget *widget);
static void nick_list_menu_ctcp_cb(gpointer data, guint action, GtkWidget *widget);

enum {
	CTCP_VERSION,
	CTCP_CLIENTINFO,
	CTCP_USERINFO,
	CTCP_PING,
	CTCP_TIME,
	CTCP_FINGER
};

static GtkItemFactoryEntry popup_menu_items[] = {
	{ N_("/Start private talk"), NULL, nick_list_menu_start_private_talk_cb, 0 },
	{ N_("/Show information [Whois]"), NULL, nick_list_menu_whois_cb, 0 },
	{ N_("/Change user mode [Mode]"), NULL, 0, 0, "<Branch>"},
	{ N_("/Change user mode [Mode]/Give channel operator privilege (+o)"), NULL, 
	     nick_list_menu_mode_give_cb, IRC_CHANNEL_MODE_OPERATOR },
	{ N_("/Change user mode [Mode]/Give voice privilege (+v)"), NULL,
             nick_list_menu_mode_give_cb, IRC_CHANNEL_MODE_VOICE },
	{    "/Change user mode [Mode]/sep1", NULL, 0, 0, "<Separator>" },
	{ N_("/Change user mode [Mode]/Deprive channel operator privilege (-o)"), NULL, 
	  nick_list_menu_mode_deprive_cb, IRC_CHANNEL_MODE_OPERATOR },
	{ N_("/Change user mode [Mode]/Deprive voice privilege (-v)"), NULL,
	  nick_list_menu_mode_deprive_cb, IRC_CHANNEL_MODE_VOICE },

	{ N_("/CTCP"), NULL, 0, 0, "<Branch>" },
	{ N_("/CTCP/Version"), NULL, nick_list_menu_ctcp_cb, CTCP_VERSION },
	{ N_("/CTCP/Clientinfo"), NULL, nick_list_menu_ctcp_cb, CTCP_CLIENTINFO },
	{ N_("/CTCP/Userinfo"), NULL, nick_list_menu_ctcp_cb, CTCP_USERINFO },
	{ N_("/CTCP/Ping"), NULL, nick_list_menu_ctcp_cb, CTCP_PING },
	{ N_("/CTCP/Time"), NULL, nick_list_menu_ctcp_cb, CTCP_TIME },
	{ N_("/CTCP/Finger"), NULL, nick_list_menu_ctcp_cb, CTCP_FINGER }
};

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
nick_list_destroy(GtkObject *object)
{
        NickList *nick_list;
	NickListPrivate *priv;
        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_NICK_LIST(object));

        nick_list = NICK_LIST(object);
	priv = nick_list->priv;

	G_OBJECT_UNREF_UNLESS_NULL(priv->op_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->nonop_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->speak_ability_icon);	
	G_OBJECT_UNREF_UNLESS_NULL(priv->away_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->busy_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->online_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->offline_icon);
	G_OBJECT_UNREF_UNLESS_NULL(priv->item_factory);

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
	priv->busy_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_BUSY,
						 GTK_ICON_SIZE_MENU, NULL);
	priv->away_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_AWAY,
						 GTK_ICON_SIZE_MENU, NULL);
	priv->online_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_ONLINE,
						   GTK_ICON_SIZE_MENU, NULL);
	priv->offline_icon = gtk_widget_render_icon(GTK_WIDGET(list), LOQUI_STOCK_OFFLINE,
						    GTK_ICON_SIZE_MENU, NULL);

}
static void nick_list_cell_data_func_op(GtkTreeViewColumn *tree_column,
					GtkCellRenderer *cell,
					GtkTreeModel *tree_model,
					GtkTreeIter *iter,
					gpointer data)
{
	UserPower power;
	NickList *nick_list;

	nick_list = NICK_LIST(data);
	gtk_tree_model_get(tree_model, iter, USERLIST_COLUMN_OP, &power, -1);

	switch(power) {
	case USER_POWER_OP:
		g_object_set(G_OBJECT(cell), "pixbuf", nick_list->priv->op_icon, NULL);
		return;
	case USER_POWER_VOICE:
		g_object_set(G_OBJECT(cell), "pixbuf", nick_list->priv->speak_ability_icon, NULL);
		return;
	case USER_POWER_NOTHING:
		g_object_set(G_OBJECT(cell), "pixbuf", nick_list->priv->nonop_icon, NULL);
		return;
	default:
		break;
	}
}
static void nick_list_cell_data_func_away(GtkTreeViewColumn *tree_column,
					  GtkCellRenderer *cell,
					  GtkTreeModel *tree_model,
					  GtkTreeIter *iter,
					  gpointer data)
{
	AwayState away_state;
	NickList *nick_list;
	NickListPrivate *priv;
	
	nick_list = NICK_LIST(data);
	gtk_tree_model_get(tree_model, iter, USERLIST_COLUMN_HOMEAWAY, &away_state, -1);

	priv = nick_list->priv;
	
	switch(away_state) {
	case AWAY_STATE_NONE:
		g_object_set(G_OBJECT(cell), "pixbuf", NULL, NULL);
		return;
	case AWAY_STATE_AWAY:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->away_icon, NULL);
		return;
	case AWAY_STATE_BUSY:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->busy_icon, NULL);
		return;
	case AWAY_STATE_ONLINE:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->online_icon, NULL);
		return;
	case AWAY_STATE_OFFLINE:
		g_object_set(G_OBJECT(cell), "pixbuf", priv->offline_icon, NULL);
		return;
	default:
		break;
	}
}
static GSList *
nick_list_menu_get_selected_nicks(NickList *nick_list)
{
	GSList *str_list = NULL;
	GList *row_list, *cur;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *nick;

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
		gtk_tree_model_get(model, &iter, USERLIST_COLUMN_NICK, &nick, -1);
		str_list = g_slist_append(str_list, nick);
	}
	g_list_foreach(row_list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(row_list);

	return str_list;
}
static void
nick_list_menu_start_private_talk_cb(gpointer data, guint action, GtkWidget *widget)
{
	NickList *nick_list;
	GSList *str_list, *cur;
	Account *account;

	nick_list = NICK_LIST(data);

	account = account_manager_get_current_account(account_manager_get());
	if(!account)
		return;

	str_list = nick_list_menu_get_selected_nicks(nick_list);
	for(cur = str_list; cur != NULL; cur = cur->next) {
		account_start_private_talk(account, (gchar *) cur->data);
		g_free(cur->data);
	}
	g_slist_free(str_list);
}
static void
nick_list_menu_whois_cb(gpointer data, guint action, GtkWidget *widget)
{
	NickList *nick_list;
	GSList *str_list, *cur;
	Account *account;

	nick_list = NICK_LIST(data);

	account = account_manager_get_current_account(account_manager_get());
	if(!account)
		return;

	str_list = nick_list_menu_get_selected_nicks(nick_list);
	for(cur = str_list; cur != NULL; cur = cur->next) {
		account_whois(account, (gchar *) cur->data);
		g_free(cur->data);
	}
	g_slist_free(str_list);
}
static void
nick_list_menu_mode_give_cb(gpointer data, guint action, GtkWidget *widget)
{
	NickList *nick_list;
	GSList *str_list, *cur;
	Channel *channel;

	nick_list = NICK_LIST(data);
	channel = account_manager_get_current_channel(account_manager_get());
	if(!channel)
		return;

	str_list = nick_list_menu_get_selected_nicks(nick_list);
	for(cur = str_list; cur != NULL; cur = cur->next) {
		channel_push_user_mode_queue(channel, TRUE, (IRCModeFlag) action, (gchar *) cur->data);
		g_free(cur->data);
	}
	g_slist_free(str_list);
	channel_flush_user_mode_queue(channel);

}
static void
nick_list_menu_mode_deprive_cb(gpointer data, guint action, GtkWidget *widget)
{
	NickList *nick_list;
	GSList *str_list, *cur;
	Channel *channel;

	nick_list = NICK_LIST(data);
	channel = account_manager_get_current_channel(account_manager_get());
	if(!channel)
		return;

	str_list = nick_list_menu_get_selected_nicks(nick_list);
	for(cur = str_list; cur != NULL; cur = cur->next) {
		channel_push_user_mode_queue(channel, FALSE, (IRCModeFlag) action, (gchar *) cur->data);
		g_free(cur->data);
	}
	g_slist_free(str_list);
	channel_flush_user_mode_queue(channel);
}
static void
nick_list_menu_ctcp_cb(gpointer data, guint action, GtkWidget *widget)
{
	NickList *nick_list;
	GSList *str_list, *cur;
	gchar *command = NULL;
	Account *account;

	nick_list = NICK_LIST(data);

	account = account_manager_get_current_account(account_manager_get());
	if(!account)
		return;

	str_list = nick_list_menu_get_selected_nicks(nick_list);
	for(cur = str_list; cur != NULL; cur = cur->next) {
		switch(action) {
		case CTCP_VERSION:
			command = IRCCTCPVersion;
			break;
		case CTCP_CLIENTINFO:
			command = IRCCTCPClientInfo;
			break;
		case CTCP_USERINFO:
			command = IRCCTCPUserInfo;
			break;
		case CTCP_PING:
			command = IRCCTCPPing;
			break;
		case CTCP_FINGER:
			command = IRCCTCPFinger;
			break;
		case CTCP_TIME:
			command = IRCCTCPTime;
			break;
		default:
			g_assert_not_reached();
			break;
		}
		account_send_ctcp_request(account, (gchar *) cur->data, command);
		g_free(cur->data);
	}
	g_slist_free(str_list);
}
static gint
nick_list_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	NickList *nick_list;
	NickListPrivate *priv;
	GtkMenu *menu;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GList *row_list = NULL;

	nick_list = NICK_LIST(widget);
	priv = nick_list->priv;

	menu = GTK_MENU(priv->popup_menu);

	if(event->type == GDK_BUTTON_PRESS && event->button == 3) {
		if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(nick_list), event->x, event->y,
						  &path, NULL, NULL, NULL))
                        return FALSE;
		gtk_tree_path_free(path);
		
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(nick_list));
                if(!selection)
                        return FALSE;
		row_list = gtk_tree_selection_get_selected_rows(selection, NULL);
		if(!row_list)
			return FALSE;
		g_list_foreach(row_list, (GFunc) gtk_tree_path_free, NULL);
		g_list_free(row_list);

		gtk_menu_popup(menu, NULL, NULL, NULL,
			       nick_list, event->button, event->time);
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

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
	if(!gtk_tree_model_get_iter(model, &iter, path)) {
		return;
	}
	gtk_tree_model_get(model, &iter, USERLIST_COLUMN_NICK, &nick, -1);

	account = account_manager_get_current_account(account_manager_get());
	if(!account)
		return;

	account_start_private_talk(account, nick);
}
GtkWidget*
nick_list_new(void)
{
        NickList *list;
	NickListPrivate *priv;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *sel;

	list = g_object_new(nick_list_get_type(), NULL);

	priv = list->priv;

	nick_list_create_icons(list);
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
	gtk_tree_view_column_set_sort_column_id(column, USERLIST_COLUMN_HOMEAWAY);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "ypad", 0, NULL);
        column = gtk_tree_view_column_new_with_attributes("@", renderer, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer,
						nick_list_cell_data_func_op,
						list, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 20);
	gtk_tree_view_column_set_sort_column_id(column, USERLIST_COLUMN_OP);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ypad", 0, NULL);
        column = gtk_tree_view_column_new_with_attributes(_("Nick"),
							  renderer,
							  "text", USERLIST_COLUMN_NICK,
							  NULL);
	gtk_tree_view_column_set_sort_column_id(column, USERLIST_COLUMN_NICK);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(list), TRUE);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(list), USERLIST_COLUMN_NICK);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(list), TRUE);

	priv->item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);
        gtk_item_factory_set_translate_func(priv->item_factory, gtkutils_menu_translate,
                                            NULL, NULL);
        gtk_item_factory_create_items(priv->item_factory, G_N_ELEMENTS(popup_menu_items),
                                      popup_menu_items, list);
	priv->popup_menu = gtk_item_factory_get_widget(priv->item_factory, "<main>");

	g_signal_connect(G_OBJECT(list), "row_activated",
			 G_CALLBACK(nick_list_row_activated_cb), NULL);
	g_signal_connect(G_OBJECT(list), "button_press_event",
			 G_CALLBACK(nick_list_button_press_event_cb), NULL);
	
	return GTK_WIDGET(list);
}
void nick_list_set_store(NickList *list, GtkListStore *store)
{
        g_return_if_fail(list != NULL);
        g_return_if_fail(IS_NICK_LIST(list));
	
        gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
}
