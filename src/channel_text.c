/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2 <http://loqui.good-day.net/>
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

#include "channel_text.h"

struct _ChannelTextPrivate
{
};

static GtkScrolledWindowClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_SCROLLED_WINDOW

static void channel_text_class_init(ChannelTextClass *klass);
static void channel_text_init(ChannelText *channel_text);
static void channel_text_finalize(GObject *object);
static void channel_text_destroy(GtkObject *object);

GType
channel_text_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelTextClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_text_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ChannelText),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_text_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ChannelText",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_text_class_init (ChannelTextClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_text_finalize;
        gtk_object_class->destroy = channel_text_destroy;
}
static void 
channel_text_init (ChannelText *channel_text)
{
	ChannelTextPrivate *priv;

	priv = g_new0(ChannelTextPrivate, 1);

	channel_text->priv = priv;
}
static void 
channel_text_finalize (GObject *object)
{
	ChannelText *channel_text;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_TEXT(object));

        channel_text = CHANNEL_TEXT(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_text->priv);
}
static void 
channel_text_destroy (GtkObject *object)
{
        ChannelText *channel_text;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_TEXT(object));

        channel_text = CHANNEL_TEXT(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

GtkWidget*
channel_text_new(void)
{
        ChannelText *channel_text;
	ChannelTextPrivate *priv;

	channel_text = g_object_new(channel_text_get_type(), NULL);
	priv = channel_text->priv;

	channel_text->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(channel_text->text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(channel_text->text), GTK_WRAP_CHAR);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(channel_text), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(channel_text), channel_text->text);

	return GTK_WIDGET(channel_text);
}
