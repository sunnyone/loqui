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
#include "account_manager.h"

struct _ChannelPrivate
{
	gchar *topic;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void channel_class_init(ChannelClass *klass);
static void channel_init(Channel *channel);
static void channel_finalize(GObject *object);

gint compare_user_nick_with_nick(gconstpointer data1, gconstpointer data2);

gint compare_user_nick_with_nick(gconstpointer data1, gconstpointer data2)
{
	User *user;

	if(data1 == NULL) return 0;

	user = (User *)data1;

	if(g_ascii_strcasecmp(user->nick, (const gchar *) data2) == 0)
		return 1;
	return 0;
}

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
/*	if(channel->text) {
		gtk_widget_destroy(GTK_WIDGET(channel->text));
		channel->text = NULL;
		} */
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
	channel->user_list = NULL;

	return channel;
}

void
channel_append_remark(Channel *channel, TextType type, gchar *name, gchar *remark)
{
	gchar *line_with_nick;
	gchar *line_with_channel;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	line_with_nick = g_strdup_printf("<%s> %s", name, remark);
	channel_text_append(CHANNEL_TEXT(channel->text), type, line_with_nick);
	g_free(line_with_nick);

	if(!account_manager_is_current_channel(account_manager_get(), channel)) {
		line_with_channel = g_strdup_printf("<%s:%s> %s", channel->name, name, remark);
		account_manager_common_text_append(account_manager_get(), TEXT_TYPE_NORMAL, line_with_channel);
		g_free(line_with_channel);
	}
}

void
channel_append_text(Channel *channel, gboolean with_common_text, TextType type, gchar *str)
{
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	channel_text_append(CHANNEL_TEXT(channel->text), type, str);
	if(with_common_text &&
	   !account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_common_text_append(account_manager_get(), type, str);
	}
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
void channel_append_user(Channel *channel, const gchar *nick, UserPower power, UserExistence exist)
{
	User *user;
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);
	g_return_if_fail(*nick != '\0');

	user = g_new0(User, 1);

	user->exist = exist;

	if(power == USER_POWER_UNDETERMINED) {
		if(*nick == '@') {
			user->nick = g_strdup(nick+1);
			user->power = USER_POWER_OP;
		} else if (*nick == '+') {
			user->nick = g_strdup(nick+1);
			user->power = USER_POWER_V;
		} else {
			user->nick = g_strdup(nick);
			user->power = USER_POWER_NOTHING;
		}
	} else {
		user->nick = g_strdup(nick);
		user->power = power;
	}

	channel->user_list = g_slist_append(channel->user_list, user);

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_nick_list_append(account_manager_get(), user);
	}
}
gboolean channel_find_user(Channel *channel, const gchar *nick, User **user)
{
	GSList *slist;

	g_return_val_if_fail(channel != NULL, FALSE);
	g_return_val_if_fail(IS_CHANNEL(channel), FALSE);
	g_return_val_if_fail(nick != NULL, FALSE);

	slist = g_slist_find_custom(channel->user_list, nick, compare_user_nick_with_nick);

	if(!slist)
		return FALSE;

	if(user != NULL)
		*user = (User *) slist->data;

	return TRUE;
}

void channel_remove_user(Channel *channel, const gchar *nick)
{
	User *user;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);

	if(!channel_find_user(channel, nick, &user))
		return;

	g_slist_remove(channel->user_list, user);
	g_free(user->nick);
	g_free(user);

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_nick_list_remove(account_manager_get(), user);
	}
}
void channel_change_user_power(Channel *channel, const gchar *nick, UserPower power)
{
	User *user;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);

	if(!channel_find_user(channel, nick, &user))
		return;
	user->power = power;

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_nick_list_update(account_manager_get(), user);
	}
}
void channel_change_user_nick(Channel *channel, const gchar *nick_orig, const gchar *nick_new)
{
	User *user;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick_orig != NULL);

	if(!channel_find_user(channel, nick_orig, &user))
		return;
	g_free(user->nick);
	user->nick = g_strdup(nick_new);

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_nick_list_update(account_manager_get(), user);
	}
}
void channel_clear_user(Channel *channel)
{
	GSList *cur;
	User *user;

	for(cur = channel->user_list; cur != NULL; cur = cur->next) {
		if(!cur->data) continue;

		user = (User *)cur->data;

		g_free(user->nick);
		g_free(user);
	}
	g_slist_free(channel->user_list);

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_nick_list_clear(account_manager_get());
	}
}
