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
#include "loqui_channel_entry_utils.h"


/*
  chent: null
   -> account = null, channel = null
  chent: channel
   -> account = channel->account, channel = channel
  chent: account
   -> account = account, channel = null
*/
gboolean
loqui_channel_entry_utils_separate(LoquiChannelEntry *chent, LoquiAccount **account, LoquiChannel **channel)
{
	LoquiAccount *ac = NULL;
	LoquiChannel *ch = NULL;

	if (chent == NULL) {
	} else if (LOQUI_IS_CHANNEL(chent)) {
		ch = LOQUI_CHANNEL(chent);
		ac = loqui_channel_get_account(ch);
	} else if (LOQUI_IS_ACCOUNT(chent)) {
		ch = NULL;
		ac = LOQUI_ACCOUNT(chent);
	} else {
		if (account)
			*account = NULL;
		if (channel)
			*channel = NULL;
		return FALSE;
	}
	if (account)
		*account = ac;
	if (channel)
		*channel = ch;

	return TRUE;
}
