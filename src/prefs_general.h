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
#include <glib-object.h>

typedef struct _PrefElement PrefElement;

struct _PrefElement {
	gchar *name;
	gchar *default_value;
	GType type;
	gpointer ptr;
};

typedef struct _PrefsGeneral {
	gint codeconv;
	gchar *codeset;
	
	gboolean save_size;

	gint window_height;
	gint window_width;

	gint common_buffer_height;

	gint channel_tree_width;
	gint channel_tree_height;

	gchar *away_message;

	gchar *browser_command;

	gboolean use_notification;
	gchar *notification_command;

} PrefsGeneral;

void prefs_general_load(void);
void prefs_general_save(void);

extern PrefsGeneral prefs_general;

#endif /* __PREFS_GENERAL_H__ */
