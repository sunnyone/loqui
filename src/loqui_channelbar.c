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
#include "config.h"

#include "loqui_channelbar.h"
#include "buffer_menu.h"
#include "account_manager.h"
#include "main.h"
#include "gtkutils.h"

struct _LoquiChannelbarPrivate
{
	GtkWidget *option_menu;
	GtkWidget *entry_topic;
	GtkWidget *button_ok;
	gboolean entry_changed;
};

static GtkHBoxClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_HBOX

static void loqui_channelbar_class_init(LoquiChannelbarClass *klass);
static void loqui_channelbar_init(LoquiChannelbar *channelbar);
static void loqui_channelbar_finalize(GObject *object);
static void loqui_channelbar_destroy(GtkObject *object);

static void loqui_channelbar_option_changed_cb(GtkWidget *widget, gpointer data);
static void loqui_channelbar_entry_topic_activated_cb(GtkWidget *widget, gpointer data);
static void loqui_channelbar_entry_changed_cb(GtkWidget *widget, gpointer data);

GType
loqui_channelbar_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelbarClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channelbar_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelbar),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channelbar_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiChannelbar",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_channelbar_class_init (LoquiChannelbarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channelbar_finalize;
        gtk_object_class->destroy = loqui_channelbar_destroy;
}
static void 
loqui_channelbar_init (LoquiChannelbar *channelbar)
{
	LoquiChannelbarPrivate *priv;

	priv = g_new0(LoquiChannelbarPrivate, 1);

	channelbar->priv = priv;
}
static void 
loqui_channelbar_finalize (GObject *object)
{
	LoquiChannelbar *channelbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(object));

        channelbar = LOQUI_CHANNELBAR(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channelbar->priv);
}
static void 
loqui_channelbar_destroy (GtkObject *object)
{
        LoquiChannelbar *channelbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(object));

        channelbar = LOQUI_CHANNELBAR(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
loqui_channelbar_entry_topic_activated_cb(GtkWidget *widget, gpointer data)
{
	LoquiChannelbar *channelbar;
	LoquiChannelbarPrivate *priv;
	Channel *channel;
	const gchar *str;

	g_return_if_fail(data != NULL);
	
	channelbar = LOQUI_CHANNELBAR(data);
	priv = channelbar->priv;
	
	if(!priv->entry_changed)
		return;
	
	channel = account_manager_get_current_channel(account_manager_get());
	str = gtk_entry_get_text(GTK_ENTRY(priv->entry_topic));
	account_set_topic(channel->account, channel_get_name(channel), str);
}
static void
loqui_channelbar_entry_changed_cb(GtkWidget *widget, gpointer data)
{
	LoquiChannelbar *channelbar;
	LoquiChannelbarPrivate *priv;
	
	g_return_if_fail(data != NULL);
	
	channelbar = LOQUI_CHANNELBAR(data);
	priv = channelbar->priv;

	priv->entry_changed = TRUE;
	gtk_widget_set_sensitive(priv->button_ok, TRUE);
}
static void
loqui_channelbar_option_changed_cb(GtkWidget *widget, gpointer data)
{
	LoquiChannelbar *channelbar;
	LoquiChannelbarPrivate *priv;
	GtkWidget *menu;
	GtkWidget *menuitem;
	Channel *channel;
	Account *account;
	gint i;

	g_return_if_fail(data != NULL);
	
	channelbar = LOQUI_CHANNELBAR(data);
	priv = channelbar->priv;

	i = gtk_option_menu_get_history(GTK_OPTION_MENU(priv->option_menu));
	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu));
	menuitem = GTK_WIDGET(g_list_nth(GTK_MENU_SHELL(menu)->children, i)->data);
	if(!menuitem)
		return;
	channel = g_object_get_data(G_OBJECT(menuitem), "channel");
	if(channel) {
		account_manager_set_current_channel(account_manager_get(), channel);
		return;
	}
	account = g_object_get_data(G_OBJECT(menuitem), "account");
	if(account) {
		account_manager_set_current_account(account_manager_get(), account);
		return;
	}	
}
GtkWidget*
loqui_channelbar_new (void)
{
        LoquiChannelbar *channelbar;
	LoquiChannelbarPrivate *priv;
	GtkWidget *image;
	GtkWidget *menu;

	channelbar = g_object_new(loqui_channelbar_get_type(), NULL);
	
	priv = channelbar->priv;

	priv->option_menu = gtk_option_menu_new();

	g_signal_connect(G_OBJECT(priv->option_menu), "changed",
			 G_CALLBACK(loqui_channelbar_option_changed_cb), channelbar);
	gtk_box_pack_start(GTK_BOX(channelbar), priv->option_menu, FALSE, FALSE, 0);

	menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(priv->option_menu), menu);

	priv->entry_topic = gtk_entry_new();
	g_signal_connect(G_OBJECT(priv->entry_topic), "changed",
			 G_CALLBACK(loqui_channelbar_entry_changed_cb), channelbar);
	g_signal_connect(G_OBJECT(priv->entry_topic), "activate",
			 G_CALLBACK(loqui_channelbar_entry_topic_activated_cb), channelbar);
	gtk_box_pack_start(GTK_BOX(channelbar), priv->entry_topic, TRUE, TRUE, 0);
	gtk_widget_set_sensitive(priv->entry_topic, FALSE);

	image = gtk_image_new_from_stock(GTK_STOCK_OK, GTK_ICON_SIZE_SMALL_TOOLBAR);
	priv->button_ok = gtk_button_new();
	g_signal_connect(G_OBJECT(priv->button_ok), "clicked",
			 G_CALLBACK(loqui_channelbar_entry_topic_activated_cb), channelbar);
	gtk_container_add(GTK_CONTAINER(priv->button_ok), image);
	gtk_box_pack_start(GTK_BOX(channelbar), priv->button_ok, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(priv->button_ok, FALSE);
	priv->entry_changed = FALSE;

	return GTK_WIDGET(channelbar);
}
void
loqui_channelbar_add_account(LoquiChannelbar *channelbar, Account *account)
{
	LoquiChannelbarPrivate *priv;
	GtkWidget *menuitem;
	GtkMenuShell *menu;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));
	menuitem = buffer_menu_add_account(menu, account);
}
void
loqui_channelbar_update_account(LoquiChannelbar *channelbar, Account *account)
{
	LoquiChannelbarPrivate *priv;
	GtkMenuShell *menu;
	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));
	buffer_menu_update_account(menu, account);
}

void
loqui_channelbar_remove_account(LoquiChannelbar *channelbar, Account *account)
{
	LoquiChannelbarPrivate *priv;
	GtkMenuShell *menu;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));
	buffer_menu_remove_account(menu, account);
}
void
loqui_channelbar_add_channel(LoquiChannelbar *channelbar, Channel *channel)
{
	LoquiChannelbarPrivate *priv;
	GtkWidget *menuitem;
	GtkMenuShell *menu;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;
	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));
	menuitem = buffer_menu_add_channel(menu, channel);
}
void
loqui_channelbar_remove_channel(LoquiChannelbar *channelbar, Channel *channel)
{
	LoquiChannelbarPrivate *priv;
	GtkMenuShell *menu;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));
	buffer_menu_remove_channel(menu, channel);
}
void
loqui_channelbar_update_channel(LoquiChannelbar *channelbar, Channel *channel)
{
	LoquiChannelbarPrivate *priv;
	GtkMenuShell *menu;
	Channel *tmp_ch;
	GList *children;
	GtkWidget *menuitem;
	GtkWidget *label;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));

	/* for current menuitem and others. FIXME: quick hack */
	menuitem = GTK_OPTION_MENU(priv->option_menu)->menu_item;

	tmp_ch = CHANNEL(g_object_get_data(G_OBJECT(menuitem), "channel"));
	if(tmp_ch == channel) {
		children = gtk_container_get_children(GTK_CONTAINER(priv->option_menu));

		if(children == NULL)
			return;

		label = children->data;

		gtkutils_set_label_color(GTK_LABEL(label), 
					 channel_get_updated(channel) ?
					 FRESH_COLOR : NONFRESH_COLOR);

	} else {
		buffer_menu_update_channel(menu, channel);
	}
}
void
loqui_channelbar_set_current_channel(LoquiChannelbar *channelbar, Channel *channel)
{
	LoquiChannelbarPrivate *priv;
	GtkMenuShell *menu;
	GtkWidget *menuitem;
	const gchar *topic;
	gint i = 0;
	Channel *tmp_ch;
	GList *cur;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));

	for(cur = menu->children; cur != NULL; cur = cur->next) {
		menuitem = GTK_WIDGET(cur->data);
		tmp_ch = g_object_get_data(G_OBJECT(menuitem), "channel");
		if(tmp_ch == channel)
			break;
		i++;
	}
	gtk_option_menu_set_history(GTK_OPTION_MENU(priv->option_menu), i);

	if(channel_is_private_talk(channel)) {
		gtk_entry_set_text(GTK_ENTRY(priv->entry_topic), "");
		gtk_widget_set_sensitive(priv->entry_topic, FALSE);
		gtk_widget_set_sensitive(priv->button_ok, FALSE);
	} else {
		topic = channel_get_topic(channel);
		gtk_entry_set_text(GTK_ENTRY(priv->entry_topic), topic ? topic : "");
		gtk_widget_set_sensitive(priv->entry_topic, TRUE);
		gtk_widget_set_sensitive(priv->button_ok, FALSE);
		priv->entry_changed = FALSE;
	}
}

void
loqui_channelbar_set_current_account(LoquiChannelbar *channelbar, Account *account)
{
	LoquiChannelbarPrivate *priv;
	GtkMenuShell *menu;
	GtkWidget *menuitem;
	gint i = 0;
	Account *tmp_ac;
	GList *cur;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	menu = GTK_MENU_SHELL(gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_menu)));
	
	for(cur = menu->children; cur != NULL; cur = cur->next) {
		menuitem = GTK_WIDGET(cur->data);
		tmp_ac = g_object_get_data(G_OBJECT(menuitem), "account");
		if(tmp_ac == account)
			break;
		i++;
	}
	gtk_option_menu_set_history(GTK_OPTION_MENU(priv->option_menu), i);

	gtk_entry_set_text(GTK_ENTRY(priv->entry_topic), "");
	gtk_widget_set_sensitive(priv->entry_topic, FALSE);
	gtk_widget_set_sensitive(priv->button_ok, FALSE);
	priv->entry_changed = FALSE;
}
