/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2005 Yoichi Imai <yoichi@silver-forest.com>
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
#include "about.h"
#include "gtkutils.h"
#include <libloqui/loqui-core.h>
#include <libloqui/loqui.h>

void
about_open(void)
{
	gtkutils_msgbox_info(GTK_MESSAGE_INFO,
			     "%s\n"
			     "IRC client for Gtk2\n"
			     "Copyright (C) 2002-2009, Yoichi Imai <sunnyone41@gmail.com>\n"
			     "http://loqui.good-day.net/", loqui_core_get_version_info(loqui_get_core()));
}
