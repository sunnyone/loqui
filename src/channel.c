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
	gchar *topic;
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
	priv->topic = NULL;

}
static void 
channel_finalize (GObject *object)
{
	Channel *channel;
	ChannelPrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL(object));

        channel = CHANNEL(object);
	priv = channel->priv;

	if(channel->name) {
		g_free(channel->name);
		channel->name = NULL;
	}
	if(channel->text) {
		gtk_widget_destroy(GTK_WIDGET(channel->text));
		channel->text = NULL;
	}
	if(priv->topic) {
		g_free(priv->topic);
		priv->topic = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(priv);
}

Channel*
channel_new (gchar *name)
{
        Channel *channel;
	ChannelPrivate *priv;

	channel = g_object_new(channel_get_type(), NULL);

	channel->name = g_strdup(name);
	channel->text = CHANNEL_TEXT(channel_text_new());

	return channel;
}

void
channel_append_remark(Channel *channel, TextType type, gchar *name, gchar *remark)
{
	gchar *line_with_nick;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	line_with_nick = g_strdup_printf("<%s> %s", name, remark);
	channel_text_append(CHANNEL_TEXT(channel->text), type, line_with_nick);
	g_free(line_with_nick);
}

void
channel_append_text(Channel *channel, TextType type, gchar *str)
{
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	channel_text_append(CHANNEL_TEXT(channel->text), type, str);
}
/* TODO: reflect the main window */
void channel_set_topic(Channel *channel, const gchar *topic)
{
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	priv = channel->priv;

	if(priv->topic)
		g_free(priv->topic);

	priv->topic = g_strdup(topic);
}
gchar *channel_get_topic(Channel *channel)
{
	g_return_val_if_fail(channel != NULL, NULL);
	g_return_val_if_fail(IS_CHANNEL(channel), NULL);

	return channel->priv->topic;
}
