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

typedef struct _PrefsGeneral {
	gint codeconv;
	gchar *codeset;
	
	gint save_size;

	gint window_height;
	gint window_width;

	gint channel_book_height;
	gint channel_book_width;

	gint channel_tree_height;
} PrefsGeneral;

void prefs_general_load(void);
void prefs_general_save(void);
void prefs_general_set_default(void);

extern PrefsGeneral *prefs_general;

#endif /* __PREFS_GENERAL_H__ */
