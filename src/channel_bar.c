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

#include "channel_bar.h"

struct _ChannelBarPrivate
{
	GtkWidget *option_menu;
	GtkWidget *entry_topic;
};

static GtkToolbarClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TOOLBAR

static void channel_bar_class_init(ChannelBarClass *klass);
static void channel_bar_init(ChannelBar *channel_bar);
static void channel_bar_finalize(GObject *object);
static void channel_bar_destroy(GtkObject *object);

GType
channel_bar_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelBarClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_bar_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ChannelBar),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_bar_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ChannelBar",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_bar_class_init (ChannelBarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_bar_finalize;
        gtk_object_class->destroy = channel_bar_destroy;
}
static void 
channel_bar_init (ChannelBar *channel_bar)
{
	ChannelBarPrivate *priv;

	priv = g_new0(ChannelBarPrivate, 1);

	channel_bar->priv = priv;
}
static void 
channel_bar_finalize (GObject *object)
{
	ChannelBar *channel_bar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_BAR(object));

        channel_bar = CHANNEL_BAR(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_bar->priv);
}
static void 
channel_bar_destroy (GtkObject *object)
{
        ChannelBar *channel_bar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_BAR(object));

        channel_bar = CHANNEL_BAR(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

GtkWidget*
channel_bar_new (void)
{
        ChannelBar *channel_bar;
	ChannelBarPrivate *priv;

	channel_bar = g_object_new(channel_bar_get_type(), NULL);
	
	return GTK_WIDGET(channel_bar);
}
