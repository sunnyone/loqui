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

#include <string.h>

struct _ChannelPrivate
{
	gchar *topic;
	gboolean fresh;

	guint user_number;
	guint op_number;

	gboolean user_number_update_function_added;

	GSList *mode_list;

	GList *mode_change_queue;
};

typedef struct {
	IRCModeFlag flag;
	gchar *argument;
} ChannelMode;

typedef struct {
	gboolean is_give;
	IRCModeFlag flag;
	gchar *nick;
} ModeChange;

#define MODE_CHANGE_MAX 3

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void channel_class_init(ChannelClass *klass);
static void channel_init(Channel *channel);
static void channel_finalize(GObject *object);

static void channel_update_user_number(Channel *channel);
static gboolean channel_update_user_number_actually(Channel *channel);

static void channel_mode_change_free(ModeChange *mode_change);
static void channel_mode_free(ChannelMode *mode);

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

	G_FREE_UNLESS_NULL(channel->name);
	G_FREE_UNLESS_NULL(priv->topic);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(priv);
}

Channel*
channel_new(Account *account, gchar *name)
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
	channel->account = account;

	return channel;
}

void
channel_append_remark(Channel *channel, TextType type, gboolean is_self, const gchar *nick, const gchar *remark)
{
	ChannelBuffer *buffer;
	gboolean is_priv = FALSE;
	gboolean exec_notification = TRUE;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	buffer = channel->buffer;

	if(!STRING_IS_CHANNEL(channel->name))
		is_priv = TRUE;
	if(is_self)
		exec_notification = FALSE;

	channel_buffer_append_remark(buffer, type, exec_notification, is_self, is_priv, NULL, nick, remark);

	if(!account_manager_is_current_channel_buffer(account_manager_get(), buffer)) {
		account_manager_common_buffer_append_remark(account_manager_get(), type, 
							    is_self, is_priv, channel->name, nick, remark);
		channel_set_fresh(channel, TRUE);
	}
}

void
channel_append_text(Channel *channel, gboolean with_common_buffer, TextType type, gchar *str)
{
	ChannelBuffer *buffer;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	buffer = channel->buffer;

	channel_buffer_append_line(buffer, type, str);
	if(with_common_buffer &&
	   !account_manager_is_current_channel_buffer(account_manager_get(), buffer)) {
		account_manager_common_buffer_append(account_manager_get(), type, str);
	}
}

void channel_set_fresh(Channel *channel, gboolean fresh)
{
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	priv = channel->priv;

	priv->fresh = fresh;

	account_manager_set_fresh(account_manager_get(), NULL, channel);
}
gboolean channel_get_fresh(Channel *channel)
{
	ChannelPrivate *priv;

	g_return_val_if_fail(channel != NULL, FALSE);
	g_return_val_if_fail(IS_CHANNEL(channel), FALSE);

	priv = channel->priv;

	return priv->fresh;
}
void channel_set_topic(Channel *channel, const gchar *topic)
{
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	priv = channel->priv;

	G_FREE_UNLESS_NULL(priv->topic);
	priv->topic = g_strdup(topic);

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_update_current_info(account_manager_get());
	}
}
gchar *channel_get_topic(Channel *channel)
{
	g_return_val_if_fail(channel != NULL, NULL);
	g_return_val_if_fail(IS_CHANNEL(channel), NULL);

	return g_strdup(channel->priv->topic);
}
void channel_append_user(Channel *channel, const gchar *nick, UserPower power, UserExistence exist)
{
	const gchar *tmp_nick;
	UserPower tmp_power;
	GtkTreeIter iter;
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);
	g_return_if_fail(*nick != '\0');
	g_return_if_fail(power != -1);
	g_return_if_fail(exist != -1);

	priv = channel->priv;

	if(power == USER_POWER_UNDETERMINED) {
		if(*nick == '@') {
			tmp_nick = nick+1;
			tmp_power = USER_POWER_OP;
		} else if (*nick == '+') {
			tmp_nick = nick+1;
			tmp_power = USER_POWER_VOICE;
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
	
	priv->user_number++;
	if(tmp_power == USER_POWER_OP)
		priv->op_number++;
	channel_update_user_number(channel);
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
		
		if(strcmp(tmp, nick) == 0)
			return TRUE;

	} while(gtk_tree_model_iter_next(model, iter));

	return FALSE;
}

void channel_remove_user(Channel *channel, const gchar *nick)
{
	GtkTreeIter iter;
	UserPower power;
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);

	priv = channel->priv;

	if(!channel_find_user(channel, nick, &iter))
		return;

	gtk_tree_model_get(GTK_TREE_MODEL(channel->user_list), &iter,
			   USERLIST_COLUMN_OP, &power, -1);
	gtk_list_store_remove(channel->user_list, &iter);

	if(power == USER_POWER_OP)
		priv->op_number--;
	priv->user_number--;

	channel_update_user_number(channel);
}
void channel_change_user_power(Channel *channel, const gchar *nick, UserPower power)
{
	GtkTreeIter iter;
	UserPower old_power;
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);

	priv = channel->priv;

	if(!channel_find_user(channel, nick, &iter))
		return;

	gtk_tree_model_get(GTK_TREE_MODEL(channel->user_list), &iter,
			   USERLIST_COLUMN_OP, &old_power, -1);
	/* meaningless change */
	if(old_power == USER_POWER_OP && power == USER_POWER_VOICE)
		return;

	gtk_list_store_set(channel->user_list, &iter, USERLIST_COLUMN_OP, power, -1);

	if(old_power == USER_POWER_OP && power != USER_POWER_OP)
		priv->op_number--;
	if(old_power != USER_POWER_OP && power == USER_POWER_OP)
		priv->op_number++;

	channel_update_user_number(channel);
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
	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	gtk_list_store_clear(channel->user_list);

	channel_update_user_number(channel);
}
static void
channel_update_user_number(Channel *channel)
{
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));

	priv = channel->priv;

	if(!priv->user_number_update_function_added) {
		g_idle_add((GSourceFunc) channel_update_user_number_actually, channel);
		priv->user_number_update_function_added = TRUE;
	}
}
static gboolean 
channel_update_user_number_actually(Channel *channel)
{
	ChannelPrivate *priv;

	g_return_val_if_fail(channel != NULL, FALSE);
	g_return_val_if_fail(IS_CHANNEL(channel), FALSE);
	
	priv = channel->priv;

	account_manager_update_channel_user_number(account_manager_get(), channel);
	priv->user_number_update_function_added = FALSE;

	return FALSE;
}
void
channel_get_user_number(Channel *channel, guint *user_number, guint *op_number)
{
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	
	priv = channel->priv;

	*user_number = priv->user_number;
	*op_number = priv->op_number;
}
void channel_push_user_mode_queue(Channel *channel, gboolean is_give, IRCModeFlag flag, const gchar *nick)
{
	ChannelPrivate *priv;
	ModeChange *mode_change_old;
	ModeChange *mode_change;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	
	priv = channel->priv;

	if(priv->mode_change_queue) {
		mode_change_old = g_list_last(priv->mode_change_queue)->data;
		if(mode_change_old->is_give != is_give || mode_change_old->flag != flag)
			channel_flush_user_mode_queue(channel);
	}

	if(g_list_length(priv->mode_change_queue) >= MODE_CHANGE_MAX)
		channel_flush_user_mode_queue(channel);

	mode_change = g_new0(ModeChange, 1);
	mode_change->is_give = is_give;
	mode_change->flag = flag;
	mode_change->nick = g_strdup(nick);

	priv->mode_change_queue = g_list_append(priv->mode_change_queue, mode_change);

}
static void
channel_mode_change_free(ModeChange *mode_change)
{
	g_return_if_fail(mode_change != NULL);

	g_free(mode_change->nick);
	g_free(mode_change);
}
static void
channel_mode_free(ChannelMode *mode)
{
	g_return_if_fail(mode);

	if(mode->argument)
		g_free(mode->argument);
	g_free(mode);
}

void channel_flush_user_mode_queue(Channel *channel)
{
	ChannelPrivate *priv;
	GList *tmp_list = NULL, *cur;
	ModeChange *mode_change;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	
	priv = channel->priv;
	
	mode_change = NULL;
	for(cur = priv->mode_change_queue; cur != NULL; cur = cur->next) {
		mode_change = (ModeChange *) cur->data;
		tmp_list = g_list_append(tmp_list, mode_change->nick);
	}
	if(mode_change)
		account_change_channel_user_mode(channel->account, channel, mode_change->is_give,
						 mode_change->flag, tmp_list);

	g_list_free(tmp_list);

	g_list_foreach(priv->mode_change_queue, (GFunc) channel_mode_change_free, NULL);
	g_list_free(priv->mode_change_queue);
	priv->mode_change_queue = NULL;
}
void
channel_change_mode(Channel *channel, gboolean is_add, IRCModeFlag flag, gchar *argument)
{
	ChannelPrivate *priv;
	GSList *cur;
	ChannelMode *matched = NULL;
	ChannelMode *mode;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	
	priv = channel->priv;

	for(cur = priv->mode_list; cur != NULL; cur = cur->next) {
		mode = (ChannelMode *) cur->data;
		if(mode->flag == flag) {
			matched = mode;
			break;
		}
	}

	if(is_add) {
		if(matched)
			return;
		
		mode = g_new0(ChannelMode, 1);
		mode->flag = flag;
		mode->argument = g_strdup(mode->argument);
		priv->mode_list = g_slist_append(priv->mode_list, mode);
	} else {
		if(!matched)
			return;

		priv->mode_list = g_slist_remove(priv->mode_list, matched);
		channel_mode_free(matched);
	}

	debug_puts("Channel mode changed: %s %c%c %s", channel->name, is_add ? '+' : '-', flag, argument ? argument : "");

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_update_current_info(account_manager_get());
	}
}
void channel_clear_mode(Channel *channel)
{
	ChannelPrivate *priv;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(IS_CHANNEL(channel));
	
	priv = channel->priv;

	if(!priv->mode_list)
		return;

	g_slist_foreach(priv->mode_list, (GFunc) channel_mode_free, NULL);
	g_slist_free(priv->mode_list);

	priv->mode_list = NULL;

	if(account_manager_is_current_channel(account_manager_get(), channel)) {
		account_manager_update_current_info(account_manager_get());
	}
}
gchar *
channel_get_mode(Channel *channel)
{
	ChannelPrivate *priv;
	GString *flag_string;
	GString *argument_string;
	gchar *str;
	GSList *cur;
	ChannelMode *mode;

	g_return_val_if_fail(channel != NULL, NULL);
	g_return_val_if_fail(IS_CHANNEL(channel), NULL);
	
	priv = channel->priv;

	if(!priv->mode_list)
		return g_strdup("");

	flag_string = g_string_sized_new(20);
	argument_string = g_string_new(NULL);

	flag_string = g_string_append_c(flag_string, '+');

	for(cur = priv->mode_list; cur != NULL; cur = cur->next) {
		mode = (ChannelMode *) cur->data;

		flag_string = g_string_append_c(flag_string, mode->flag);
		if(mode->argument)
			g_string_append_printf(argument_string, "%s ", mode->argument);
	}
	if(argument_string->len > 1) {
		g_string_append_len(flag_string, argument_string->str, argument_string->len-1);
	}
	g_string_free(argument_string, TRUE);

	str = flag_string->str;
	g_string_free(flag_string, FALSE);

	return str;
}
