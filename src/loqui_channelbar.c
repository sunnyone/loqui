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

struct _LoquiChannelBarPrivate
{
	GtkWidget *option_menu;
	GtkWidget *entry_topic;
	GtkWidget *button_ok;
};

static GtkHBoxClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_HBOX

static void loqui_channelbar_class_init(LoquiChannelBarClass *klass);
static void loqui_channelbar_init(LoquiChannelBar *channelbar);
static void loqui_channelbar_finalize(GObject *object);
static void loqui_channelbar_destroy(GtkObject *object);

GType
loqui_channelbar_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelBarClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channelbar_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelBar),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channelbar_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiChannelBar",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_channelbar_class_init (LoquiChannelBarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channelbar_finalize;
        gtk_object_class->destroy = loqui_channelbar_destroy;
}
static void 
loqui_channelbar_init (LoquiChannelBar *channelbar)
{
	LoquiChannelBarPrivate *priv;

	priv = g_new0(LoquiChannelBarPrivate, 1);

	channelbar->priv = priv;
}
static void 
loqui_channelbar_finalize (GObject *object)
{
	LoquiChannelBar *channelbar;

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
        LoquiChannelBar *channelbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNELBAR(object));

        channelbar = LOQUI_CHANNELBAR(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

GtkWidget*
loqui_channelbar_new (void)
{
        LoquiChannelBar *channelbar;
	LoquiChannelBarPrivate *priv;

	channelbar = g_object_new(loqui_channelbar_get_type(), NULL);
	
	priv = channelbar->priv;

	priv->option_menu = gtk_option_menu_new();
	gtk_box_pack_start(GTK_BOX(channelbar), priv->option_menu, FALSE, FALSE, 0);

	priv->entry_topic = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(channelbar), priv->entry_topic, TRUE, TRUE, 0);

	return GTK_WIDGET(channelbar);
}
