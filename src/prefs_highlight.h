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
#ifndef __PREFS_HIGHLIGHT_H__
#define __PREFS_HIGHLIGHT_H__

#include <glib.h>

typedef struct _PrefsEmphaisWords {
	GSList *allow_list;
	/* GSList *deny_list; */

} PrefsHighlightWords;

void prefs_highlight_init(void);
void prefs_highlight_load(void);
void prefs_highlight_save(void);

extern PrefsHighlightWords prefs_highlight;

#endif /* __PREFS_HIGHLIGHT_H__ */
