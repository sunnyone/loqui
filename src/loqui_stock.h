/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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
#ifndef __LOQUI_STOCK_H__
#define __LOQUI_STOCK_H__

#include <loqui_user.h>

#define LOQUI_STOCK_CONSOLE      "loqui-console"
#define LOQUI_STOCK_ONLINE       "loqui-online"
#define LOQUI_STOCK_OFFLINE      "loqui-offline"
#define LOQUI_STOCK_AWAY         "loqui-away"
#define LOQUI_STOCK_BUSY         "loqui-busy"
#define LOQUI_STOCK_COMMAND      "loqui-command"
#define LOQUI_STOCK_OPERATOR     "loqui-operator"
#define LOQUI_STOCK_SPEAK_ABILITY "loqui-speak-ability"
#define LOQUI_STOCK_WHETHER_SCROLL "loqui-whether-scroll"
#define LOQUI_STOCK_NOTICE       "loqui-notice"
#define LOQUI_STOCK_LOQUI       "loqui-loqui"
#define LOQUI_STOCK_LOQUI_HILIGHTED   "loqui-loqui-hilighted"

extern int LOQUI_ICON_SIZE_FONT;

void loqui_stock_init(void);

G_CONST_RETURN gchar* loqui_stock_get_id_from_basic_away_type(LoquiBasicAwayType basic_away);

#endif /* __LOQUI_STOCK_H__ */
