/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
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
#include "account_manager.h"
#include "utils.h"
#include "main.h"

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
	channel->mode = NULL;
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

	G_FREE_UNLESS_NULL(channel->name);
	G_FREE_UNLESS_NULL(priv->topic);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(priv);
}

Channel*
channel_new (gchar *name)
{
        Channel *channel;

	channel = g_object_new(channel_get_type(), NULL);

	channel->name = g_strdup(name);
	channel->buffer = channel_buffer_new();
	channel->user_list = gtk_list_store_new(USERLIST_COLUMN_NUMBER, 
						G_TYPE_INT,
						G_TYPE_INT,
						G_TYPE_STRING);
	channel->end_names = TRUE;
	
	return channel;
}

void
channel_append_remark(Channel *channel, TextType type, gchar *nick, gchar *remark)
{
	gchar *line_with_nick;
	gchar *line_with_channel;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	if(STRING_IS_CHANNEL(channel->name))
		line_with_nick = g_strdup_printf("<%s> %s", nick, remark);
	else
		line_with_nick = g_strdup_printf("=%s= %s", nick, remark);
	channel_buffer_append(CHANNEL_BUFFER(channel->buffer), type, line_with_nick);
	g_free(line_with_nick);

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_scroll_channel_textview(account_manager_get());
	} else {
		if(STRING_IS_CHANNEL(channel->name))
			line_with_channel = g_strdup_printf("<%s:%s> %s", channel->name, nick, remark);
		else
			line_with_channel = g_strdup_printf("=%s= %s", nick, remark);
		account_manager_common_buffer_append(account_manager_get(), TEXT_TYPE_NORMAL, line_with_channel);
		g_free(line_with_channel);
	}
}

void
channel_append_text(Channel *channel, gboolean with_common_buffer, TextType type, gchar *str)
{
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	channel_buffer_append(CHANNEL_BUFFER(channel->buffer), type, str);
	if(with_common_buffer &&
	   !account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_common_buffer_append(account_manager_get(), type, str);
	}
	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_scroll_channel_textview(account_manager_get());
	}
}
void channel_set_topic(Channel *channel, const gchar *topic)
{
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	priv = channel->priv;

	if(priv->topic)
		g_free(priv->topic);

	priv->topic = g_strdup(topic);

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_set_topic(account_manager_get(), topic);
	}
}
gchar *channel_get_topic(Channel *channel)
{
	g_return_val_if_fail(channel != NULL, NULL);
	g_return_val_if_fail(IS_CHANNEL(channel), NULL);

	return channel->priv->topic;
}
void channel_append_user(Channel *channel, const gchar *nick, UserPower power, UserExistence exist)
{
	const gchar *tmp_nick;
	UserPower tmp_power;
	GtkTreeIter iter;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);
	g_return_if_fail(*nick != '\0');
	g_return_if_fail(power != -1);
	g_return_if_fail(exist != -1);

	if(power == USER_POWER_UNDETERMINED) {
		if(*nick == '@') {
			tmp_nick = nick+1;
			tmp_power = USER_POWER_OP;
		} else if (*nick == '+') {
			tmp_nick = nick+1;
			tmp_power = USER_POWER_V;
		} else {
			tmp_nick = nick;
			tmp_power = USER_POWER_NOTHING;
		}
	} else {
		tmp_nick = nick;
		tmp_power = power;
	}

	gtk_list_store_append(channel->user_list, &iter);
	gtk_list_store_set(channel->user_list, &iter,
			   USERLIST_COLUMN_HOMEAWAY, exist,
			   USERLIST_COLUMN_OP, tmp_power,
			   USERLIST_COLUMN_NICK, tmp_nick,
			   -1);
	
}
gboolean channel_find_user(Channel *channel, const gchar *nick, GtkTreeIter *iter_ptr)
{
	GtkTreeIter *iter;
	GtkTreeIter tmp_iter;
	GtkTreeModel *model;
	gchar *tmp;

	g_return_val_if_fail(channel != NULL, FALSE);
	g_return_val_if_fail(IS_CHANNEL(channel), FALSE);
	g_return_val_if_fail(nick != NULL, FALSE);

	model = GTK_TREE_MODEL(channel->user_list);

	if(iter_ptr != NULL)
		iter = iter_ptr;
	else
		iter = &tmp_iter;

	if(!gtk_tree_model_get_iter_first(model, iter))
		return FALSE;

	do {
		gtk_tree_model_get(model, iter, USERLIST_COLUMN_NICK, &tmp, -1);
		if(tmp == NULL) {
			g_warning("NULL user!");
			continue;
		}
		
		if(g_ascii_strcasecmp(tmp, nick) == 0)
			return TRUE;

	} while(gtk_tree_model_iter_next(model, iter));

	return FALSE;
}

void channel_remove_user(Channel *channel, const gchar *nick)
{
	GtkTreeIter iter;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);

	if(!channel_find_user(channel, nick, &iter))
		return;

	gtk_list_store_remove(channel->user_list, &iter);
}
void channel_change_user_power(Channel *channel, const gchar *nick, UserPower power)
{
	GtkTreeIter iter;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);

	if(!channel_find_user(channel, nick, &iter))
		return;
	
	gtk_list_store_set(channel->user_list, &iter, USERLIST_COLUMN_OP, power, -1);
}
void channel_change_user_nick(Channel *channel, const gchar *nick_orig, const gchar *nick_new)
{
	GtkTreeIter iter;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick_orig != NULL);
	g_return_if_fail(nick_new != NULL);

	if(!channel_find_user(channel, nick_orig, &iter))
		return;
	
	gtk_list_store_set(channel->user_list, &iter, USERLIST_COLUMN_NICK, nick_new, -1);
}
void channel_clear_user(Channel *channel)
{
	gtk_list_store_clear(channel->user_list);
}
