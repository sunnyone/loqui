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

#include "channel.h"

struct _ChannelPrivate
{
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void channel_class_init(ChannelClass *klass);
static void channel_init(Channel *channel);
static void channel_finalize(GObject *object);

GType
channel_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(Channel),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "Channel",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_class_init (ChannelClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_finalize;
}
static void 
channel_init (Channel *channel)
{
	ChannelPrivate *priv;

	priv = g_new0(ChannelPrivate, 1);

	channel->priv = priv;
}
static void 
channel_finalize (GObject *object)
{
	Channel *channel;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL(object));

        channel = CHANNEL(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel->priv);
}

Channel*
channel_new (void)
{
        Channel *channel;
	ChannelPrivate *priv;

	channel = g_object_new(channel_get_type(), NULL);
	
	return channel;
}


