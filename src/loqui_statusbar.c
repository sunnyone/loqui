/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
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

#include "loqui_statusbar.h"
#include "intl.h"
#include "loqui_stock.h"
#include "gtkutils.h"
#include "account_manager.h"
#include "main.h"
#include "loqui_dropdown_box.h"

#include <string.h>

#define STRING_DISCONNECTED _("(disconnected)")
#define STRING_UNSELECTED _("(unselected)")

enum {
	NICK_CHANGE,
	NICK_SELECTED,
	AWAY_SELECTED,
        LAST_SIGNAL
};

struct _LoquiStatusbarPrivate
{
	LoquiApp *app;
	
	GtkWidget *label_account;

	GtkWidget *image_away;
	GtkWidget *dbox_away;

	GtkWidget *button_nick;
	GtkWidget *label_nick;
	GtkWidget *dbox_preset;

	GtkWidget *progress_lag;
	GtkWidget *toggle_scroll;
	
	GtkWidget *menu_away;
	GtkWidget *menu_preset;
	
	guint toggle_scroll_handler_id;	
};

static GtkStatusbarClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_STATUSBAR

static guint loqui_statusbar_signals[LAST_SIGNAL] = { 0 };

static void loqui_statusbar_class_init(LoquiStatusbarClass *klass);
static void loqui_statusbar_init(LoquiStatusbar *statusbar);
static void loqui_statusbar_finalize(GObject *object);
static void loqui_statusbar_destroy(GtkObject *object);

static void loqui_statusbar_set_preset_menu(LoquiStatusbar *statusbar, GList *nick_list);

static void loqui_statusbar_set_away_state(LoquiStatusbar *statusbar, AwayState away_state);
static void loqui_statusbar_away_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar);
static void loqui_statusbar_set_away_menu(LoquiStatusbar *statusbar);

static void loqui_statusbar_nick_button_clicked_cb(GtkWidget *widget, LoquiStatusbar *statusbar);
static void loqui_statusbar_toggle_scrolling_cb(GtkWidget *widget, LoquiStatusbar *statusbar);

GType
loqui_statusbar_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiStatusbarClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_statusbar_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiStatusbar),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_statusbar_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiStatusbar",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_statusbar_class_init (LoquiStatusbarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);


        loqui_statusbar_signals[NICK_CHANGE] = g_signal_new("nick-change",
							    G_OBJECT_CLASS_TYPE(object_class),
							    G_SIGNAL_RUN_FIRST,
							    0,
						            NULL, NULL,
							    g_cclosure_marshal_VOID__VOID,
							    G_TYPE_NONE, 0);
        loqui_statusbar_signals[NICK_SELECTED] = g_signal_new("nick-selected",
						  	      G_OBJECT_CLASS_TYPE(object_class),
						   	      G_SIGNAL_RUN_FIRST,
						   	      0,
						              NULL, NULL,
						   	      g_cclosure_marshal_VOID__STRING,
						              G_TYPE_NONE, 1,
						   	      G_TYPE_STRING);
        loqui_statusbar_signals[AWAY_SELECTED] = g_signal_new("away-selected",
						  	      G_OBJECT_CLASS_TYPE(object_class),
						   	      G_SIGNAL_RUN_FIRST,
						   	      0,
						              NULL, NULL,
						   	      g_cclosure_marshal_VOID__STRING,
						              G_TYPE_NONE, 1,
						   	      G_TYPE_INT);
						   	      
	object_class->finalize = loqui_statusbar_finalize;
        gtk_object_class->destroy = loqui_statusbar_destroy;
}
static void 
loqui_statusbar_init (LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;

	priv = g_new0(LoquiStatusbarPrivate, 1);

	statusbar->priv = priv;
}
static void 
loqui_statusbar_finalize (GObject *object)
{
	LoquiStatusbar *statusbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(object));

        statusbar = LOQUI_STATUSBAR(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(statusbar->priv);
}
static void 
loqui_statusbar_destroy (GtkObject *object)
{
        LoquiStatusbar *statusbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(object));

        statusbar = LOQUI_STATUSBAR(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
loqui_statusbar_set_away_state(LoquiStatusbar *statusbar, AwayState away_state)
{
	LoquiStatusbarPrivate *priv;
	const gchar *stock_id;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;	
		
	switch (away_state) {
	case AWAY_STATE_ONLINE:
		stock_id = LOQUI_STOCK_ONLINE;
		break;
	case AWAY_STATE_AWAY:
		stock_id = LOQUI_STOCK_AWAY;
		break;
	case AWAY_STATE_BUSY:
		stock_id = LOQUI_STOCK_BUSY;
		break;		
	case AWAY_STATE_OFFLINE:
		stock_id = LOQUI_STOCK_OFFLINE;
		break;
	default:
		g_warning("Invalid AwayState.");
		return;
	}
	gtk_image_set_from_stock(GTK_IMAGE(priv->image_away), stock_id, LOQUI_ICON_SIZE_FONT);
}
static void
loqui_statusbar_preset_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
	const gchar *str;

	g_return_if_fail(GTK_IS_MENU_ITEM(widget));
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

        priv = statusbar->priv;		

	str = g_object_get_data(G_OBJECT(widget), "nick");
	g_signal_emit(statusbar, loqui_statusbar_signals[NICK_SELECTED], 0, str);
}
static void
loqui_statusbar_away_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
	gint away_state;

	g_return_if_fail(GTK_IS_MENU_ITEM(widget));
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

        priv = statusbar->priv;		

	away_state = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "away-state"));
	g_signal_emit(statusbar, loqui_statusbar_signals[AWAY_SELECTED], 0, away_state);
}

static void
loqui_statusbar_set_preset_menu(LoquiStatusbar *statusbar, GList *nick_list)
{
	LoquiStatusbarPrivate *priv;
	GtkWidget *menuitem;
	GList *cur, *tmp_list = NULL;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;	
     
	for (cur = GTK_MENU_SHELL(priv->menu_preset)->children; cur != NULL; cur = cur->next) {
		tmp_list = g_list_append(tmp_list, cur->data);
	}
	for (cur = tmp_list; cur != NULL; cur = cur->next) {
		gtk_container_remove(GTK_CONTAINER(priv->menu_preset), cur->data);
	}
	for (cur = nick_list; cur != NULL; cur = cur->next) {
		if (!cur->data)
			continue;
		
		menuitem = gtk_menu_item_new_with_label(cur->data);
		g_object_set_data_full(G_OBJECT(menuitem), "nick", g_strdup(cur->data), (GDestroyNotify) g_free);
		g_signal_connect(G_OBJECT(menuitem), "activate",
				 G_CALLBACK(loqui_statusbar_preset_menuitem_activated_cb), statusbar);
		gtk_widget_show(menuitem);
		gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_preset), menuitem);
	}
}
static void
loqui_statusbar_set_away_menu(LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
	GtkWidget *menuitem;
	GtkWidget *image;
	GList *cur, *tmp_list = NULL;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;	
     
	for (cur = GTK_MENU_SHELL(priv->menu_preset)->children; cur != NULL; cur = cur->next) {
		tmp_list = g_list_append(tmp_list, cur->data);
	}
	for (cur = tmp_list; cur != NULL; cur = cur->next) {
		gtk_container_remove(GTK_CONTAINER(priv->menu_preset), cur->data);
	}

#define ADD_MENU_ITEM(title, stock_id, state, usable) { \
	menuitem = gtk_image_menu_item_new_with_label(title); \
	image = gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU); \
	gtk_widget_show(image); \
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image); \
	g_object_set_data(G_OBJECT(menuitem), "away-state", GINT_TO_POINTER(state)); \
	g_signal_connect(G_OBJECT(menuitem), "activate", \
			 G_CALLBACK(loqui_statusbar_away_menuitem_activated_cb), statusbar); \
	gtk_widget_show(menuitem); \
	gtk_widget_set_sensitive(menuitem, usable); \
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_away), menuitem); \
}

	ADD_MENU_ITEM(_("Online"), LOQUI_STOCK_ONLINE, AWAY_STATE_ONLINE, TRUE);
	
	menuitem = gtk_separator_menu_item_new();
	gtk_widget_show(menuitem);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_away), menuitem);
		
	ADD_MENU_ITEM(_("Away"), LOQUI_STOCK_AWAY, AWAY_STATE_AWAY, TRUE);
	/*
	ADD_MENU_ITEM(_("Busy"), LOQUI_STOCK_BUSY, AWAY_STATE_BUSY, FALSE);
	ADD_MENU_ITEM(_("Away message..."), LOQUI_STOCK_AWAY, AWAY_STATE_AWAY_WITH_MESSAGE, FALSE);
	ADD_MENU_ITEM(_("Configure away messages..."), GTK_STOCK_PREFERENCES, AWAY_STATE_CONFIGURE, FALSE);
	
	menuitem = gtk_separator_menu_item_new();
	gtk_widget_show(menuitem);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_away), menuitem);
	
	ADD_MENU_ITEM(_("Quit..."), LOQUI_STOCK_OFFLINE, AWAY_STATE_QUIT, FALSE);
	ADD_MENU_ITEM(_("Offline"), LOQUI_STOCK_OFFLINE, AWAY_STATE_OFFLINE, FALSE);
	ADD_MENU_ITEM(_("Disconnect"), LOQUI_STOCK_OFFLINE, AWAY_STATE_DISCONNECT, FALSE);
	*/
}
static void
loqui_statusbar_nick_button_clicked_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

	priv = statusbar->priv;
	
	g_signal_emit(statusbar, loqui_statusbar_signals[NICK_CHANGE], 0);
}
static void
loqui_statusbar_toggle_scrolling_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	gboolean is_active;
	LoquiStatusbarPrivate *priv;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

	priv = statusbar->priv;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	account_manager_set_whether_scrolling(account_manager_get(), 
					      is_active);

	if(is_active)
		loqui_app_scroll_channel_buffer(priv->app);
}

GtkWidget*
loqui_statusbar_new(LoquiApp *app)
{
        LoquiStatusbar *statusbar;
	LoquiStatusbarPrivate *priv;
	GtkWidget *hsep;
	
	statusbar = g_object_new(loqui_statusbar_get_type(), NULL);
	priv = statusbar->priv;
	
	priv->app = app;
	
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(statusbar), FALSE);
	gtk_label_set_selectable(GTK_LABEL(GTK_STATUSBAR(statusbar)->label), TRUE);
	
	priv->image_away = gtk_image_new();

/* FIXME: why statusbar becomes taller when button widget is on it? */
#define WIDGET_MINIMIZE_HEIGHT(widget) gtk_widget_set_usize(widget, -1, 1);

	priv->dbox_away = loqui_dropdown_box_new(priv->image_away);
	WIDGET_MINIMIZE_HEIGHT(priv->dbox_away);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->dbox_away, FALSE, FALSE, 0);
	gtk_button_set_relief(GTK_BUTTON(LOQUI_DROPDOWN_BOX(priv->dbox_away)->drop_button), GTK_RELIEF_NONE);
	
	priv->dbox_preset = loqui_dropdown_box_new(NULL);
	WIDGET_MINIMIZE_HEIGHT(priv->dbox_preset);
	gtk_button_set_relief(GTK_BUTTON(LOQUI_DROPDOWN_BOX(priv->dbox_preset)->drop_button), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->dbox_preset, FALSE, FALSE, 0);

	priv->button_nick = gtk_button_new();
	WIDGET_MINIMIZE_HEIGHT(priv->button_nick);
	g_signal_connect(G_OBJECT(priv->button_nick), "clicked",
			 G_CALLBACK(loqui_statusbar_nick_button_clicked_cb), statusbar);
	gtk_button_set_relief(GTK_BUTTON(priv->button_nick), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(priv->dbox_preset), priv->button_nick, FALSE, FALSE, 0);

	priv->label_nick = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(priv->button_nick), priv->label_nick);

	hsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), hsep, FALSE, FALSE, 2);

	priv->label_account = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(statusbar), priv->label_account, FALSE, FALSE, 0);

	priv->progress_lag = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(statusbar), priv->progress_lag, FALSE, FALSE, 0);
	gtk_widget_set_size_request(priv->progress_lag, 70, -1);

	priv->toggle_scroll = gtk_toggle_button_new_with_mnemonic("Scroll");
	WIDGET_MINIMIZE_HEIGHT(priv->toggle_scroll);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->toggle_scroll, FALSE, FALSE, 0);
	
	priv->menu_preset = gtk_menu_new();
	loqui_dropdown_box_set_menu(LOQUI_DROPDOWN_BOX(priv->dbox_preset), priv->menu_preset);

	priv->menu_away = gtk_menu_new();
	loqui_dropdown_box_set_menu(LOQUI_DROPDOWN_BOX(priv->dbox_away), priv->menu_away);

	loqui_statusbar_set_away_menu(statusbar);
	
	priv->toggle_scroll_handler_id = g_signal_connect(G_OBJECT(priv->toggle_scroll),
							 "toggled",
							 G_CALLBACK(loqui_statusbar_toggle_scrolling_cb),
							 statusbar);
	
	return GTK_WIDGET(statusbar);
}

void
loqui_statusbar_set_current_channel(LoquiStatusbar *statusbar, Channel *channel)
{
	LoquiStatusbarPrivate *priv;
	
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
    
    	priv = statusbar->priv;
}
void
loqui_statusbar_set_current_account(LoquiStatusbar *statusbar, Account *account)
{
	LoquiStatusbarPrivate *priv;
	AwayState away_state;
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;
        
        // set account label
        gtk_label_set(GTK_LABEL(priv->label_account),
        	       account ? loqui_profile_account_get_name(account_get_profile(account)) : STRING_UNSELECTED);

	// set nick        
        if (account == NULL) {
        	gtk_widget_set_sensitive(priv->dbox_preset, FALSE);
        	gtk_widget_set_sensitive(priv->dbox_away, FALSE);        	
        	gtk_label_set(GTK_LABEL(priv->label_nick), STRING_UNSELECTED);
        } else if (!account_is_connected(account)) {
      	 	gtk_widget_set_sensitive(priv->dbox_preset, FALSE);
        	gtk_widget_set_sensitive(priv->dbox_away, FALSE);
        	gtk_label_set(GTK_LABEL(priv->label_nick), STRING_DISCONNECTED);
        } else {
        	gtk_widget_set_sensitive(priv->button_nick, TRUE);
        	gtk_widget_set_sensitive(priv->dbox_preset, TRUE);
        	gtk_widget_set_sensitive(priv->dbox_away, TRUE);
        	gtk_label_set(GTK_LABEL(priv->label_nick), account_get_current_nick(account));
        }
        
        if (account) 
        	loqui_statusbar_set_preset_menu(statusbar, loqui_profile_account_get_nick_list(account_get_profile(account)));
        
        // set icon
        if (account == NULL) {
        	away_state = AWAY_STATE_OFFLINE;
        } else if (!account_is_connected(account)) {
        	away_state = AWAY_STATE_OFFLINE;
        } else if (account_get_away_status(account)) {
        	away_state = AWAY_STATE_AWAY;
        } else {
        	away_state = AWAY_STATE_ONLINE;
        }
        
        loqui_statusbar_set_away_state(statusbar, away_state);
}
void
loqui_statusbar_set_default(LoquiStatusbar *statusbar, const gchar *str)
{
	guint context_id;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        	
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "Default");
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar), context_id);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), context_id, str);
}
void
loqui_statusbar_toggle_scrolling_with_signal_handler_blocked(LoquiStatusbar *statusbar, gboolean is_scroll)
{
	LoquiStatusbarPrivate *priv;
	
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
    
    	priv = statusbar->priv;
    	
	gtkutils_toggle_button_with_signal_handler_blocked(GTK_TOGGLE_BUTTON(priv->toggle_scroll),
							   priv->toggle_scroll_handler_id,
							   is_scroll);
}
