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
#ifndef __PREFS_GENERAL_H__
#define __PREFS_GENERAL_H__

#include <glib.h>

typedef struct _PrefElement PrefElement;

typedef enum {
	PREF_SORT_NONE,
	PREF_SORT_NICK,
	PREF_SORT_POWER_NICK,
	PREF_SORT_AWAY_NICK,
	PREF_SORT_POWER_AWAY_NICK,
	PREF_SORT_AWAY_POWER_NICK,
	PREF_SORT_TIME_NICK,
	PREF_SORT_TIME_AWAY_POWER_NICK,
	PREF_SORT_TIME_POWER_AWAY_NICK,
} PrefSortType;

typedef enum {
	PREF_TYPE_NONE,
	PREF_TYPE_BOOLEAN,
	PREF_TYPE_INT,
	PREF_TYPE_UINT,
	PREF_TYPE_STRING,
	PREF_TYPE_STRING_LIST = 101
} PrefType;

#define PREF_TYPE_IS_LIST(type) (type >= 100)

struct _PrefElement {
	gchar *name;
	gchar *default_value;
	PrefType type;
	gpointer ptr;
};

typedef struct _PrefsGeneral {
	gboolean save_size;

	guint window_height;
	guint window_width;

	guint common_buffer_height;

	guint channel_tree_width;
	guint channel_tree_height;

	gboolean show_statusbar;
	gboolean show_channelbar;

	gchar *away_message;

	gchar *browser_command;

	gboolean use_notification;
	gboolean exec_notification_by_notice;
	gchar *notification_command;

	gboolean auto_switch_scrolling;
	gboolean auto_switch_scrolling_common_buffer;
	gboolean parse_plum_recent;

	GList *highlight_list;

	guint remark_history_number;
	
	guint channel_buffer_max_line_number;
	guint common_buffer_max_line_number;
	
	gboolean auto_reconnect;

	gboolean use_transparent_ignore;
	GList *transparent_ignore_list;

	gboolean connect_startup;
	
	gchar *time_format;
	
	gboolean auto_command_mode;
	gchar *command_prefix;
	
	gboolean save_log;

	guint nick_list_sort_type;

	gboolean select_channel_joined;
} PrefsGeneral;

void prefs_general_load(void);
void prefs_general_save(void);

extern PrefsGeneral prefs_general;

#endif /* __PREFS_GENERAL_H__ */
