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

#include "channel_book.h"

struct _ChannelBookPrivate
{
};

static GtkNotebookClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_NOTEBOOK

static void channel_book_class_init(ChannelBookClass *klass);
static void channel_book_init(ChannelBook *channel_book);
static void channel_book_finalize(GObject *object);
static void channel_book_destroy(GtkObject *object);

GType
channel_book_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelBookClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_book_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ChannelBook),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_book_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ChannelBook",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_book_class_init (ChannelBookClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_book_finalize;
        gtk_object_class->destroy = channel_book_destroy;
}
static void 
channel_book_init (ChannelBook *channel_book)
{
	ChannelBookPrivate *priv;

	priv = g_new0(ChannelBookPrivate, 1);

	channel_book->priv = priv;
}
static void 
channel_book_finalize (GObject *object)
{
	ChannelBook *channel_book;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_BOOK(object));

        channel_book = CHANNEL_BOOK(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_book->priv);
}
static void 
channel_book_destroy (GtkObject *object)
{
        ChannelBook *channel_book;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_BOOK(object));

        channel_book = CHANNEL_BOOK(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

GtkWidget*
channel_book_new (void)
{
        ChannelBook *channel_book;
	ChannelBookPrivate *priv;

	channel_book = g_object_new(channel_book_get_type(), NULL);
//	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(channel_book), FALSE);

	return GTK_WIDGET(channel_book);
}
