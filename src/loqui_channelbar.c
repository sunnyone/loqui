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
#include "account_manager.h"
#include "loqui_gtk.h"
#include "gtkutils.h"
#include "loqui_dropdown_box.h"

struct _LoquiChannelbarPrivate
{
	LoquiApp *app;

	GtkWidget *dbox_buffers;
	GtkWidget *button_channel;
	GtkWidget *label_channel;

	GtkWidget *label_channel_mode;
	GtkWidget *label_user_number;
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
	
	channel = loqui_app_get_current_channel(priv->app);
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

GtkWidget*
loqui_channelbar_new(LoquiApp *app, GtkWidget *menu_dropdown)
{
        LoquiChannelbar *channelbar;
	LoquiChannelbarPrivate *priv;
	GtkWidget *image;

	channelbar = g_object_new(loqui_channelbar_get_type(), NULL);
	
	priv = channelbar->priv;
	priv->app = app;

	priv->dbox_buffers = loqui_dropdown_box_new(NULL);
	loqui_dropdown_box_set_menu(LOQUI_DROPDOWN_BOX(priv->dbox_buffers), menu_dropdown);
	gtk_box_pack_start(GTK_BOX(channelbar), priv->dbox_buffers, FALSE, FALSE, 0);

	priv->label_channel_mode = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(channelbar), priv->label_channel_mode, 0, 0, FALSE);
	
	priv->label_user_number = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(channelbar), priv->label_user_number, 0, 0, FALSE);
	
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
loqui_channelbar_set_current_channel(LoquiChannelbar *channelbar, Channel *channel)
{
	LoquiChannelbarPrivate *priv;
	const gchar *topic;
	gchar *buf, *channel_mode;
	guint user_num_all, user_num_op;
	
	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	gtk_entry_set_text(GTK_ENTRY(priv->entry_topic), "");
	
	gtk_label_set(GTK_LABEL(priv->label_channel_mode), "");
	gtk_label_set(GTK_LABEL(priv->label_user_number), "");
			
	if(channel && !channel_is_private_talk(channel)) {
		topic = channel_get_topic(channel);
		if(topic)
			gtk_entry_set_text(GTK_ENTRY(priv->entry_topic), topic);
		gtk_widget_set_sensitive(priv->entry_topic, TRUE);
		
		channel_mode = channel_get_mode(channel);
		buf = g_strdup_printf("[%s]", channel_mode);
		g_free(channel_mode);
		gtk_label_set(GTK_LABEL(priv->label_channel_mode), buf);
		g_free(buf);
		
		channel_get_user_number(channel, &user_num_all, &user_num_op);
		buf = g_strdup_printf("(%d/%d)", user_num_op, user_num_all);
		gtk_label_set(GTK_LABEL(priv->label_user_number), buf);
		g_free(buf);
	} else {
		gtk_widget_set_sensitive(priv->entry_topic, FALSE);
	}
	
	gtk_widget_set_sensitive(priv->button_ok, FALSE);	
	priv->entry_changed = FALSE;
}

void
loqui_channelbar_set_current_account(LoquiChannelbar *channelbar, Account *account)
{
	LoquiChannelbarPrivate *priv;

	g_return_if_fail(channelbar != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(channelbar));

	priv = channelbar->priv;

	gtk_entry_set_text(GTK_ENTRY(priv->entry_topic), "");
	gtk_widget_set_sensitive(priv->entry_topic, FALSE);
	gtk_widget_set_sensitive(priv->button_ok, FALSE);
	priv->entry_changed = FALSE;
	
	gtk_label_set(GTK_LABEL(priv->label_channel_mode), "");
	gtk_label_set(GTK_LABEL(priv->label_user_number), "");
}
