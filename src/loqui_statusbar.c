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
#include <glib/gi18n.h>
#include "loqui_stock.h"
#include "gtkutils.h"
#include <loqui_account_manager.h>
#include "main.h"
#include "loqui_dropdown_box.h"
#include "command_dialog.h"
#include "loqui_sender.h"

#include <string.h>

#define STRING_DISCONNECTED _("(disconnected)")
#define STRING_UNSELECTED _("(unselected)")

enum {
        LAST_SIGNAL
};

struct _LoquiStatusbarPrivate
{
	LoquiApp *app;
	
	GtkWidget *label_account;

	GtkWidget *image_away;
	GtkWidget *label_away;
	GtkWidget *button_away;

	GtkWidget *dbox_away;

	GtkWidget *button_nick;
	GtkWidget *label_nick;
	GtkWidget *dbox_preset;

	GtkWidget *progress;
	GtkWidget *toggle_scroll_common_buffer;
	
	GtkWidget *menu_away;
	GtkWidget *menu_preset;
	
	guint toggle_scroll_handler_id;	
};

static GtkStatusbarClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_STATUSBAR

/* static guint loqui_statusbar_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_statusbar_class_init(LoquiStatusbarClass *klass);
static void loqui_statusbar_init(LoquiStatusbar *statusbar);
static void loqui_statusbar_finalize(GObject *object);
static void loqui_statusbar_destroy(GtkObject *object);

static void loqui_statusbar_set_preset_menu(LoquiStatusbar *statusbar, GList *nick_list);

static void loqui_statusbar_set_away(LoquiStatusbar *statusbar, LoquiUserClass *user_class, LoquiAwayType away);
static void loqui_statusbar_away_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar);
static void loqui_statusbar_set_away_menu(LoquiStatusbar *statusbar, LoquiUserClass *user_class);

static void loqui_statusbar_nick_button_clicked_cb(GtkWidget *widget, LoquiStatusbar *statusbar);

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
loqui_statusbar_set_away(LoquiStatusbar *statusbar, LoquiUserClass *user_class, LoquiAwayType away)
{
	LoquiStatusbarPrivate *priv;
	LoquiAwayInfo *awinfo;
	const gchar *stock_id;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;	
	
	awinfo = loqui_user_class_away_type_get_info(user_class, away);
	if (awinfo == NULL)
		return;
	
	gtk_label_set(GTK_LABEL(priv->label_away), awinfo->nick);

	stock_id = loqui_stock_get_id_from_basic_away_type(awinfo->basic_away_type);
	if (stock_id == NULL) {
		g_warning("Invalid BasicAwayType.");
		return;
	}

	gtk_image_set_from_stock(GTK_IMAGE(priv->image_away), stock_id, LOQUI_ICON_SIZE_FONT);
}
static void
loqui_statusbar_preset_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
	const gchar *nick;
	LoquiAccount *account;

	g_return_if_fail(GTK_IS_MENU_ITEM(widget));
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

        priv = statusbar->priv;		

	nick = g_object_get_data(G_OBJECT(widget), "nick");

	account = loqui_app_get_current_account(priv->app);
	if (account)
		if (loqui_account_get_is_connected(account))
			loqui_sender_nick(loqui_account_get_sender(account), nick);
	        else
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("The account is not connected!"));
	else
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected an account!"));	
}
static void
loqui_statusbar_away_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
	LoquiAwayType away_type;
	LoquiAccount *account;

	g_return_if_fail(GTK_IS_MENU_ITEM(widget));
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

        priv = statusbar->priv;		

	away_type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "away-type"));

	account = loqui_app_get_current_account(priv->app);
	if (account) {
		if (loqui_account_get_is_connected(account))
			loqui_sender_away(loqui_account_get_sender(account), away_type, NULL);
		else
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("The account is not connected!"));
	} else {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected an account!"));	
	}
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
loqui_statusbar_set_away_menu(LoquiStatusbar *statusbar, LoquiUserClass *user_class)
{
	LoquiStatusbarPrivate *priv;
	GtkWidget *menuitem;
	GtkWidget *image;
	GList *cur, *tmp_list = NULL;
	LoquiAwayType away_type;
	LoquiAwayInfo *awinfo;
	const gchar *stock_id;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;

	for (cur = GTK_MENU_SHELL(priv->menu_away)->children; cur != NULL; cur = cur->next) {
		tmp_list = g_list_append(tmp_list, cur->data);
	}
	for (cur = tmp_list; cur != NULL; cur = cur->next) {
		gtk_container_remove(GTK_CONTAINER(priv->menu_away), cur->data);
	}

	tmp_list = loqui_user_class_get_away_type_list(user_class);
	for (cur = tmp_list; cur != NULL; cur = cur->next) {
		away_type = GPOINTER_TO_INT(cur->data);
		if (away_type == LOQUI_AWAY_TYPE_UNKNOWN) {
			menuitem = gtk_separator_menu_item_new();
			gtk_widget_show(menuitem);
			gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_away), menuitem);
			continue;
		}

		awinfo = loqui_user_class_away_type_get_info(user_class, GPOINTER_TO_INT(cur->data));
		g_assert(awinfo != NULL);

		stock_id = loqui_stock_get_id_from_basic_away_type(awinfo->basic_away_type);
		g_assert(stock_id != NULL);

		g_assert(awinfo->nick != NULL);
		menuitem = gtk_image_menu_item_new_with_label(awinfo->nick);

		image = gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU);
		gtk_widget_show(image);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);

		g_object_set_data(G_OBJECT(menuitem), "away-type", GINT_TO_POINTER(awinfo->away_type));
		g_signal_connect(G_OBJECT(menuitem), "activate",
				 G_CALLBACK(loqui_statusbar_away_menuitem_activated_cb), statusbar);
		
		if (awinfo->basic_away_type == LOQUI_BASIC_AWAY_TYPE_OFFLINE)
			gtk_widget_set_sensitive(menuitem, FALSE);

		gtk_widget_show(menuitem);
		gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_away), menuitem);
	}
}
static void
loqui_statusbar_nick_button_clicked_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
	LoquiAccount *account;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

	priv = statusbar->priv;
	
	account = loqui_app_get_current_account(priv->app);
	if (account)
		command_dialog_nick(priv->app, account);
	else
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected an account!"));
}
GtkWidget*
loqui_statusbar_new(LoquiApp *app, GtkToggleAction *toggle_scroll_common_buffer_action)
{
        LoquiStatusbar *statusbar;
	LoquiStatusbarPrivate *priv;
	GtkWidget *vsep;
	GtkWidget *hbox_away;
	GtkWidget *image;
	gchar *text;

	statusbar = g_object_new(loqui_statusbar_get_type(), NULL);
	priv = statusbar->priv;
	
	priv->app = app;
	
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(statusbar), FALSE);
	gtk_label_set_selectable(GTK_LABEL(GTK_STATUSBAR(statusbar)->label), TRUE);
	
	vsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), vsep, FALSE, FALSE, 2);

	priv->progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(statusbar), priv->progress, FALSE, FALSE, 0);
	gtk_widget_set_size_request(priv->progress, 70, -1);

	vsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), vsep, FALSE, FALSE, 2);

	priv->dbox_away = loqui_dropdown_box_new(NULL);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->dbox_away, FALSE, FALSE, 0);
	gtk_button_set_relief(GTK_BUTTON(LOQUI_DROPDOWN_BOX(priv->dbox_away)->drop_button), GTK_RELIEF_NONE);

	priv->button_away = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(priv->button_away), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(priv->dbox_away), priv->button_away, FALSE, FALSE, 0);
	
	hbox_away = gtk_hbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(priv->button_away), hbox_away);

	priv->image_away = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(hbox_away), priv->image_away, FALSE, FALSE, 0);

	priv->label_away = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox_away), priv->label_away, FALSE, FALSE, 0);

	vsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), vsep, FALSE, FALSE, 2);

	priv->dbox_preset = loqui_dropdown_box_new(NULL);
	gtk_button_set_relief(GTK_BUTTON(LOQUI_DROPDOWN_BOX(priv->dbox_preset)->drop_button), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->dbox_preset, FALSE, FALSE, 0);

	priv->button_nick = gtk_button_new();
	g_signal_connect(G_OBJECT(priv->button_nick), "clicked",
			 G_CALLBACK(loqui_statusbar_nick_button_clicked_cb), statusbar);
	gtk_button_set_relief(GTK_BUTTON(priv->button_nick), GTK_RELIEF_NONE);
	gtk_tooltips_set_tip(app->tooltips, priv->button_nick, _("Change nick"), NULL);
	gtk_box_pack_start(GTK_BOX(priv->dbox_preset), priv->button_nick, FALSE, FALSE, 0);

	priv->label_nick = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(priv->button_nick), priv->label_nick);

	vsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), vsep, FALSE, FALSE, 2);

	priv->label_account = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(statusbar), priv->label_account, FALSE, FALSE, 0);

	vsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), vsep, FALSE, FALSE, 2);

	priv->toggle_scroll_common_buffer = gtk_toggle_button_new();
	gtk_action_connect_proxy(GTK_ACTION(toggle_scroll_common_buffer_action), priv->toggle_scroll_common_buffer);
	gtkutils_bin_remove_child_if_exist(GTK_BIN((priv->toggle_scroll_common_buffer)));
	image = gtk_image_new_from_stock(LOQUI_STOCK_WHETHER_SCROLL, LOQUI_ICON_SIZE_FONT);
	gtk_container_add(GTK_CONTAINER(priv->toggle_scroll_common_buffer), image);
	gtk_widget_show(image);
	gtk_button_set_focus_on_click(GTK_BUTTON(priv->toggle_scroll_common_buffer), FALSE);
	g_object_get(G_OBJECT(toggle_scroll_common_buffer_action), "tooltip", &text, NULL);
	gtk_tooltips_set_tip(app->tooltips, priv->toggle_scroll_common_buffer, text, NULL);

	gtk_box_pack_start(GTK_BOX(statusbar), priv->toggle_scroll_common_buffer, FALSE, FALSE, 0);
	
	priv->menu_preset = gtk_menu_new();
	loqui_dropdown_box_set_menu(LOQUI_DROPDOWN_BOX(priv->dbox_preset), priv->menu_preset);

	priv->menu_away = gtk_menu_new();
	loqui_dropdown_box_set_menu(LOQUI_DROPDOWN_BOX(priv->dbox_away), priv->menu_away);

	return GTK_WIDGET(statusbar);
}
void
loqui_statusbar_update_account_name(LoquiStatusbar *statusbar, LoquiAccount *account)
{
	LoquiStatusbarPrivate *priv;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

        priv = statusbar->priv;
        
	gtk_label_set(GTK_LABEL(priv->label_account),
		      account ? loqui_profile_account_get_name(loqui_account_get_profile(account)) : STRING_UNSELECTED);
}
void
loqui_statusbar_update_nick(LoquiStatusbar *statusbar, LoquiAccount *account)
{
	LoquiStatusbarPrivate *priv;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

        priv = statusbar->priv;
        
        if (account == NULL) {
        	gtk_widget_set_sensitive(priv->dbox_preset, FALSE);
        	gtk_widget_set_sensitive(priv->dbox_away, FALSE);        	
        	gtk_label_set(GTK_LABEL(priv->label_nick), STRING_UNSELECTED);
        } else if (!loqui_account_get_is_connected(account)) {
      	 	gtk_widget_set_sensitive(priv->dbox_preset, FALSE);
        	gtk_widget_set_sensitive(priv->dbox_away, FALSE);
        	gtk_label_set(GTK_LABEL(priv->label_nick), STRING_DISCONNECTED);
        } else {
        	gtk_widget_set_sensitive(priv->button_nick, TRUE);
        	gtk_widget_set_sensitive(priv->dbox_preset, TRUE);
        	gtk_widget_set_sensitive(priv->dbox_away, TRUE);
        	gtk_label_set(GTK_LABEL(priv->label_nick), loqui_user_get_nick(loqui_account_get_user_self(account)));
        }
}
void
loqui_statusbar_update_preset_menu(LoquiStatusbar *statusbar, LoquiAccount *account)
{
        if (account) 
        	loqui_statusbar_set_preset_menu(statusbar, loqui_profile_account_get_nick_list(loqui_account_get_profile(account)));
}
void
loqui_statusbar_update_away_menu(LoquiStatusbar *statusbar, LoquiAccount *account)
{
        if (account)
		loqui_statusbar_set_away_menu(statusbar, LOQUI_USER_GET_CLASS(loqui_account_get_user_self(account)));
}
void
loqui_statusbar_update_away_icon(LoquiStatusbar *statusbar, LoquiAccount *account)
{
	LoquiAwayType away;

        if (account == NULL) {
        	away = LOQUI_AWAY_TYPE_OFFLINE;
	} else {
		away = loqui_user_get_away(loqui_account_get_user_self(account));
	}
        
	if (account)
		loqui_statusbar_set_away(statusbar, LOQUI_USER_GET_CLASS(loqui_account_get_user_self(account)), away);
}
void
loqui_statusbar_update_current_account(LoquiStatusbar *statusbar, LoquiAccount *account)
{
	LoquiStatusbarPrivate *priv;
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;
        
	loqui_statusbar_update_account_name(statusbar, account);
	loqui_statusbar_update_nick(statusbar, account);

        loqui_statusbar_update_preset_menu(statusbar, account);
	loqui_statusbar_update_away_menu(statusbar, account);

	loqui_statusbar_update_away_icon(statusbar, account);
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
