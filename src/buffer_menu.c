/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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
/* Utilities to operate menu of buffers,
   like "Buffers" in the main menu or the option menu on the Location bar. */

#include "config.h"
#include "buffer_menu.h"
#include "loqui_stock.h"
#include "gtkutils.h"
#include "loqui_gtk.h"
#include "intl.h"

GtkWidget *
buffer_menu_add_account(GtkMenuShell *menu, Account *account)
{
	GtkWidget *image;
	GtkWidget *menuitem;
	
	g_return_val_if_fail(menu != NULL, NULL);
	g_return_val_if_fail(GTK_IS_MENU_SHELL(menu), NULL);
	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(IS_ACCOUNT(account), NULL);

	menuitem = gtk_image_menu_item_new_with_label(loqui_profile_account_get_name(account_get_profile(account)));
	image = gtk_image_new_from_stock(LOQUI_STOCK_CONSOLE, GTK_ICON_SIZE_MENU);
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);

	g_object_ref(account);
	g_object_set_data_full(G_OBJECT(menuitem), "account", account, g_object_unref);
	gtk_widget_show(menuitem);

	gtk_menu_shell_append(menu, menuitem);

	return menuitem;
}
void
buffer_menu_update_account(GtkMenuShell *menu, Account *account)
{
	GtkWidget *menuitem;
	Account *tmp_ac;
	GList *cur;
	GList *children;

	g_return_if_fail(menu != NULL);
	g_return_if_fail(GTK_IS_MENU_SHELL(menu));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	for(cur = menu->children; cur != NULL; cur = cur->next) {
		menuitem = GTK_WIDGET(cur->data);
		tmp_ac = g_object_get_data(G_OBJECT(menuitem), "account");
		if(tmp_ac != account)
			continue;
		if(g_object_get_data(G_OBJECT(menuitem), "channel")) /* is channel? */
			continue;
		children = gtk_container_get_children(GTK_CONTAINER(menuitem));

		/* FIXME: dirty way */
		if(children == NULL) {
			g_warning(_("Menuitem's children is null in Buffer menu"));
			break;
		}
		if(GTK_IS_LABEL(children->data))
			gtk_label_set_text(GTK_LABEL(children->data), loqui_profile_account_get_name(account_get_profile(account)));
	}

}
void
buffer_menu_remove_account(GtkMenuShell *menu, Account *account)
{
	Account *tmp_ac;
	GList *cur;
	GList *removing_items = NULL;
	
	g_return_if_fail(menu != NULL);
	g_return_if_fail(GTK_IS_MENU_SHELL(menu));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	for(cur = menu->children; cur != NULL; cur = cur->next) {
		tmp_ac = ACCOUNT(g_object_get_data(G_OBJECT(cur->data), "account"));
		if(tmp_ac == account)
			removing_items = g_list_append(removing_items, cur->data);
	}
	for(cur = removing_items; cur != NULL; cur = cur->next) {
		gtk_container_remove(GTK_CONTAINER(menu), GTK_WIDGET(cur->data));
	}
	g_list_free(removing_items);
}

GtkWidget *
buffer_menu_add_channel(GtkMenuShell *menu, Channel *channel)
{
	GtkWidget *menuitem;
	Account *account;
	Account *tmp_ac;
	GList *cur;
	GList *children;
	guint i;

	g_return_val_if_fail(menu != NULL, NULL);
	g_return_val_if_fail(GTK_IS_MENU_SHELL(menu), NULL);
	g_return_val_if_fail(channel != NULL, NULL);
	g_return_val_if_fail(IS_CHANNEL(channel), NULL);

	account = channel->account;
	children = menu->children;
	i = g_list_length(children);
	for(cur = g_list_last(children); cur != NULL; cur = cur->prev) {
		tmp_ac = g_object_get_data(G_OBJECT(cur->data), "account");
		if(tmp_ac == account)
			break;
		i--;
	}

	menuitem = gtk_menu_item_new_with_label(channel_get_name(channel));
	g_object_ref(account);
	g_object_set_data_full(G_OBJECT(menuitem), "account", account, g_object_unref);
	g_object_ref(channel);
	g_object_set_data_full(G_OBJECT(menuitem), "channel", channel, g_object_unref);
	gtk_widget_show(menuitem);

	gtk_menu_shell_insert(menu, menuitem, i);

	return menuitem;
}

void
buffer_menu_update_channel(GtkMenuShell *menu, Channel *channel)
{
	GtkWidget *menuitem;
	Channel *tmp_ch;
	GList *cur;
	GList *children;
	GtkWidget *label;

	g_return_if_fail(menu != NULL);
	g_return_if_fail(GTK_IS_MENU_SHELL(menu));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	for(cur = menu->children; cur != NULL; cur = cur->next) {
		menuitem = GTK_WIDGET(cur->data);
		tmp_ch = g_object_get_data(G_OBJECT(menuitem), "channel");
		if(tmp_ch != channel)
			continue;

		children = gtk_container_get_children(GTK_CONTAINER(menuitem));

		/* FIXME: though this should not happen, it DOES. */
		if(children == NULL)
			continue;

		label = children->data;

		gtkutils_set_label_color(GTK_LABEL(label), 
					 channel_get_updated(channel) ?
					 FRESH_COLOR : NONFRESH_COLOR);
	}
}

void
buffer_menu_remove_channel(GtkMenuShell *menu, Channel *channel)
{
	Channel *tmp_ch;
	GList *cur;
	GList *removing_items = NULL;

	g_return_if_fail(menu != NULL);
	g_return_if_fail(GTK_IS_MENU_SHELL(menu));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	for(cur = menu->children; cur != NULL; cur = cur->next) {
		tmp_ch = CHANNEL(g_object_get_data(G_OBJECT(cur->data), "channel"));
		if(tmp_ch == channel)
			removing_items = g_list_append(removing_items, cur->data);
	}
	for(cur = removing_items; cur != NULL; cur = cur->next) {
		gtk_container_remove(GTK_CONTAINER(menu), GTK_WIDGET(cur->data));
	}
	g_list_free(removing_items);
}
void
buffer_menu_update_accel_keys(GtkMenuShell *menu)
{
	GList *cur;
	int i = 0;
	GtkMenuItem *item;
	gchar *path;
	
	for (cur = menu->children; cur != NULL; cur = cur->next) {
		item = GTK_MENU_ITEM(cur->data);	
		if (GTK_IS_TEAROFF_MENU_ITEM(item))
			continue;
					
		if (i > MAX_SHORTCUT_CHANNEL_NUMBER) {
			gtk_menu_item_set_accel_path(item, NULL);
			continue;
		}
				
		path = g_strdup_printf(SHORTCUT_CHANNEL_ACCEL_MAP_PREFIX "%d", i);
		gtk_menu_item_set_accel_path(item, path);
		g_free(path);
		
		i++;
	}	
}
