/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_H__
#define __LOQUI_H__

#include <glib-object.h>

void loqui_set_debug_mode(gboolean debug_mode);
gboolean loqui_get_debug_mode(void);

void loqui_set_show_msg_mode(gboolean show_msg_mode);
gboolean loqui_get_show_msg_mode(void);

void loqui_set_send_status_commands_mode(gboolean send_status_commands_mode);
gboolean loqui_get_send_status_commands_mode(void);

#define PREFS_DIR ".loqui"

#endif /* __LOQUI_H__ */
