/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2
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

#include "loqui_menu.h"
#include "loqui_app.h"
#include "account.h"
#include "about.h"
#include "account_manager.h"
#include "intl.h"
#include "gtkutils.h"
#include "channel_input_dialog.h"
#include "command_dialog.h"

struct _LoquiMenuPrivate
{
	LoquiApp *app;

	GtkAccelGroup *accel_group;      
	GtkItemFactory *item_factory;
};

static void loqui_menu_class_init(LoquiMenuClass *klass);
static void loqui_menu_init(LoquiMenu *loqui_menu);
static void loqui_menu_finalize(GObject *object);

static void loqui_menu_edit_activate_cb(GtkWidget *widget, gpointer data);

static void loqui_menu_connect_cb(gpointer data, guint callback_action, GtkWidget *widget);
static void loqui_menu_quit_cb(gpointer data, guint callback_action, GtkWidget *widget);

static void loqui_menu_edit_cb(gpointer data, guint callback_action, GtkWidget *widget);
static void loqui_menu_find_cb(gpointer data, guint callback_action, GtkWidget *widget);

static void loqui_menu_join_channel_cb(gpointer data, guint callback_action, GtkWidget *widget);
static void loqui_menu_part_channel_cb(gpointer data, guint callback_action, GtkWidget *widget);
static void loqui_menu_topic_cb(gpointer data, guint callback_action, GtkWidget *widget);

static void loqui_menu_view_toolbar_cb(gpointer data, guint callback_action, GtkWidget *widget);
static void loqui_menu_view_statusbar_cb(gpointer data, guint callback_action, GtkWidget *widget);
static void loqui_menu_view_move_cb(gpointer data, guint callback_action, GtkWidget *widget);

static void loqui_menu_account_settings_cb(gpointer data, guint callback_action, GtkWidget *widget);
static void loqui_menu_common_settings_cb(gpointer data, guint callback_action, GtkWidget *widget);

static void loqui_menu_about_cb(gpointer data, guint callback_action, GtkWidget *widget);

enum {
	LOQUI_MENU_CUT,
	LOQUI_MENU_COPY,
	LOQUI_MENU_PASTE,
	LOQUI_MENU_PASTE_WITH_LINEFEEDS_CUT,
	LOQUI_MENU_CLEAR
};

static GtkItemFactoryEntry menu_items[] = {
	{ N_("/_File"),     NULL, 0, 0, "<Branch>" },
	{ N_("/File/_Connect"), NULL, loqui_menu_connect_cb, 0 },
	{ N_("/File/_Quit"), "<control>Q", loqui_menu_quit_cb, 0, "<StockItem>", GTK_STOCK_QUIT },
	{ N_("/_Edit"),     NULL, 0, 0, "<Branch>" },
	{ N_("/Edit/Cut"),      NULL, loqui_menu_edit_cb, LOQUI_MENU_CUT, "<StockItem>", GTK_STOCK_CUT },
	{ N_("/Edit/_Copy"),    NULL, loqui_menu_edit_cb, LOQUI_MENU_COPY, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/Edit/_Paste"),   NULL, loqui_menu_edit_cb, LOQUI_MENU_PASTE, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/Edit/Paste with linefeeds cut"), NULL, loqui_menu_edit_cb, LOQUI_MENU_PASTE_WITH_LINEFEEDS_CUT,
	  "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/Edit/Clear"), NULL, loqui_menu_edit_cb,  LOQUI_MENU_CLEAR, "<StockItem>", GTK_STOCK_CLEAR },
	{ "/Edit/sep",        NULL,         0,       0, "<Separator>" },
	{ N_("/Edit/_Find"),    "<control>F", loqui_menu_find_cb, 0, "<StockItem>", GTK_STOCK_FIND },
	{ N_("/Edit/_Find again"), NULL, loqui_menu_find_cb, 1, "<StockItem>", GTK_STOCK_FIND },
	{ N_("/_Command"), NULL, 0, 0, "<Branch>" },
	{ N_("/Command/_Join channel"), "<Alt>J", loqui_menu_join_channel_cb, 0 },
	{ N_("/Command/_Part channel"), NULL, loqui_menu_part_channel_cb, 0, },
	{ "/Command/sep",        NULL,         0,       0, "<Separator>" },
	{ N_("/Command/_Set channel topic"), "<Alt>T", loqui_menu_topic_cb, 0, },
	{ N_("/_View"), NULL, 0, 0, "<Branch>" },
	{ N_("/View/Toolbar"), NULL, 0, 0, "<Branch>" },
	{ N_("/View/Toolbar/Icon"), NULL, loqui_menu_view_toolbar_cb, GTK_TOOLBAR_ICONS, "<RadioItem>" },
	{ N_("/View/Toolbar/Text"), NULL, loqui_menu_view_toolbar_cb, GTK_TOOLBAR_TEXT, "/View/Toolbar/Icon" },
	{ N_("/View/Toolbar/Both icons and text"), NULL, loqui_menu_view_toolbar_cb, GTK_TOOLBAR_BOTH, "/View/Toolbar/Icon" },
	{ N_("/View/Toolbar/Both icons and text horizontally"), NULL, loqui_menu_view_toolbar_cb, GTK_TOOLBAR_BOTH_HORIZ, "/View/Toolbar/Icon"},
	{ N_("/View/Toolbar/Hide"), NULL, loqui_menu_view_toolbar_cb, 100, "/View/Toolbar/Icon" },
	{ N_("/View/Status bar"), NULL, loqui_menu_view_statusbar_cb, 0, "<ToggleItem>" },
	{ "/View/sep",        NULL,         0,       0, "<Separator>" },
	{ N_("/View/Previous updated channel"), "<Alt>P", loqui_menu_view_move_cb, 0, "<StockItem>", GTK_STOCK_GO_UP},
	{ N_("/View/Next updated channel"), "<Alt>N", loqui_menu_view_move_cb, 1, "<StockItem>", GTK_STOCK_GO_DOWN},
	{ "/View/sep2",        NULL,         0,       0, "<Separator>" },
	{ N_("/View/Previous channel"), "<Control>P", loqui_menu_view_move_cb, 2, "<StockItem>", GTK_STOCK_GO_UP},
	{ N_("/View/Next channel"), "<Control>N", loqui_menu_view_move_cb, 3, "<StockItem>", GTK_STOCK_GO_DOWN},
	{ N_("/_Settings"), NULL, 0, 0, "<Branch>" },
	{ N_("/Settings/Common preferences..."), NULL, loqui_menu_common_settings_cb, 0 },
	{ N_("/Settings/Account settings..."), NULL, loqui_menu_account_settings_cb, 0 },
	{ N_("/_Help"), NULL, 0, 0, "<Branch>" },
	{ N_("/Help/_About"), NULL, loqui_menu_about_cb, 0 },
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

GType
loqui_menu_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiMenuClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_menu_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiMenu),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_menu_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiMenu",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_menu_class_init (LoquiMenuClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_menu_finalize;
}
static void 
loqui_menu_init (LoquiMenu *loqui_menu)
{
	LoquiMenuPrivate *priv;

	priv = g_new0(LoquiMenuPrivate, 1);

	loqui_menu->priv = priv;
}
static void 
loqui_menu_finalize (GObject *object)
{
	LoquiMenu *menu;

        g_return_if_fail (object != NULL);
        g_return_if_fail (LOQUI_IS_MENU (object));

        menu = LOQUI_MENU(object);

	if(menu->priv->item_factory) {
		g_object_unref(menu->priv->item_factory);
		menu->priv->item_factory = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(menu->priv);
}

static void
loqui_menu_edit_activate_cb(GtkWidget *widget, gpointer data)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;
	gboolean cutable, copiable, pastable, clearable, findable;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	loqui_app_get_current_widget_editing_status(priv->app, &cutable, &copiable, &pastable, &clearable, &findable);

#define SET_SENSITIVE_FACTORY_ITEM(bool, name) { \
   if(bool) \
        gtk_widget_set_sensitive(gtk_item_factory_get_item(priv->item_factory, name), TRUE); \
   else \
        gtk_widget_set_sensitive(gtk_item_factory_get_item(priv->item_factory, name), FALSE); \
}
	SET_SENSITIVE_FACTORY_ITEM(cutable, "/Edit/Cut");
	SET_SENSITIVE_FACTORY_ITEM(pastable, "/Edit/Paste");
	SET_SENSITIVE_FACTORY_ITEM(FALSE, "/Edit/Paste with linefeeds cut");
	
	SET_SENSITIVE_FACTORY_ITEM(copiable, "/Edit/Copy");
	SET_SENSITIVE_FACTORY_ITEM(clearable, "/Edit/Clear");

	SET_SENSITIVE_FACTORY_ITEM(FALSE, "/Edit/Find");
	SET_SENSITIVE_FACTORY_ITEM(FALSE, "/Edit/Find again");
}

LoquiMenu*
loqui_menu_new(LoquiApp *app)
{
        LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = g_object_new(loqui_menu_get_type(), NULL);
	g_return_val_if_fail(menu != NULL, NULL);
	
	priv = menu->priv;

	priv->app = app;

	priv->accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(app), priv->accel_group);
	g_object_unref(priv->accel_group);
      
	priv->item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", priv->accel_group);
	gtk_item_factory_set_translate_func(priv->item_factory, gtkutils_menu_translate,
                                            NULL, NULL);

	gtk_item_factory_create_items(priv->item_factory, G_N_ELEMENTS(menu_items),
				      menu_items, menu);

	g_signal_connect(G_OBJECT(gtk_item_factory_get_item(priv->item_factory, "/Edit")), "activate",
			 G_CALLBACK(loqui_menu_edit_activate_cb), menu);

	return menu;
}
GtkWidget* loqui_menu_get_widget(LoquiMenu *menu)
{
	g_return_val_if_fail(menu != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_MENU(menu), NULL);

	return gtk_item_factory_get_widget(menu->priv->item_factory, "<main>");
}


static void
loqui_menu_edit_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	switch(callback_action) {
	case LOQUI_MENU_CUT:
		loqui_app_current_widget_cut(priv->app);
		break;
	case LOQUI_MENU_COPY:
		loqui_app_current_widget_copy(priv->app);
		break;
	case LOQUI_MENU_PASTE:
		loqui_app_current_widget_paste(priv->app);
		break;
	case LOQUI_MENU_CLEAR:
		loqui_app_current_widget_clear(priv->app);
		break;
	default:
		break;
	}
}
static void
loqui_menu_find_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	if(callback_action == 0) {
		loqui_app_current_widget_find(priv->app);
	} else {
		loqui_app_current_widget_find_next(priv->app);
	}
}

static void
loqui_menu_about_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	about_open();
}

static void
loqui_menu_account_settings_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	account_manager_open_account_list_dialog(account_manager_get());
}
static void
loqui_menu_common_settings_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	account_manager_open_prefs_dialog(account_manager_get());
}
static void
loqui_menu_quit_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	gtk_main_quit();
}

static void loqui_menu_connect_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	account_manager_open_connect_dialog(account_manager_get());
}

static void loqui_menu_view_toolbar_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	loqui_app_set_toolbar_style(priv->app, callback_action);
}

static void loqui_menu_view_statusbar_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));
	g_return_if_fail(GTK_IS_CHECK_MENU_ITEM(widget));

	priv = menu->priv;

	loqui_app_set_show_statusbar(priv->app, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)));
}
static void loqui_menu_view_move_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	switch(callback_action) {
	case 0:
		channel_tree_select_prev_channel(priv->app->channel_tree, TRUE);
		break;
	case 1:
		channel_tree_select_next_channel(priv->app->channel_tree, TRUE);
		break;
	case 2:
		channel_tree_select_prev_channel(priv->app->channel_tree, FALSE);
		break;
	case 3:
		channel_tree_select_next_channel(priv->app->channel_tree, FALSE);
		break;
	default:
		break;
	}
}
void loqui_menu_set_view_toolbar(LoquiMenu *menu, guint style)
{
	LoquiMenuPrivate *priv;
	GtkWidget *item;

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	switch(style) {
	case GTK_TOOLBAR_ICONS:
		item = gtk_item_factory_get_item(priv->item_factory, "/View/Toolbar/Icon");
		break;
	case GTK_TOOLBAR_TEXT:
		item = gtk_item_factory_get_item(priv->item_factory, "/View/Toolbar/Text");
		break;
	case GTK_TOOLBAR_BOTH:
		item = gtk_item_factory_get_item(priv->item_factory, "/View/Toolbar/Both icons and text");
		break;
	case GTK_TOOLBAR_BOTH_HORIZ:
		item = gtk_item_factory_get_item(priv->item_factory, "/View/Toolbar/Both icons and text horizontally");
		break;
	default:
		item = gtk_item_factory_get_item(priv->item_factory, "/View/Toolbar/Hide");
		break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
}
void loqui_menu_set_view_statusbar(LoquiMenu *menu, gboolean show)
{
	LoquiMenuPrivate *priv;
	GtkWidget *item;

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	item = gtk_item_factory_get_item(priv->item_factory, "/View/Status bar");

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), show);
}
static void
loqui_menu_join_channel_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	command_dialog_join(GTK_WINDOW(priv->app),
			    account_manager_get_current_account(account_manager_get()));
}
static void loqui_menu_part_channel_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;
	AccountManager *manager;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	manager = account_manager_get();
	command_dialog_part(GTK_WINDOW(priv->app),
			    account_manager_get_current_account(manager),
			    account_manager_get_current_channel(manager));
}
static void loqui_menu_topic_cb(gpointer data, guint callback_action, GtkWidget *widget)
{
	LoquiMenu *menu;
	LoquiMenuPrivate *priv;
	AccountManager *manager;

	menu = LOQUI_MENU(data);

	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));

	priv = menu->priv;

	manager = account_manager_get();
	command_dialog_topic(GTK_WINDOW(priv->app),
			     account_manager_get_current_account(manager),
			     account_manager_get_current_channel(manager));

}
