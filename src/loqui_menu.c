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

#include "loqui_menu.h"
#include "loqui_app.h"
#include "account.h"
#include "about.h"

struct _LoquiMenuPrivate
{
	GtkWidget *connect_menu;
};

static void loqui_menu_class_init(LoquiMenuClass *klass);
static void loqui_menu_init(LoquiMenu *loqui_menu);
static void loqui_menu_finalize(GObject *object);

static void loqui_menu_about_cb(GtkWidget *widget, gpointer data);
static void loqui_menu_quit_cb(GtkWidget *widget, gpointer data);
static void loqui_menu_connect_cb(GtkWidget *widget, gpointer data);

#define MENU_NUMBER_CONNECT 1

static GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_ITEM_NONE(N_("_Connect All"), N_("Connect to IRC server marked"), NULL),
        GNOMEUIINFO_ITEM_NONE(N_("_Connect"), N_("Connect to IRC server"), NULL),
	GNOMEUIINFO_MENU_QUIT_ITEM(loqui_menu_quit_cb, NULL),
	GNOMEUIINFO_END
};
static GnomeUIInfo edit_menu[] = {
	GNOMEUIINFO_MENU_CUT_ITEM(NULL, NULL),
	GNOMEUIINFO_MENU_COPY_ITEM(NULL, NULL),
	GNOMEUIINFO_MENU_PASTE_ITEM(NULL, NULL),
	GNOMEUIINFO_ITEM_STOCK(N_("Paste with linefeeds cut"), N_("Paste selection with linefeeds cut"),
			       NULL, GNOME_STOCK_PIXMAP_CLEAR),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_FIND_ITEM(NULL, NULL),
	GNOMEUIINFO_MENU_FIND_AGAIN_ITEM(NULL, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_ITEM_STOCK(N_("Clear buffer"), N_("Clear buffer of current channel"),
			       NULL, GNOME_STOCK_PIXMAP_CLEAR),
	GNOMEUIINFO_ITEM_STOCK(N_("Clear common buffer"), N_("Clear common buffer"),
			       NULL, GNOME_STOCK_PIXMAP_CLEAR),
	GNOMEUIINFO_END
};
/* static GnomeUIInfo server_menu[] = {
	GNOMEUIINFO_ITEM_NONE(N_("_Disconnect"), N_("Disconnect to current server"), NULL),
	GNOMEUIINFO_END
};
static GnomeUIInfo channel_menu[] = {
	GNOMEUIINFO_END
};
static GnomeUIInfo user_menu[] = {
	GNOMEUIINFO_END
	}; */
static GnomeUIInfo settings_menu[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM(NULL, NULL),
	GNOMEUIINFO_END
};
static GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_MENU_ABOUT_ITEM(loqui_menu_about_cb, NULL),
	GNOMEUIINFO_END
};
static GnomeUIInfo menubar[] = {
	GNOMEUIINFO_MENU_FILE_TREE(file_menu),
	GNOMEUIINFO_MENU_EDIT_TREE(edit_menu),
/*	GNOMEUIINFO_SUBTREE(N_("Ser_ver"), server_menu),
	GNOMEUIINFO_SUBTREE(N_("_Channel"), channel_menu),
	GNOMEUIINFO_SUBTREE(N_("_User"), user_menu), */
	GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
	GNOMEUIINFO_END
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

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(menu->priv);
}
LoquiMenu*
loqui_menu_new (void)
{
        LoquiMenu *menu;

	menu = g_object_new(loqui_menu_get_type(), NULL);
	g_return_val_if_fail(menu != NULL, NULL);
	
	return menu;
}
void
loqui_menu_attach(LoquiMenu *menu, GnomeApp *app)
{
	g_return_if_fail(menu != NULL);
        g_return_if_fail(LOQUI_IS_MENU(menu));
	g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	
        gnome_app_create_menus_with_data(app, menubar, app);
	gnome_app_install_appbar_menu_hints(GNOME_APPBAR(GNOME_APP(app)->statusbar), menubar);
	menu->priv->connect_menu = file_menu[MENU_NUMBER_CONNECT].widget;
	
	gtk_widget_set_sensitive(file_menu[0].widget, FALSE);
	gtk_widget_set_sensitive(menubar[1].widget, FALSE);
	gtk_widget_set_sensitive(menubar[2].widget, FALSE);	
}

void loqui_menu_create_connect_submenu(LoquiMenu *menu, GSList *account_list)
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

        gtk_widget_set_sensitive (priv->connect_menu, added);
}

static void loqui_menu_about_cb(GtkWidget *widget, gpointer data)
{
	about_open(data);
}

static void loqui_menu_quit_cb(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static void loqui_menu_connect_cb(GtkWidget *widget, gpointer data)
{
	Account *account;
	
	account = ACCOUNT(data);
	account_connect(account, 0, TRUE);
}
