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
#ifndef __MAIN_H__
#define __MAIN_H__

#define FRESH_COLOR "red"
#define NONFRESH_COLOR "black"
#define MAX_SHORTCUT_CHANNEL_NUMBER 19
#define SHORTCUT_CHANNEL_ACCEL_MAP_PREFIX "<main>/Channels/Channel "

extern int debug_mode;
extern int show_msg_mode;
extern int send_status_commands_mode;

#define PREFS_DIR ".loqui"

enum {
	USERLIST_COLUMN_HOMEAWAY,
	USERLIST_COLUMN_OP,
	USERLIST_COLUMN_NICK,
	USERLIST_COLUMN_NUMBER
};

#define LOQUI_STOCK_CONSOLE      "loqui-console"
#define LOQUI_STOCK_ONLINE       "loqui-online"
#define LOQUI_STOCK_OFFLINE      "loqui-offline"
#define LOQUI_STOCK_AWAY         "loqui-away"
#define LOQUI_STOCK_BUSY         "loqui-busy"
#define LOQUI_STOCK_COMMAND      "loqui-command"
#define LOQUI_STOCK_NARUTO	 "loqui-naruto"
#define LOQUI_STOCK_SPEAKER	 "loqui-speaker"

#endif /* __MAIN_H__ */
