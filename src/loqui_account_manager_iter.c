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
#include "loqui_account_manager_iter.h"

void
loqui_account_manager_iter_init(LoquiAccountManager *manager, LoquiAccountManagerIter *iter)
{
	iter->account_list = loqui_account_manager_get_account_list(manager);
	iter->cur_ac = NULL;
	iter->channel_list = NULL;
	iter->cur_ch = NULL;
}
LoquiAccountManagerIter *
loqui_account_manager_iter_new(LoquiAccountManager *manager)
{
	LoquiAccountManagerIter *iter;
	
	g_return_val_if_fail(manager != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager), NULL);
	
	iter = g_new(LoquiAccountManagerIter, 1);
	loqui_account_manager_iter_init(manager, iter);

	return iter;
}
gboolean
loqui_account_manager_iter_equal(LoquiAccountManagerIter *a, LoquiAccountManagerIter *b)
{
	return (a->cur_ac == b->cur_ac && a->cur_ch == a->cur_ch) ? TRUE : FALSE;
}
void
loqui_account_manager_iter_free(LoquiAccountManagerIter *iter)
{
	g_free(iter);
}
void
loqui_account_manager_iter_set_first_channel_entry(LoquiAccountManagerIter *iter)
{
	iter->cur_ac = iter->account_list;
}
void
loqui_account_manager_iter_set_last_channel_entry(LoquiAccountManagerIter *iter)
{
	iter->cur_ac = g_list_last(iter->cur_ac ? iter->cur_ac : iter->account_list);
	iter->channel_list = loqui_account_get_channel_list(iter->cur_ac->data);
	iter->cur_ch = g_list_last(iter->channel_list);
}
gboolean
loqui_account_manager_iter_set_channel_entry(LoquiAccountManagerIter *iter, LoquiChannelEntry *chent)
{
	loqui_account_manager_iter_set_first_channel_entry(iter);

	do {
		if (loqui_account_manager_iter_get_current_channel_entry(iter) == chent)
			return TRUE;
	} while(loqui_account_manager_iter_channel_entry_next(iter));;

	return FALSE;
}

LoquiChannelEntry *
loqui_account_manager_iter_get_current_channel_entry(LoquiAccountManagerIter *iter)
{
	if (iter->cur_ch != NULL)
		return iter->cur_ch->data;
	else
		return iter->cur_ac->data;
}	
LoquiChannelEntry *
loqui_account_manager_iter_channel_entry_next(LoquiAccountManagerIter *iter)
{
	LoquiChannelEntry *chent;

	if (iter->cur_ch != NULL) {
		chent = iter->cur_ch->data;

		iter->cur_ch = iter->cur_ch->next;
		if (iter->cur_ch == NULL) {
			iter->channel_list = NULL;
			iter->cur_ac = iter->cur_ac->next;
		}

		return chent;
	} else if (iter->cur_ac != NULL) {
		chent = iter->cur_ac->data;

		iter->channel_list = loqui_account_get_channel_list(iter->cur_ac->data);
		if (iter->channel_list)
			iter->cur_ch = iter->channel_list;
		else
			iter->cur_ac = iter->cur_ac->next;
		return chent;
	}
	return NULL;
}
LoquiChannelEntry *
loqui_account_manager_iter_channel_entry_previous(LoquiAccountManagerIter *iter)
{
	LoquiChannelEntry *chent;

	if (iter->cur_ch != NULL) {
		chent = iter->cur_ch->data;
		iter->cur_ch = iter->cur_ch->prev;
		if (iter->cur_ch == NULL) {
			iter->channel_list = NULL;
		}
		return chent;
	} else if (iter->cur_ac != NULL) {
		chent = iter->cur_ac->data;
		iter->cur_ac = iter->cur_ac->prev;
		if (iter->cur_ac) {
			iter->channel_list = loqui_account_get_channel_list(iter->cur_ac->data);
			iter->cur_ch = g_list_last(iter->channel_list);
		}
		return chent;
	}
	return NULL;	
}
