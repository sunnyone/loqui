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
#include "config.h"
#include "loqui_utils_irc.h"

void
loqui_utils_irc_parse_nick(const gchar *nick, LoquiMemberPowerFlags *flags_ptr, gchar **nick_ptr)
{
        const gchar *tmp_nick;
	LoquiMemberPowerFlags tmp_power = 0;
	
	if(*nick == '@') {
		tmp_nick = nick+1;
		tmp_power |= LOQUI_MEMBER_POWER_OPERATOR;
	} else if (*nick == '+') {
		tmp_nick = nick+1;
		tmp_power |= LOQUI_MEMBER_POWER_VOICE;
	} else {
		tmp_nick = nick;
	}
	*nick_ptr = (gchar *) tmp_nick;
	*flags_ptr = tmp_power;
}
