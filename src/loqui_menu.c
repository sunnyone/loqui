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

struct _LoquiMenuPrivate
{
	GtkAccelGroup *accel_group;      
	GtkItemFactory *item_factory;

	GtkWidget *connect_menu;
};

static void loqui_menu_class_init(LoquiMenuClass *klass);
static void loqui_menu_init(LoquiMenu *loqui_menu);
static void loqui_menu_finalize(GObject *object);

static void loqui_menu_about_cb(GtkWidget *widget, guint callback_action, gpointer data);
static void loqui_menu_quit_cb(GtkWidget *widget, guint callback_action, gpointer data);
static void loqui_menu_connect_cb(GtkWidget *widget, gpointer data);
static void loqui_menu_account_settings_cb(GtkWidget *widget, guint callback_action, gpointer data);

static gchar *loqui_menu_translate(const gchar *path, gpointer data);

#define MENU_NUMBER_CONNECT 1

static GtkItemFactoryEntry menu_items[] = {
	{ N_("/_File"),     NULL, 0, 0, "<Branch>" },
	{ N_("/File/_Connect"), NULL, 0, 0, "<Branch>" },
	{ N_("/File/_Quit"), "<control>Q", loqui_menu_quit_cb, 0, "<StockItem>", GTK_STOCK_QUIT },
	{ N_("/_Edit"),     NULL, 0, 0, "<Branch>" },
	{ N_("/Edit/Cut"),      "<control>X", 0, 0, "<StockItem>", GTK_STOCK_CUT },
	{ N_("/Edit/_Copy"),    "<control>C", 0, 0, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/Edit/_Paste"),   "<control>P", 0, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/Edit/Paste with linefeeds cut"), NULL, 0, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/Edit/_Find"),    "<control>F", 0, 0, "<StockItem>", GTK_STOCK_FIND },
	{ N_("/Edit/_Find again"), "<control>N", 0, 0, "<StockItem>", GTK_STOCK_FIND },
	{ "/Edit/sep2",        NULL,         0,       0, "<Separator>" },
	{ N_("/Edit/Clear _buffer"), NULL,      0,   0, "<StockItem>", GTK_STOCK_CLEAR },
	{ N_("/Edit/Clear co_mmon buffer"), NULL, 0, 0, "<StockItem>" , GTK_STOCK_CLEAR },
	{ N_("/_Server"), NULL, 0, 0, "<Branch>" },
	{ N_("/_Channel"), NULL, 0, 0, "<Branch>" },
	{ N_("/_User"), NULL, 0, 0, "<Branch>" },
	{ N_("/_View"), NULL, 0, 0, "<Branch>" },
	{ N_("/_Settings"), NULL, 0, 0, "<Branch>" },
	{ N_("/Settings/Account Settings..."), NULL, loqui_menu_account_settings_cb, 0 },
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
static gchar *
loqui_menu_translate(const gchar *path, gpointer data)
{
	return gettext(path);
}

LoquiMenu*
loqui_menu_new(GtkWindow *window)
{
        LoquiMenu *menu;
	LoquiMenuPrivate *priv;

	menu = g_object_new(loqui_menu_get_type(), NULL);
	g_return_val_if_fail(menu != NULL, NULL);
	
	priv = menu->priv;

	priv->accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(window, priv->accel_group);
	g_object_unref(priv->accel_group);
      
	priv->item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", priv->accel_group);
	gtk_item_factory_set_translate_func(priv->item_factory, loqui_menu_translate,
                                            NULL, NULL);

	gtk_item_factory_create_items(priv->item_factory, G_N_ELEMENTS(menu_items),
				      menu_items, window);
	gtk_widget_set_sensitive(gtk_item_factory_get_item(priv->item_factory, "/Edit"), FALSE);
	gtk_widget_set_sensitive(gtk_item_factory_get_item(priv->item_factory, "/User"), FALSE);
	gtk_widget_set_sensitive(gtk_item_factory_get_item(priv->item_factory, "/Channel"), FALSE);
	gtk_widget_set_sensitive(gtk_item_factory_get_item(priv->item_factory, "/Server"), FALSE);
	gtk_widget_set_sensitive(gtk_item_factory_get_item(priv->item_factory, "/View"), FALSE);

	priv->connect_menu = gtk_item_factory_get_item(priv->item_factory, "/File/Connect");

	return menu;
}
GtkWidget* loqui_menu_get_widget(LoquiMenu *menu)
{
	g_return_val_if_fail(menu != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_MENU(menu), NULL);

	return gtk_item_factory_get_widget(menu->priv->item_factory, "<main>");
}

void loqui_menu_update_connect_submenu(LoquiMenu *menu, GSList *account_list)
{
	LoquiMenuPrivate *priv;
	GSList *cur;
	GtkWidget *submenu = NULL;
	GtkWidget *item;
	gboolean added = FALSE;
	Account *account;

	priv = menu->priv;
	
	if (GTK_MENU_ITEM(priv->connect_menu)->submenu) 
                gtk_menu_item_remove_submenu (GTK_MENU_ITEM (priv->connect_menu));

	for(cur = account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);
		if(!submenu) {
			submenu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(priv->connect_menu), submenu);
		}
		item = gtk_menu_item_new_with_label(account->name);
		g_signal_connect(G_OBJECT(item), "activate",
				 G_CALLBACK(loqui_menu_connect_cb), account);
		gtk_menu_append(GTK_MENU(submenu), item);
		gtk_widget_show_all(item);

		added = TRUE;
	}

        gtk_widget_set_sensitive(priv->connect_menu, added);
}

static void
loqui_menu_about_cb(GtkWidget *widget, guint callback_action, gpointer data)
{
	about_open();
}

static void
loqui_menu_account_settings_cb(GtkWidget *widget, guint callback_action, gpointer data)
{
	account_manager_open_account_list_dialog(account_manager_get());
}

static void
loqui_menu_quit_cb(GtkWidget *widget, guint callback_action, gpointer data)
{
	gtk_main_quit();
}

static void loqui_menu_connect_cb(GtkWidget *widget, gpointer data)
{
	Account *account;
	
	account = ACCOUNT(data);
	account_connect(account, 0, TRUE);
}

