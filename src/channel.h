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
#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "channel_buffer.h"
#include <glib.h>
#include "gobject_utils.h"

G_BEGIN_DECLS

#define TYPE_CHANNEL                 (channel_get_type ())
#define CHANNEL(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CHANNEL, Channel))
#define CHANNEL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CHANNEL, ChannelClass))
#define IS_CHANNEL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CHANNEL))
#define IS_CHANNEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CHANNEL))
#define CHANNEL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CHANNEL, ChannelClass))

typedef struct _Channel            Channel;
typedef struct _ChannelClass       ChannelClass;

typedef struct _ChannelPrivate     ChannelPrivate;

#include "irc_constants.h"
#include "account.h"

typedef enum {
	USER_POWER_UNKNOWN,
	USER_POWER_OP,
	USER_POWER_VOICE,
	USER_POWER_NOTHING,
	USER_POWER_UNDETERMINED = 255,
} UserPower;

typedef enum {
	USER_EXISTENCE_UNKNOWN,
	USER_EXISTENCE_HOME,
	USER_EXISTENCE_AWAY
} UserExistence;

struct _Channel
{
        GObject parent;

	Account *account;

	GtkListStore *user_list;
	ChannelBuffer *buffer;

	gboolean end_names;

        ChannelPrivate *priv;
};

struct _ChannelClass
{
        GObjectClass parent_class;

	void (* updated)             (Channel *channel);
	void (* topic_changed)       (Channel *channel);
	void (* user_number_changed) (Channel *channel);
	void (* mode_changed)        (Channel *channel);
};

#define STRING_IS_CHANNEL(s) ((*s == '#' || *s =='&' || *s =='!' || *s == '+'))

GType channel_get_type(void) G_GNUC_CONST;

Channel* channel_new(Account *account, const gchar *name);
void channel_append_text(Channel *channel, gboolean with_common_buffer, TextType type, gchar *str);
void channel_append_remark(Channel *channel, TextType type, gboolean is_self, const gchar *nick, const gchar *remark);

/* return value must not be freed. */
gchar *channel_get_name(Channel *channel);

void channel_set_topic(Channel *channel, const gchar *topic);
gchar *channel_get_topic(Channel *channel);

void channel_set_updated(Channel *channel, gboolean updated);
gboolean channel_get_updated(Channel *channel);

void channel_change_mode(Channel *channel, gboolean is_add, IRCModeFlag flag, gchar *argument);
void channel_clear_mode(Channel *channel);
gchar *channel_get_mode(Channel *channel);

void channel_push_user_mode_queue(Channel *channel, gboolean is_give, IRCModeFlag flag, const gchar *nick);
void channel_flush_user_mode_queue(Channel *channel);

void channel_append_user(Channel *channel, const gchar *nick, UserPower power, UserExistence exist);
void channel_remove_user(Channel *channel, const gchar *nick);
void channel_clear_user(Channel *channel);
void channel_change_user_power(Channel *channel, 
			       const gchar *nick, UserPower power);
void channel_change_user_nick(Channel *channel, 
			      const gchar *nick_orig, const gchar *nick_new);
gboolean channel_find_user(Channel *channel, const gchar *nick, GtkTreeIter *iter);
void channel_get_user_number(Channel *channel, guint *user_number, guint *op_number);

G_END_DECLS

#endif /* __CHANNEL_H__ */
